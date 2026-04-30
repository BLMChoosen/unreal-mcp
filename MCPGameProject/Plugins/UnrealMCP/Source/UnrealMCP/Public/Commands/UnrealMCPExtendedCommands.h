#pragma once

#include "CoreMinimal.h"

class FJsonObject;

/**
 * Extended editor automation commands for Unreal 5 production workflows.
 *
 * This command group intentionally uses reflection for optional UE plugins
 * such as PCG, MetaSound, IK Rig, and Control Rig so the MCP plugin can still
 * compile in projects where those plugins are disabled.
 */
class FUnrealMCPExtendedCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	// PCG and landscape-oriented workflows
	static TSharedPtr<FJsonObject> HandleCreatePCGGraph(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleConfigurePCGGraph(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreatePCGVolume(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateBiomeFromPrompt(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateLandscape(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleApplyLandscapeMaterial(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateRoadSpline(const TSharedPtr<FJsonObject>& Params);

	// Animation asset setup
	static TSharedPtr<FJsonObject> HandleCreateAnimBlueprint(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateBlendSpace(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateAnimMontageFromSequence(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddAnimNotify(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateIKRig(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateIKRetargeter(const TSharedPtr<FJsonObject>& Params);

	// MetaSound and dialogue audio
	static TSharedPtr<FJsonObject> HandleCreateMetaSoundSource(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleConfigureMetaSoundGraph(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleImportDialogueBatch(const TSharedPtr<FJsonObject>& Params);

	// Art direction and optimization
	static TSharedPtr<FJsonObject> HandleApplyLightingPreset(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAuditOptimization(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleSetNaniteEnabled(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGenerateHLODs(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCaptureProfileSnapshot(const TSharedPtr<FJsonObject>& Params);

	// C++ / Live Coding and multiplayer Blueprint helpers
	static TSharedPtr<FJsonObject> HandleTriggerLiveCodingCompile(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleSetBlueprintVariableReplication(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateBlueprintRPCEvent(const TSharedPtr<FJsonObject>& Params);
};
