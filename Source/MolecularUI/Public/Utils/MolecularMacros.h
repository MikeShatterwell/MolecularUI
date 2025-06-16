// Copyright Mike Desrosiers, All Rights Reserved.

#pragma once

#include <CoreMinimal.h>
#include <Templates/RemoveReference.h>

#define UE_MVVM_BIND_FIELD(ViewModel, Property, Handler) \
do \
	/**
	* Binds a member function of the current UObject to a ViewModel's Field-Notify property.
	* Supports both raw pointers (UMyViewModel*) and TObjectPtr<UMyViewModel>.
	* Includes compile-time checks for type safety.
	*
	* @param ViewModel      Pointer or TObjectPtr to the ViewModel instance.
	* @param Property       The name of the property in the ViewModel's FFieldNotificationClassDescriptor.
	* @param Handler        The name of the member function in the current class to bind. Must have signature: void(UObject*, UE::FieldNotification::FFieldId).
	*/ \
{ \
	using ViewModelParamType = std::decay_t<decltype(ViewModel)>; \
	static_assert(UE::MolecularMacros::Private::TIsObjectPointerLike<ViewModelParamType>::value, \
		"UE_MVVM_BIND_FIELD: 'ViewModel' parameter must be a raw pointer (e.g., UMyViewModel*) or TObjectPtr."); \
	\
	using ViewModelClass = typename UE::MolecularMacros::Private::TGetPointerElementType<ViewModelParamType>::Type; \
	using ListenerClass = ThisClass; \
	\
	static_assert(std::is_base_of_v<UMVVMViewModelBase, ViewModelClass>, \
		"UE_MVVM_BIND_FIELD: 'ViewModel' must be a UClass derived from UMVVMViewModelBase."); \
	static_assert(std::is_invocable_v<decltype(&ListenerClass::Handler), ListenerClass*, UObject*, UE::FieldNotification::FFieldId>, \
		"UE_MVVM_BIND_FIELD: 'Handler' function signature must be 'void FunctionName(UObject* SourceObject, UE::FieldNotification::FFieldId FieldId)'."); \
	\
	if (ViewModel) \
	{ \
		const UE::FieldNotification::FFieldId FieldId = ViewModelClass::FFieldNotificationClassDescriptor::Property; \
		if (FieldId.IsValid()) \
		{ \
			ViewModel->AddFieldValueChangedDelegate(FieldId, \
				INotifyFieldValueChanged::FFieldValueChangedDelegate::CreateUObject(this, &ListenerClass::Handler)); \
		} \
	} \
} while(0)

#define UE_MVVM_UNBIND_FIELD(ViewModel, Property) \
do \
	/**
	* Unbinds the current object ('this') from a specific ViewModel property.
	* The counterpart to UE_MVVM_BIND_FIELD.
	*
	* @param ViewModel	  Pointer or TObjectPtr to the ViewModel instance.
	* @param Property	   The name of the property in the ViewModel's FFieldNotificationClassDescriptor.
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
		auto ArgParser = [](float InFailureChance = 0.0f, float InMinDelay = 0.0f, float InMaxDelay = 0.3f) \
		{ \
			return TTuple<float, float, float>(InFailureChance, InMinDelay, InMaxDelay); \
		}; \
		const auto ParsedArgs = ArgParser(__VA_ARGS__); \
		const float FailureChance_Internal = ParsedArgs.Get<0>(); \
		const float MinDelay_Internal = ParsedArgs.Get<1>(); \
		const float MaxDelay_Internal = ParsedArgs.Get<2>(); \
		\
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