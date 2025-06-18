#include "Utils/MolecularCVars.h"
#include "HAL/IConsoleManager.h"

namespace MolecularUI::CVars
{
    float DefaultMockFailureChance = 0.0f;
    static FAutoConsoleVariableRef CVarDefaultMockFailureChance(
        TEXT("molecularui.MockFailureChance"),
        DefaultMockFailureChance,
        TEXT("Chance for mock data operations to fail [0..1]."),
        ECVF_Cheat);

    float DefaultMockMinDelay = 0.0f;
    static FAutoConsoleVariableRef CVarDefaultMockMinDelay(
        TEXT("molecularui.MockMinDelay"),
        DefaultMockMinDelay,
        TEXT("Minimum delay for mock data operations in seconds."),
        ECVF_Cheat);

    float DefaultMockMaxDelay = 0.3f;
    static FAutoConsoleVariableRef CVarDefaultMockMaxDelay(
        TEXT("molecularui.MockMaxDelay"),
        DefaultMockMaxDelay,
        TEXT("Maximum delay for mock data operations in seconds."),
        ECVF_Cheat);

    float StoreItemsFailureChance = 0.15f;
    static FAutoConsoleVariableRef CVarStoreItemsFailureChance(
        TEXT("molecularui.StoreItemsFailureChance"),
        StoreItemsFailureChance,
        TEXT("Chance for FetchStoreItems to fail [0..1]."),
        ECVF_Cheat);

    float StoreItemsMinDelay = 3.5f;
    static FAutoConsoleVariableRef CVarStoreItemsMinDelay(
        TEXT("molecularui.StoreItemsMinDelay"),
        StoreItemsMinDelay,
        TEXT("Minimum delay for FetchStoreItems in seconds."),
        ECVF_Cheat);

    float StoreItemsMaxDelay = 8.5f;
    static FAutoConsoleVariableRef CVarStoreItemsMaxDelay(
        TEXT("molecularui.StoreItemsMaxDelay"),
        StoreItemsMaxDelay,
        TEXT("Maximum delay for FetchStoreItems in seconds."),
        ECVF_Cheat);

    float OwnedItemsFailureChance = 0.0f;
    static FAutoConsoleVariableRef CVarOwnedItemsFailureChance(
        TEXT("molecularui.OwnedItemsFailureChance"),
        OwnedItemsFailureChance,
        TEXT("Chance for FetchOwnedItems to fail [0..1]."),
        ECVF_Cheat);

    float OwnedItemsMinDelay = 0.0f;
    static FAutoConsoleVariableRef CVarOwnedItemsMinDelay(
        TEXT("molecularui.OwnedItemsMinDelay"),
        OwnedItemsMinDelay,
        TEXT("Minimum delay for FetchOwnedItems in seconds."),
        ECVF_Cheat);

    float OwnedItemsMaxDelay = 0.3f;
    static FAutoConsoleVariableRef CVarOwnedItemsMaxDelay(
        TEXT("molecularui.OwnedItemsMaxDelay"),
        OwnedItemsMaxDelay,
        TEXT("Maximum delay for FetchOwnedItems in seconds."),
        ECVF_Cheat);

    float PlayerCurrencyFailureChance = 0.0f;
    static FAutoConsoleVariableRef CVarPlayerCurrencyFailureChance(
        TEXT("molecularui.PlayerCurrencyFailureChance"),
        PlayerCurrencyFailureChance,
        TEXT("Chance for FetchPlayerCurrency to fail [0..1]."),
        ECVF_Cheat);

    float PlayerCurrencyMinDelay = 0.0f;
    static FAutoConsoleVariableRef CVarPlayerCurrencyMinDelay(
        TEXT("molecularui.PlayerCurrencyMinDelay"),
        PlayerCurrencyMinDelay,
        TEXT("Minimum delay for FetchPlayerCurrency in seconds."),
        ECVF_Cheat);

    float PlayerCurrencyMaxDelay = 0.3f;
    static FAutoConsoleVariableRef CVarPlayerCurrencyMaxDelay(
        TEXT("molecularui.PlayerCurrencyMaxDelay"),
        PlayerCurrencyMaxDelay,
        TEXT("Maximum delay for FetchPlayerCurrency in seconds."),
        ECVF_Cheat);

    float PurchaseFailureChance = 0.0f;
    static FAutoConsoleVariableRef CVarPurchaseFailureChance(
        TEXT("molecularui.PurchaseFailureChance"),
        PurchaseFailureChance,
        TEXT("Chance for PurchaseItem to fail [0..1]."),
        ECVF_Cheat);

    float PurchaseMinDelay = 0.0f;
    static FAutoConsoleVariableRef CVarPurchaseMinDelay(
        TEXT("molecularui.PurchaseMinDelay"),
        PurchaseMinDelay,
        TEXT("Minimum delay for PurchaseItem in seconds."),
        ECVF_Cheat);

    float PurchaseMaxDelay = 0.3f;
    static FAutoConsoleVariableRef CVarPurchaseMaxDelay(
        TEXT("molecularui.PurchaseMaxDelay"),
        PurchaseMaxDelay,
        TEXT("Maximum delay for PurchaseItem in seconds."),
        ECVF_Cheat);
}

