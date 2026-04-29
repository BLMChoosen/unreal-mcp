#include "Commands/UnrealMCPLevelDesignCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"

TSharedPtr<FJsonObject> FUnrealMCPLevelDesignCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("snap_actor_to_grid")) return HandleSnapActorToGrid(Params);
    if (CommandType == TEXT("align_actors")) return HandleAlignActors(Params);
    if (CommandType == TEXT("procedural_scatter")) return HandleProceduralScatter(Params);
    if (CommandType == TEXT("get_actor_bounds")) return HandleGetActorBounds(Params);
    if (CommandType == TEXT("find_overlapping_actors")) return HandleFindOverlappingActors(Params);
    if (CommandType == TEXT("create_volume")) return HandleCreateVolume(Params);
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Unknown level design command"));
}

TSharedPtr<FJsonObject> FUnrealMCPLevelDesignCommands::HandleSnapActorToGrid(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPLevelDesignCommands::HandleAlignActors(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPLevelDesignCommands::HandleProceduralScatter(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPLevelDesignCommands::HandleGetActorBounds(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPLevelDesignCommands::HandleFindOverlappingActors(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPLevelDesignCommands::HandleCreateVolume(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
