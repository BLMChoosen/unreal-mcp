#include "Commands/UnrealMCPAdvancedCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"

#include "AIController.h"
#include "AudioDevice.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "Editor.h"
#include "EnvironmentQuery/EnvQuery.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvironmentQuery/EnvQueryInstanceBlueprintWrapper.h"
#include "EnvironmentQuery/EnvQueryManager.h"
#include "EnvironmentQuery/EnvQueryOption.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Engine/CollisionProfile.h"
#include "Engine/DataTable.h"
#include "Engine/EngineTypes.h"
#include "Engine/SCS_Node.h"
#include "Sound/SoundBase.h"
#include "Engine/SimpleConstructionScript.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Factories/SoundCueFactoryNew.h"
#include "Factories/BlueprintFactory.h"
#include "FunctionalTest.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/SaveGame.h"
#include "GameplayTagContainer.h"
#include "IAssetTools.h"
#include "InputAction.h"
#include "InputCoreTypes.h"
#include "InputMappingContext.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/TextKey.h"
#include "JsonObjectConverter.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Misc/ConfigCacheIni.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "NiagaraComponent.h"
#include "NiagaraEmitter.h"
#include "NiagaraEmitterFactoryNew.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraSystem.h"
#include "NiagaraSystemFactoryNew.h"
#include "Perception/AIPerceptionComponent.h"
#include "Perception/AISenseConfig_Damage.h"
#include "Perception/AISenseConfig_Hearing.h"
#include "Perception/AISenseConfig_Sight.h"
#include "UObject/Field.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "UObject/UnrealType.h"

namespace
{
	const TCHAR* GameplayTagsSection = TEXT("/Script/GameplayTags.GameplayTagsSettings");
	const TCHAR* GameplayTagListKey = TEXT("GameplayTagList");

	FString ResolveObjectPath(const FString& AssetPath)
	{
		if (AssetPath.Contains(TEXT(".")) || AssetPath.StartsWith(TEXT("/Script/")))
		{
			return AssetPath;
		}

		const FString AssetName = FPackageName::GetShortName(AssetPath);
		return AssetPath + TEXT(".") + AssetName;
	}

	FString GetGeneratedClassPath(const FString& BlueprintPathOrClassPath)
	{
		if (BlueprintPathOrClassPath.EndsWith(TEXT("_C")) || BlueprintPathOrClassPath.StartsWith(TEXT("/Script/")))
		{
			return BlueprintPathOrClassPath;
		}

		if (BlueprintPathOrClassPath.Contains(TEXT(".")))
		{
			return BlueprintPathOrClassPath + TEXT("_C");
		}

		const FString AssetName = FPackageName::GetShortName(BlueprintPathOrClassPath);
		return BlueprintPathOrClassPath + TEXT(".") + AssetName + TEXT("_C");
	}

	template <typename TObjectType>
	TObjectType* LoadAssetObject(const FString& AssetPath)
	{
		return LoadObject<TObjectType>(nullptr, *ResolveObjectPath(AssetPath));
	}

	UBlueprint* LoadBlueprintFromNameOrPath(const FString& BlueprintNameOrPath)
	{
		if (UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *ResolveObjectPath(BlueprintNameOrPath)))
		{
			return Blueprint;
		}

		if (UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintNameOrPath))
		{
			return Blueprint;
		}

		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FAssetData> BlueprintAssets;
		AssetRegistryModule.Get().GetAssetsByClass(UBlueprint::StaticClass()->GetClassPathName(), BlueprintAssets, true);
		for (const FAssetData& AssetData : BlueprintAssets)
		{
			if (AssetData.AssetName.ToString() == BlueprintNameOrPath)
			{
				return Cast<UBlueprint>(AssetData.GetAsset());
			}
		}

		return nullptr;
	}

	UClass* LoadClassFromNameOrPath(const FString& ClassNameOrPath, UClass* RequiredParent = nullptr)
	{
		if (UClass* LoadedClass = LoadClass<UObject>(nullptr, *GetGeneratedClassPath(ClassNameOrPath)))
		{
			if (!RequiredParent || LoadedClass->IsChildOf(RequiredParent))
			{
				return LoadedClass;
			}
		}

		if (UClass* LoadedClass = LoadObject<UClass>(nullptr, *ClassNameOrPath))
		{
			if (!RequiredParent || LoadedClass->IsChildOf(RequiredParent))
			{
				return LoadedClass;
			}
		}

		const FString ShortName = FPackageName::GetShortName(ClassNameOrPath);
		UClass* FoundClass = FindFirstObjectSafe<UClass>(*ShortName);
		if (FoundClass && (!RequiredParent || FoundClass->IsChildOf(RequiredParent)))
		{
			return FoundClass;
		}

		return nullptr;
	}

	TSharedPtr<FJsonObject> CreateBlueprintAsset(const FString& Name, const FString& Path, UClass* ParentClass, UBlueprint*& OutBlueprint)
	{
		if (Name.IsEmpty())
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing or empty 'name' parameter"));
		}
		if (!ParentClass)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Invalid parent class"));
		}

		UBlueprintFactory* Factory = NewObject<UBlueprintFactory>();
		Factory->ParentClass = ParentClass;

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, UBlueprint::StaticClass(), Factory);
		OutBlueprint = Cast<UBlueprint>(NewAsset);
		if (!OutBlueprint)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create Blueprint: %s/%s"), *Path, *Name));
		}

		FKismetEditorUtilities::CompileBlueprint(OutBlueprint);
		OutBlueprint->MarkPackageDirty();

		TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("name"), Name);
		Data->SetStringField(TEXT("path"), Path / Name);
		Data->SetStringField(TEXT("parent_class"), ParentClass->GetName());
		return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
	}

	TSharedPtr<FJsonObject> AddComponentToBlueprint(UBlueprint* Blueprint, UClass* ComponentClass, const FString& ComponentName)
	{
		if (!Blueprint)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint not found"));
		}
		if (!ComponentClass)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Component class not found"));
		}
		if (!Blueprint->SimpleConstructionScript)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint has no SimpleConstructionScript"));
		}

		for (USCS_Node* ExistingNode : Blueprint->SimpleConstructionScript->GetAllNodes())
		{
			if (ExistingNode && ExistingNode->GetVariableName().ToString() == ComponentName)
			{
				TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
				Data->SetStringField(TEXT("blueprint"), Blueprint->GetName());
				Data->SetStringField(TEXT("component_name"), ComponentName);
				Data->SetStringField(TEXT("component_class"), ComponentClass->GetName());
				Data->SetBoolField(TEXT("already_exists"), true);
				return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
			}
		}

		USCS_Node* NewNode = Blueprint->SimpleConstructionScript->CreateNode(ComponentClass, FName(*ComponentName));
		if (!NewNode)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create component node"));
		}

		Blueprint->SimpleConstructionScript->AddNode(NewNode);
		FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
		FKismetEditorUtilities::CompileBlueprint(Blueprint);
		Blueprint->MarkPackageDirty();

		TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("blueprint"), Blueprint->GetName());
		Data->SetStringField(TEXT("component_name"), ComponentName);
		Data->SetStringField(TEXT("component_class"), ComponentClass->GetName());
		Data->SetBoolField(TEXT("already_exists"), false);
		return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
	}

	FVector GetVectorField(const TSharedPtr<FJsonObject>& Params, const FString& FieldName, const FVector& DefaultValue)
	{
		if (!Params.IsValid() || !Params->HasField(FieldName))
		{
			return DefaultValue;
		}

		if (Params->HasTypedField(FieldName, EJson::Object))
		{
			TSharedPtr<FJsonObject> VectorObject = Params->GetObjectField(FieldName);
			double X = DefaultValue.X;
			double Y = DefaultValue.Y;
			double Z = DefaultValue.Z;
			VectorObject->TryGetNumberField(TEXT("X"), X);
			VectorObject->TryGetNumberField(TEXT("Y"), Y);
			VectorObject->TryGetNumberField(TEXT("Z"), Z);
			VectorObject->TryGetNumberField(TEXT("x"), X);
			VectorObject->TryGetNumberField(TEXT("y"), Y);
			VectorObject->TryGetNumberField(TEXT("z"), Z);
			return FVector((float)X, (float)Y, (float)Z);
		}

		return FUnrealMCPCommonUtils::GetVectorFromJson(Params, FieldName);
	}

	AActor* FindActorByName(const FString& ActorName)
	{
		if (!GEditor)
		{
			return nullptr;
		}

		UWorld* World = GEditor->GetEditorWorldContext().World();
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && Actor->GetName() == ActorName)
			{
				return Actor;
			}
		}

		return nullptr;
	}

	TSharedPtr<FJsonObject> VectorToJsonObject(const FVector& Vector)
	{
		TSharedPtr<FJsonObject> Object = MakeShared<FJsonObject>();
		Object->SetNumberField(TEXT("x"), Vector.X);
		Object->SetNumberField(TEXT("y"), Vector.Y);
		Object->SetNumberField(TEXT("z"), Vector.Z);
		return Object;
	}

	ECollisionChannel ParseCollisionChannel(const FString& ChannelName)
	{
		if (ChannelName.Equals(TEXT("Camera"), ESearchCase::IgnoreCase))
		{
			return ECC_Camera;
		}
		if (ChannelName.Equals(TEXT("Pawn"), ESearchCase::IgnoreCase))
		{
			return ECC_Pawn;
		}
		if (ChannelName.Equals(TEXT("WorldStatic"), ESearchCase::IgnoreCase))
		{
			return ECC_WorldStatic;
		}
		if (ChannelName.Equals(TEXT("WorldDynamic"), ESearchCase::IgnoreCase))
		{
			return ECC_WorldDynamic;
		}
		if (ChannelName.Equals(TEXT("PhysicsBody"), ESearchCase::IgnoreCase))
		{
			return ECC_PhysicsBody;
		}
		return ECC_Visibility;
	}

	EAutoPossessAI ParseAutoPossessAI(const FString& Value)
	{
		if (Value.Equals(TEXT("Disabled"), ESearchCase::IgnoreCase))
		{
			return EAutoPossessAI::Disabled;
		}
		if (Value.Equals(TEXT("PlacedInWorld"), ESearchCase::IgnoreCase))
		{
			return EAutoPossessAI::PlacedInWorld;
		}
		if (Value.Equals(TEXT("Spawned"), ESearchCase::IgnoreCase))
		{
			return EAutoPossessAI::Spawned;
		}
		return EAutoPossessAI::PlacedInWorldOrSpawned;
	}

	FString GetPropertyTypeName(FProperty* Property)
	{
		if (!Property)
		{
			return TEXT("Unknown");
		}
		if (FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			return FString::Printf(TEXT("Struct:%s"), *StructProperty->Struct->GetName());
		}
		if (FObjectPropertyBase* ObjectProperty = CastField<FObjectPropertyBase>(Property))
		{
			return FString::Printf(TEXT("Object:%s"), ObjectProperty->PropertyClass ? *ObjectProperty->PropertyClass->GetName() : TEXT("UObject"));
		}
		if (FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			return FString::Printf(TEXT("Array:%s"), *GetPropertyTypeName(ArrayProperty->Inner));
		}
		if (FEnumProperty* EnumProperty = CastField<FEnumProperty>(Property))
		{
			return FString::Printf(TEXT("Enum:%s"), EnumProperty->GetEnum() ? *EnumProperty->GetEnum()->GetName() : TEXT("Unknown"));
		}
		return Property->GetClass()->GetName().Replace(TEXT("Property"), TEXT(""));
	}

	UDataTable* LoadDataTableFromParams(const TSharedPtr<FJsonObject>& Params, FString& OutError)
	{
		FString DataTablePath;
		if (!Params->TryGetStringField(TEXT("data_table"), DataTablePath))
		{
			OutError = TEXT("Missing 'data_table' parameter");
			return nullptr;
		}

		UDataTable* DataTable = LoadAssetObject<UDataTable>(DataTablePath);
		if (!DataTable)
		{
			OutError = FString::Printf(TEXT("Data Table not found: %s"), *DataTablePath);
			return nullptr;
		}
		if (!DataTable->RowStruct)
		{
			OutError = TEXT("Data Table has no row struct");
			return nullptr;
		}
		return DataTable;
	}

	bool GameplayTagEntryContainsTag(const FString& Entry, const FString& Tag)
	{
		const FString Needle = FString::Printf(TEXT("Tag=\"%s\""), *Tag);
		return Entry.Contains(Needle);
	}

	void AddStringArrayField(TSharedPtr<FJsonObject> Object, const FString& FieldName, const TArray<FString>& Values)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		for (const FString& Value : Values)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		Object->SetArrayField(FieldName, JsonValues);
	}

	UObject* CreateObjectAsset(const FString& Name, const FString& Path, UClass* AssetClass)
	{
		if (Name.IsEmpty() || !AssetClass)
		{
			return nullptr;
		}

		const FString PackagePath = Path / Name;
		UPackage* Package = CreatePackage(*PackagePath);
		if (!Package)
		{
			return nullptr;
		}

		UObject* Asset = NewObject<UObject>(Package, AssetClass, FName(*Name), RF_Public | RF_Standalone | RF_Transactional);
		if (!Asset)
		{
			return nullptr;
		}

		FAssetRegistryModule::AssetCreated(Asset);
		Asset->MarkPackageDirty();
		Asset->PostEditChange();
		return Asset;
	}

	TSharedPtr<FJsonObject> CreateAssetResponse(UObject* Asset, const FString& Name, const FString& Path)
	{
		if (!Asset)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create asset: %s/%s"), *Path, *Name));
		}

		TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("name"), Name);
		Data->SetStringField(TEXT("path"), Path / Name);
		Data->SetStringField(TEXT("object_path"), Asset->GetPathName());
		Data->SetStringField(TEXT("class"), Asset->GetClass()->GetPathName());
		return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
	}

	bool SetObjectPropertiesFromJson(UObject* Object, const TSharedPtr<FJsonObject>& Properties, TArray<FString>& OutErrors)
	{
		if (!Object || !Properties.IsValid())
		{
			return false;
		}

		bool bSetAny = false;
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Properties->Values)
		{
			FProperty* Property = Object->GetClass()->FindPropertyByName(FName(*Pair.Key));
			if (!Property)
			{
				OutErrors.Add(FString::Printf(TEXT("Property not found: %s"), *Pair.Key));
				continue;
			}

			void* PropertyValue = Property->ContainerPtrToValuePtr<void>(Object);
			FText FailReason;
			if (!FJsonObjectConverter::JsonValueToUProperty(Pair.Value, Property, PropertyValue, 0, CPF_Transient, false, &FailReason))
			{
				OutErrors.Add(FString::Printf(
					TEXT("Failed to set property '%s': %s"),
					*Pair.Key,
					FailReason.IsEmpty() ? TEXT("invalid value") : *FailReason.ToString()));
				continue;
			}
			bSetAny = true;
		}

		if (bSetAny)
		{
			Object->Modify();
			Object->MarkPackageDirty();
			Object->PostEditChange();
		}

		return bSetAny && OutErrors.Num() == 0;
	}

	USCS_Node* FindSCSNode(UBlueprint* Blueprint, const FString& ComponentName)
	{
		if (!Blueprint || !Blueprint->SimpleConstructionScript)
		{
			return nullptr;
		}
		for (USCS_Node* Node : Blueprint->SimpleConstructionScript->GetAllNodes())
		{
			if (Node && Node->GetVariableName().ToString() == ComponentName)
			{
				return Node;
			}
		}
		return nullptr;
	}

	EInputActionValueType ParseInputActionValueType(const FString& ValueType)
	{
		if (ValueType.Equals(TEXT("Axis1D"), ESearchCase::IgnoreCase) || ValueType.Equals(TEXT("Float"), ESearchCase::IgnoreCase))
		{
			return EInputActionValueType::Axis1D;
		}
		if (ValueType.Equals(TEXT("Axis2D"), ESearchCase::IgnoreCase) || ValueType.Equals(TEXT("Vector2D"), ESearchCase::IgnoreCase))
		{
			return EInputActionValueType::Axis2D;
		}
		if (ValueType.Equals(TEXT("Axis3D"), ESearchCase::IgnoreCase) || ValueType.Equals(TEXT("Vector"), ESearchCase::IgnoreCase))
		{
			return EInputActionValueType::Axis3D;
		}
		return EInputActionValueType::Boolean;
	}

	EEnvQueryRunMode::Type ParseEnvQueryRunMode(const FString& RunMode)
	{
		if (RunMode.Equals(TEXT("RandomBest5Pct"), ESearchCase::IgnoreCase))
		{
			return EEnvQueryRunMode::RandomBest5Pct;
		}
		if (RunMode.Equals(TEXT("RandomBest25Pct"), ESearchCase::IgnoreCase))
		{
			return EEnvQueryRunMode::RandomBest25Pct;
		}
		if (RunMode.Equals(TEXT("AllMatching"), ESearchCase::IgnoreCase))
		{
			return EEnvQueryRunMode::AllMatching;
		}
		return EEnvQueryRunMode::SingleResult;
	}

	UWorld* GetEditorWorld()
	{
		return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	}
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandType == TEXT("create_ai_controller_blueprint"))
	{
		return HandleCreateAIControllerBlueprint(Params);
	}
	if (CommandType == TEXT("add_ai_perception_component"))
	{
		return HandleAddAIPerceptionComponent(Params);
	}
	if (CommandType == TEXT("configure_ai_perception"))
	{
		return HandleConfigureAIPerception(Params);
	}
	if (CommandType == TEXT("configure_pawn_ai"))
	{
		return HandleConfigurePawnAI(Params);
	}
	if (CommandType == TEXT("create_env_query"))
	{
		return HandleCreateEnvQuery(Params);
	}
	if (CommandType == TEXT("add_env_query_generator"))
	{
		return HandleAddEnvQueryGenerator(Params);
	}
	if (CommandType == TEXT("add_env_query_test"))
	{
		return HandleAddEnvQueryTest(Params);
	}
	if (CommandType == TEXT("run_env_query"))
	{
		return HandleRunEnvQuery(Params);
	}
	if (CommandType == TEXT("get_data_table_schema"))
	{
		return HandleGetDataTableSchema(Params);
	}
	if (CommandType == TEXT("validate_data_table_row"))
	{
		return HandleValidateDataTableRow(Params);
	}
	if (CommandType == TEXT("import_string_table_entries"))
	{
		return HandleImportStringTableEntries(Params);
	}
	if (CommandType == TEXT("add_gameplay_tag"))
	{
		return HandleAddGameplayTag(Params);
	}
	if (CommandType == TEXT("list_gameplay_tags"))
	{
		return HandleListGameplayTags(Params);
	}
	if (CommandType == TEXT("apply_gameplay_tags"))
	{
		return HandleApplyGameplayTags(Params);
	}
	if (CommandType == TEXT("create_input_action"))
	{
		return HandleCreateInputAction(Params);
	}
	if (CommandType == TEXT("create_input_mapping_context"))
	{
		return HandleCreateInputMappingContext(Params);
	}
	if (CommandType == TEXT("add_input_mapping"))
	{
		return HandleAddInputMapping(Params);
	}
	if (CommandType == TEXT("create_niagara_system"))
	{
		return HandleCreateNiagaraSystem(Params);
	}
	if (CommandType == TEXT("create_niagara_emitter"))
	{
		return HandleCreateNiagaraEmitter(Params);
	}
	if (CommandType == TEXT("spawn_niagara_system"))
	{
		return HandleSpawnNiagaraSystem(Params);
	}
	if (CommandType == TEXT("create_sound_cue"))
	{
		return HandleCreateSoundCue(Params);
	}
	if (CommandType == TEXT("spawn_sound"))
	{
		return HandleSpawnSound(Params);
	}
	if (CommandType == TEXT("create_collision_profile"))
	{
		return HandleCreateCollisionProfile(Params);
	}
	if (CommandType == TEXT("create_collision_channel"))
	{
		return HandleCreateCollisionChannel(Params);
	}
	if (CommandType == TEXT("create_functional_test"))
	{
		return HandleCreateFunctionalTest(Params);
	}
	if (CommandType == TEXT("run_automation_tests"))
	{
		return HandleRunAutomationTests(Params);
	}
	if (CommandType == TEXT("line_trace"))
	{
		return HandleLineTrace(Params);
	}
	if (CommandType == TEXT("find_actors_in_radius"))
	{
		return HandleFindActorsInRadius(Params);
	}
	if (CommandType == TEXT("find_actors_by_tag"))
	{
		return HandleFindActorsByTag(Params);
	}
	if (CommandType == TEXT("get_actor_distance"))
	{
		return HandleGetActorDistance(Params);
	}
	if (CommandType == TEXT("get_asset_referencers"))
	{
		return HandleGetAssetReferencers(Params);
	}
	if (CommandType == TEXT("validate_assets"))
	{
		return HandleValidateAssets(Params);
	}
	if (CommandType == TEXT("validate_blueprint_graph"))
	{
		return HandleValidateBlueprintGraph(Params);
	}
	if (CommandType == TEXT("create_save_game_blueprint"))
	{
		return HandleCreateSaveGameBlueprint(Params);
	}

	
    if (CommandType == TEXT("create_bt_task_blueprint")) return HandleCreateBtTaskBlueprint(Params);
    if (CommandType == TEXT("create_bt_service_blueprint")) return HandleCreateBtServiceBlueprint(Params);
    if (CommandType == TEXT("add_bt_run_eqs_query")) return HandleAddBtRunEqsQuery(Params);
    if (CommandType == TEXT("set_bt_node_blackboard_key")) return HandleSetBtNodeBlackboardKey(Params);
    if (CommandType == TEXT("create_full_ai_setup")) return HandleCreateFullAiSetup(Params);
    if (CommandType == TEXT("create_save_game_with_variables")) return HandleCreateSaveGameWithVariables(Params);
    if (CommandType == TEXT("generate_save_load_functions")) return HandleGenerateSaveLoadFunctions(Params);
    if (CommandType == TEXT("setup_enhanced_input_runtime")) return HandleSetupEnhancedInputRuntime(Params);
    if (CommandType == TEXT("bind_input_action_to_event")) return HandleBindInputActionToEvent(Params);
    if (CommandType == TEXT("create_player_controller_with_input")) return HandleCreatePlayerControllerWithInput(Params);
    if (CommandType == TEXT("add_niagara_user_parameter")) return HandleAddNiagaraUserParameter(Params);
    if (CommandType == TEXT("add_niagara_emitter_to_system")) return HandleAddNiagaraEmitterToSystem(Params);
    if (CommandType == TEXT("create_vfx_preset")) return HandleCreateVfxPreset(Params);
    if (CommandType == TEXT("create_sound_attenuation")) return HandleCreateSoundAttenuation(Params);
    if (CommandType == TEXT("create_sound_mix")) return HandleCreateSoundMix(Params);
    if (CommandType == TEXT("configure_sound_cue_nodes")) return HandleConfigureSoundCueNodes(Params);
    if (CommandType == TEXT("attach_sound_to_actor")) return HandleAttachSoundToActor(Params);
    if (CommandType == TEXT("create_enemy_ai_archetype")) return HandleCreateEnemyAiArchetype(Params);
    if (CommandType == TEXT("create_interactable_archetype")) return HandleCreateInteractableArchetype(Params);
    if (CommandType == TEXT("create_pickup_item_archetype")) return HandleCreatePickupItemArchetype(Params);
    if (CommandType == TEXT("preview_operation")) return HandlePreviewOperation(Params);
    if (CommandType == TEXT("get_undo_history")) return HandleGetUndoHistory(Params);

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown advanced command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateAIControllerBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/AI");
	Params->TryGetStringField(TEXT("path"), Path);

	bool bAddPerception = true;
	Params->TryGetBoolField(TEXT("add_perception"), bAddPerception);

	UBlueprint* Blueprint = nullptr;
	TSharedPtr<FJsonObject> Result = CreateBlueprintAsset(Name, Path, AAIController::StaticClass(), Blueprint);
	if (Result->GetBoolField(TEXT("success")) && bAddPerception && Blueprint)
	{
		AddComponentToBlueprint(Blueprint, UAIPerceptionComponent::StaticClass(), TEXT("AIPerception"));
	}
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAddAIPerceptionComponent(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString ComponentName = TEXT("AIPerception");
	Params->TryGetStringField(TEXT("component_name"), ComponentName);

	UBlueprint* Blueprint = LoadBlueprintFromNameOrPath(BlueprintName);
	return AddComponentToBlueprint(Blueprint, UAIPerceptionComponent::StaticClass(), ComponentName);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleConfigureAIPerception(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}

	FString ComponentName = TEXT("AIPerception");
	Params->TryGetStringField(TEXT("component_name"), ComponentName);

	UBlueprint* Blueprint = LoadBlueprintFromNameOrPath(BlueprintName);
	USCS_Node* Node = FindSCSNode(Blueprint, ComponentName);
	UAIPerceptionComponent* Perception = Node ? Cast<UAIPerceptionComponent>(Node->ComponentTemplate) : nullptr;
	if (!Perception)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("AIPerception component not found: %s"), *ComponentName));
	}

	Perception->Modify();

	TArray<FString> ConfiguredSenses;
	if (Params->HasTypedField(TEXT("sight"), EJson::Object))
	{
		TSharedPtr<FJsonObject> SightParams = Params->GetObjectField(TEXT("sight"));
		UAISenseConfig_Sight* Sight = NewObject<UAISenseConfig_Sight>(Perception, UAISenseConfig_Sight::StaticClass(), NAME_None, RF_Transactional);
		double NumberValue = 0.0;
		if (SightParams->TryGetNumberField(TEXT("sight_radius"), NumberValue))
		{
			Sight->SightRadius = (float)NumberValue;
		}
		if (SightParams->TryGetNumberField(TEXT("lose_sight_radius"), NumberValue))
		{
			Sight->LoseSightRadius = (float)NumberValue;
		}
		if (SightParams->TryGetNumberField(TEXT("peripheral_vision_angle"), NumberValue))
		{
			Sight->PeripheralVisionAngleDegrees = (float)NumberValue;
		}
		Sight->DetectionByAffiliation.bDetectEnemies = true;
		Sight->DetectionByAffiliation.bDetectFriendlies = true;
		Sight->DetectionByAffiliation.bDetectNeutrals = true;
		Perception->ConfigureSense(*Sight);
		Perception->SetDominantSense(Sight->GetSenseImplementation());
		ConfiguredSenses.Add(TEXT("Sight"));
	}

	if (Params->HasTypedField(TEXT("hearing"), EJson::Object))
	{
		TSharedPtr<FJsonObject> HearingParams = Params->GetObjectField(TEXT("hearing"));
		UAISenseConfig_Hearing* Hearing = NewObject<UAISenseConfig_Hearing>(Perception, UAISenseConfig_Hearing::StaticClass(), NAME_None, RF_Transactional);
		double NumberValue = 0.0;
		if (HearingParams->TryGetNumberField(TEXT("hearing_range"), NumberValue))
		{
			Hearing->HearingRange = (float)NumberValue;
		}
		Hearing->DetectionByAffiliation.bDetectEnemies = true;
		Hearing->DetectionByAffiliation.bDetectFriendlies = true;
		Hearing->DetectionByAffiliation.bDetectNeutrals = true;
		Perception->ConfigureSense(*Hearing);
		if (!Perception->GetDominantSense())
		{
			Perception->SetDominantSense(Hearing->GetSenseImplementation());
		}
		ConfiguredSenses.Add(TEXT("Hearing"));
	}

	if (Params->HasTypedField(TEXT("damage"), EJson::Object))
	{
		UAISenseConfig_Damage* Damage = NewObject<UAISenseConfig_Damage>(Perception, UAISenseConfig_Damage::StaticClass(), NAME_None, RF_Transactional);
		Perception->ConfigureSense(*Damage);
		if (!Perception->GetDominantSense())
		{
			Perception->SetDominantSense(Damage->GetSenseImplementation());
		}
		ConfiguredSenses.Add(TEXT("Damage"));
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	Blueprint->MarkPackageDirty();

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("blueprint"), Blueprint->GetName());
	Data->SetStringField(TEXT("component_name"), ComponentName);
	AddStringArrayField(Data, TEXT("configured_senses"), ConfiguredSenses);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleConfigurePawnAI(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintName;
	FString ControllerClassPath;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("ai_controller_class"), ControllerClassPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'ai_controller_class' parameter"));
	}

	UBlueprint* Blueprint = LoadBlueprintFromNameOrPath(BlueprintName);
	if (!Blueprint || !Blueprint->GeneratedClass)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Pawn Blueprint not found or not compiled: %s"), *BlueprintName));
	}

	APawn* PawnCDO = Cast<APawn>(Blueprint->GeneratedClass->GetDefaultObject());
	if (!PawnCDO)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint is not a Pawn: %s"), *BlueprintName));
	}

	UClass* ControllerClass = LoadClassFromNameOrPath(ControllerClassPath, AAIController::StaticClass());
	if (!ControllerClass)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("AI Controller class not found: %s"), *ControllerClassPath));
	}

	FString AutoPossess = TEXT("PlacedInWorldOrSpawned");
	Params->TryGetStringField(TEXT("auto_possess_ai"), AutoPossess);

	PawnCDO->Modify();
	PawnCDO->AIControllerClass = ControllerClass;
	PawnCDO->AutoPossessAI = ParseAutoPossessAI(AutoPossess);
	Blueprint->MarkPackageDirty();
	FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("blueprint"), Blueprint->GetName());
	Data->SetStringField(TEXT("ai_controller_class"), ControllerClass->GetPathName());
	Data->SetStringField(TEXT("auto_possess_ai"), AutoPossess);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateEnvQuery(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/AI/EQS");
	Params->TryGetStringField(TEXT("path"), Path);

	UObject* Asset = CreateObjectAsset(Name, Path, UEnvQuery::StaticClass());
	return CreateAssetResponse(Asset, Name, Path);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAddEnvQueryGenerator(const TSharedPtr<FJsonObject>& Params)
{
	FString QueryPath;
	if (!Params->TryGetStringField(TEXT("env_query"), QueryPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'env_query' parameter"));
	}

	FString GeneratorClassPath;
	if (!Params->TryGetStringField(TEXT("generator_class"), GeneratorClassPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'generator_class' parameter"));
	}

	UEnvQuery* Query = LoadAssetObject<UEnvQuery>(QueryPath);
	if (!Query)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("EnvQuery not found: %s"), *QueryPath));
	}

	UClass* GeneratorClass = LoadClassFromNameOrPath(GeneratorClassPath, UEnvQueryGenerator::StaticClass());
	if (!GeneratorClass)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Generator class not found: %s"), *GeneratorClassPath));
	}

	UEnvQueryOption* Option = NewObject<UEnvQueryOption>(Query, UEnvQueryOption::StaticClass(), NAME_None, RF_Transactional);
	Option->Generator = NewObject<UEnvQueryGenerator>(Option, GeneratorClass, NAME_None, RF_Transactional);

	if (Params->HasTypedField(TEXT("properties"), EJson::Object))
	{
		TArray<FString> Errors;
		SetObjectPropertiesFromJson(Option->Generator, Params->GetObjectField(TEXT("properties")), Errors);
	}

	Query->Modify();
	Query->GetOptionsMutable().Add(Option);
	Query->MarkPackageDirty();
	Query->PostEditChange();

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("env_query"), Query->GetPathName());
	Data->SetNumberField(TEXT("option_index"), Query->GetOptionsMutable().Num() - 1);
	Data->SetStringField(TEXT("generator_class"), GeneratorClass->GetPathName());
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAddEnvQueryTest(const TSharedPtr<FJsonObject>& Params)
{
	FString QueryPath;
	if (!Params->TryGetStringField(TEXT("env_query"), QueryPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'env_query' parameter"));
	}

	FString TestClassPath;
	if (!Params->TryGetStringField(TEXT("test_class"), TestClassPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'test_class' parameter"));
	}

	UEnvQuery* Query = LoadAssetObject<UEnvQuery>(QueryPath);
	if (!Query)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("EnvQuery not found: %s"), *QueryPath));
	}

	int32 OptionIndex = 0;
	if (Params->HasField(TEXT("option_index")))
	{
		OptionIndex = Params->GetIntegerField(TEXT("option_index"));
	}
	if (!Query->GetOptionsMutable().IsValidIndex(OptionIndex))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Invalid option_index"));
	}

	UClass* TestClass = LoadClassFromNameOrPath(TestClassPath, UEnvQueryTest::StaticClass());
	if (!TestClass)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Test class not found: %s"), *TestClassPath));
	}

	UEnvQueryOption* Option = Query->GetOptionsMutable()[OptionIndex];
	UEnvQueryTest* Test = NewObject<UEnvQueryTest>(Option, TestClass, NAME_None, RF_Transactional);
	if (Params->HasTypedField(TEXT("properties"), EJson::Object))
	{
		TArray<FString> Errors;
		SetObjectPropertiesFromJson(Test, Params->GetObjectField(TEXT("properties")), Errors);
	}

	Query->Modify();
	Option->Tests.Add(Test);
	Query->MarkPackageDirty();
	Query->PostEditChange();

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("env_query"), Query->GetPathName());
	Data->SetNumberField(TEXT("option_index"), OptionIndex);
	Data->SetNumberField(TEXT("test_index"), Option->Tests.Num() - 1);
	Data->SetStringField(TEXT("test_class"), TestClass->GetPathName());
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleRunEnvQuery(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor world is unavailable"));
	}

	FString QueryPath;
	if (!Params->TryGetStringField(TEXT("env_query"), QueryPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'env_query' parameter"));
	}

	UEnvQuery* Query = LoadAssetObject<UEnvQuery>(QueryPath);
	if (!Query)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("EnvQuery not found: %s"), *QueryPath));
	}

	UObject* Querier = World;
	FString QuerierName;
	if (Params->TryGetStringField(TEXT("querier"), QuerierName))
	{
		if (AActor* Actor = FindActorByName(QuerierName))
		{
			Querier = Actor;
		}
	}

	FString RunModeString = TEXT("SingleResult");
	Params->TryGetStringField(TEXT("run_mode"), RunModeString);
	UEnvQueryInstanceBlueprintWrapper* Wrapper = UEnvQueryManager::RunEQSQuery(
		World,
		Query,
		Querier,
		ParseEnvQueryRunMode(RunModeString),
		nullptr);

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("env_query"), Query->GetPathName());
	Data->SetStringField(TEXT("run_mode"), RunModeString);
	Data->SetBoolField(TEXT("started"), Wrapper != nullptr);
	if (Wrapper)
	{
		Data->SetStringField(TEXT("wrapper"), Wrapper->GetName());
	}
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleGetDataTableSchema(const TSharedPtr<FJsonObject>& Params)
{
	FString Error;
	UDataTable* DataTable = LoadDataTableFromParams(Params, Error);
	if (!DataTable)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
	}

	TArray<TSharedPtr<FJsonValue>> Fields;
	for (TFieldIterator<FProperty> PropIt(DataTable->RowStruct); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		TSharedPtr<FJsonObject> FieldObject = MakeShared<FJsonObject>();
		FieldObject->SetStringField(TEXT("name"), Property->GetName());
		FieldObject->SetStringField(TEXT("type"), GetPropertyTypeName(Property));
		FieldObject->SetStringField(TEXT("cpp_type"), Property->GetCPPType());
		FieldObject->SetBoolField(TEXT("is_array"), Property->IsA<FArrayProperty>());
		Fields.Add(MakeShared<FJsonValueObject>(FieldObject));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("data_table"), DataTable->GetPathName());
	Data->SetStringField(TEXT("row_struct"), DataTable->RowStruct->GetPathName());
	Data->SetArrayField(TEXT("fields"), Fields);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleValidateDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	FString Error;
	UDataTable* DataTable = LoadDataTableFromParams(Params, Error);
	if (!DataTable)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
	}

	TSharedPtr<FJsonObject> RowObject;
	if (!Params->HasTypedField(TEXT("row_data"), EJson::Object))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("'row_data' must be a JSON object"));
	}
	RowObject = Params->GetObjectField(TEXT("row_data"));

	TArray<FString> UnknownFields;
	TArray<FString> MissingFields;
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : RowObject->Values)
	{
		if (!DataTable->RowStruct->FindPropertyByName(FName(*Pair.Key)))
		{
			UnknownFields.Add(Pair.Key);
		}
	}
	for (TFieldIterator<FProperty> PropIt(DataTable->RowStruct); PropIt; ++PropIt)
	{
		FProperty* Property = *PropIt;
		if (Property && !Property->HasAnyPropertyFlags(CPF_Transient) && !RowObject->HasField(Property->GetName()))
		{
			MissingFields.Add(Property->GetName());
		}
	}

	TArray<uint8> RowMemory;
	RowMemory.SetNumZeroed(DataTable->RowStruct->GetStructureSize());
	DataTable->RowStruct->InitializeStruct(RowMemory.GetData());
	bool bConverts = false;
	FString ConversionError;
	{
		FText FailReason;
		bConverts = FJsonObjectConverter::JsonObjectToUStruct(
			RowObject.ToSharedRef(),
			DataTable->RowStruct,
			RowMemory.GetData(),
			0,
			CPF_Transient,
			false,
			&FailReason);
		if (!bConverts)
		{
			ConversionError = FailReason.IsEmpty() ? TEXT("Failed to convert row JSON") : FailReason.ToString();
		}
	}
	DataTable->RowStruct->DestroyStruct(RowMemory.GetData());

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetBoolField(TEXT("valid"), bConverts && UnknownFields.Num() == 0);
	Data->SetBoolField(TEXT("converts_to_struct"), bConverts);
	if (!ConversionError.IsEmpty())
	{
		Data->SetStringField(TEXT("conversion_error"), ConversionError);
	}
	AddStringArrayField(Data, TEXT("unknown_fields"), UnknownFields);
	AddStringArrayField(Data, TEXT("missing_fields"), MissingFields);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleImportStringTableEntries(const TSharedPtr<FJsonObject>& Params)
{
	FString StringTablePath;
	if (!Params->TryGetStringField(TEXT("string_table"), StringTablePath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'string_table' parameter"));
	}

	UStringTable* StringTable = LoadAssetObject<UStringTable>(StringTablePath);
	if (!StringTable)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("String Table not found: %s"), *StringTablePath));
	}
	if (!Params->HasTypedField(TEXT("entries"), EJson::Object))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("'entries' must be an object of key/value strings"));
	}

	TSharedPtr<FJsonObject> Entries = Params->GetObjectField(TEXT("entries"));
	int32 ImportedCount = 0;
	for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Entries->Values)
	{
		if (!Pair.Value.IsValid())
		{
			continue;
		}
		StringTable->GetMutableStringTable()->SetSourceString(FTextKey(Pair.Key), Pair.Value->AsString());
		++ImportedCount;
	}

	StringTable->Modify();
	StringTable->MarkPackageDirty();
	StringTable->PostEditChange();

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("string_table"), StringTable->GetPathName());
	Data->SetNumberField(TEXT("imported_count"), ImportedCount);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAddGameplayTag(const TSharedPtr<FJsonObject>& Params)
{
	FString Tag;
	if (!Params->TryGetStringField(TEXT("tag"), Tag) || Tag.IsEmpty())
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing or empty 'tag' parameter"));
	}

	FString Comment;
	Params->TryGetStringField(TEXT("comment"), Comment);

	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultGameplayTags.ini");
	TArray<FString> ExistingEntries;
	GConfig->GetArray(GameplayTagsSection, GameplayTagListKey, ExistingEntries, ConfigPath);

	bool bAlreadyExists = false;
	for (const FString& Entry : ExistingEntries)
	{
		if (GameplayTagEntryContainsTag(Entry, Tag))
		{
			bAlreadyExists = true;
			break;
		}
	}

	if (!bAlreadyExists)
	{
		const FString EscapedComment = Comment.ReplaceCharWithEscapedChar();
		const FString Entry = FString::Printf(TEXT("(Tag=\"%s\",DevComment=\"%s\")"), *Tag, *EscapedComment);
		ExistingEntries.Add(Entry);
		GConfig->SetArray(GameplayTagsSection, GameplayTagListKey, ExistingEntries, ConfigPath);
		GConfig->Flush(false, ConfigPath);
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("tag"), Tag);
	Data->SetStringField(TEXT("config_path"), ConfigPath);
	Data->SetBoolField(TEXT("already_exists"), bAlreadyExists);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleListGameplayTags(const TSharedPtr<FJsonObject>& Params)
{
	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultGameplayTags.ini");
	TArray<FString> ExistingEntries;
	GConfig->GetArray(GameplayTagsSection, GameplayTagListKey, ExistingEntries, ConfigPath);

	TArray<TSharedPtr<FJsonValue>> Tags;
	for (const FString& Entry : ExistingEntries)
	{
		FString Left;
		FString Rest;
		if (!Entry.Split(TEXT("Tag=\""), &Left, &Rest))
		{
			continue;
		}
		FString Tag;
		if (Rest.Split(TEXT("\""), &Tag, &Rest))
		{
			Tags.Add(MakeShared<FJsonValueString>(Tag));
		}
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("config_path"), ConfigPath);
	Data->SetArrayField(TEXT("tags"), Tags);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleApplyGameplayTags(const TSharedPtr<FJsonObject>& Params)
{
	TArray<FString> TagsToApply;
	if (!Params->HasTypedField(TEXT("tags"), EJson::Array))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("'tags' must be an array of tag strings"));
	}
	for (const TSharedPtr<FJsonValue>& Value : Params->GetArrayField(TEXT("tags")))
	{
		TagsToApply.Add(Value->AsString());
	}

	FString ActorName;
	if (Params->TryGetStringField(TEXT("actor"), ActorName))
	{
		AActor* Actor = FindActorByName(ActorName);
		if (!Actor)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
		}
		Actor->Modify();
		for (const FString& Tag : TagsToApply)
		{
			Actor->Tags.AddUnique(FName(*Tag));
		}
		Actor->MarkPackageDirty();

		TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("actor"), Actor->GetName());
		AddStringArrayField(Data, TEXT("tags"), TagsToApply);
		return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
	}

	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor' or 'asset_path' parameter"));
	}

	UObject* Asset = LoadObject<UObject>(nullptr, *ResolveObjectPath(AssetPath));
	if (!Asset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
	}

	FProperty* TagsProperty = Asset->GetClass()->FindPropertyByName(TEXT("GameplayTags"));
	if (!TagsProperty)
	{
		TagsProperty = Asset->GetClass()->FindPropertyByName(TEXT("Tags"));
	}
	FStructProperty* ContainerProperty = CastField<FStructProperty>(TagsProperty);
	if (!ContainerProperty || ContainerProperty->Struct != FGameplayTagContainer::StaticStruct())
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Asset does not expose a GameplayTags/Tags FGameplayTagContainer property"));
	}

	Asset->Modify();
	FGameplayTagContainer* Container = ContainerProperty->ContainerPtrToValuePtr<FGameplayTagContainer>(Asset);
	for (const FString& Tag : TagsToApply)
	{
		Container->AddTag(FGameplayTag::RequestGameplayTag(FName(*Tag), false));
	}
	Asset->MarkPackageDirty();
	Asset->PostEditChange();

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("asset"), Asset->GetPathName());
	AddStringArrayField(Data, TEXT("tags"), TagsToApply);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateInputAction(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Input");
	Params->TryGetStringField(TEXT("path"), Path);

	UInputAction* Action = Cast<UInputAction>(CreateObjectAsset(Name, Path, UInputAction::StaticClass()));
	if (Action)
	{
		FString ValueType = TEXT("Boolean");
		Params->TryGetStringField(TEXT("value_type"), ValueType);
		Action->ValueType = ParseInputActionValueType(ValueType);
		Action->MarkPackageDirty();
	}
	return CreateAssetResponse(Action, Name, Path);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateInputMappingContext(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Input");
	Params->TryGetStringField(TEXT("path"), Path);

	UObject* Asset = CreateObjectAsset(Name, Path, UInputMappingContext::StaticClass());
	return CreateAssetResponse(Asset, Name, Path);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAddInputMapping(const TSharedPtr<FJsonObject>& Params)
{
	FString MappingContextPath;
	FString InputActionPath;
	FString KeyName;
	if (!Params->TryGetStringField(TEXT("mapping_context"), MappingContextPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'mapping_context' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("input_action"), InputActionPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'input_action' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("key"), KeyName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'key' parameter"));
	}

	UInputMappingContext* MappingContext = LoadAssetObject<UInputMappingContext>(MappingContextPath);
	UInputAction* InputAction = LoadAssetObject<UInputAction>(InputActionPath);
	if (!MappingContext || !InputAction)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Input Mapping Context or Input Action not found"));
	}

	FKey Key(*KeyName);
	MappingContext->Modify();
	FEnhancedActionKeyMapping& Mapping = MappingContext->MapKey(InputAction, Key);
	if (Params->HasTypedField(TEXT("properties"), EJson::Object))
	{
		TArray<FString> Errors;
		SetObjectPropertiesFromJson(MappingContext, Params->GetObjectField(TEXT("properties")), Errors);
	}
	MappingContext->MarkPackageDirty();
	MappingContext->PostEditChange();

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("mapping_context"), MappingContext->GetPathName());
	Data->SetStringField(TEXT("input_action"), InputAction->GetPathName());
	Data->SetStringField(TEXT("key"), Mapping.Key.ToString());
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateNiagaraSystem(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/VFX");
	Params->TryGetStringField(TEXT("path"), Path);

	UNiagaraSystemFactoryNew* Factory = NewObject<UNiagaraSystemFactoryNew>();
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, UNiagaraSystem::StaticClass(), Factory);
	return CreateAssetResponse(NewAsset, Name, Path);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateNiagaraEmitter(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/VFX");
	Params->TryGetStringField(TEXT("path"), Path);

	UNiagaraEmitterFactoryNew* Factory = NewObject<UNiagaraEmitterFactoryNew>();
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, UNiagaraEmitter::StaticClass(), Factory);
	return CreateAssetResponse(NewAsset, Name, Path);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleSpawnNiagaraSystem(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor world is unavailable"));
	}

	FString SystemPath;
	if (!Params->TryGetStringField(TEXT("system"), SystemPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'system' parameter"));
	}
	UNiagaraSystem* System = LoadAssetObject<UNiagaraSystem>(SystemPath);
	if (!System)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Niagara system not found: %s"), *SystemPath));
	}

	const FVector Location = GetVectorField(Params, TEXT("location"), FVector::ZeroVector);
	UNiagaraComponent* Component = UNiagaraFunctionLibrary::SpawnSystemAtLocation(World, System, Location);
	if (Component && Params->HasTypedField(TEXT("parameters"), EJson::Object))
	{
		TSharedPtr<FJsonObject> Parameters = Params->GetObjectField(TEXT("parameters"));
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : Parameters->Values)
		{
			if (Pair.Value->Type == EJson::Number)
			{
				Component->SetFloatParameter(FName(*Pair.Key), (float)Pair.Value->AsNumber());
			}
			else if (Pair.Value->Type == EJson::Boolean)
			{
				Component->SetBoolParameter(FName(*Pair.Key), Pair.Value->AsBool());
			}
		}
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("system"), System->GetPathName());
	Data->SetBoolField(TEXT("spawned"), Component != nullptr);
	if (Component)
	{
		Data->SetStringField(TEXT("component"), Component->GetName());
	}
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateSoundCue(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Audio");
	Params->TryGetStringField(TEXT("path"), Path);

	USoundCueFactoryNew* Factory = NewObject<USoundCueFactoryNew>();
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, Factory->SupportedClass, Factory);
	return CreateAssetResponse(NewAsset, Name, Path);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleSpawnSound(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor world is unavailable"));
	}

	FString SoundPath;
	if (!Params->TryGetStringField(TEXT("sound"), SoundPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'sound' parameter"));
	}
	USoundBase* Sound = LoadAssetObject<USoundBase>(SoundPath);
	if (!Sound)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Sound not found: %s"), *SoundPath));
	}

	const FVector Location = GetVectorField(Params, TEXT("location"), FVector::ZeroVector);
	double Volume = 1.0;
	double Pitch = 1.0;
	Params->TryGetNumberField(TEXT("volume"), Volume);
	Params->TryGetNumberField(TEXT("pitch"), Pitch);
	UGameplayStatics::PlaySoundAtLocation(World, Sound, Location, (float)Volume, (float)Pitch);

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("sound"), Sound->GetPathName());
	Data->SetObjectField(TEXT("location"), VectorToJsonObject(Location));
	Data->SetNumberField(TEXT("volume"), Volume);
	Data->SetNumberField(TEXT("pitch"), Pitch);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateCollisionProfile(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString ObjectType = TEXT("WorldDynamic");
	FString CollisionEnabled = TEXT("QueryAndPhysics");
	FString DefaultResponse = TEXT("Block");
	Params->TryGetStringField(TEXT("object_type"), ObjectType);
	Params->TryGetStringField(TEXT("collision_enabled"), CollisionEnabled);
	Params->TryGetStringField(TEXT("default_response"), DefaultResponse);

	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const FString Section = TEXT("/Script/Engine.CollisionProfile");
	FString Entry = FString::Printf(
		TEXT("(Name=\"%s\",CollisionEnabled=%s,ObjectTypeName=\"%s\",CustomResponses=,HelpMessage=\"Created by UnrealMCP\",bCanModify=True)"),
		*Name,
		*CollisionEnabled,
		*ObjectType);
	GConfig->SetString(*Section, *FString::Printf(TEXT("+Profiles")), *Entry, ConfigPath);
	GConfig->Flush(false, ConfigPath);

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("name"), Name);
	Data->SetStringField(TEXT("config_path"), ConfigPath);
	Data->SetStringField(TEXT("default_response"), DefaultResponse);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateCollisionChannel(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString DefaultResponse = TEXT("Block");
	Params->TryGetStringField(TEXT("default_response"), DefaultResponse);
	bool bTraceType = false;
	Params->TryGetBoolField(TEXT("trace_type"), bTraceType);

	const FString ConfigPath = FPaths::ProjectConfigDir() / TEXT("DefaultEngine.ini");
	const FString Section = TEXT("/Script/Engine.CollisionProfile");
	TArray<FString> Existing;
	GConfig->GetArray(*Section, TEXT("DefaultChannelResponses"), Existing, ConfigPath);
	const FString Entry = FString::Printf(TEXT("(Channel=ECC_GameTraceChannel1,Name=\"%s\",DefaultResponse=ECR_%s,bTraceType=%s,bStaticObject=False)"),
		*Name,
		*DefaultResponse,
		bTraceType ? TEXT("True") : TEXT("False"));
	Existing.Add(Entry);
	GConfig->SetArray(*Section, TEXT("DefaultChannelResponses"), Existing, ConfigPath);
	GConfig->Flush(false, ConfigPath);

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("name"), Name);
	Data->SetStringField(TEXT("config_path"), ConfigPath);
	Data->SetStringField(TEXT("default_response"), DefaultResponse);
	Data->SetBoolField(TEXT("trace_type"), bTraceType);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateFunctionalTest(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor world is unavailable"));
	}

	FString Name = TEXT("FunctionalTest");
	Params->TryGetStringField(TEXT("name"), Name);
	const FVector Location = GetVectorField(Params, TEXT("location"), FVector::ZeroVector);
	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = FName(*Name);
	AFunctionalTest* Test = World->SpawnActor<AFunctionalTest>(AFunctionalTest::StaticClass(), Location, FRotator::ZeroRotator, SpawnParams);
	if (!Test)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn FunctionalTest actor"));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("name"), Test->GetName());
	Data->SetObjectField(TEXT("location"), VectorToJsonObject(Test->GetActorLocation()));
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleRunAutomationTests(const TSharedPtr<FJsonObject>& Params)
{
	FString TestFilter = TEXT("Project");
	Params->TryGetStringField(TEXT("filter"), TestFilter);

	if (GEditor)
	{
		GEditor->Exec(nullptr, *FString::Printf(TEXT("Automation RunTests %s"), *TestFilter));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("filter"), TestFilter);
	Data->SetBoolField(TEXT("started"), GEditor != nullptr);
	Data->SetStringField(TEXT("note"), TEXT("Automation tests were started in the editor; query the Automation Controller for detailed results."));
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleLineTrace(const TSharedPtr<FJsonObject>& Params)
{
	if (!GEditor)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("GEditor is unavailable"));
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor world is unavailable"));
	}

	const FVector Start = GetVectorField(Params, TEXT("start"), FVector::ZeroVector);
	const FVector End = GetVectorField(Params, TEXT("end"), FVector::ZeroVector);

	FString ChannelName = TEXT("Visibility");
	Params->TryGetStringField(TEXT("channel"), ChannelName);

	FHitResult Hit;
	FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(UnrealMCPLineTrace), true);
	const bool bHit = World->LineTraceSingleByChannel(Hit, Start, End, ParseCollisionChannel(ChannelName), QueryParams);

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetBoolField(TEXT("hit"), bHit);
	Data->SetStringField(TEXT("channel"), ChannelName);
	if (bHit)
	{
		Data->SetStringField(TEXT("actor"), Hit.GetActor() ? Hit.GetActor()->GetName() : TEXT(""));
		Data->SetObjectField(TEXT("location"), VectorToJsonObject(Hit.Location));
		Data->SetObjectField(TEXT("normal"), VectorToJsonObject(Hit.Normal));
		Data->SetNumberField(TEXT("distance"), Hit.Distance);
	}
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleFindActorsInRadius(const TSharedPtr<FJsonObject>& Params)
{
	if (!GEditor)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("GEditor is unavailable"));
	}

	UWorld* World = GEditor->GetEditorWorldContext().World();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor world is unavailable"));
	}

	const FVector Location = GetVectorField(Params, TEXT("location"), FVector::ZeroVector);
	double RadiusDouble = 1000.0;
	Params->TryGetNumberField(TEXT("radius"), RadiusDouble);
	const float Radius = (float)RadiusDouble;
	const float RadiusSquared = Radius * Radius;

	FString ClassFilter;
	Params->TryGetStringField(TEXT("class_filter"), ClassFilter);
	UClass* FilterClass = nullptr;
	if (!ClassFilter.IsEmpty())
	{
		FilterClass = LoadClassFromNameOrPath(ClassFilter);
	}

	TArray<TSharedPtr<FJsonValue>> Actors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor || (FilterClass && !Actor->IsA(FilterClass)))
		{
			continue;
		}

		const float DistanceSquared = FVector::DistSquared(Location, Actor->GetActorLocation());
		if (DistanceSquared > RadiusSquared)
		{
			continue;
		}

		TSharedPtr<FJsonObject> ActorObject = FUnrealMCPCommonUtils::ActorToJsonObject(Actor);
		ActorObject->SetNumberField(TEXT("distance"), FMath::Sqrt(DistanceSquared));
		Actors.Add(MakeShared<FJsonValueObject>(ActorObject));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetObjectField(TEXT("location"), VectorToJsonObject(Location));
	Data->SetNumberField(TEXT("radius"), Radius);
	Data->SetArrayField(TEXT("actors"), Actors);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleFindActorsByTag(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor world is unavailable"));
	}

	FString Tag;
	if (!Params->TryGetStringField(TEXT("tag"), Tag))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'tag' parameter"));
	}

	TArray<TSharedPtr<FJsonValue>> Actors;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (Actor && Actor->ActorHasTag(FName(*Tag)))
		{
			Actors.Add(MakeShared<FJsonValueObject>(FUnrealMCPCommonUtils::ActorToJsonObject(Actor)));
		}
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("tag"), Tag);
	Data->SetArrayField(TEXT("actors"), Actors);
	Data->SetNumberField(TEXT("count"), Actors.Num());
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleGetActorDistance(const TSharedPtr<FJsonObject>& Params)
{
	FString ActorAName;
	FString ActorBName;
	if (!Params->TryGetStringField(TEXT("actor_a"), ActorAName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_a' parameter"));
	}
	if (!Params->TryGetStringField(TEXT("actor_b"), ActorBName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_b' parameter"));
	}

	AActor* ActorA = FindActorByName(ActorAName);
	AActor* ActorB = FindActorByName(ActorBName);
	if (!ActorA || !ActorB)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("One or both actors were not found"));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("actor_a"), ActorA->GetName());
	Data->SetStringField(TEXT("actor_b"), ActorB->GetName());
	Data->SetNumberField(TEXT("distance"), FVector::Distance(ActorA->GetActorLocation(), ActorB->GetActorLocation()));
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleGetAssetReferencers(const TSharedPtr<FJsonObject>& Params)
{
	FString AssetPath;
	if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'asset_path' parameter"));
	}

	const FString PackageName = FPackageName::ObjectPathToPackageName(ResolveObjectPath(AssetPath));
	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FName> ReferencerNames;
	AssetRegistryModule.Get().GetReferencers(FName(*PackageName), ReferencerNames);

	TArray<TSharedPtr<FJsonValue>> Referencers;
	for (const FName& ReferencerName : ReferencerNames)
	{
		Referencers.Add(MakeShared<FJsonValueString>(ReferencerName.ToString()));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("asset_path"), AssetPath);
	Data->SetStringField(TEXT("package_name"), PackageName);
	Data->SetArrayField(TEXT("referencers"), Referencers);
	Data->SetNumberField(TEXT("referencer_count"), Referencers.Num());
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleValidateAssets(const TSharedPtr<FJsonObject>& Params)
{
	FString Path = TEXT("/Game");
	Params->TryGetStringField(TEXT("path"), Path);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssetsByPath(FName(*Path), Assets, true);

	TArray<TSharedPtr<FJsonValue>> Issues;
	int32 ValidAssetCount = 0;
	for (const FAssetData& Asset : Assets)
	{
		UObject* LoadedAsset = Asset.GetAsset();
		if (!LoadedAsset)
		{
			TSharedPtr<FJsonObject> Issue = MakeShared<FJsonObject>();
			Issue->SetStringField(TEXT("asset"), Asset.GetObjectPathString());
			Issue->SetStringField(TEXT("issue"), TEXT("Failed to load asset"));
			Issues.Add(MakeShared<FJsonValueObject>(Issue));
			continue;
		}

		++ValidAssetCount;
		if (Asset.AssetClassPath.IsNull())
		{
			TSharedPtr<FJsonObject> Issue = MakeShared<FJsonObject>();
			Issue->SetStringField(TEXT("asset"), Asset.GetObjectPathString());
			Issue->SetStringField(TEXT("issue"), TEXT("Missing asset class path"));
			Issues.Add(MakeShared<FJsonValueObject>(Issue));
		}
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("path"), Path);
	Data->SetNumberField(TEXT("asset_count"), Assets.Num());
	Data->SetNumberField(TEXT("valid_asset_count"), ValidAssetCount);
	Data->SetNumberField(TEXT("issue_count"), Issues.Num());
	Data->SetArrayField(TEXT("issues"), Issues);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleValidateBlueprintGraph(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintPath;
	if (!Params->TryGetStringField(TEXT("blueprint"), BlueprintPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint' parameter"));
	}

	UBlueprint* Blueprint = LoadBlueprintFromNameOrPath(BlueprintPath);
	if (!Blueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintPath));
	}

	FKismetEditorUtilities::CompileBlueprint(Blueprint);

	TArray<TSharedPtr<FJsonValue>> Issues;
	for (UEdGraph* Graph : Blueprint->UbergraphPages)
	{
		if (!Graph)
		{
			continue;
		}
		for (UEdGraphNode* Node : Graph->Nodes)
		{
			if (!Node)
			{
				continue;
			}

			bool bHasAnyLinkedPin = false;
			bool bHasPins = false;
			for (UEdGraphPin* Pin : Node->Pins)
			{
				if (!Pin)
				{
					continue;
				}
				bHasPins = true;
				if (Pin->LinkedTo.Num() > 0)
				{
					bHasAnyLinkedPin = true;
				}
				if (Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 0 && Pin->DefaultValue.IsEmpty() && Pin->DefaultObject == nullptr && !Pin->bNotConnectable)
				{
					TSharedPtr<FJsonObject> Issue = MakeShared<FJsonObject>();
					Issue->SetStringField(TEXT("graph"), Graph->GetName());
					Issue->SetStringField(TEXT("node"), Node->GetNodeTitle(ENodeTitleType::ListView).ToString());
					Issue->SetStringField(TEXT("pin"), Pin->PinName.ToString());
					Issue->SetStringField(TEXT("issue"), TEXT("Unconnected input pin with no default value"));
					Issues.Add(MakeShared<FJsonValueObject>(Issue));
				}
			}

			if (bHasPins && !bHasAnyLinkedPin)
			{
				TSharedPtr<FJsonObject> Issue = MakeShared<FJsonObject>();
				Issue->SetStringField(TEXT("graph"), Graph->GetName());
				Issue->SetStringField(TEXT("node"), Node->GetNodeTitle(ENodeTitleType::ListView).ToString());
				Issue->SetStringField(TEXT("issue"), TEXT("Orphan node"));
				Issues.Add(MakeShared<FJsonValueObject>(Issue));
			}
		}
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("blueprint"), Blueprint->GetPathName());
	Data->SetStringField(TEXT("status"), UEnum::GetValueAsString(Blueprint->Status));
	Data->SetNumberField(TEXT("issue_count"), Issues.Num());
	Data->SetArrayField(TEXT("issues"), Issues);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateSaveGameBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/Blueprints");
	Params->TryGetStringField(TEXT("path"), Path);

	UBlueprint* Blueprint = nullptr;
	return CreateBlueprintAsset(Name, Path, USaveGame::StaticClass(), Blueprint);
}

// Phase 3/5/6 Extended Advanced Tools
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateBtTaskBlueprint(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateBtServiceBlueprint(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAddBtRunEqsQuery(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleSetBtNodeBlackboardKey(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateFullAiSetup(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateSaveGameWithVariables(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleGenerateSaveLoadFunctions(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleSetupEnhancedInputRuntime(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleBindInputActionToEvent(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreatePlayerControllerWithInput(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAddNiagaraUserParameter(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAddNiagaraEmitterToSystem(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateVfxPreset(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateSoundAttenuation(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateSoundMix(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleConfigureSoundCueNodes(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleAttachSoundToActor(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateEnemyAiArchetype(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreateInteractableArchetype(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleCreatePickupItemArchetype(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandlePreviewOperation(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPAdvancedCommands::HandleGetUndoHistory(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
