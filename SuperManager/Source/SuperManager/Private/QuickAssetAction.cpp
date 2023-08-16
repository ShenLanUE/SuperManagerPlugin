// Fill out your copyright notice in the Description page of Project Settings.


#include "QuickAssetAction.h"
#include "DebugHeader.h"
#include "EditorAssetLibrary.h"
#include "EditorUtilityLibrary.h"
#include "ObjectTools.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"

using namespace DebugHeader;

void UQuickAssetAction::TestFun()
{
	Print("working",FColor::Cyan);

	
}

void UQuickAssetAction::DuplicateAssets(int32 NumOfDuplicates)
{
	if (NumOfDuplicates <= 0)
	{
		//Print(TEXT("请输入一个有效的数值"),FColor::Red);
		FText Message = NSLOCTEXT("MessageNameSpace","Warning","请输入一个有效的值！");
		ShowMsgDialog(EAppMsgType::Ok,Message);
		return;
	}

	//获取用户选择的数据
	TArray<FAssetData> SelectedAssetDatas = UEditorUtilityLibrary::GetSelectedAssetData();
	uint32 Counter = 0;

	for (const FAssetData& SelecteAssetData:SelectedAssetDatas)
	{
		for (int32 i = 0; i < NumOfDuplicates; i++)
		{
			const FString SourceAssetPath = SelecteAssetData.GetObjectPathString(); //选择的资产源路径
			//新复制的资产名
			const FString NewDuplicatedAssetName = SelecteAssetData.AssetName.ToString() + "_" + FString::FromInt(i+1);
			//新资产的路径名,PackagePath为文件夹名字
			const FString NewPathName = FPaths::Combine(SelecteAssetData.PackagePath.ToString(),NewDuplicatedAssetName); 

			if (UEditorAssetLibrary::DuplicateAsset(SourceAssetPath,NewPathName))
			{
				UEditorAssetLibrary::SaveAsset(NewPathName,false);//参数置为False,立即保存新的资产
				++Counter;
			}

		}
		if (Counter > 0)
		{
			
			FText Message = FText::Format(NSLOCTEXT("DuplicateAssetNameSpace", "Duplicate", "成功复制了{0}个文件"),FText::FromString(FString::FromInt(Counter)));
			ShowNotifyInfo(Message);
			//Print(TEXT("成功复制了  " + FString::FromInt(Counter) + "  个文件"),FColor::Green);
		}
		
	}

	
}

void UQuickAssetAction::AddPrefixes()
{
	TArray<UObject*> SelectedAssets = UEditorUtilityLibrary::GetSelectedAssets();
	uint32 Counter = 0;
	for (UObject* SelectedObject : SelectedAssets)
	{
		if (!SelectedObject) continue;
		
		FString* PrefixFound = PrefixMap.Find(SelectedObject->GetClass());

		if (!PrefixFound || PrefixFound->IsEmpty())
		{
			FText Message = NSLOCTEXT("AddPrefixesNameSpace","AddPrefixes","在前缀名Map里没有发现该类对应的前缀名");
			ShowMsgDialog(EAppMsgType::Ok,Message);
			continue;
		}
		
		FString OldName = SelectedObject->GetName();

		if (OldName.StartsWith(*PrefixFound))
		{
			FText Message = FText::Format(NSLOCTEXT("AddPrefixesNameSpace","AddPrefixes", "{0}的前缀名已添加！"),FText::FromString(OldName));
			//如果选择的资产已经是以对应前缀名开头
			ShowMsgDialog(EAppMsgType::Ok,Message);
			continue;
		}
		
		if (SelectedObject->IsA<UMaterialInstanceConstant>())
		{
			//如果选择的资产是材质实例
			OldName.RemoveFromStart(TEXT("M_"));
			OldName.RemoveFromEnd(TEXT("_Inst"));
		}

		const FString NewNameWithPreFix = *PrefixFound + OldName;

		UEditorUtilityLibrary::RenameAsset(SelectedObject,NewNameWithPreFix);
		
		++Counter;
	}

	if (Counter > 0)
	{
		FText Message = FText::Format(NSLOCTEXT("RenameSpace", "RenameKey", "成功重命名{0}个资产！"),FText::FromString(FString::FromInt(Counter)));
		ShowNotifyInfo(Message);
	}
	
}

void UQuickAssetAction::RemoveUnusedAssets()
{
	TArray<FAssetData> SelectedAssetDatas = UEditorUtilityLibrary::GetSelectedAssetData();
	TArray<FAssetData> UnusedAssetData;

	FixUpRedirectors();
	for (const FAssetData& SelectedAssetData:SelectedAssetDatas)
	{
		TArray<FString> AssetRefrencers =
		UEditorAssetLibrary::FindPackageReferencersForAsset(SelectedAssetData.GetObjectPathString());

		if (AssetRefrencers.Num()==0)
		{
			UnusedAssetData.Add(SelectedAssetData);
		}
	}
	if (UnusedAssetData.Num()==0)
	{
		ShowMsgDialog(EAppMsgType::Ok,NSLOCTEXT("MessageSpace","Message","选择的资产中未能发现未使用的资产！"),false);
		return;
	}

	//调用编辑器对象工具删除
	const int32 NumOfAssetsDeleted = ObjectTools::DeleteAssets(UnusedAssetData);

	if (NumOfAssetsDeleted == 0) return;

	ShowNotifyInfo(FText::Format(NSLOCTEXT("MessageSpace","Message","成功删除了{0}个资产")
		,FText::FromString(FString::FromInt(NumOfAssetsDeleted))));

	


}

void UQuickAssetAction::FixUpRedirectors()
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
