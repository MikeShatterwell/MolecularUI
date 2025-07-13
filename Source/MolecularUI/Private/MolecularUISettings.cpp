// Fill out your copyright notice in the Description page of Project Settings.


#include "MolecularUISettings.h"

const UMolecularUISettings* UMolecularUISettings::Get()
{
	return GetDefault<UMolecularUISettings>();
}

TSoftObjectPtr<UDataTable> UMolecularUISettings::GetDefaultStoreItemsDataTable()
{
	return Get()->DefaultItemsDataTable;
}

const FSlateBrush& UMolecularUISettings::GetDefaultStoreIcon()
{
	return Get()->DefaultStoreIcon;
}