// Fill out your copyright notice in the Description page of Project Settings.

#include "SuperManagerStyle.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"
#include "Styling/StyleColors.h"

FName FSuperManagerStyle::StyleSetName = FName("SuperManagerStyle");
TSharedPtr<FSlateStyleSet> FSuperManagerStyle::CreatedSlateStyleSet = nullptr;

void FSuperManagerStyle::InitializeIcons()
{
	if (!CreatedSlateStyleSet.IsValid())
	{
		CreatedSlateStyleSet = CreateSlateStyleSet();

		FSlateStyleRegistry::RegisterSlateStyle(*CreatedSlateStyleSet);
		
	}
	
}

void FSuperManagerStyle::ShutDown()
{
	if (CreatedSlateStyleSet.IsValid())
	{
		FSlateStyleRegistry::UnRegisterSlateStyle(*CreatedSlateStyleSet);
		CreatedSlateStyleSet.Reset();
	}
}

TSharedRef<FSlateStyleSet> FSuperManagerStyle::CreateSlateStyleSet()
{
	TSharedRef<FSlateStyleSet> CustomStyleSet =
	MakeShareable(new FSlateStyleSet(StyleSetName));

	const FString IconDirectory =
	IPluginManager::Get().FindPlugin(TEXT("SuperManager"))->GetBaseDir() /"Resources";

	CustomStyleSet->SetContentRoot(IconDirectory); //设置图标样式的目录

	const FVector2d Icon16x16(16.f,16.f);

	CustomStyleSet->Set("ContentBrowser.DeleteUnusedAssets"
		,new FSlateImageBrush(IconDirectory/"DeleteUnusedAsset.png",Icon16x16));
	
	CustomStyleSet->Set("ContentBrowser.DeleteEmptyFolders"
		,new FSlateImageBrush(IconDirectory/"DeleteEmptyFolders.png",Icon16x16));
	
	CustomStyleSet->Set("ContentBrowser.AdvanceDeletion"
		,new FSlateImageBrush(IconDirectory/"AdvanceDeletion.png",Icon16x16));

	CustomStyleSet->Set("LevelEditor.LockSelection"
		,new FSlateImageBrush(IconDirectory/"SelectionLock.png",Icon16x16));

	CustomStyleSet->Set("LevelEditor.UnLockSelection"
		,new FSlateImageBrush(IconDirectory/"SelectionUnlock.png",Icon16x16));

	const FCheckBoxStyle SelectionLockToggleButtonStyle = FCheckBoxStyle()
	.SetCheckBoxType(ESlateCheckBoxType::ToggleButton)
	.SetPadding(FMargin(10.f))

	//未选择的样式
	.SetUncheckedImage(FSlateImageBrush(IconDirectory/"SelectionLock.png",Icon16x16
		,FStyleColors::White25))
	.SetUncheckedHoveredImage(FSlateImageBrush(IconDirectory/"SelectionLock.png",Icon16x16
		,FStyleColors::AccentBlue))
	.SetUncheckedPressedImage(FSlateImageBrush(IconDirectory/"SelectionLock.png",Icon16x16
		,FStyleColors::Foreground))

	//选中时
	.SetCheckedImage(FSlateImageBrush(IconDirectory/"SelectionLock.png",Icon16x16
		,FStyleColors::Foreground))
	.SetCheckedHoveredImage(FSlateImageBrush(IconDirectory/"SelectionLock.png",Icon16x16
		,FStyleColors::AccentBlack))
	.SetCheckedPressedImage(FSlateImageBrush(IconDirectory/"SelectionLock.png",Icon16x16
		,FStyleColors::AccentGray));
	
	CustomStyleSet->Set("SceneOutliner.SelectionLock",SelectionLockToggleButtonStyle);
	
	return CustomStyleSet;
}
