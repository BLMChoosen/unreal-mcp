#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Sequencer-related MCP commands
 * Create level sequences, bind actors, add transform tracks and keyframes.
 */
class UNREALMCP_API FUnrealMCPSequencerCommands
{
public:
    FUnrealMCPSequencerCommands();

    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    TSharedPtr<FJsonObject> HandleCreateLevelSequence(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddActorToSequence(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTransformTrack(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTransformKeyframe(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleGetSequencesInLevel(const TSharedPtr<FJsonObject>& Params);
};
