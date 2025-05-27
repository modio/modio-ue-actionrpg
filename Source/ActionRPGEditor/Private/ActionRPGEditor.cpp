#include "ActionRPGEditor.h"

DEFINE_LOG_CATEGORY(ActionRPGEditor);

#define LOCTEXT_NAMESPACE "FActionRPGEditor"

void FActionRPGEditor::StartupModule()
{
	UE_LOG(ActionRPGEditor, Display, TEXT("ActionRPGEditor module has been loaded"));
}

void FActionRPGEditor::ShutdownModule()
{
	UE_LOG(ActionRPGEditor, Display, TEXT("ActionRPGEditor module has been unloaded"));
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FActionRPGEditor, ActionRPGEditor)