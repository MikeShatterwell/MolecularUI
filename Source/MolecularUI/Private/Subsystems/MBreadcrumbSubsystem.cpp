// Fill out your copyright notice in the Description page of Project Settings.


#include "Subsystems/MBreadcrumbSubsystem.h"

void UMBreadcrumbSubsystem::Push(const FName& MenuID, FText DisplayName)
{
	if (!Trail.IsEmpty() && Trail.Last().MenuID == MenuID) return;      // duplicate guard

	Trail.Emplace(MenuID, DisplayName);
	OnBreadcrumbChanged.Broadcast(Trail);
}

void UMBreadcrumbSubsystem::Pop()
{
	if (!Trail.IsEmpty())
	{
		Trail.Pop();
		OnBreadcrumbChanged.Broadcast(Trail);
	}
}

void UMBreadcrumbSubsystem::PopToDepth(const int32 Depth)
{
	if (Depth < 0 || Depth >= Trail.Num()) return;
	Trail.SetNum(Depth + 1);
	OnBreadcrumbChanged.Broadcast(Trail);
}
