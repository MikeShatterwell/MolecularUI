#pragma once

#include "CoreMinimal.h"
#include "Templates/RemoveReference.h"

/**
 * Binds a member function of the current UObject to a ViewModel's Field-Notify property.
 * Supports both raw pointers (UMyViewModel*) and TObjectPtr<UMyViewModel>.
 * Includes COMPILE-TIME checks for type safety.
 *
 * @param ViewModel      Pointer or TObjectPtr to the ViewModel instance.
 * @param Property       The name of the property in the ViewModel's FFieldNotificationClassDescriptor.
 * @param Handler        The name of the member function in the current class to bind. Must have signature: void(UObject*, UE::FieldNotification::FFieldId).
 */
#define UE_MVVM_BIND_FIELD(ViewModel, Property, Handler) \
do \
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

/**
 * Unbinds the current object ('this') from a specific ViewModel property.
 * The counterpart to UE_MVVM_BIND_FIELD.
 *
 * @param ViewModel      Pointer or TObjectPtr to the ViewModel instance.
 * @param Property       The name of the property in the ViewModel's FFieldNotificationClassDescriptor.
 */
#define UE_MVVM_UNBIND_FIELD(ViewModel, Property) \
do \
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

namespace UE::MolecularMacros::Private
{
	/** * Trait to determine if a type is a raw pointer or a TObjectPtr.
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