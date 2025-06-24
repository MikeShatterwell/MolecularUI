// Fill out your copyright notice in the Description page of Project Settings.


#include "MolecularUISettings.h"

const UMolecularUISettings* UMolecularUISettings::Get()
{
	return GetDefault<UMolecularUISettings>();
}

TSubclassOf<UObject> UMolecularUISettings::GetDefaultStoreDataProviderClass()
{
	return Get()->DefaultStoreDataProviderClass;
}

TSoftObjectPtr<UDataTable> UMolecularUISettings::GetDefaultStoreItemsDataTable()
{
	return Get()->DefaultStoreItemsDataTable;
}

TSoftObjectPtr<UDataTable> UMolecularUISettings::GetDefaultOwnedItemsDataTable()
{
	return Get()->DefaultOwnedItemsDataTable;
}

const FSlateBrush& UMolecularUISettings::GetDefaultStoreIcon()
{
	return Get()->DefaultStoreIcon;
}
