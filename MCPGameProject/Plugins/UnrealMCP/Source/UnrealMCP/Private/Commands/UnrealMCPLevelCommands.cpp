#include "Commands/UnrealMCPLevelCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "FileHelpers.h"
#include "LevelEditorSubsystem.h"
#include "Editor.h"
#include "Engine/World.h"
#include "Engine/Level.h"
#include "Engine/LevelStreaming.h"
#include "Engine/LevelStreamingDynamic.h"
#include "EditorLevelUtils.h"

FUnrealMCPLevelCommands::FUnrealMCPLevelCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPLevelCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("save_current_level"))
    {
        return HandleSaveCurrentLevel(Params);
    }
    else if (CommandType == TEXT("open_level"))
    {
        return HandleOpenLevel(Params);
    }
    else if (CommandType == TEXT("get_current_level_name"))
    {
        return HandleGetCurrentLevelName(Params);
    }
    else if (CommandType == TEXT("create_new_level"))
    {
        return HandleCreateNewLevel(Params);
    }
    else if (CommandType == TEXT("add_sublevel"))
    {
        return HandleAddSublevel(Params);
    }
    else if (CommandType == TEXT("remove_sublevel"))
    {
        return HandleRemoveSublevel(Params);
    }
    else if (CommandType == TEXT("get_sublevels"))
    {
        return HandleGetSublevels(Params);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown level command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPLevelCommands::HandleSaveCurrentLevel(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active editor world"));
    }

    FEditorFileUtils::SaveCurrentLevel();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("saved"), true);
    Result->SetStringField(TEXT("level_name"), World->GetMapName());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPLevelCommands::HandleOpenLevel(const TSharedPtr<FJsonObject>& Params)
{
    FString LevelPath;
    if (!Params->TryGetStringField(TEXT("level_path"), LevelPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'level_path' parameter"));
    }

    ULevelEditorSubsystem* LevelSubsystem = GEditor ? GEditor->GetEditorSubsystem<ULevelEditorSubsystem>() : nullptr;
    if (!LevelSubsystem)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("LevelEditorSubsystem unavailable"));
    }

    const bool bLoaded = LevelSubsystem->LoadLevel(LevelPath);
    if (!bLoaded)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to load level: %s"), *LevelPath));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("level_path"), LevelPath);
    Result->SetBoolField(TEXT("loaded"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPLevelCommands::HandleGetCurrentLevelName(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active editor world"));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("level_name"), World->GetMapName());
    Result->SetStringField(TEXT("level_path"), World->GetPathName());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPLevelCommands::HandleCreateNewLevel(const TSharedPtr<FJsonObject>& Params)
{
    FString LevelName;
    if (!Params->TryGetStringField(TEXT("level_name"), LevelName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'level_name' parameter"));
    }

    FString Path = TEXT("/Game/Maps");
    Params->TryGetStringField(TEXT("path"), Path);

    ULevelEditorSubsystem* LevelSubsystem = GEditor ? GEditor->GetEditorSubsystem<ULevelEditorSubsystem>() : nullptr;
    if (!LevelSubsystem)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("LevelEditorSubsystem unavailable"));
    }

    const FString FullPath = FString::Printf(TEXT("%s/%s"), *Path, *LevelName);
    const bool bCreated = LevelSubsystem->NewLevel(FullPath);
    if (!bCreated)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create level: %s"), *FullPath));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("level_path"), FullPath);
    Result->SetBoolField(TEXT("created"), true);
    return Result;
}

// ---------------------------------------------------------------------------
// Sub-level (streaming level) management
// ---------------------------------------------------------------------------

TSharedPtr<FJsonObject> FUnrealMCPLevelCommands::HandleAddSublevel(const TSharedPtr<FJsonObject>& Params)
{
    FString LevelPath;
    if (!Params->TryGetStringField(TEXT("level_path"), LevelPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'level_path' parameter"));
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active editor world"));
    }

    UEditorLevelUtils::FAddLevelToWorldParams AddParams(ULevelStreamingDynamic::StaticClass(), FName(*LevelPath));
    ULevelStreaming* AddedStreamingLevel = UEditorLevelUtils::AddLevelToWorld(World, AddParams);
    if (!AddedStreamingLevel)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Failed to add sublevel: %s"), *LevelPath));
    }

    FEditorFileUtils::SaveCurrentLevel();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("level_path"), LevelPath);
    Result->SetBoolField(TEXT("added"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPLevelCommands::HandleRemoveSublevel(const TSharedPtr<FJsonObject>& Params)
{
    FString LevelPath;
    if (!Params->TryGetStringField(TEXT("level_path"), LevelPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'level_path' parameter"));
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active editor world"));
    }

    // Find the streaming level matching this path
    ULevelStreaming* TargetStreaming = nullptr;
    for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
    {
        if (!StreamingLevel) continue;
        if (StreamingLevel->GetWorldAssetPackageName() == LevelPath ||
            StreamingLevel->GetWorldAssetPackageFName().ToString() == LevelPath)
        {
            TargetStreaming = StreamingLevel;
            break;
        }
    }

    if (!TargetStreaming)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Sublevel not found in world: %s"), *LevelPath));
    }

    ULevel* LoadedLevel = TargetStreaming->GetLoadedLevel();
    if (LoadedLevel)
    {
        UEditorLevelUtils::RemoveLevelFromWorld(LoadedLevel);
    }
    else
    {
        // Level is not loaded — remove the streaming entry directly
        World->RemoveStreamingLevel(TargetStreaming);
        World->MarkPackageDirty();
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("level_path"), LevelPath);
    Result->SetBoolField(TEXT("removed"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPLevelCommands::HandleGetSublevels(const TSharedPtr<FJsonObject>& Params)
{
    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active editor world"));
    }

    TArray<TSharedPtr<FJsonValue>> SublevelsArray;
    for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
    {
        if (!StreamingLevel) continue;

        TSharedPtr<FJsonObject> LevelObj = MakeShared<FJsonObject>();
        LevelObj->SetStringField(TEXT("package_name"), StreamingLevel->GetWorldAssetPackageName());
        LevelObj->SetBoolField(TEXT("is_loaded"), StreamingLevel->GetLoadedLevel() != nullptr);
        LevelObj->SetBoolField(TEXT("is_visible"), StreamingLevel->IsLevelVisible());
        SublevelsArray.Add(MakeShared<FJsonValueObject>(LevelObj));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetArrayField(TEXT("sublevels"), SublevelsArray);
    Result->SetNumberField(TEXT("count"), SublevelsArray.Num());
    return Result;
}
