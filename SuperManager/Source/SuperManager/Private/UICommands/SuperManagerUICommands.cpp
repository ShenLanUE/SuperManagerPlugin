// Fill out your copyright notice in the Description page of Project Settings.


#include "UICommands/SuperManagerUICommands.h"

#define LOCTEXT_NAMESPACE "FSuperManagerModule"

void FSuperManagerUICommands::RegisterCommands()
{
	UI_COMMAND(
		LockActorSelection,
		"Lock Actor Selection",
		"在关卡中锁住选择的Actor,使其在关卡中无法被选中",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W,EModifierKey::Alt));

	UI_COMMAND(
		UnLockActorSelection,
		"UnLock Actor Selection",
		"在关卡中解除所有Actor的选择约束",
		EUserInterfaceActionType::Button,
		FInputChord(EKeys::W,EModifierKey::Alt | EModifierKey::Shift));
}

#undef LOCTEXT_NAMESPACE