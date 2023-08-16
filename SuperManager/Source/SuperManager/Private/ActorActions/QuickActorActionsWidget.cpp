// Fill out your copyright notice in the Description page of Project Settings.


#include "ActorActions/QuickActorActionsWidget.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "DebugHeader.h"

void UQuickActorActionsWidget::SelectAllActorsWithSimilarName()
{
	if (GetEditorActorSubsystem()) return;

	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCounter = 0;

	if (SelectedActors.Num()==0)
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","没有Actor被选中"));
		return;
	}

	if (SelectedActors.Num()>1)
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","你只能选择一个actor进行操作"));
		return;
	}

	FString SelectedActorName = SelectedActors[0]->GetActorLabel();//获取选择的actor的标签
	const FString NameToSearch = SelectedActorName.LeftChop(4);

	TArray<AActor*> AllLeveActors = EditorActorSubsystem->GetAllLevelActors();

	for (AActor* ActorInLevel: AllLeveActors)
	{
		if (!ActorInLevel) continue;

		if (ActorInLevel->GetActorLabel().Contains(NameToSearch,SearchCase))
		{
			//设置actor的选取状态
			EditorActorSubsystem->SetActorSelectionState(ActorInLevel,true);
			SelectionCounter++;
		}
	}

	if (SelectionCounter>0)
	{
		DebugHeader::ShowNotifyInfo(
			FText::Format(NSLOCTEXT("MessageNameSpace","MessageKey","成功选择{0}个actor")
				,FText::FromString(FString::FromInt(SelectionCounter))));
	}
	else
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","没有发现相似的actor"));
	}
}
#pragma region ActorBatchDuplication

void UQuickActorActionsWidget::DuplicateActors()
{
	if (GetEditorActorSubsystem()) return;
	
	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCounter = 0;

	if (SelectedActors.Num()==0)
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","没有Actor被选中"));
		return;
	}

	if (NumberOfDuplicates<=0 || OffsetDist ==0)
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","没有指定复制数量或者偏移量"));
		return;
	}

	for (AActor* SelectedActor: SelectedActors)
	{
		if(!SelectedActor) continue;

		for (int32 i = 0;i<NumberOfDuplicates;i++)
		{
			AActor* DuplicatedActor =
			EditorActorSubsystem->DuplicateActor(SelectedActor,SelectedActor->GetWorld());

			if(!DuplicatedActor) continue;

			const float DuplicationOffsetDist = (i+1)*OffsetDist;

			switch (AxisForDuplication) {
			case E_DuplicationAxis::EDA_XAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(DuplicationOffsetDist,0.f,0.f));
				break;
			case E_DuplicationAxis::EDA_YAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.f,DuplicationOffsetDist,0.f));
				break;
			case E_DuplicationAxis::EDA_ZAxis:
				DuplicatedActor->AddActorWorldOffset(FVector(0.f,0.f,DuplicationOffsetDist));
				break;
			case E_DuplicationAxis::EDA_MAX:
				
				break;
			default:
				break;;
			}

			EditorActorSubsystem->SetActorSelectionState(DuplicatedActor,true);
			SelectionCounter++;
		}
	}
	
	if (SelectionCounter>0)
	{
		DebugHeader::ShowNotifyInfo(FText::Format(NSLOCTEXT("MessageNameSpace","MessageKey"
			,"成功复制{0}个Actor"),FText::FromString(FString::FromInt(SelectionCounter))));
	}
	
}

#pragma endregion

void UQuickActorActionsWidget::RandomizeActorTransform()
{
	const bool ConditionSet =
	RandomActorRotation.bRandomizeRotYaw
	|| RandomActorRotation.bRandomizeRotPitch
	|| RandomActorRotation.bRandomizeRotRoll
	|| RandomActorRotation.bRandomizeOffset
	|| RandomActorRotation.bRandomizeScale;
	
	if (!ConditionSet)
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","没有指定变化条件"));
		return;
	}
	
	if (GetEditorActorSubsystem()) return;
	
	TArray<AActor*> SelectedActors = EditorActorSubsystem->GetSelectedLevelActors();
	uint32 SelectionCounter = 0;

	if (SelectedActors.Num()==0)
	{
		DebugHeader::ShowNotifyInfo(NSLOCTEXT("MessageNameSpace","MessageKey","没有Actor被选中"));
		return;
	}

	for (AActor* SelectedActor : SelectedActors)
	{
		if(!SelectedActor) continue;

		if (RandomActorRotation.bRandomizeRotYaw)
		{
			const float RandomRotYawValue = FMath::RandRange(RandomActorRotation.RotYawMin,RandomActorRotation.RotYawMax);
			SelectedActor->AddActorWorldRotation(FRotator(0.f,RandomRotYawValue,0.f));
			
		}

		if (RandomActorRotation.bRandomizeRotPitch)
		{
			const float RandomRotPitchValue = FMath::RandRange(RandomActorRotation.RotPitchMin,RandomActorRotation.RotPitchMax);
			SelectedActor->AddActorWorldRotation(FRotator(RandomRotPitchValue,0.f,0.f));
			
		}

		if (RandomActorRotation.bRandomizeRotRoll)
		{
			const float RandomRotRollValue = FMath::RandRange(RandomActorRotation.RotRollMin,RandomActorRotation.RotRollMax);
			SelectedActor->AddActorWorldRotation(FRotator(0.f,0.f,RandomRotRollValue));
			
		}

		if (RandomActorRotation.bRandomizeOffset)
		{
			const float RandomOffsetValue = FMath::RandRange(RandomActorRotation.OffsetMin,RandomActorRotation.OffsetMax);
			SelectedActor->AddActorWorldOffset(FVector(RandomOffsetValue,RandomOffsetValue,0.f));
			
		}

		if (RandomActorRotation.bRandomizeScale)
		{
			SelectedActor->SetActorScale3D(FVector(FMath::RandRange(RandomActorRotation.ScaleMin,RandomActorRotation.ScaleMax)));
		}
		SelectionCounter++;
		
	}

	if (SelectionCounter>0)
	{
		DebugHeader::ShowNotifyInfo(FText::Format(NSLOCTEXT("MessageNameSpace","MessageKey"
			,"成功设置{0}个Actor"),FText::FromString(FString::FromInt(SelectionCounter))));
	}
	
}

bool UQuickActorActionsWidget::GetEditorActorSubsystem()
{
	if (!EditorActorSubsystem)
	{
		EditorActorSubsystem = GEditor->GetEditorSubsystem<UEditorActorSubsystem>();
	}
	
	return EditorActorSubsystem == nullptr;
}
