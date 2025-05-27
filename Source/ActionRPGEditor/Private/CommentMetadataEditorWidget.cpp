
// Fill out your copyright notice in the Description page of Project Settings.
#include "CommentMetadataEditorWidget.h"
#include "Compat/MetadataCompat.h"
#include "DesktopPlatformModule.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Framework/Application/SlateApplication.h"
#include "IDesktopPlatform.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Misc/FileHelper.h"
#include "ScopedTransaction.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/MetaData.h"

void UCommentMetadataEditorWidget::SetCommentMetadata(UObject* TargetObject, const FString& CommentMetadata)
{
	if (TargetObject)
	{
		FScopedTransaction CommentSetTransaction = FScopedTransaction(
			TEXT("Comment Editor Commit"), FText::FromString("Edit Blueprint Class Comment Metadata"), TargetObject);
		TargetObject->Modify();

		auto* BPMetadata = ActionRPGEditorMetadataEngineCompat::GetMetaData(TargetObject->GetOutermost()->GetPackage());
		BPMetadata->SetValue(TargetObject, FName("Comment"), *CommentMetadata);

		FBlueprintEditorUtils::MarkBlueprintAsModified(Cast<UBlueprint>(TargetObject));
	}
}

bool UCommentMetadataEditorWidget::HasCommentMetadata(UObject* TargetObject)
{
	if (TargetObject)
	{
		auto* BPMetadata = ActionRPGEditorMetadataEngineCompat::GetMetaData(TargetObject->GetOutermost()->GetPackage());
		return BPMetadata->HasValue(TargetObject, FName("Comment"));
	}
	return false;
}

FString UCommentMetadataEditorWidget::GetCommentMetadata(UObject* TargetObject)
{
	if (TargetObject)
	{
		auto* BPMetadata = ActionRPGEditorMetadataEngineCompat::GetMetaData(TargetObject->GetOutermost()->GetPackage());
		return BPMetadata->GetValue(TargetObject, FName("Comment"));
	}
	return FString {};
}

void UCommentMetadataEditorWidget::ExportCommentMetadata(const TArray<FAssetData>& AssetData, const FString& OutputPath)
{
	FString OutputString;
	auto Writer = TJsonWriterFactory<TCHAR, TPrettyJsonPrintPolicy<TCHAR>>::Create(&OutputString, 0);
	Writer->WriteObjectStart();
	for (const FAssetData& Asset : AssetData)
	{
		UObject* AssetObject = Asset.GetAsset();
		if (AssetObject && HasCommentMetadata(AssetObject))
		{
			FString CommentMetadata = GetCommentMetadata(AssetObject);
			FString AssetPath = AssetObject->GetPathName();
			Writer->WriteValue(AssetPath, CommentMetadata);
		}
	}
	Writer->WriteObjectEnd();
	Writer->Close();
	FFileHelper::SaveStringToFile(OutputString, *OutputPath);
}

void UCommentMetadataEditorWidget::ImportCommentMetadata(const FString& InputPath)
{
	FString InputString;
	FFileHelper::LoadFileToString(InputString, *InputPath);
	TSharedPtr<FJsonObject> JsonObject;
	TSharedRef<TJsonReader<TCHAR>> Reader = TJsonReaderFactory<TCHAR>::Create(InputString);
	if (FJsonSerializer::Deserialize(Reader, JsonObject))
	{
		for (const auto& Pair : JsonObject->Values)
		{
			UObject* AssetObject = StaticLoadObject(UObject::StaticClass(), nullptr, *Pair.Key);
			if (AssetObject)
			{
				SetCommentMetadata(AssetObject, Pair.Value->AsString());
			}
		}
	}
}

bool UCommentMetadataEditorWidget::OpenCommentFilePickerDialog(const FString& DialogTitle, const FString& DefaultPath,
															   const FString& FileTypes, bool bSaveFile,
															   TArray<FString>& OutFileNames)
{
	void* ParentWindowPtr = FSlateApplication::Get().GetActiveTopLevelWindow()->GetNativeWindow()->GetOSWindowHandle();
	IDesktopPlatform* DesktopPlatform = FDesktopPlatformModule::Get();
	if (DesktopPlatform)
	{
		uint32 SelectionFlag = 0; // A value of 0 represents single file selection while a value of 1 represents
								  // multiple file selection
		if (bSaveFile)
		{
			return DesktopPlatform->SaveFileDialog(ParentWindowPtr, DialogTitle, DefaultPath, FString(""), FileTypes,
												   SelectionFlag, OutFileNames);
		}
		else
		{
			return DesktopPlatform->OpenFileDialog(ParentWindowPtr, DialogTitle, DefaultPath, FString(""), FileTypes,
												   SelectionFlag, OutFileNames);
		}
	}
	return false;
}
