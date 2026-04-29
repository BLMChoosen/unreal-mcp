#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Level-related MCP commands
 * Save / load / create levels and query the active level.
 */
class UNREALMCP_API FUnrealMCPLevelCommands
{
public:
    FUnrealMCPLevelCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    TSharedPtr<FJsonObject> HandleSaveCurrentLevel(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleOpenLevel(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleGetCurrentLevelName(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleCreateNewLevel(const TSharedPtr<FJsonObject>& Params);

    // Streaming sub-level management
    TSharedPtr<FJsonObject> HandleAddSublevel(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleRemoveSublevel(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleGetSublevels(const TSharedPtr<FJsonObject>& Params);
};
