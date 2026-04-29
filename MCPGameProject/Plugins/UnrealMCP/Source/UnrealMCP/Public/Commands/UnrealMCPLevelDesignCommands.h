#pragma once

#include "CoreMinimal.h"

class FJsonObject;

/**
 * Handler for level design commands.
 * Snap, align, scatter, bounds, overlaps, volumes.
 */
class FUnrealMCPLevelDesignCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	static TSharedPtr<FJsonObject> HandleSnapActorToGrid(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAlignActors(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleProceduralScatter(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetActorBounds(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleFindOverlappingActors(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateVolume(const TSharedPtr<FJsonObject>& Params);
};
