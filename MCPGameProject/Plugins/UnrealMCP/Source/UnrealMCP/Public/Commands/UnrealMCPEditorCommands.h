#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Editor-related MCP commands
 * Handles viewport control, actor manipulation, and level management
 */
class UNREALMCP_API FUnrealMCPEditorCommands
{
public:
    FUnrealMCPEditorCommands();

    // Handle editor commands
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // Actor manipulation commands
    TSharedPtr<FJsonObject> HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleFindActorsByName(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSpawnActor(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleDeleteActor(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleGetActorProperties(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetActorProperty(const TSharedPtr<FJsonObject>& Params);

    // Blueprint actor spawning
    TSharedPtr<FJsonObject> HandleSpawnBlueprintActor(const TSharedPtr<FJsonObject>& Params);

    // Editor viewport commands
    TSharedPtr<FJsonObject> HandleFocusViewport(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleTakeScreenshot(const TSharedPtr<FJsonObject>& Params);

    // Play In Editor commands
    TSharedPtr<FJsonObject> HandleStartPlayInEditor(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleStopPlayInEditor(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleGetPlayInEditorStatus(const TSharedPtr<FJsonObject>& Params);

    // Batch actor commands
    TSharedPtr<FJsonObject> HandleGetActorsByTag(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetActorsTransformByTag(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTagToActorsByName(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetActorPropertyBatch(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSelectActorsByTag(const TSharedPtr<FJsonObject>& Params);

    // Viewport display mode commands
    TSharedPtr<FJsonObject> HandleSetViewportDisplayMode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleGetViewportDisplayMode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetViewportShowFlags(const TSharedPtr<FJsonObject>& Params);

    // Phase 1: Core Editor Enhancements
    TSharedPtr<FJsonObject> HandleUndoLastAction(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleRedoLastAction(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleBulkSetActorTransform(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleExportLevelToJson(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleDuplicateActor(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetActorMaterial(const TSharedPtr<FJsonObject>& Params);
};