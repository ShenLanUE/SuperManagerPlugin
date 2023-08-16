// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Widgets/SCompoundWidget.h"

class SAdvanceDeletionTab : public SCompoundWidget
{
	/*小部件作者可以使用SLATE_BEGIN_ARGS和SLATE_END_ARS来通过SNew和SAssignNew添加对小部件构造的支持。
	 *如:SLATE_BEGIN_ARGS（ SMyWidget），_PreferredWidth（ 150.Of），_ForegroundColor（ 颜色：白色）
	 *SLATE_ATTRIBUTE(float, PreferredWidth)前景色）
	 *SLATE_END_ARGSO
	 */
	SLATE_BEGIN_ARGS(SAdvanceDeletionTab){}

	SLATE_ARGUMENT(FString,TestString) //使用这个宏来声明一个slate参数。参数与属性的不同之处在于它们只能是值
	SLATE_ARGUMENT(TArray< TSharedPtr<FAssetData> >,AssetsDataToStore)
	SLATE_ARGUMENT(FString,CurrentSelectedFolder)

	SLATE_END_ARGS()

public:
	void Construct(const FArguments& InArgs);

private:
	TArray< TSharedPtr<FAssetData> > StoredAssetsData;
	TArray< TSharedPtr<FAssetData> > DisplayedAssetsData;
	TArray<TSharedPtr<FAssetData>> AssetsDataToDeleteArray;
	TArray<TSharedRef<SCheckBox>> CheckBoxArray;

	TSharedPtr<SListView<TSharedPtr<FAssetData> >> ConstructedAssetListView;

	TSharedRef<SListView<TSharedPtr<FAssetData> > > ConstructAssetListView();

	void RefreshAssetListView();

#pragma region ComboBoxForListingCondition

	TSharedRef<SComboBox<TSharedPtr<FString>>> ConstructComboBox();

	TArray<TSharedPtr<FString>> ComboBoxSourceItems;

	TSharedRef<SWidget> OnGenerateComboContent(TSharedPtr<FString> SourceItem);

	void OnComboSelectionChanged(TSharedPtr<FString> SelectedOption,ESelectInfo::Type InSelectInfo);

	TSharedPtr<STextBlock> ComboDisplaTextBlock;

	TSharedRef<STextBlock> ConstructComboHelpTetxs(const FString& TextContent,ETextJustify::Type TextJustify);
	
#pragma endregion
	
#pragma region RowWidgetForAssetListView
	TSharedRef<ITableRow> OnGenerateRowForList(TSharedPtr<FAssetData> AssetDataToDisplay,
		const TSharedRef<STableViewBase>& OwnerTable);

	void OnRowWidgetMouseButtonClicked(TSharedPtr<FAssetData> ClickedData);
	
	void OnCheckBoxStateChanged(ECheckBoxState NewState,TSharedPtr<FAssetData> AssetData);

	FReply OnDeleteButtonClick(TSharedPtr<FAssetData> ClickedAssetData);

	TSharedRef<SCheckBox> ConstructCheckBox(const TSharedPtr<FAssetData>& AssetDataToDisplay);
	
	TSharedRef<STextBlock> ConstructTextForRowWidget(const FString& TextContent,const FSlateFontInfo& FontInfo);

	TSharedRef<SButton> ConstructButtonForRowWidget(const TSharedPtr<FAssetData>& AssetDataToDisplay);
#pragma endregion

#pragma region TabButtons
	TSharedRef<SButton> ConstructDeleteAllButton();
	TSharedRef<SButton> ConstructSelectAllButton();
	TSharedRef<SButton> ConstructDeselectAllButton();

	FReply OnDeleteAllButtonClicked();
	FReply OnSelectAllButtonClicked();
	FReply OnDeselectAllButtonClicked();

	TSharedRef<STextBlock> ConstructTextForTabButtons(const FString& TextContent);
	
#pragma endregion
	
	FSlateFontInfo GetEmbossedTextFont() const {return FCoreStyle::Get().GetFontStyle(FName("EmbossedTexy"));}

};