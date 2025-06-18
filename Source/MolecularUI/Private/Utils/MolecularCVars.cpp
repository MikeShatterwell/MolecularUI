// Copyright Mike Desrosiers, All Rights Reserved.
#include "Utils/MolecularCVars.h"

#include <HAL/IConsoleManager.h>

// Example CVar definitions for the MolecularUI demo.
namespace MolecularUI::CVars
{
	namespace Default
	{
		float MockFailureChance = 0.0f;
		static FAutoConsoleVariableRef CVarMockFailureChance(
			TEXT("MolecularUI.Default.MockFailureChance"),
			MockFailureChance,
			TEXT("Chance for mock data operations to fail [0..1]."),
			ECVF_Cheat);

		float MockMinDelay = 0.0f;
		static FAutoConsoleVariableRef CVarMockMinDelay(
			TEXT("MolecularUI.Default.MockMinDelay"),
			MockMinDelay,
			TEXT("Minimum delay for mock data operations in seconds."),
			ECVF_Cheat);

		float MockMaxDelay = 0.3f;
		static FAutoConsoleVariableRef CVarMockMaxDelay(
			TEXT("MolecularUI.Default.MockMaxDelay"),
			MockMaxDelay,
			TEXT("Maximum delay for mock data operations in seconds."),
			ECVF_Cheat);
	}

	// Store operations
        namespace Store
        {
                float FailureChance = 0.15f;
                static FAutoConsoleVariableRef CVarFailureChance(
                        TEXT("MolecularUI.Store.ItemsFailureChance"),
                        FailureChance,
                        TEXT("Chance for FetchStoreItems to fail [0..1]."),
                        ECVF_Cheat);

		float MinDelay = 3.5f;
		static FAutoConsoleVariableRef CVarMinDelay(
			TEXT("MolecularUI.Store.ItemsMinDelay"),
			MinDelay,
			TEXT("Minimum delay for FetchStoreItems in seconds."),
			ECVF_Cheat);

		float MaxDelay = 8.5f;
                static FAutoConsoleVariableRef CVarMaxDelay(
                        TEXT("MolecularUI.Store.ItemsMaxDelay"),
                        MaxDelay,
                        TEXT("Maximum delay for FetchStoreItems in seconds."),
                        ECVF_Cheat);

                int32 NumDummyItems = 3;
                static FAutoConsoleVariableRef CVarNumDummyItems(
                        TEXT("MolecularUI.Store.NumDummyItems"),
                        NumDummyItems,
                        TEXT("Number of dummy store items to generate."),
                        ECVF_Cheat);
        }

	// Owned items operations
	namespace OwnedItems
	{
		float FailureChance = 0.0f;
		static FAutoConsoleVariableRef CVarFailureChance(
			TEXT("MolecularUI.OwnedItems.ItemsFailureChance"),
			FailureChance,
			TEXT("Chance for FetchOwnedItems to fail [0..1]."),
			ECVF_Cheat);

		float MinDelay = 0.0f;
		static FAutoConsoleVariableRef CVarMinDelay(
			TEXT("MolecularUI.OwnedItems.ItemsMinDelay"),
			MinDelay,
			TEXT("Minimum delay for FetchOwnedItems in seconds."),
			ECVF_Cheat);

		float MaxDelay = 0.3f;
		static FAutoConsoleVariableRef CVarMaxDelay(
			TEXT("MolecularUI.OwnedItems.ItemsMaxDelay"),
			MaxDelay,
			TEXT("Maximum delay for FetchOwnedItems in seconds."),
			ECVF_Cheat);
	}

	// Player currency operations
	namespace PlayerCurrency
	{
		float FailureChance = 0.0f;
		static FAutoConsoleVariableRef CVarFailureChance(
			TEXT("MolecularUI.PlayerCurrency.FailureChance"),
			FailureChance,
			TEXT("Chance for FetchPlayerCurrency to fail [0..1]."),
			ECVF_Cheat);

		float MinDelay = 0.0f;
		static FAutoConsoleVariableRef CVarMinDelay(
			TEXT("MolecularUI.PlayerCurrency.MinDelay"),
			MinDelay,
			TEXT("Minimum delay for FetchPlayerCurrency in seconds."),
			ECVF_Cheat);

		float MaxDelay = 0.3f;
		static FAutoConsoleVariableRef CVarMaxDelay(
			TEXT("MolecularUI.PlayerCurrency.MaxDelay"),
			MaxDelay,
			TEXT("Maximum delay for FetchPlayerCurrency in seconds."),
			ECVF_Cheat);
	}

	// Transaction operations
	namespace Transaction
	{
		float FailureChance = 0.0f;
		static FAutoConsoleVariableRef CVarFailureChance(
			TEXT("MolecularUI.Transaction.FailureChance"),
			FailureChance,
			TEXT("Chance for TransactionItem to fail [0..1]."),
			ECVF_Cheat);

		float MinDelay = 0.0f;
		static FAutoConsoleVariableRef CVarMinDelay(
			TEXT("MolecularUI.Transaction.MinDelay"),
			MinDelay,
			TEXT("Minimum delay for TransactionItem in seconds."),
			ECVF_Cheat);

		float MaxDelay = 0.3f;
		static FAutoConsoleVariableRef CVarMaxDelay(
			TEXT("MolecularUI.Transaction.MaxDelay"),
			MaxDelay,
			TEXT("Maximum delay for TransactionItem in seconds."),
			ECVF_Cheat);
	}
}

