// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

class FSuperManagerModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	
private:

#pragma region ContentBrowserMenuExtention
	void InitCBMenuExtention();

	//选择的文件夹路径数组
	TArray<FString> FolderPathsSelects;
	
	//自定义内容菜单扩展器
	TSharedRef<FExtender> CustomCBMenuExtender(const TArray<FString>& SelectedPaths);

	//添加内容菜单条目
	void AddCBMenuEntry(class FMenuBuilder& MenuBuilder);

	//按下按钮后执行功能函数
	void OnDeleteUnusedAssetButtonClicked();
	void OnDeleteEmptyFoldersButtonClicked();
	void OnAdvanceDeletionButtonClicked();

	//修复重定向器
	void FixUpRedirectors();
	
#pragma endregion

#pragma region CustomEditorTab

	void RegisterAdvanceDeletionTab();

	TSharedRef<SDockTab> OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs);
	TSharedPtr<SDockTab> ConstructedDockTab;

	TArray< TSharedPtr<FAssetData> > GetAllAssetDataUnderSelectedFolder();

	void OnAdvanceDeletionTabClosed(TSharedRef<SDockTab> TabToClose);
#pragma endregion

#pragma region LevelEditorMenuExtension

	void InitLevelEditorExtention();

	TSharedRef<FExtender> CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList,const TArray<AActor*> SelectedActors);

	void AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder);

	void OnLockActorSelectionButtonClicked();
	void OnUnLockActorSelectionButtonClicked();
	
#pragma endregion

#pragma region SelectionLock
	
	void InitCustomSelectionEvent();

	void OnActorSelected(UObject* SelectedObject);

	void LockActorSelection(AActor* ActorToProcess);
	void UnLockActorSelection(AActor* ActorToProcess);
	
	void RefreshSceneOutliner();
	
public:
	bool CheckIsActorSelectionLocked(AActor* ActorToProcess);
	void ProcessLockingForOutliner(AActor* ActorToProcess,bool bShouldLock);
	
#pragma endregion
	
private:
	
#pragma region CustomEditorUICommands

	TSharedPtr<FUICommandList> CustomUICommands;

	void InitCustomUICommands();

	void OnSelectionLockHotKeyPressed();
	void OnSelectionUnLockHotKeyPressed();
	
#pragma endregion

#pragma region SceneOutlinerExtension
	
	void InitSceneOutlinerColumnExtension();

	TSharedRef<class ISceneOutlinerColumn> OnCreateSelectionLockColumn(class ISceneOutliner& SceneOutliner);

	void UnregisterSceneOutlinerColumnExtension();
	
#pragma endregion
	TWeakObjectPtr<class UEditorActorSubsystem> WeakEditorActorSubsystem;

	bool GetEditorActorSubsystem();
public:

#pragma region ProccessDataForAdvanceDeletionTab

	bool DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete);
	bool DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete);
	void ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter
		,TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData);
	
	void ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter
		,TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData);

	void SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync);

#pragma endregion
	
};
