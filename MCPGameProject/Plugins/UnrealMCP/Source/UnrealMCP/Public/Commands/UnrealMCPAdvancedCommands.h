#pragma once

#include "CoreMinimal.h"

class FJsonObject;

class FUnrealMCPAdvancedCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	// AI controllers and pawns
	static TSharedPtr<FJsonObject> HandleCreateAIControllerBlueprint(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddAIPerceptionComponent(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleConfigureAIPerception(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleConfigurePawnAI(const TSharedPtr<FJsonObject>& Params);

	// EQS
	static TSharedPtr<FJsonObject> HandleCreateEnvQuery(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddEnvQueryGenerator(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddEnvQueryTest(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleRunEnvQuery(const TSharedPtr<FJsonObject>& Params);

	// Data authoring helpers
	static TSharedPtr<FJsonObject> HandleGetDataTableSchema(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleValidateDataTableRow(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleImportStringTableEntries(const TSharedPtr<FJsonObject>& Params);

	// Gameplay config
	static TSharedPtr<FJsonObject> HandleAddGameplayTag(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleListGameplayTags(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleApplyGameplayTags(const TSharedPtr<FJsonObject>& Params);

	// Save/load, input, VFX, audio, collision, tests
	static TSharedPtr<FJsonObject> HandleCreateInputAction(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateInputMappingContext(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddInputMapping(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateNiagaraSystem(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateNiagaraEmitter(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleSpawnNiagaraSystem(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateSoundCue(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleSpawnSound(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateCollisionProfile(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateCollisionChannel(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateFunctionalTest(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleRunAutomationTests(const TSharedPtr<FJsonObject>& Params);

	// World queries
	static TSharedPtr<FJsonObject> HandleLineTrace(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleFindActorsInRadius(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleFindActorsByTag(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetActorDistance(const TSharedPtr<FJsonObject>& Params);

	// Asset auditing and generated framework assets
	static TSharedPtr<FJsonObject> HandleGetAssetReferencers(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleValidateAssets(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleValidateBlueprintGraph(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateSaveGameBlueprint(const TSharedPtr<FJsonObject>& Params);

    // Phase 3/5/6 Extended Advanced Tools
    static TSharedPtr<FJsonObject> HandleCreateBtTaskBlueprint(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreateBtServiceBlueprint(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleAddBtRunEqsQuery(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleSetBtNodeBlackboardKey(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreateFullAiSetup(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreateSaveGameWithVariables(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleGenerateSaveLoadFunctions(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleSetupEnhancedInputRuntime(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleBindInputActionToEvent(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreatePlayerControllerWithInput(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleAddNiagaraUserParameter(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleAddNiagaraEmitterToSystem(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreateVfxPreset(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreateSoundAttenuation(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreateSoundMix(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleConfigureSoundCueNodes(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleAttachSoundToActor(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreateEnemyAiArchetype(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreateInteractableArchetype(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleCreatePickupItemArchetype(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandlePreviewOperation(const TSharedPtr<FJsonObject>& Params);
    static TSharedPtr<FJsonObject> HandleGetUndoHistory(const TSharedPtr<FJsonObject>& Params);

};
