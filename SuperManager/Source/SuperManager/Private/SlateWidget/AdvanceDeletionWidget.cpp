// Fill out your copyright notice in the Description page of Project Settings.


#include "SlateWidget/AdvanceDeletionWidget.h"

#include "DebugHeader.h"
#include "SlateBasics.h"
#include "SuperManager.h"

#define ListAll TEXT("所有可用资产")
#define ListUnused TEXT("未引用资产")
#define ListUnusedSameName TEXT("同名资产")

void SAdvanceDeletionTab::Construct(const FArguments& InArgs)
{
	bCanSupportFocus = true; //这个小部件能支持键盘焦点吗

	StoredAssetsData = InArgs._AssetsDataToStore;
	DisplayedAssetsData = StoredAssetsData;
	CheckBoxArray.Empty();
	AssetsDataToDeleteArray.Empty();
	ComboBoxSourceItems.Empty();

	ComboBoxSourceItems.Add(MakeShared<FString>(ListAll));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnused));
	ComboBoxSourceItems.Add(MakeShared<FString>(ListUnusedSameName));
	
	FSlateFontInfo TitleTextFont = GetEmbossedTextFont();
	TitleTextFont.Size = 30.f;

	ChildSlot
	[
		/*SNew(STextBlock)
		.Text(FText::Format(NSLOCTEXT("TextNameSpace","TextKey","{0}")
			,FText::FromString(InArgs._TestString)))*/
		//主垂直框
		SNew(SVerticalBox)
		+SVerticalBox::Slot() //主垂直框第一个插槽用于标题
		.AutoHeight()
		[
			SNew(STextBlock)
			.Text(FText::Format(NSLOCTEXT("TextNameSpace","TextKey","{0}")
			,FText::FromString(InArgs._TestString)))
			.Font(TitleTextFont)
			.Justification(ETextJustify::Center)
			.ColorAndOpacity(FColor::White)
		]

		//第二个槽位为用于下拉列表来指定列表条件
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)

			//组合下拉选择框插槽
			+SHorizontalBox::Slot()
			.AutoWidth()
			[
				ConstructComboBox()
			]

			+SHorizontalBox::Slot()
			.FillWidth(.6f)
			[
				ConstructComboHelpTetxs(TEXT("在下拉选框选择筛选的条件！鼠标左键点击资产行可以定位到它在内容浏览器的位置")
					,ETextJustify::Center)
			]

			+SHorizontalBox::Slot()
			.FillWidth(.1f)
			[
				ConstructComboHelpTetxs(TEXT("当前选中的文件夹:\n") + InArgs._CurrentSelectedFolder,ETextJustify::Right)
			]
		]

		//第三个槽位用于显示实际的资产列表
		+SVerticalBox::Slot()
		.VAlign(VAlign_Fill)
		[
			SNew(SScrollBox)
			+SScrollBox::Slot()
			[
				ConstructAssetListView()
			]
		]

		//第四个插槽将用于容纳3个按钮
		+SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SHorizontalBox)
			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeleteAllButton()
			]

			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructSelectAllButton()
			]

			+SHorizontalBox::Slot()
			.FillWidth(10.f)
			.Padding(5.f)
			[
				ConstructDeselectAllButton()
			]
		]
	];
	
}

TSharedRef<SListView<TSharedPtr<FAssetData>>> SAdvanceDeletionTab::ConstructAssetListView()
{
	ConstructedAssetListView =
	SNew(SListView<TSharedPtr<FAssetData>>)
	.ItemHeight(24.f)
	.ListItemsSource(&DisplayedAssetsData)
	.OnGenerateRow(this,&SAdvanceDeletionTab::OnGenerateRowForList)
	.OnMouseButtonClick(this,&SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked);
	
	return ConstructedAssetListView.ToSharedRef();
}

void SAdvanceDeletionTab::RefreshAssetListView()
{
	CheckBoxArray.Empty();
	AssetsDataToDeleteArray.Empty();
	
	if (ConstructedAssetListView.IsValid())
	{
		ConstructedAssetListView->RebuildList();
	}
}



#pragma region RowWidgetForAssetListView

TSharedRef<ITableRow> SAdvanceDeletionTab::OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
                                                                const TSharedRef<STableViewBase>& OwnerTable)
{
	if (!AssetDataToDisplay.IsValid()) return SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable);
	
	const FString DisplayAssetName = AssetDataToDisplay->AssetName.ToString();

	FSlateFontInfo AssetClassNameFont = GetEmbossedTextFont();
	AssetClassNameFont.Size = 10.f;

	FSlateFontInfo AssetNameFont = GetEmbossedTextFont();
	AssetNameFont.Size = 15.f;
	
	TSharedRef<STableRow<TSharedPtr<FAssetData>>> ListViewRowWidget =
	SNew(STableRow<TSharedPtr<FAssetData>>,OwnerTable).Padding(FMargin(5.f))
	[
		SNew(SHorizontalBox)
		//复选框
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Center)
		.FillWidth(.05f)
		[
			ConstructCheckBox(AssetDataToDisplay)
		]
		
		//资产类型
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Center)
		.VAlign(VAlign_Center)
		.FillWidth(.55f)
		[
			ConstructTextForRowWidget(AssetDataToDisplay->AssetClassPath.GetAssetName().ToString(),AssetClassNameFont)
		]
		//资产名字
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Left)
		.VAlign(VAlign_Fill)
		[
			ConstructTextForRowWidget(DisplayAssetName,AssetNameFont)
		]
		
		//删除资产按钮
		+SHorizontalBox::Slot()
		.HAlign(HAlign_Right)
		.VAlign(VAlign_Fill)
		[
			ConstructButtonForRowWidget(AssetDataToDisplay)
		]
	];
	return ListViewRowWidget;
}

void SAdvanceDeletionTab::OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData)
{
	FSuperManagerModule& SuperManagerModule =
	FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	SuperManagerModule.SyncCBToClickedAssetForAssetList(ClickedData->GetObjectPathString());
	
}

TSharedRef<SCheckBox> SAdvanceDeletionTab::ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SCheckBox> ConstructedCheckBox =
	SNew(SCheckBox)
	.Type(ESlateCheckBoxType::CheckBox)
	.OnCheckStateChanged(this,&SAdvanceDeletionTab::OnCheckBoxStateChanged,AssetDataToDisplay)
	.Visibility(EVisibility::Visible);

	CheckBoxArray.Add(ConstructedCheckBox);
	return ConstructedCheckBox;
}

void SAdvanceDeletionTab::OnCheckBoxStateChanged(ECheckBoxState NewState, TSharedPtr<FAssetData> AssetData)
{
	switch (NewState)
	{
	case ECheckBoxState::Unchecked:
		{
			if (AssetsDataToDeleteArray.Contains(AssetData))
			{
				AssetsDataToDeleteArray.Remove(AssetData);
			}
			break;
		}		
	case ECheckBoxState::Checked:
		AssetsDataToDeleteArray.AddUnique(AssetData);
		break;
	case ECheckBoxState::Undetermined:
		
		break;
	default:
		
		break;;
	}
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForRowWidget(const FString& TextContent,
                                                                      const FSlateFontInfo& FontInfo)
{
	const FText LocalTextContent = FText::Format(NSLOCTEXT("TextNameSpace","TextKey","{0}")
		,FText::FromString(TextContent));
	
	TSharedRef<STextBlock> ConstructedTextBlock =
	SNew(STextBlock)
	.Text(LocalTextContent)
	.Font(FontInfo)
	.ColorAndOpacity(FColor::White);

	return ConstructedTextBlock;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay)
{
	TSharedRef<SButton> ConstructedButton =
	SNew(SButton)
	.Text(NSLOCTEXT("TextNameSpace","TextKey","删除"))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeleteButtonClick,AssetDataToDisplay);

	return ConstructedButton;
}

FReply SAdvanceDeletionTab::OnDeleteButtonClick(TSharedPtr<FAssetData> ClickedAssetData)
{
	FSuperManagerModule& SuperManagerModule =
	FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));

	const bool bAssetDeleted = SuperManagerModule.DeleteSingleAssetForAssetList(*ClickedAssetData.Get());

	if (bAssetDeleted)
	{
		//更新列表源
		if (StoredAssetsData.Contains(ClickedAssetData))
		{
			StoredAssetsData.Remove(ClickedAssetData);
		}
		if (DisplayedAssetsData.Contains(ClickedAssetData))
		{
			DisplayedAssetsData.Remove(ClickedAssetData);
		}
		//刷新列表
		RefreshAssetListView();
	}
	return FReply::Handled();
}
#pragma endregion

#pragma region TabButtons

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeleteAllButton()
{
	TSharedRef<SButton> DeleteAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeleteAllButtonClicked);

	DeleteAllButton->SetContent(ConstructTextForTabButtons(TEXT("删除所有")));

	return DeleteAllButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructSelectAllButton()
{
	TSharedRef<SButton> SelectAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnSelectAllButtonClicked);

	SelectAllButton->SetContent(ConstructTextForTabButtons(TEXT("选择所有")));

	return SelectAllButton;
}

TSharedRef<SButton> SAdvanceDeletionTab::ConstructDeselectAllButton()
{
	TSharedRef<SButton> DeselectAllButton = SNew(SButton)
	.ContentPadding(FMargin(5.f))
	.OnClicked(this,&SAdvanceDeletionTab::OnDeselectAllButtonClicked);

	DeselectAllButton->SetContent(ConstructTextForTabButtons(TEXT("取消选择所有")));

	return DeselectAllButton;
}


FReply SAdvanceDeletionTab::OnDeleteAllButtonClicked()
{
	if (AssetsDataToDeleteArray.Num() == 0)
	{
		DebugHeader::ShowMsgDialog(EAppMsgType::Ok,
			NSLOCTEXT("MessageNameSpace","MessageKey","没有选择要删除的选项"),false);
		return FReply::Handled();
	}
	
	TArray<FAssetData> AssetDataToDelete;
	
	for (const TSharedPtr<FAssetData>& Data : AssetsDataToDeleteArray)
	{
		AssetDataToDelete.Add(*Data.Get());
	}
	FSuperManagerModule& SuperManagerModule =
	FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	
	//将数据传递给模块
	const bool bAssetsDeleted = SuperManagerModule.DeleteMultipleAssetsForAssetList(AssetDataToDelete);

	if (bAssetsDeleted)
	{
		for (const TSharedPtr<FAssetData>& DeleteData : AssetsDataToDeleteArray)
		{
			//更新保存的资产数据
			if (StoredAssetsData.Contains(DeleteData))
			{
				StoredAssetsData.Remove(DeleteData);
			}

			if (DisplayedAssetsData.Contains(DeleteData))
			{
				DisplayedAssetsData.Remove(DeleteData);
			}
		}
		RefreshAssetListView();
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnSelectAllButtonClicked()
{
	if (CheckBoxArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox>& CheckBox: CheckBoxArray)
	{
		if (!CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState(); //勾选检查框
		}
	}
	return FReply::Handled();
}

FReply SAdvanceDeletionTab::OnDeselectAllButtonClicked()
{
	if (CheckBoxArray.Num() == 0) return FReply::Handled();

	for (const TSharedRef<SCheckBox>& CheckBox: CheckBoxArray)
	{
		if (CheckBox->IsChecked())
		{
			CheckBox->ToggleCheckedState(); //取消勾选检查框
		}
	}
	return FReply::Handled();
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructTextForTabButtons(const FString& TextContent)
{
	FSlateFontInfo ButtonTextFont = GetEmbossedTextFont();
	ButtonTextFont.Size = 15.f;

	TSharedRef<STextBlock> ConstructedTextBlock = SNew(STextBlock)
	.Text(FText::Format(NSLOCTEXT("TextSpace","TextKey","{0}")
		,FText::FromString(TextContent)))
	.Font(ButtonTextFont)
	.Justification(ETextJustify::Center);

	return ConstructedTextBlock;
}
#pragma endregion

#pragma region ComboBoxForListingCondition

TSharedRef<SComboBox<TSharedPtr<FString>>> SAdvanceDeletionTab::ConstructComboBox()
{
	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructedComboBox =
	SNew(SComboBox<TSharedPtr<FString>>)
	.OptionsSource(&ComboBoxSourceItems) //设置选项来源
	.OnGenerateWidget(this,&SAdvanceDeletionTab::OnGenerateComboContent)
	.OnSelectionChanged(this,&SAdvanceDeletionTab::OnComboSelectionChanged)
	[
		SAssignNew(ComboDisplaTextBlock,STextBlock) //必须要用这个宏创建，否则看不到任何组合框文本
		.Text(NSLOCTEXT("TextNameSpace","TextKey","列表资产选项")) //组合框的默认文本
	]; 
	
	return ConstructedComboBox;
}

TSharedRef<SWidget> SAdvanceDeletionTab::OnGenerateComboContent(TSharedPtr<FString> SourceItem)
{
	TSharedRef<STextBlock> ConstructedComboText =
	SNew(STextBlock)
	.Text(FText::Format(NSLOCTEXT("TextNameSpace","TextKey","{0}"),FText::FromString(*SourceItem.Get())));

	return  ConstructedComboText;
}

void SAdvanceDeletionTab::OnComboSelectionChanged(TSharedPtr<FString> SelectedOption, ESelectInfo::Type InSelectInfo)
{
	ComboDisplaTextBlock->SetText(FText::Format(NSLOCTEXT("TextNameSpace","TextKey","{0}")
		,FText::FromString(*SelectedOption)));

	FSuperManagerModule& SuperManagerModule =
	FModuleManager::LoadModuleChecked<FSuperManagerModule>(TEXT("SuperManager"));
	
	if (*SelectedOption.Get() == ListAll)
	{
		//列出所有可用资产
		DisplayedAssetsData = StoredAssetsData;
		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListUnused)
	{
		//列出未使用的资产
		SuperManagerModule.ListUnusedAssetsForAssetList(StoredAssetsData
			,DisplayedAssetsData);

		RefreshAssetListView();
	}
	else if (*SelectedOption.Get() == ListUnusedSameName)
	{
		//列出同名的资产
		SuperManagerModule.ListSameNameAssetsForAssetList(StoredAssetsData
			,DisplayedAssetsData);

		RefreshAssetListView();
	}
	
}

TSharedRef<STextBlock> SAdvanceDeletionTab::ConstructComboHelpTetxs(const FString& TextContent,
	ETextJustify::Type TextJustify)
{
	TSharedRef<STextBlock> ConstructedHelpText =
	SNew(STextBlock)
	.Text(FText::Format(NSLOCTEXT("TextNameSpace","TextKey","{0}"),FText::FromString(TextContent)))
	.Justification(TextJustify)
	.AutoWrapText(true);

	return ConstructedHelpText;
}
#pragma endregion 
