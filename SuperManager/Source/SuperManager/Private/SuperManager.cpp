// Copyright Epic Games, Inc. All Rights Reserved.

#include "SuperManager.h"
#include "SuperManagerStyle.h"
#include "AssetToolsModule.h"
#include "ContentBrowserModule.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "SlateWidget/AdvanceDeletionWidget.h"
#include "LevelEditor.h"
#include "Engine/Selection.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "UICommands/SuperManagerUICommands.h"
#include "Framework/Commands/UICommandList.h"
#include "SceneOutlinerModule.h"
#include "CustomOutlinerColumn/OutlinerSelectionColumn.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerModule::StartupModule()
{
	// This code will execute after your module is loaded into memory; the exact timing is specified in the .uplugin file per-module
	FSuperManagerStyle::InitializeIcons();//初始化编辑器按钮图标
	InitCBMenuExtention(); //初始化内容菜单扩展器
	RegisterAdvanceDeletionTab(); //注册高级删除选项卡
	FSuperManagerUICommands::Register();//注册快捷命令集
	InitCustomUICommands();//初始化自定义快捷键命令
	InitLevelEditorExtention();//初始化关卡编辑器扩展器
	InitCustomSelectionEvent();//初始化自定义选择事件
	InitSceneOutlinerColumnExtension();
	
}

void FSuperManagerModule::ShutdownModule()
{
	// This function may be called during shutdown to clean up your module.  For modules that support dynamic reloading,
	// we call this function before unloading the module.

	FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(FName("AdvanceDeletion"));

	FSuperManagerStyle::ShutDown();
	
	FSuperManagerUICommands::Unregister();

	UnregisterSceneOutlinerColumnExtension();
}

#pragma region ContentBrowserMenuExtention

void FSuperManagerModule::InitCBMenuExtention()
{
	//加载内容浏览器模块
	FContentBrowserModule& ContentBrowserModule =
	FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));

	//获取路径视图内容菜单扩展器代理数组
	TArray<FContentBrowserMenuExtender_SelectedPaths> &ContentBrowserModuleMenuExtenders =
	ContentBrowserModule.GetAllPathViewContextMenuExtenders();

	FContentBrowserMenuExtender_SelectedPaths CustomCBMenuDelegate; //创建代理对象
	
	CustomCBMenuDelegate.BindRaw(this,&FSuperManagerModule::CustomCBMenuExtender);
	
	ContentBrowserModuleMenuExtenders.Add(CustomCBMenuDelegate);
}

TSharedRef<FExtender> FSuperManagerModule::CustomCBMenuExtender(const TArray<FString>& SelectedPaths)
{
	TSharedRef<FExtender> MenuExtender(new FExtender());
	if (SelectedPaths.Num() > 0)
	{
		MenuExtender->AddMenuExtension(FName("Delete"),
			EExtensionHook::After,
			TSharedPtr<FUICommandList>(),
			FMenuExtensionDelegate::CreateRaw(this,&FSuperManagerModule::AddCBMenuEntry));
	}
	
	FolderPathsSelects = SelectedPaths;
	
	return MenuExtender;
}

void FSuperManagerModule::AddCBMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("UITitleSpace","UITitleKey","删除文件夹下未使用的资产"),
		NSLOCTEXT("UITipSpace","UITipKey","安全删除文件夹下未使用的资产"),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.DeleteUnusedAssets"),
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnDeleteUnusedAssetButtonClicked)
	);

	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("UITitleSpace","UITitleKey","删除文件夹下的空文件夹"),
		NSLOCTEXT("UITipSpace","UITipKey","安全删除文件夹下的空文件夹"),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.DeleteEmptyFolders"),
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked)
	);

	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("UITitleSpace","UITitleKey","高级删除"),
		NSLOCTEXT("UITipSpace","UITipKey","将待删除的资产按特定的条件列在一个表里！"),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.AdvanceDeletion"),
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnAdvanceDeletionButtonClicked)
	);
}

void FSuperManagerModule::OnDeleteUnusedAssetButtonClicked()
{
	if (ConstructedDockTab.IsValid())
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,NSLOCTEXT("MessageSpace","MessageKey","请先关闭高级删除选项卡"));
		return;
	}
	if (FolderPathsSelects.Num()>1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,NSLOCTEXT("MessageSpace","MessageKey","你只能对一个文件夹进行此操作"));
		return;
	}

	TArray<FString> AssetsPathNames =
	UEditorAssetLibrary::ListAssets(FolderPathsSelects[0]);

	if (AssetsPathNames.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,NSLOCTEXT("MessageSpace","MessageKey","在选定的文件夹内没有发现资产！"));
		return;
	}

	EAppReturnType::Type ConfirmResult =
	DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,FText::Format(NSLOCTEXT("MessageSpace","MessageKey",
		"一共在文件夹内找到{0}个资产\n你确定要继续吗？"),FText::FromString(FString::FromInt(AssetsPathNames.Num()))),false);

	if (ConfirmResult == EAppReturnType::No) return;

	FixUpRedirectors();

	TArray<FAssetData> UnusedAssetsDatas;

	for (const FString& AssetPathName:AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Collections")) || AssetPathName.Contains(TEXT("Developers"))
			|| AssetPathName.Contains("__ExternalActors__") || AssetPathName.Contains("__ExternalObjects__"))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		//获取该资产的被引用列表
		TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(AssetPathName);

		if (AssetReferencers.Num() == 0)
		{
			const FAssetData UnusedAssetData = UEditorAssetLibrary::FindAssetData(AssetPathName);
			UnusedAssetsDatas.Add(UnusedAssetData);
		}
	}

	if (UnusedAssetsDatas.Num() > 0)
	{
		ObjectTools::DeleteAssets(UnusedAssetsDatas);

		EAppReturnType::Type MessageReturnType =
		DebugHeader::ShowMsgDialog(EAppMsgType::YesNo,NSLOCTEXT("MessageNameSpace","MessageKey","你需要删除空白的文件夹吗？"),false);

		if (MessageReturnType == EAppReturnType::No) return;
		OnDeleteEmptyFoldersButtonClicked();
	}
	else
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,NSLOCTEXT("MessageSpace","MessageKey","在选定的文件夹内没有发现未使用的资产！"));
	}
}

void FSuperManagerModule::OnDeleteEmptyFoldersButtonClicked()
{
	if (FolderPathsSelects.Num()>1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,NSLOCTEXT("MessageSpace","MessageKey","你只能对一个文件夹进行此操作"));
		return;
	}
	
	TArray<FString> FolderPathNames =
	UEditorAssetLibrary::ListAssets(FolderPathsSelects[0],true,true);

	uint32 Counter = 0;

	FString EmptyFolderPathsNames; //发现的空白文件夹路径名
	TArray<FString> EmptyFoldersPathsArray; //空的文件夹路径数组

	for (const FString& FolderPath:FolderPathNames)
	{
		if (FolderPath.Contains(TEXT("Collections")) || FolderPath.Contains(TEXT("Developers"))
			|| FolderPath.Contains("__ExternalActors__") || FolderPath.Contains("__ExternalObjects__"))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesDirectoryExist(FolderPath)) continue;

		//判断目录下是否有资产
		if (!UEditorAssetLibrary::DoesDirectoryHaveAssets(FolderPath))
		{
			
			EmptyFolderPathsNames.Append(FolderPath);
			EmptyFolderPathsNames.Append(TEXT("\n"));

			EmptyFoldersPathsArray.Add(FolderPath);
		}
		
	}

	if (EmptyFoldersPathsArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,NSLOCTEXT("MessageSpace","MessageKey","文件夹下没有空白文件夹！"),false);
		return;
	}

	EAppReturnType::Type ConfirmResult = DebugHeader::ShowMsgDialog(EAppMsgType::OkCancel,
		FText::Format(NSLOCTEXT("MessageSpace","MessageKey","发现空白文件夹:\n"
													  "{0}\n{1}"),FText::FromString(EmptyFolderPathsNames),NSLOCTEXT("MessageNameSpace","MessageKey","确定要删除它们吗？")),false);

	if (ConfirmResult == EAppReturnType::Cancel) return;

	FixUpRedirectors();

	for (const FString& EmptyFolderPath : EmptyFoldersPathsArray)
	{
		UEditorAssetLibrary::DeleteDirectory(EmptyFolderPath)?
			++Counter:DebugHeader::ShowNotifyInfo(FText::Format(NSLOCTEXT("MessageNameSpace","MessageKey","删除失败{0}"),FText::FromString(EmptyFolderPath)));
	}
	if (Counter>0)
	{
		DebugHeader::ShowNotifyInfo(FText::Format(NSLOCTEXT("MessageNameSpace","MessageKey","成功删除{0}个空文件夹")
			,FText::FromString(FString::FromInt(Counter))));
	}

	
}

void FSuperManagerModule::OnAdvanceDeletionButtonClicked()
{
	FixUpRedirectors();
	FGlobalTabmanager::Get()->TryInvokeTab(FName("AdvanceDeletion")); //尝试打开选项卡
}

void FSuperManagerModule::FixUpRedirectors()
{
	TArray<UObjectRedirector*> RedirectorsToFixArray;

	//加载资产注册表模块
	FAssetRegistryModule& AssetRegistryModule = 
	FModuleManager::Get().LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));

	//资产查询过滤器
	FARFilter Filter;
	Filter.bRecursivePaths = true; //允许进入所有子文件夹内
	Filter.PackagePaths.Emplace("/Game");
	Filter.ClassPaths.Emplace("/ObjectRedirector");

	TArray<FAssetData> OutRedirector;
	AssetRegistryModule.Get().GetAssets(Filter,OutRedirector);

	for (const FAssetData& RedirectorData : OutRedirector)
	{
		if (UObjectRedirector* RedirectorToFix = Cast<UObjectRedirector>(RedirectorData.GetAsset()))
		{
			RedirectorsToFixArray.Add(RedirectorToFix);
		}
	}

	FAssetToolsModule& AssetToolsModule = 
	FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	AssetToolsModule.Get().FixupReferencers(RedirectorsToFixArray);
}

#pragma endregion

#pragma region CustomEditorTab

void FSuperManagerModule::RegisterAdvanceDeletionTab()
{
	FGlobalTabmanager::Get()->RegisterNomadTabSpawner(FName("AdvanceDeletion"),FOnSpawnTab::CreateRaw(this
		,&FSuperManagerModule::OnSpawnAdvanceDeletionTab))
	.SetDisplayName(NSLOCTEXT("TextNameSpace","TextNameKey","高级删除"))
	.SetIcon(FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"ContentBrowser.AdvanceDeletion"));
	
}

TSharedRef<SDockTab> FSuperManagerModule::OnSpawnAdvanceDeletionTab(const FSpawnTabArgs& SpawnTabArgs)
{
	if(FolderPathsSelects.Num() == 0) return SNew(SDockTab).TabRole(NomadTab);
	
	ConstructedDockTab =
	SNew(SDockTab).TabRole(NomadTab) //新建一张普通的选项卡
	[
		SNew(SAdvanceDeletionTab)
		.TestString(TEXT("高级删除"))
		.AssetsDataToStore(GetAllAssetDataUnderSelectedFolder())
		.CurrentSelectedFolder(FolderPathsSelects[0])
	];
	
	ConstructedDockTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateRaw(this
		,&FSuperManagerModule::OnAdvanceDeletionTabClosed));
	
	return ConstructedDockTab.ToSharedRef();
}

TArray<TSharedPtr<FAssetData>> FSuperManagerModule::GetAllAssetDataUnderSelectedFolder()
{
	TArray< TSharedPtr<FAssetData> > AvaiableAssetsData;
	if (FolderPathsSelects.Num()>1)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,NSLOCTEXT("MessageSpace","MessageKey","你只能对一个文件夹进行此操作"));
		return AvaiableAssetsData;
	}
	
	TArray<FString> AssetsPathNames = UEditorAssetLibrary::ListAssets(FolderPathsSelects[0]);

	for (const FString& AssetPathName:AssetsPathNames)
	{
		if (AssetPathName.Contains(TEXT("Collections")) || AssetPathName.Contains(TEXT("Developers"))
			|| AssetPathName.Contains("__ExternalActors__") || AssetPathName.Contains("__ExternalObjects__"))
		{
			continue;
		}

		if (!UEditorAssetLibrary::DoesAssetExist(AssetPathName)) continue;

		const FAssetData Data = UEditorAssetLibrary::FindAssetData(AssetPathName);

		AvaiableAssetsData.Add(MakeShared<FAssetData>(Data));
		
	}
	return AvaiableAssetsData;
	
}

void FSuperManagerModule::OnAdvanceDeletionTabClosed(TSharedRef<SDockTab> TabToClose)
{
	if (ConstructedDockTab.IsValid())
	{
		ConstructedDockTab.Reset();
		FolderPathsSelects.Empty();
	}
	
}

#pragma endregion 

#pragma region ProccessDataForAdvanceDeletionTab

bool FSuperManagerModule::DeleteSingleAssetForAssetList(const FAssetData& AssetDataToDelete)
{
	TArray<FAssetData> AssetDataForDeletion;
	AssetDataForDeletion.Add(AssetDataToDelete);

	if (ObjectTools::DeleteAssets(AssetDataForDeletion) > 0)
	{
		return true;
	}
	return false;
}

bool FSuperManagerModule::DeleteMultipleAssetsForAssetList(const TArray<FAssetData>& AssetsToDelete)
{
	if (ObjectTools::DeleteAssets(AssetsToDelete) > 0)
	{
		return true;
	}
	return false;
}

void FSuperManagerModule::ListUnusedAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutUnusedAssetsData)
{
	OutUnusedAssetsData.Empty();
	for (const TSharedPtr<FAssetData>& DataSharedPtr:AssetsDataToFilter)
	{
		TArray<FString> AssetReferencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(DataSharedPtr->GetObjectPathString());

		if (AssetReferencers.Num() == 0)
		{
			OutUnusedAssetsData.Add(DataSharedPtr);
		}
	}
}

void FSuperManagerModule::ListSameNameAssetsForAssetList(const TArray<TSharedPtr<FAssetData>>& AssetsDataToFilter,
	TArray<TSharedPtr<FAssetData>>& OutSameNameAssetsData)
{
	OutSameNameAssetsData.Empty();
	TMultiMap<FString,TSharedPtr<FAssetData>> AssetsInfoMultiMap;

	for (const TSharedPtr<FAssetData>& DataSharedPtr : AssetsDataToFilter)
	{
		AssetsInfoMultiMap.Emplace(DataSharedPtr->AssetName.ToString(),DataSharedPtr);
	}

	for (const TSharedPtr<FAssetData>& DataSharedPtr:AssetsDataToFilter)
	{
		TArray<TSharedPtr<FAssetData>> OutAssetsData;
		AssetsInfoMultiMap.MultiFind(DataSharedPtr->AssetName.ToString(),OutAssetsData);

		if(OutAssetsData.Num()<=1) continue;

		for(const TSharedPtr<FAssetData>& SameNameData:OutAssetsData)
		{
			if (SameNameData.IsValid())
			{
				OutSameNameAssetsData.AddUnique(SameNameData);
			}
		}
	}
}

void FSuperManagerModule::SyncCBToClickedAssetForAssetList(const FString& AssetPathToSync)
{
	TArray<FString> AssetsPathToSync;
	AssetsPathToSync.Add(AssetPathToSync);
	UEditorAssetLibrary::SyncBrowserToObjects(AssetsPathToSync);//将内容浏览器同步到给定资源
}

#pragma endregion

#pragma region LevelEditorMenuExtension

void FSuperManagerModule::InitLevelEditorExtention()
{
	FLevelEditorModule& LevelEditorModule =
	FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));

	TSharedRef<FUICommandList> ExistingLevelCommands = LevelEditorModule.GetGlobalLevelEditorActions();
	ExistingLevelCommands->Append(CustomUICommands.ToSharedRef());

	TArray<FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors>& LevelEditorMenuExtenders =
	LevelEditorModule.GetAllLevelViewportContextMenuExtenders();

	LevelEditorMenuExtenders.Add(FLevelEditorModule::FLevelViewportMenuExtender_SelectedActors::
		CreateRaw(this,&FSuperManagerModule::CustomLevelEditorMenuExtender));
}

TSharedRef<FExtender> FSuperManagerModule::CustomLevelEditorMenuExtender(const TSharedRef<FUICommandList> UICommandList,
	const TArray<AActor*> SelectedActors)
{
	TSharedRef<FExtender> MenuExtender = MakeShareable(new FExtender);

	if (SelectedActors.Num()>0)
	{
		MenuExtender->AddMenuExtension(
			FName("ActorOptions"),
			EExtensionHook::Before,
			UICommandList,
			FMenuExtensionDelegate::CreateRaw(this,&FSuperManagerModule::AddLevelEditorMenuEntry));
	}
	return MenuExtender;
}

void FSuperManagerModule::AddLevelEditorMenuEntry(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("TextTitleSpace","TextTitleKey","锁定Actor"),
		NSLOCTEXT("TextTitleSpace","TextTitleKey","锁定选择的Actor"),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"LevelEditor.LockSelection"),
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnLockActorSelectionButtonClicked)
		
	);
	MenuBuilder.AddMenuEntry(
		NSLOCTEXT("TextTitleSpace","TextTitleKey","解锁Actor"),
		NSLOCTEXT("TextTitleSpace","TextTitleKey","解锁被约束选择的Actor"),
		FSlateIcon(FSuperManagerStyle::GetStyleSetName(),"LevelEditor.UnLockSelection"),
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnUnLockActorSelectionButtonClicked)
		
	);
}

void FSuperManagerModule::OnLockActorSelectionButtonClicked()
{
	if(!GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = WeakEditorActorSubsystem->GetSelectedLevelActors();

	if (SelectedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","没有选择Actor"));
		return;
	}

	FString CurrentLockedActorNames = TEXT("锁定选择的Actor:");
	
	for (AActor* SelectedActor : SelectedActors)
	{
		if(!SelectedActor) continue;

		LockActorSelection(SelectedActor);
		
		WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor,false);

		CurrentLockedActorNames.Append(TEXT("\n"));
		CurrentLockedActorNames.Append(SelectedActor->GetActorLabel());
		
	}

	RefreshSceneOutliner();
	
	DebugHeader::ShowNotifyInfo(FText::Format(NSLOCTEXT("MessageNameSpace","MessageKey","{0}")
		,FText::FromString(CurrentLockedActorNames)));
}

void FSuperManagerModule::OnUnLockActorSelectionButtonClicked()
{
	if(!GetEditorActorSubsystem()) return;

	TArray<AActor*> AllActorsInLevel = WeakEditorActorSubsystem->GetAllLevelActors();
	TArray<AActor*> AllLockedActors;

	for (AActor* ActorInLevel: AllActorsInLevel)
	{
		if(!ActorInLevel) continue;

		if (CheckIsActorSelectionLocked(ActorInLevel))
		{
			AllLockedActors.Add(ActorInLevel);
		}
	}

	if (AllLockedActors.Num() == 0)
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","当前关卡没有被锁定的Actor"));
		return;
	}

	
	FString UnLockedActorNames = TEXT("解锁被约束选择的Actor:");
	for (AActor* LockedActor : AllLockedActors)
	{
		UnLockActorSelection(LockedActor);
		UnLockedActorNames.Append("\n");
		UnLockedActorNames.Append(LockedActor->GetActorLabel());
	}
	
	RefreshSceneOutliner();
	
	DebugHeader::ShowNotifyInfo(FText::Format(NSLOCTEXT("MessageNameSpace","MessageKey","{0}")
		,FText::FromString(UnLockedActorNames)));
}

#pragma endregion

#pragma region SelectionLock
	
void FSuperManagerModule::InitCustomSelectionEvent()
{
	USelection* UserSelection = GEditor->GetSelectedActors();

	UserSelection->SelectObjectEvent.AddRaw(this,&FSuperManagerModule::OnActorSelected);
}

void FSuperManagerModule::OnActorSelected(UObject* SelectedObject)
{
	if(!GetEditorActorSubsystem()) return;
	
	if (AActor* SelectedActor = Cast<AActor>(SelectedObject))
	{
		if (CheckIsActorSelectionLocked(SelectedActor))
		{
			WeakEditorActorSubsystem->SetActorSelectionState(SelectedActor,false);
		}
	}
}

void FSuperManagerModule::LockActorSelection(AActor* ActorToProcess)
{
	if(!ActorToProcess) return;

	if (!ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Add(FName("Locked"));
	}
	
}

void FSuperManagerModule::UnLockActorSelection(AActor* ActorToProcess)
{
	if(!ActorToProcess) return;

	if (ActorToProcess->ActorHasTag(FName("Locked")))
	{
		ActorToProcess->Tags.Remove(FName("Locked"));
	}
}

void FSuperManagerModule::RefreshSceneOutliner()
{
	FLevelEditorModule& LevelEditorModule =
	FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
	
	TSharedPtr<ISceneOutliner> SceneOutliner = LevelEditorModule.GetLevelEditorInstance()
	.Pin()->GetMostRecentlyUsedSceneOutliner();

	if (SceneOutliner.IsValid())
	{
		SceneOutliner->FullRefresh();
	}
}

bool FSuperManagerModule::CheckIsActorSelectionLocked(AActor* ActorToProcess)
{
	if(!ActorToProcess) return false;
	
	return ActorToProcess->ActorHasTag(FName("Locked"));
}

void FSuperManagerModule::ProcessLockingForOutliner(AActor* ActorToProcess, bool bShouldLock)
{
	if(!GetEditorActorSubsystem()) return;
	
	if (bShouldLock)
	{
		LockActorSelection(ActorToProcess);
		WeakEditorActorSubsystem->SetActorSelectionState(ActorToProcess,false);
		DebugHeader::ShowNotifyInfo(FText::Format(
			NSLOCTEXT("MessageNameSpace","MessageKey","锁定Actor:\n{0}")
			,FText::FromString(ActorToProcess->GetActorLabel())));
	}
	else
	{
		UnLockActorSelection(ActorToProcess);
		DebugHeader::ShowNotifyInfo(FText::Format(
			NSLOCTEXT("MessageNameSpace","MessageKey","解除锁定Actor:\n{0}")
			,FText::FromString(ActorToProcess->GetActorLabel())));
	}
}

#pragma endregion

#pragma region CustomEditorUICommands

void FSuperManagerModule::InitCustomUICommands()
{
	CustomUICommands = MakeShareable(new FUICommandList);
	CustomUICommands->MapAction(
		FSuperManagerUICommands::Get().LockActorSelection,
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnSelectionLockHotKeyPressed)
		);

	CustomUICommands->MapAction(
		FSuperManagerUICommands::Get().UnLockActorSelection,
		FExecuteAction::CreateRaw(this,&FSuperManagerModule::OnSelectionUnLockHotKeyPressed)
		);
}

void FSuperManagerModule::OnSelectionLockHotKeyPressed()
{
	OnLockActorSelectionButtonClicked();
}

void FSuperManagerModule::OnSelectionUnLockHotKeyPressed()
{
	OnUnLockActorSelectionButtonClicked();
}

#pragma endregion

#pragma region SceneOutlinerExtension
	
void FSuperManagerModule::InitSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule =
	FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	FSceneOutlinerColumnInfo SelectionLockColumnInfo(
		ESceneOutlinerColumnVisibility::Visible,
		1,
		FCreateSceneOutlinerColumn::CreateRaw(this,&FSuperManagerModule::OnCreateSelectionLockColumn)
	);

	SceneOutlinerModule.RegisterDefaultColumnType<FOutlinerSelectionLockColumn>(SelectionLockColumnInfo);
}

TSharedRef<ISceneOutlinerColumn> FSuperManagerModule::OnCreateSelectionLockColumn(ISceneOutliner& SceneOutliner)
{
	return MakeShareable(new FOutlinerSelectionLockColumn(SceneOutliner));
}

void FSuperManagerModule::UnregisterSceneOutlinerColumnExtension()
{
	FSceneOutlinerModule& SceneOutlinerModule =
	FModuleManager::LoadModuleChecked<FSceneOutlinerModule>(TEXT("SceneOutliner"));

	SceneOutlinerModule.UnRegisterColumnType<FOutlinerSelectionLockColumn>();
}

#pragma endregion 

bool FSuperManagerModule::GetEditorActorSubsystem()
{
	if (!WeakEditorActorSubsystem.IsValid())
	{
		WeakEditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	return WeakEditorActorSubsystem.IsValid();
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSuperManagerModule, SuperManager)