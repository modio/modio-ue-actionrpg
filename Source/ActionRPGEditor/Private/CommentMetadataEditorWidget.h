// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "EditorUtilityWidget.h"

#include "CommentMetadataEditorWidget.generated.h"

/**
 *
 */
UCLASS()
class UCommentMetadataEditorWidget : public UEditorUtilityWidget
{
	GENERATED_BODY()

	UFUNCTION(BlueprintCallable, Category = "Comment Metadata Editor")
	static void SetCommentMetadata(UObject* TargetObject, const FString& CommentMetadata);

	UFUNCTION(BlueprintCallable, Category = "Comment Metadata Editor", meta = (ExpandEnumAsExecs = ReturnValue))
	static bool HasCommentMetadata(UObject* TargetObject);

	UFUNCTION(BlueprintCallable, Category = "Comment Metadata Editor")
	static FString GetCommentMetadata(UObject* TargetObject);

	UFUNCTION(BlueprintCallable, Category = "Comment Metadata Editor")
	static void ExportCommentMetadata(const TArray<FAssetData>& AssetData, const FString& OutputPath);

	UFUNCTION(BlueprintCallable, Category = "Comment Metadata Editor")
	static void ImportCommentMetadata(const FString& InputPath);

	UFUNCTION(BlueprintCallable, Category = "Comment Metadata Editor",
			  meta = (ExpandEnumAsExecs = ReturnValue, AutoCreateRefTerm = "DialogTitle,DefaultPath,FileTypes"))
	static bool OpenCommentFilePickerDialog(const FString& DialogTitle, const FString& DefaultPath,
											const FString& FileTypes, bool bSaveFile, TArray<FString>& OutFileNames);
};
