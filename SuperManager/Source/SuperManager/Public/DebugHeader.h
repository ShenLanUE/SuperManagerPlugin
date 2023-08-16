#pragma once
#include "Misc/MessageDialog.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Framework/Notifications/NotificationManager.h"

namespace DebugHeader
{
	static void Print(const FString& Message,const FColor& Color)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1,8.f,Color,Message);
		}
	}

	static void PrintLog(const FString& Message)
	{
		UE_LOG(LogTemp, Warning, TEXT("%s"),*Message);
	
	}

	static EAppReturnType::Type ShowMsgDialog(EAppMsgType::Type MsgType,const FText& Message,
		bool bShowMessageWarning = true)
	{
		if (bShowMessageWarning)
		{
			FText MsgTitle = FText::FromString(TEXT("警告"));
			return FMessageDialog::Open(MsgType,Message,&MsgTitle);
		}
		else
		{
			return FMessageDialog::Open(MsgType,Message);
		}
	}

	static void ShowNotifyInfo(const FText& Message)
	{
		FNotificationInfo NotificationInfo(Message); //设置通知信息结构体
		NotificationInfo.bUseLargeFont = true; //使用粗字体
		NotificationInfo.FadeOutDuration = 7.f; //字体淡入持续时间

		FSlateNotificationManager::Get().AddNotification(NotificationInfo);//添加通知信息到编辑器通知管理器
	
	}
}
