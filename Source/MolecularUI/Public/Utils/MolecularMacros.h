// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Templates/RemoveReference.h>
#include "Utils/MolecularCVars.h"

/**
 * Binds a Blueprint-overridable UFUNCTION to a ViewModel's Field-Notify property.
 * This macro automatically creates a lambda to adapt the delegate's FFieldId to the FName expected by the UFUNCTION.
 *
 * @param ExpectedType   The UObject-derived type for the handler's first parameter (e.g., UMyViewModel).
 * @param ViewModel	  Pointer or TObjectPtr to the ViewModel instance (or UObject that implements INotifyFieldValueChanged).
 * @param Property	   The name of the property in the ViewModel's FFieldNotificationClassDescriptor.
 * @param Handler		The name of the UFUNCTION in the current class to bind.
 */
#define UE_MVVM_BIND_FIELD(ExpectedType, ViewModel, Property, Handler) \
	do \
	{ \
		using ViewModelParamType = std::decay_t<decltype(ViewModel)>; \
		static_assert(UE::MolecularMacros::Private::TIsObjectPointerLike<ViewModelParamType>::value, \
			"UE_MVVM_BIND_FIELD: 'ViewModel' parameter must be a raw pointer (e.g., UMyViewModel*) or TObjectPtr."); \
		\
		using ViewModelClass = typename UE::MolecularMacros::Private::TGetPointerElementType<ViewModelParamType>::Type; \
		using ListenerClass = ThisClass; \
		\
		/* The ViewModel itself must be a UObject. */ \
		static_assert(std::is_base_of_v<UObject, ViewModelClass>, \
			"UE_MVVM_BIND_FIELD: 'ViewModel' must be a UClass derived from UObject."); \
		\
		/* The specified handler type must also be a UObject. */ \
		static_assert(std::is_base_of_v<UObject, ExpectedType>, \
			"UE_MVVM_BIND_FIELD: ExpectedType, must be a UObject-derived type (usually a UMVVMViewModelBase subclass or other UObject that implements INotifyFieldValueChanged)."); \
		\
		/* Verify that the handler function has the correct signature using the provided ExpectedType. */ \
		static_assert(std::is_invocable_v<decltype(&ListenerClass::Handler), ListenerClass*, ExpectedType*, FFieldNotificationId>, \
			"UE_MVVM_BIND_FIELD: 'Handler' UFUNCTION signature mismatch. " \
			"Expected 'void FunctionName(ExpectedType* SourceObject, FFieldNotificationId Field)'."); \
		\
		if (ViewModel) \
		{ \
			/* Ensure the ViewModel has a valid FieldId for the specified property. */ \
			const UE::FieldNotification::FFieldId FieldId = ViewModelClass::FFieldNotificationClassDescriptor::Property; \
			if (FieldId.IsValid()) \
			{ \
				/* Bind a weak lambda that adapts the parameters for the BP-exposed UFUNCTION. */ \
				ViewModel->AddFieldValueChangedDelegate(FieldId, \
					INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateWeakLambda(this, [this](UObject* InObject, UE::FieldNotification::FFieldId InField) \
					{ \
						/* Safely cast the source object to the expected type before invoking the handler. */ \
						if (auto* SpecificObject = Cast<ExpectedType>(InObject)) \
						{ \
							this->Handler(SpecificObject, FFieldNotificationId(InField.GetName())); \
						} \
					})); \
			} \
		} \
	} while(0)

#define UE_MVVM_UNBIND_FIELD(ViewModel, Property) \
	do \
		/** \
		 * Unbinds the current object ('this') from a specific ViewModel property. \
		 * The counterpart to UE_MVVM_BIND_FIELD. \
		 * \
		 * @param ViewModel   Pointer or TObjectPtr to the ViewModel instance. \
		 * @param Property	 The name of the property in the ViewModel's FFieldNotificationClassDescriptor. \
		 */ \
	{ \
		using ViewModelParamType = std::decay_t<decltype(ViewModel)>; \
		static_assert(UE::MolecularMacros::Private::TIsObjectPointerLike<ViewModelParamType>::value, \
			"UE_MVVM_UNBIND_FIELD: 'ViewModel' parameter must be a raw pointer (e.g., UMyViewModel*) or TObjectPtr."); \
		\
		using ViewModelClass = typename UE::MolecularMacros::Private::TGetPointerElementType<ViewModelParamType>::Type; \
		\
		static_assert(std::is_base_of_v<UMVVMViewModelBase, ViewModelClass>, \
			"UE_MVVM_UNBIND_FIELD: 'ViewModel' must be a UClass derived from UMVVMViewModelBase."); \
		\
		if (ViewModel) \
		{ \
			const UE::FieldNotification::FFieldId FieldId = ViewModelClass::FFieldNotificationClassDescriptor::Property; \
			if (FieldId.IsValid()) \
			{ \
				ViewModel->RemoveAllFieldValueChangedDelegates(FieldId, this); \
			} \
		} \
	} while (0)

#define FETCH_MOCK_DATA(TimerHandle, SuccessCallback, FailureCallback, ...) \
do \
{ \
	/**
	* @brief Simulates a data fetch with distinct success and failure outcomes.
	*
	* Randomly decides whether the operation succeeds or fails and executes the corresponding callback
	* after a simulated network delay. This is useful for testing loading and error states in the UI.
	*
	* The macro provides flexibility with optional parameters to adjust failure likelihood and network delay.
	* This can emulate various scenarios to assist debugging and fine-tuning UI behavior.
	*
	* @param TimerHandle The FTimerHandle member variable for this operation.
	* @param SuccessCallback A TFunction<void()> lambda to be called on success.
	* @param FailureCallback A TFunction<void()> lambda to be called on failure.
	* @param ... Optional parameters: FailureChance (float, default 0.0f), MinDelay (float, default 0.0f), MaxDelay (float, default 0.3f)
	*/ \
	if (UWorld* World = GetWorld()) \
	{ \
		World->GetTimerManager().ClearTimer(TimerHandle); \
		\
		/* Use a helper lambda to parse optional arguments with default values. */ \
		auto ArgParser = [](float InFailureChance = MolecularUI::CVars::Default::FailureChance, float InMinDelay = MolecularUI::CVars::Default::MinDelay, float InMaxDelay = MolecularUI::CVars::Default::MaxDelay) \
		{ \
				return TTuple<float, float, float>(InFailureChance, InMinDelay, InMaxDelay); \
		}; \
		const auto ParsedArgs = ArgParser(__VA_ARGS__); \
		const float FailureChance_Internal = ParsedArgs.Get<0>(); \
		const float MinDelay_Internal = ParsedArgs.Get<1>(); \
		const float MaxDelay_Internal = ParsedArgs.Get<2>(); \
		\
		if (MinDelay_Internal <= 0.0f && MaxDelay_Internal <= 0.0f) \
		{ \
			/* Immediately execute the callback without setting a timer */ \
			if (FMath::FRand() < FailureChance_Internal) \
			{ \
				FailureCallback(); \
			} \
			else \
			{ \
				SuccessCallback(); \
			} \
			return; \
		} \
		FTimerDelegate TimerDelegate; \
		TimerDelegate.BindWeakLambda(this, [this, SuccessCallback, FailureCallback, FailureChance_Internal]() \
		{ \
			/* Randomly decide if the mock request "fails" based on the provided chance */ \
			if (FMath::FRand() < FailureChance_Internal) \
			{ \
				FailureCallback(); \
			} \
			else \
			{ \
				SuccessCallback(); \
			} \
		}); \
		\
		const float MockDelay = FMath::RandRange(MinDelay_Internal, MaxDelay_Internal); \
		World->GetTimerManager().SetTimer(TimerHandle, TimerDelegate, MockDelay, false); \
	} \
} \
while (0)


namespace UE::MolecularMacros::Private
{
	/**
	 * Trait to determine if a type is a raw pointer or a TObjectPtr.
	 * This is used for our static_assert checks.
	 */
	template<typename T, typename = void>
	struct TIsObjectPointerLike : std::false_type {};

	template<typename T>
	struct TIsObjectPointerLike<T*, void> : std::true_type {};

	template<typename T>
	struct TIsObjectPointerLike<TObjectPtr<T>, void> : std::true_type {};

	/**
	 * Trait to extract the element type (e.g., UMyViewModel) from a 
	 * raw pointer (UMyViewModel*) or a TObjectPtr<UMyViewModel>.
	 */
	template<typename T>
	struct TGetPointerElementType;

	template<typename T>
	struct TGetPointerElementType<T*> { using Type = T; };

	template<typename T>
	struct TGetPointerElementType<TObjectPtr<T>> { using Type = T; };
}