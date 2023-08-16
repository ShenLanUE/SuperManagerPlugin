// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Styling/SlateStyle.h"

class FSuperManagerStyle
{
public:
	static void InitializeIcons();
	static void ShutDown();

private:
	static FName StyleSetName;

	static TSharedRef<FSlateStyleSet> CreateSlateStyleSet(); //创建slate自定义样式集

	static TSharedPtr<FSlateStyleSet> CreatedSlateStyleSet;

public:
	static FName GetStyleSetName(){return StyleSetName;}

	static TSharedRef<FSlateStyleSet> GetCreatedSlateStyleSet(){return CreatedSlateStyleSet.ToSharedRef();}
};

