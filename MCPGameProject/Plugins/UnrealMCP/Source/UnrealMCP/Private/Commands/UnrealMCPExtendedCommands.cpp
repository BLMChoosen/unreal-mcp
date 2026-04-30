#include "Commands/UnrealMCPExtendedCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"

#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Components/AudioComponent.h"
#include "Components/ActorComponent.h"
#include "Components/ExponentialHeightFogComponent.h"
#include "Components/LightComponent.h"
#include "Components/SkyLightComponent.h"
#include "Components/SplineComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "EdGraph/EdGraph.h"
#include "Editor.h"
#include "EditorAssetLibrary.h"
#include "Engine/Blueprint.h"
#include "Engine/DirectionalLight.h"
#include "Engine/ExponentialHeightFog.h"
#include "Engine/PostProcessVolume.h"
#include "Engine/SkyLight.h"
#include "Engine/StaticMesh.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/Texture2D.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "AutomatedAssetImportData.h"
#include "Factories/Factory.h"
#include "GameFramework/Actor.h"
#include "HAL/PlatformFilemanager.h"
#include "K2Node_CustomEvent.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Materials/MaterialInterface.h"
#include "Misc/PackageName.h"
#include "Misc/Paths.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/MetaData.h"
#include "UObject/Package.h"
#include "UObject/SoftObjectPath.h"
#include "UObject/UnrealType.h"

namespace
{
	FString ResolveObjectPath(const FString& AssetPath)
	{
		if (AssetPath.IsEmpty() || AssetPath.Contains(TEXT(".")) || AssetPath.StartsWith(TEXT("/Script/")))
		{
			return AssetPath;
		}

		const FString AssetName = FPackageName::GetShortName(AssetPath);
		return AssetPath + TEXT(".") + AssetName;
	}

	UWorld* GetEditorWorld()
	{
		return GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
	}

	FString JsonToString(const TSharedPtr<FJsonObject>& Object)
	{
		if (!Object.IsValid())
		{
			return TEXT("{}");
		}

		FString Output;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
		FJsonSerializer::Serialize(Object.ToSharedRef(), Writer);
		return Output;
	}

	FString JsonArrayToString(const TArray<TSharedPtr<FJsonValue>>& Values)
	{
		FString Output;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&Output);
		FJsonSerializer::Serialize(Values, Writer);
		return Output;
	}

	void SetAssetMetadata(UObject* Asset, const FString& Key, const FString& Value)
	{
		if (!Asset)
		{
			return;
		}

		UPackage* Package = Asset->GetOutermost();
		if (!Package)
		{
			return;
		}

		FMetaData& MetaData = Package->GetMetaData();
		MetaData.SetValue(Asset, *Key, *Value);
		Package->MarkPackageDirty();
	}

	FString GetAssetMetadata(UObject* Asset, const FString& Key)
	{
		if (!Asset || !Asset->GetOutermost())
		{
			return TEXT("");
		}

		FMetaData& MetaData = Asset->GetOutermost()->GetMetaData();
		return MetaData.GetValue(Asset, *Key);
	}

	TSharedPtr<FJsonObject> MakeDataResponse(const FString& Message)
	{
		TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("message"), Message);
		return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
	}

	UClass* LoadClassFromCandidates(const TArray<FString>& ClassNames)
	{
		for (const FString& ClassName : ClassNames)
		{
			if (ClassName.IsEmpty())
			{
				continue;
			}

			if (UClass* LoadedClass = LoadObject<UClass>(nullptr, *ClassName))
			{
				return LoadedClass;
			}

			const FString ShortName = FPackageName::GetShortName(ClassName);
			if (UClass* FoundClass = FindFirstObjectSafe<UClass>(*ShortName))
			{
				return FoundClass;
			}
		}

		return nullptr;
	}

	UObject* LoadAssetObject(const FString& AssetPath)
	{
		if (AssetPath.IsEmpty())
		{
			return nullptr;
		}

		if (UObject* Object = UEditorAssetLibrary::LoadAsset(AssetPath))
		{
			return Object;
		}

		return LoadObject<UObject>(nullptr, *ResolveObjectPath(AssetPath));
	}

	TArray<FString> GetStringArray(const TSharedPtr<FJsonObject>& Params, const FString& FieldName)
	{
		TArray<FString> Values;
		const TArray<TSharedPtr<FJsonValue>>* JsonValues = nullptr;
		if (Params.IsValid() && Params->TryGetArrayField(FieldName, JsonValues))
		{
			for (const TSharedPtr<FJsonValue>& Value : *JsonValues)
			{
				if (Value.IsValid())
				{
					Values.Add(Value->AsString());
				}
			}
		}
		return Values;
	}

	TArray<TSharedPtr<FJsonValue>> StringsToJson(const TArray<FString>& Values)
	{
		TArray<TSharedPtr<FJsonValue>> JsonValues;
		for (const FString& Value : Values)
		{
			JsonValues.Add(MakeShared<FJsonValueString>(Value));
		}
		return JsonValues;
	}

	FVector GetVectorField(const TSharedPtr<FJsonObject>& Params, const FString& FieldName, const FVector& DefaultValue)
	{
		if (!Params.IsValid() || !Params->HasField(FieldName))
		{
			return DefaultValue;
		}

		if (Params->HasTypedField(FieldName, EJson::Object))
		{
			const TSharedPtr<FJsonObject> Object = Params->GetObjectField(FieldName);
			double X = DefaultValue.X;
			double Y = DefaultValue.Y;
			double Z = DefaultValue.Z;
			Object->TryGetNumberField(TEXT("X"), X);
			Object->TryGetNumberField(TEXT("Y"), Y);
			Object->TryGetNumberField(TEXT("Z"), Z);
			Object->TryGetNumberField(TEXT("x"), X);
			Object->TryGetNumberField(TEXT("y"), Y);
			Object->TryGetNumberField(TEXT("z"), Z);
			return FVector((float)X, (float)Y, (float)Z);
		}

		return FUnrealMCPCommonUtils::GetVectorFromJson(Params, FieldName);
	}

	AActor* FindActorByName(const FString& ActorName)
	{
		UWorld* World = GetEditorWorld();
		if (!World)
		{
			return nullptr;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (Actor && (Actor->GetName() == ActorName || Actor->GetActorLabel() == ActorName))
			{
				return Actor;
			}
		}

		return nullptr;
	}

	bool SetObjectProperty(UObject* Object, const FName PropertyName, UObject* Value)
	{
		if (!Object || !Value)
		{
			return false;
		}

		if (FObjectPropertyBase* ObjectProperty = FindFProperty<FObjectPropertyBase>(Object->GetClass(), PropertyName))
		{
			ObjectProperty->SetObjectPropertyValue_InContainer(Object, Value);
			return true;
		}

		if (FSoftObjectProperty* SoftObjectProperty = FindFProperty<FSoftObjectProperty>(Object->GetClass(), PropertyName))
		{
			SoftObjectProperty->SetPropertyValue_InContainer(Object, FSoftObjectPtr(Value));
			return true;
		}

		return false;
	}

	bool SetClassProperty(UObject* Object, const FName PropertyName, UClass* Value)
	{
		if (!Object || !Value)
		{
			return false;
		}

		if (FClassProperty* ClassProperty = FindFProperty<FClassProperty>(Object->GetClass(), PropertyName))
		{
			ClassProperty->SetPropertyValue_InContainer(Object, Value);
			return true;
		}

		return SetObjectProperty(Object, PropertyName, Value);
	}

	bool SetBoolInsideStructProperty(UObject* Object, const FName StructPropertyName, const TArray<FName>& BoolNames, bool bValue)
	{
		if (!Object)
		{
			return false;
		}

		FStructProperty* StructProperty = FindFProperty<FStructProperty>(Object->GetClass(), StructPropertyName);
		if (!StructProperty)
		{
			return false;
		}

		void* StructValue = StructProperty->ContainerPtrToValuePtr<void>(Object);
		for (const FName& BoolName : BoolNames)
		{
			if (FBoolProperty* BoolProperty = FindFProperty<FBoolProperty>(StructProperty->Struct, BoolName))
			{
				BoolProperty->SetPropertyValue_InContainer(StructValue, bValue);
				return true;
			}
		}

		return false;
	}

	bool GetBoolInsideStructProperty(UObject* Object, const FName StructPropertyName, const TArray<FName>& BoolNames, bool& bOutValue)
	{
		if (!Object)
		{
			return false;
		}

		FStructProperty* StructProperty = FindFProperty<FStructProperty>(Object->GetClass(), StructPropertyName);
		if (!StructProperty)
		{
			return false;
		}

		void* StructValue = StructProperty->ContainerPtrToValuePtr<void>(Object);
		for (const FName& BoolName : BoolNames)
		{
			if (FBoolProperty* BoolProperty = FindFProperty<FBoolProperty>(StructProperty->Struct, BoolName))
			{
				bOutValue = BoolProperty->GetPropertyValue_InContainer(StructValue);
				return true;
			}
		}

		return false;
	}

	UObject* CreateAssetWithOptionalFactory(
		const FString& Name,
		const FString& Path,
		const TArray<FString>& AssetClassCandidates,
		const TArray<FString>& FactoryClassCandidates,
		const TFunction<void(UFactory*)>& ConfigureFactory,
		FString& OutError)
	{
		if (Name.IsEmpty())
		{
			OutError = TEXT("Missing or empty 'name' parameter");
			return nullptr;
		}

		UClass* AssetClass = LoadClassFromCandidates(AssetClassCandidates);
		if (!AssetClass)
		{
			OutError = FString::Printf(TEXT("Asset class not available. Checked: %s"), *FString::Join(AssetClassCandidates, TEXT(", ")));
			return nullptr;
		}

		UFactory* Factory = nullptr;
		if (UClass* FactoryClass = LoadClassFromCandidates(FactoryClassCandidates))
		{
			if (FactoryClass->IsChildOf(UFactory::StaticClass()) && !FactoryClass->HasAnyClassFlags(CLASS_Abstract))
			{
				Factory = NewObject<UFactory>(GetTransientPackage(), FactoryClass);
				if (Factory && ConfigureFactory)
				{
					ConfigureFactory(Factory);
				}
			}
		}

		FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
		UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, AssetClass, Factory);
		if (!NewAsset)
		{
			OutError = Factory
				? FString::Printf(TEXT("Failed to create asset %s/%s using %s"), *Path, *Name, *Factory->GetClass()->GetName())
				: FString::Printf(TEXT("Failed to create asset %s/%s; no compatible factory was available"), *Path, *Name);
			return nullptr;
		}

		NewAsset->MarkPackageDirty();
		return NewAsset;
	}

	TSharedPtr<FJsonObject> AssetResponse(UObject* Asset, const FString& Kind, bool bCreated = true)
	{
		TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
		Data->SetStringField(TEXT("kind"), Kind);
		Data->SetBoolField(TEXT("created"), bCreated);
		if (Asset)
		{
			Data->SetStringField(TEXT("name"), Asset->GetName());
			Data->SetStringField(TEXT("path"), Asset->GetPathName());
			Data->SetStringField(TEXT("class"), Asset->GetClass()->GetPathName());
		}
		return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
	}

	TArray<FString> KeywordsForPrompt(const FString& Prompt)
	{
		const FString LowerPrompt = Prompt.ToLower();
		TArray<FString> Keywords;

		if (LowerPrompt.Contains(TEXT("floresta")) || LowerPrompt.Contains(TEXT("forest")) || LowerPrompt.Contains(TEXT("selva")) || LowerPrompt.Contains(TEXT("jungle")))
		{
			Keywords.Append({TEXT("tree"), TEXT("arvore"), TEXT("bush"), TEXT("grass"), TEXT("fern"), TEXT("pine"), TEXT("forest"), TEXT("foliage")});
		}
		else if (LowerPrompt.Contains(TEXT("desert")) || LowerPrompt.Contains(TEXT("deserto")))
		{
			Keywords.Append({TEXT("rock"), TEXT("cactus"), TEXT("sand"), TEXT("desert"), TEXT("stone")});
		}
		else if (LowerPrompt.Contains(TEXT("city")) || LowerPrompt.Contains(TEXT("cidade")) || LowerPrompt.Contains(TEXT("urban")))
		{
			Keywords.Append({TEXT("building"), TEXT("street"), TEXT("road"), TEXT("wall"), TEXT("prop")});
		}
		else
		{
			Keywords.Append({TEXT("tree"), TEXT("rock"), TEXT("grass"), TEXT("mesh"), TEXT("prop")});
		}

		return Keywords;
	}

	TArray<FAssetData> FindStaticMeshAssetsForPrompt(const TArray<FString>& SearchPaths, const FString& Prompt, int32 MaxAssets)
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		const TArray<FString> Keywords = KeywordsForPrompt(Prompt);
		TArray<FAssetData> Matches;
		TArray<FAssetData> Fallbacks;

		for (const FString& SearchPath : SearchPaths)
		{
			TArray<FAssetData> Assets;
			AssetRegistryModule.Get().GetAssetsByPath(FName(*SearchPath), Assets, true);
			for (const FAssetData& AssetData : Assets)
			{
				if (AssetData.AssetClassPath.GetAssetName() != FName(TEXT("StaticMesh")))
				{
					continue;
				}

				Fallbacks.Add(AssetData);
				const FString Name = AssetData.AssetName.ToString().ToLower();
				for (const FString& Keyword : Keywords)
				{
					if (Name.Contains(Keyword))
					{
						Matches.Add(AssetData);
						break;
					}
				}
			}
		}

		if (Matches.IsEmpty())
		{
			Matches = Fallbacks;
		}

		if (MaxAssets > 0 && Matches.Num() > MaxAssets)
		{
			Matches.SetNum(MaxAssets);
		}

		return Matches;
	}

	bool SetMaterialOnLandscapeActors(const FString& ActorName, UMaterialInterface* Material, int32& OutCount)
	{
		OutCount = 0;
		UWorld* World = GetEditorWorld();
		if (!World || !Material)
		{
			return false;
		}

		for (TActorIterator<AActor> It(World); It; ++It)
		{
			AActor* Actor = *It;
			if (!Actor)
			{
				continue;
			}

			const bool bNameMatches = ActorName.IsEmpty() || Actor->GetName() == ActorName || Actor->GetActorLabel() == ActorName;
			const bool bLandscapeLike = Actor->GetClass()->GetName().Contains(TEXT("Landscape"));
			if (bNameMatches && bLandscapeLike && SetObjectProperty(Actor, TEXT("LandscapeMaterial"), Material))
			{
				Actor->Modify();
				Actor->MarkPackageDirty();
				OutCount++;
			}
		}

		return OutCount > 0;
	}
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandType == TEXT("create_pcg_graph")) return HandleCreatePCGGraph(Params);
	if (CommandType == TEXT("configure_pcg_graph")) return HandleConfigurePCGGraph(Params);
	if (CommandType == TEXT("create_pcg_volume")) return HandleCreatePCGVolume(Params);
	if (CommandType == TEXT("create_biome_from_prompt")) return HandleCreateBiomeFromPrompt(Params);
	if (CommandType == TEXT("create_landscape")) return HandleCreateLandscape(Params);
	if (CommandType == TEXT("apply_landscape_material")) return HandleApplyLandscapeMaterial(Params);
	if (CommandType == TEXT("create_road_spline")) return HandleCreateRoadSpline(Params);
	if (CommandType == TEXT("create_anim_blueprint")) return HandleCreateAnimBlueprint(Params);
	if (CommandType == TEXT("create_blend_space")) return HandleCreateBlendSpace(Params);
	if (CommandType == TEXT("create_anim_montage_from_sequence")) return HandleCreateAnimMontageFromSequence(Params);
	if (CommandType == TEXT("add_anim_notify")) return HandleAddAnimNotify(Params);
	if (CommandType == TEXT("create_ik_rig")) return HandleCreateIKRig(Params);
	if (CommandType == TEXT("create_ik_retargeter")) return HandleCreateIKRetargeter(Params);
	if (CommandType == TEXT("create_metasound_source")) return HandleCreateMetaSoundSource(Params);
	if (CommandType == TEXT("configure_metasound_graph")) return HandleConfigureMetaSoundGraph(Params);
	if (CommandType == TEXT("import_dialogue_batch")) return HandleImportDialogueBatch(Params);
	if (CommandType == TEXT("apply_lighting_preset")) return HandleApplyLightingPreset(Params);
	if (CommandType == TEXT("audit_optimization")) return HandleAuditOptimization(Params);
	if (CommandType == TEXT("set_nanite_enabled")) return HandleSetNaniteEnabled(Params);
	if (CommandType == TEXT("generate_hlods")) return HandleGenerateHLODs(Params);
	if (CommandType == TEXT("capture_profile_snapshot")) return HandleCaptureProfileSnapshot(Params);
	if (CommandType == TEXT("trigger_live_coding_compile")) return HandleTriggerLiveCodingCompile(Params);
	if (CommandType == TEXT("set_blueprint_variable_replication")) return HandleSetBlueprintVariableReplication(Params);
	if (CommandType == TEXT("create_blueprint_rpc_event")) return HandleCreateBlueprintRPCEvent(Params);
	return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown extended command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreatePCGGraph(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/PCG");
	Params->TryGetStringField(TEXT("path"), Path);

	FString Error;
	UObject* Asset = CreateAssetWithOptionalFactory(
		Name,
		Path,
		{TEXT("/Script/PCG.PCGGraph")},
		{TEXT("/Script/PCGEditor.PCGGraphFactory"), TEXT("/Script/PCGEditor.PCGGraphFactoryNew")},
		nullptr,
		Error);

	if (!Asset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error + TEXT(". Enable the PCG and PCG Editor plugins, then restart the editor."));
	}

	SetAssetMetadata(Asset, TEXT("UnrealMCP.PCGGraphSpec"), TEXT("{}"));
	return AssetResponse(Asset, TEXT("PCGGraph"));
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleConfigurePCGGraph(const TSharedPtr<FJsonObject>& Params)
{
	FString GraphPath;
	if (!Params->TryGetStringField(TEXT("graph_path"), GraphPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'graph_path' parameter"));
	}

	UObject* Graph = LoadAssetObject(GraphPath);
	if (!Graph)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("PCG graph not found: %s"), *GraphPath));
	}

	const TArray<TSharedPtr<FJsonValue>>* Nodes = nullptr;
	const TArray<TSharedPtr<FJsonValue>>* Edges = nullptr;
	Params->TryGetArrayField(TEXT("nodes"), Nodes);
	Params->TryGetArrayField(TEXT("edges"), Edges);

	TSharedPtr<FJsonObject> Spec = MakeShared<FJsonObject>();
	if (Nodes)
	{
		Spec->SetArrayField(TEXT("nodes"), *Nodes);
	}
	if (Edges)
	{
		Spec->SetArrayField(TEXT("edges"), *Edges);
	}
	if (Params->HasField(TEXT("description")))
	{
		FString Description;
		Params->TryGetStringField(TEXT("description"), Description);
		Spec->SetStringField(TEXT("description"), Description);
	}

	const FString SerializedSpec = JsonToString(Spec);
	SetAssetMetadata(Graph, TEXT("UnrealMCP.PCGGraphSpec"), SerializedSpec);

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("graph_path"), Graph->GetPathName());
	Data->SetStringField(TEXT("graph_spec"), SerializedSpec);
	Data->SetBoolField(TEXT("native_graph_mutation"), false);
	Data->SetStringField(TEXT("note"), TEXT("Stored graph node intent as asset metadata. Native PCG node mutation uses editor-private APIs and requires a PCG-specific implementation pass."));
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreatePCGVolume(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
	}

	UClass* PCGVolumeClass = LoadClassFromCandidates({TEXT("/Script/PCG.PCGVolume"), TEXT("PCGVolume")});
	if (!PCGVolumeClass)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("PCGVolume class not available. Enable the PCG plugin and restart the editor."));
	}

	FString Name = TEXT("MCP_PCGVolume");
	Params->TryGetStringField(TEXT("name"), Name);
	const FVector Location = GetVectorField(Params, TEXT("location"), FVector::ZeroVector);
	const FVector Extent = GetVectorField(Params, TEXT("extent"), FVector(2500.0f, 2500.0f, 1000.0f));

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = MakeUniqueObjectName(World, PCGVolumeClass, FName(*Name));
	AActor* Volume = World->SpawnActor<AActor>(PCGVolumeClass, Location, FRotator::ZeroRotator, SpawnParams);
	if (!Volume)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn PCG volume"));
	}

	Volume->SetActorScale3D(FVector(
		FMath::Max(0.01f, Extent.X / 100.0f),
		FMath::Max(0.01f, Extent.Y / 100.0f),
		FMath::Max(0.01f, Extent.Z / 100.0f)));

	FString GraphPath;
	if (Params->TryGetStringField(TEXT("graph_path"), GraphPath))
	{
		UObject* Graph = LoadAssetObject(GraphPath);
		if (Graph)
		{
			SetObjectProperty(Volume, TEXT("Graph"), Graph);
			TArray<UActorComponent*> Components;
			Volume->GetComponents(Components);
			for (UActorComponent* Component : Components)
			{
				SetObjectProperty(Component, TEXT("Graph"), Graph);
			}
		}
	}

	Volume->Tags.AddUnique(TEXT("UnrealMCP.PCG"));
	Volume->MarkPackageDirty();

	TSharedPtr<FJsonObject> Data = FUnrealMCPCommonUtils::ActorToJsonObject(Volume, true);
	Data->SetStringField(TEXT("class"), Volume->GetClass()->GetPathName());
	Data->SetStringField(TEXT("graph_path"), GraphPath);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateBiomeFromPrompt(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
	}

	FString Prompt;
	if (!Params->TryGetStringField(TEXT("prompt"), Prompt))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'prompt' parameter"));
	}

	FString Name = TEXT("MCP_Biome");
	Params->TryGetStringField(TEXT("name"), Name);
	double Density = Prompt.ToLower().Contains(TEXT("densa")) || Prompt.ToLower().Contains(TEXT("dense")) ? 1.0 : 0.45;
	Params->TryGetNumberField(TEXT("density"), Density);
	int32 Count = FMath::Clamp((int32)FMath::RoundToInt(120.0 * Density), 1, 1000);
	Params->TryGetNumberField(TEXT("count"), Count);
	Count = FMath::Clamp(Count, 1, 2000);

	const FVector BoundsMin = GetVectorField(Params, TEXT("bounds_min"), FVector(-2500.0f, -2500.0f, 0.0f));
	const FVector BoundsMax = GetVectorField(Params, TEXT("bounds_max"), FVector(2500.0f, 2500.0f, 0.0f));
	const double ScaleMin = Params->HasField(TEXT("scale_min")) ? Params->GetNumberField(TEXT("scale_min")) : 0.75;
	const double ScaleMax = Params->HasField(TEXT("scale_max")) ? Params->GetNumberField(TEXT("scale_max")) : 1.8;
	int32 Seed = 1337;
	Params->TryGetNumberField(TEXT("seed"), Seed);

	TArray<FString> SearchPaths = GetStringArray(Params, TEXT("asset_paths"));
	if (SearchPaths.IsEmpty())
	{
		SearchPaths.Add(TEXT("/Game"));
	}

	TArray<FAssetData> MeshAssets = FindStaticMeshAssetsForPrompt(SearchPaths, Prompt, 12);
	if (MeshAssets.IsEmpty())
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No StaticMesh assets found for biome generation"));
	}

	FRandomStream Random(Seed);
	TArray<TSharedPtr<FJsonValue>> SpawnedActors;
	TArray<FString> UsedAssets;
	for (int32 Index = 0; Index < Count; ++Index)
	{
		const FAssetData& AssetData = MeshAssets[Random.RandRange(0, MeshAssets.Num() - 1)];
		UStaticMesh* Mesh = Cast<UStaticMesh>(AssetData.GetAsset());
		if (!Mesh)
		{
			continue;
		}

		const FVector Location(
			Random.FRandRange(BoundsMin.X, BoundsMax.X),
			Random.FRandRange(BoundsMin.Y, BoundsMax.Y),
			Random.FRandRange(BoundsMin.Z, BoundsMax.Z));
		const FRotator Rotation(0.0f, Random.FRandRange(0.0f, 360.0f), 0.0f);

		FActorSpawnParameters SpawnParams;
		SpawnParams.Name = MakeUniqueObjectName(World, AStaticMeshActor::StaticClass(), FName(*(Name + TEXT("_Instance"))));
		AStaticMeshActor* MeshActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
		if (!MeshActor)
		{
			continue;
		}

		MeshActor->GetStaticMeshComponent()->SetStaticMesh(Mesh);
		const float Scale = (float)Random.FRandRange((float)ScaleMin, (float)ScaleMax);
		MeshActor->SetActorScale3D(FVector(Scale));
		MeshActor->Tags.AddUnique(TEXT("UnrealMCP.Biome"));
		MeshActor->Tags.AddUnique(FName(*Name));
		MeshActor->MarkPackageDirty();
		SpawnedActors.Add(FUnrealMCPCommonUtils::ActorToJson(MeshActor));
		UsedAssets.AddUnique(AssetData.GetSoftObjectPath().ToString());
	}

	const bool bCreatePCGVolume = Params->HasField(TEXT("create_pcg_volume")) ? Params->GetBoolField(TEXT("create_pcg_volume")) : true;
	TSharedPtr<FJsonObject> PCGVolumeData;
	if (bCreatePCGVolume)
	{
		TSharedPtr<FJsonObject> VolumeParams = MakeShared<FJsonObject>();
		VolumeParams->SetStringField(TEXT("name"), Name + TEXT("_PCGVolume"));
		VolumeParams->SetArrayField(TEXT("location"), {MakeShared<FJsonValueNumber>((BoundsMin.X + BoundsMax.X) * 0.5), MakeShared<FJsonValueNumber>((BoundsMin.Y + BoundsMax.Y) * 0.5), MakeShared<FJsonValueNumber>((BoundsMin.Z + BoundsMax.Z) * 0.5)});
		VolumeParams->SetArrayField(TEXT("extent"), {MakeShared<FJsonValueNumber>(FMath::Abs(BoundsMax.X - BoundsMin.X) * 0.5), MakeShared<FJsonValueNumber>(FMath::Abs(BoundsMax.Y - BoundsMin.Y) * 0.5), MakeShared<FJsonValueNumber>(1000.0)});
		TSharedPtr<FJsonObject> VolumeResult = HandleCreatePCGVolume(VolumeParams);
		if (VolumeResult.IsValid() && VolumeResult->GetBoolField(TEXT("success")))
		{
			PCGVolumeData = VolumeResult->GetObjectField(TEXT("data"));
		}
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("name"), Name);
	Data->SetStringField(TEXT("prompt"), Prompt);
	Data->SetNumberField(TEXT("density"), Density);
	Data->SetNumberField(TEXT("requested_count"), Count);
	Data->SetNumberField(TEXT("spawned_count"), SpawnedActors.Num());
	Data->SetArrayField(TEXT("spawned_actors"), SpawnedActors);
	Data->SetArrayField(TEXT("used_assets"), StringsToJson(UsedAssets));
	if (PCGVolumeData.IsValid())
	{
		Data->SetObjectField(TEXT("pcg_volume"), PCGVolumeData);
	}
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateLandscape(const TSharedPtr<FJsonObject>& Params)
{
	UClass* LandscapeClass = LoadClassFromCandidates({TEXT("/Script/Landscape.Landscape"), TEXT("Landscape")});
	if (!LandscapeClass)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Landscape class not available. Enable the Landscape module/plugin for this project."));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	FString Name = TEXT("MCP_Landscape");
	Params->TryGetStringField(TEXT("name"), Name);
	Data->SetStringField(TEXT("name"), Name);
	Data->SetStringField(TEXT("landscape_class"), LandscapeClass->GetPathName());
	Data->SetBoolField(TEXT("created"), false);
	Data->SetStringField(TEXT("note"), TEXT("Landscape actor creation requires editor landscape import APIs with height data. This command validates availability and returns a setup descriptor; use create_road_spline and apply_landscape_material after creating/importing the landscape."));
	Data->SetNumberField(TEXT("section_size"), Params->HasField(TEXT("section_size")) ? Params->GetNumberField(TEXT("section_size")) : 63.0);
	Data->SetNumberField(TEXT("component_count_x"), Params->HasField(TEXT("component_count_x")) ? Params->GetNumberField(TEXT("component_count_x")) : 8.0);
	Data->SetNumberField(TEXT("component_count_y"), Params->HasField(TEXT("component_count_y")) ? Params->GetNumberField(TEXT("component_count_y")) : 8.0);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleApplyLandscapeMaterial(const TSharedPtr<FJsonObject>& Params)
{
	FString MaterialPath;
	if (!Params->TryGetStringField(TEXT("material_path"), MaterialPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_path' parameter"));
	}

	UMaterialInterface* Material = Cast<UMaterialInterface>(LoadAssetObject(MaterialPath));
	if (!Material)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialPath));
	}

	FString ActorName;
	Params->TryGetStringField(TEXT("actor_name"), ActorName);
	int32 UpdatedCount = 0;
	if (!SetMaterialOnLandscapeActors(ActorName, Material, UpdatedCount))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No matching Landscape actors were found or LandscapeMaterial could not be set"));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("material_path"), Material->GetPathName());
	Data->SetStringField(TEXT("actor_name"), ActorName);
	Data->SetNumberField(TEXT("updated_count"), UpdatedCount);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateRoadSpline(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
	}

	FString Name = TEXT("MCP_RoadSpline");
	Params->TryGetStringField(TEXT("name"), Name);

	const TArray<TSharedPtr<FJsonValue>>* PointValues = nullptr;
	if (!Params->TryGetArrayField(TEXT("points"), PointValues) || !PointValues || PointValues->Num() < 2)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'points' array with at least two [x,y,z] points"));
	}

	FActorSpawnParameters SpawnParams;
	SpawnParams.Name = MakeUniqueObjectName(World, AActor::StaticClass(), FName(*Name));
	AActor* SplineActor = World->SpawnActor<AActor>(AActor::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator, SpawnParams);
	if (!SplineActor)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create spline actor"));
	}

	USplineComponent* Spline = NewObject<USplineComponent>(SplineActor, TEXT("RoadSpline"));
	SplineActor->SetRootComponent(Spline);
	SplineActor->AddInstanceComponent(Spline);
	Spline->RegisterComponent();
	Spline->ClearSplinePoints(false);

	for (const TSharedPtr<FJsonValue>& PointValue : *PointValues)
	{
		const TArray<TSharedPtr<FJsonValue>>* PointArray = nullptr;
		if (PointValue.IsValid() && PointValue->TryGetArray(PointArray) && PointArray && PointArray->Num() >= 3)
		{
			const FVector Point(
				(float)(*PointArray)[0]->AsNumber(),
				(float)(*PointArray)[1]->AsNumber(),
				(float)(*PointArray)[2]->AsNumber());
			Spline->AddSplinePoint(Point, ESplineCoordinateSpace::World, false);
		}
	}
	Spline->UpdateSpline();

	double Width = 600.0;
	Params->TryGetNumberField(TEXT("width"), Width);
	SplineActor->Tags.AddUnique(TEXT("UnrealMCP.RoadSpline"));
	SetAssetMetadata(SplineActor, TEXT("UnrealMCP.RoadWidth"), FString::SanitizeFloat(Width));
	SplineActor->MarkPackageDirty();

	TSharedPtr<FJsonObject> Data = FUnrealMCPCommonUtils::ActorToJsonObject(SplineActor, true);
	Data->SetNumberField(TEXT("point_count"), Spline->GetNumberOfSplinePoints());
	Data->SetNumberField(TEXT("width"), Width);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateAnimBlueprint(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Animation");
	Params->TryGetStringField(TEXT("path"), Path);
	FString SkeletonPath;
	Params->TryGetStringField(TEXT("skeleton_path"), SkeletonPath);
	UObject* Skeleton = LoadAssetObject(SkeletonPath);

	FString Error;
	UObject* Asset = CreateAssetWithOptionalFactory(
		Name,
		Path,
		{TEXT("/Script/Engine.AnimBlueprint")},
		{TEXT("/Script/UnrealEd.AnimBlueprintFactory")},
		[Skeleton](UFactory* Factory)
		{
			if (Skeleton)
			{
				SetObjectProperty(Factory, TEXT("TargetSkeleton"), Skeleton);
			}
			if (UClass* AnimInstanceClass = LoadClassFromCandidates({TEXT("/Script/Engine.AnimInstance")}))
			{
				SetClassProperty(Factory, TEXT("ParentClass"), AnimInstanceClass);
			}
		},
		Error);

	if (!Asset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
	}

	return AssetResponse(Asset, TEXT("AnimBlueprint"));
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateBlendSpace(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Animation");
	Params->TryGetStringField(TEXT("path"), Path);
	UObject* Skeleton = LoadAssetObject(Params->HasField(TEXT("skeleton_path")) ? Params->GetStringField(TEXT("skeleton_path")) : TEXT(""));

	FString Error;
	UObject* Asset = CreateAssetWithOptionalFactory(
		Name,
		Path,
		{TEXT("/Script/Engine.BlendSpace")},
		{TEXT("/Script/UnrealEd.BlendSpaceFactoryNew"), TEXT("/Script/UnrealEd.BlendSpaceFactory1D")},
		[Skeleton](UFactory* Factory)
		{
			if (Skeleton)
			{
				SetObjectProperty(Factory, TEXT("TargetSkeleton"), Skeleton);
			}
		},
		Error);

	if (!Asset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
	}

	if (Params->HasField(TEXT("axis_spec")))
	{
		SetAssetMetadata(Asset, TEXT("UnrealMCP.BlendSpaceAxisSpec"), JsonToString(Params->GetObjectField(TEXT("axis_spec"))));
	}

	return AssetResponse(Asset, TEXT("BlendSpace"));
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateAnimMontageFromSequence(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Animation");
	Params->TryGetStringField(TEXT("path"), Path);
	UObject* SourceAnimation = LoadAssetObject(Params->HasField(TEXT("sequence_path")) ? Params->GetStringField(TEXT("sequence_path")) : TEXT(""));

	FString Error;
	UObject* Asset = CreateAssetWithOptionalFactory(
		Name,
		Path,
		{TEXT("/Script/Engine.AnimMontage")},
		{TEXT("/Script/UnrealEd.AnimMontageFactory")},
		[SourceAnimation](UFactory* Factory)
		{
			if (SourceAnimation)
			{
				SetObjectProperty(Factory, TEXT("SourceAnimation"), SourceAnimation);
				SetObjectProperty(Factory, TEXT("PreviewSkeletalMesh"), SourceAnimation);
			}
		},
		Error);

	if (!Asset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error);
	}

	if (SourceAnimation)
	{
		SetAssetMetadata(Asset, TEXT("UnrealMCP.SourceAnimation"), SourceAnimation->GetPathName());
	}
	return AssetResponse(Asset, TEXT("AnimMontage"));
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleAddAnimNotify(const TSharedPtr<FJsonObject>& Params)
{
	FString AnimationPath;
	if (!Params->TryGetStringField(TEXT("animation_path"), AnimationPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'animation_path' parameter"));
	}

	UObject* Animation = LoadAssetObject(AnimationPath);
	if (!Animation)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Animation asset not found: %s"), *AnimationPath));
	}

	FString NotifyName = TEXT("MCPNotify");
	Params->TryGetStringField(TEXT("notify_name"), NotifyName);
	double TimeSeconds = 0.0;
	Params->TryGetNumberField(TEXT("time_seconds"), TimeSeconds);
	FString NotifyClass;
	Params->TryGetStringField(TEXT("notify_class"), NotifyClass);

	TArray<TSharedPtr<FJsonValue>> ExistingNotifies;
	const FString Existing = GetAssetMetadata(Animation, TEXT("UnrealMCP.AnimationNotifies"));
	if (!Existing.IsEmpty())
	{
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(Existing);
		FJsonSerializer::Deserialize(Reader, ExistingNotifies);
	}

	TSharedPtr<FJsonObject> Notify = MakeShared<FJsonObject>();
	Notify->SetStringField(TEXT("name"), NotifyName);
	Notify->SetNumberField(TEXT("time_seconds"), TimeSeconds);
	Notify->SetStringField(TEXT("notify_class"), NotifyClass);
	ExistingNotifies.Add(MakeShared<FJsonValueObject>(Notify));
	SetAssetMetadata(Animation, TEXT("UnrealMCP.AnimationNotifies"), JsonArrayToString(ExistingNotifies));

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("animation_path"), Animation->GetPathName());
	Data->SetStringField(TEXT("notify_name"), NotifyName);
	Data->SetNumberField(TEXT("time_seconds"), TimeSeconds);
	Data->SetBoolField(TEXT("native_notify_inserted"), false);
	Data->SetStringField(TEXT("note"), TEXT("Notify intent stored as metadata. Native notify insertion requires AnimationEditor API handling for project-specific notify classes."));
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateIKRig(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Animation/IK");
	Params->TryGetStringField(TEXT("path"), Path);
	UObject* SkeletalMesh = LoadAssetObject(Params->HasField(TEXT("skeletal_mesh_path")) ? Params->GetStringField(TEXT("skeletal_mesh_path")) : TEXT(""));

	FString Error;
	UObject* Asset = CreateAssetWithOptionalFactory(
		Name,
		Path,
		{TEXT("/Script/IKRig.IKRigDefinition")},
		{TEXT("/Script/IKRigEditor.IKRigDefinitionFactory")},
		[SkeletalMesh](UFactory* Factory)
		{
			if (SkeletalMesh)
			{
				SetObjectProperty(Factory, TEXT("PreviewSkeletalMesh"), SkeletalMesh);
				SetObjectProperty(Factory, TEXT("SkeletalMesh"), SkeletalMesh);
			}
		},
		Error);

	if (!Asset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error + TEXT(". Enable the IK Rig plugin if needed."));
	}

	return AssetResponse(Asset, TEXT("IKRig"));
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateIKRetargeter(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Animation/IK");
	Params->TryGetStringField(TEXT("path"), Path);

	UObject* SourceRig = LoadAssetObject(Params->HasField(TEXT("source_ik_rig_path")) ? Params->GetStringField(TEXT("source_ik_rig_path")) : TEXT(""));
	UObject* TargetRig = LoadAssetObject(Params->HasField(TEXT("target_ik_rig_path")) ? Params->GetStringField(TEXT("target_ik_rig_path")) : TEXT(""));

	FString Error;
	UObject* Asset = CreateAssetWithOptionalFactory(
		Name,
		Path,
		{TEXT("/Script/IKRig.IKRetargeter")},
		{TEXT("/Script/IKRigEditor.IKRetargeterFactory")},
		[SourceRig, TargetRig](UFactory* Factory)
		{
			if (SourceRig)
			{
				SetObjectProperty(Factory, TEXT("SourceIKRig"), SourceRig);
			}
			if (TargetRig)
			{
				SetObjectProperty(Factory, TEXT("TargetIKRig"), TargetRig);
			}
		},
		Error);

	if (!Asset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error + TEXT(". Enable the IK Rig plugin if needed."));
	}

	if (SourceRig) SetAssetMetadata(Asset, TEXT("UnrealMCP.SourceIKRig"), SourceRig->GetPathName());
	if (TargetRig) SetAssetMetadata(Asset, TEXT("UnrealMCP.TargetIKRig"), TargetRig->GetPathName());
	return AssetResponse(Asset, TEXT("IKRetargeter"));
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateMetaSoundSource(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}
	FString Path = TEXT("/Game/Audio/MetaSounds");
	Params->TryGetStringField(TEXT("path"), Path);

	FString Error;
	UObject* Asset = CreateAssetWithOptionalFactory(
		Name,
		Path,
		{TEXT("/Script/MetasoundEngine.MetaSoundSource")},
		{TEXT("/Script/MetasoundEditor.MetaSoundSourceFactory"), TEXT("/Script/MetasoundEditor.MetaSoundFactory")},
		nullptr,
		Error);

	if (!Asset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(Error + TEXT(". Enable MetaSound and MetaSound Editor plugins if needed."));
	}

	SetAssetMetadata(Asset, TEXT("UnrealMCP.MetaSoundGraphSpec"), TEXT("{}"));
	return AssetResponse(Asset, TEXT("MetaSoundSource"));
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleConfigureMetaSoundGraph(const TSharedPtr<FJsonObject>& Params)
{
	FString MetaSoundPath;
	if (!Params->TryGetStringField(TEXT("metasound_path"), MetaSoundPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'metasound_path' parameter"));
	}

	UObject* MetaSound = LoadAssetObject(MetaSoundPath);
	if (!MetaSound)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("MetaSound asset not found: %s"), *MetaSoundPath));
	}

	TSharedPtr<FJsonObject> Spec = MakeShared<FJsonObject>();
	const TArray<TSharedPtr<FJsonValue>>* Nodes = nullptr;
	const TArray<TSharedPtr<FJsonValue>>* Connections = nullptr;
	if (Params->TryGetArrayField(TEXT("nodes"), Nodes) && Nodes)
	{
		Spec->SetArrayField(TEXT("nodes"), *Nodes);
	}
	if (Params->TryGetArrayField(TEXT("connections"), Connections) && Connections)
	{
		Spec->SetArrayField(TEXT("connections"), *Connections);
	}
	SetAssetMetadata(MetaSound, TEXT("UnrealMCP.MetaSoundGraphSpec"), JsonToString(Spec));

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("metasound_path"), MetaSound->GetPathName());
	Data->SetStringField(TEXT("graph_spec"), JsonToString(Spec));
	Data->SetBoolField(TEXT("native_graph_mutation"), false);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleImportDialogueBatch(const TSharedPtr<FJsonObject>& Params)
{
	const TArray<TSharedPtr<FJsonValue>>* Lines = nullptr;
	if (!Params->TryGetArrayField(TEXT("dialogue"), Lines) || !Lines)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'dialogue' array"));
	}

	FString DestinationPath = TEXT("/Game/Audio/Dialogues");
	Params->TryGetStringField(TEXT("destination_path"), DestinationPath);
	bool bImportAudio = true;
	Params->TryGetBoolField(TEXT("import_audio"), bImportAudio);

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
	TArray<TSharedPtr<FJsonValue>> Rows;
	TArray<TSharedPtr<FJsonValue>> ImportedAudio;
	TArray<TSharedPtr<FJsonValue>> Issues;

	for (const TSharedPtr<FJsonValue>& LineValue : *Lines)
	{
		const TSharedPtr<FJsonObject>* LineObjectPtr = nullptr;
		if (!LineValue.IsValid() || !LineValue->TryGetObject(LineObjectPtr) || !LineObjectPtr || !LineObjectPtr->IsValid())
		{
			continue;
		}

		TSharedPtr<FJsonObject> LineObject = *LineObjectPtr;
		TSharedPtr<FJsonObject> Row = MakeShared<FJsonObject>();
		Row->SetStringField(TEXT("id"), LineObject->HasField(TEXT("id")) ? LineObject->GetStringField(TEXT("id")) : FGuid::NewGuid().ToString(EGuidFormats::Short));
		Row->SetStringField(TEXT("speaker"), LineObject->HasField(TEXT("speaker")) ? LineObject->GetStringField(TEXT("speaker")) : TEXT(""));
		Row->SetStringField(TEXT("text"), LineObject->HasField(TEXT("text")) ? LineObject->GetStringField(TEXT("text")) : TEXT(""));

		FString AudioFile;
		if (bImportAudio && LineObject->TryGetStringField(TEXT("audio_file"), AudioFile) && !AudioFile.IsEmpty())
		{
			if (FPaths::FileExists(AudioFile))
			{
				UAutomatedAssetImportData* ImportData = NewObject<UAutomatedAssetImportData>();
				ImportData->DestinationPath = DestinationPath;
				ImportData->Filenames.Add(AudioFile);
				ImportData->bReplaceExisting = true;
				TArray<UObject*> Assets = AssetToolsModule.Get().ImportAssetsAutomated(ImportData);
				if (Assets.Num() > 0 && Assets[0])
				{
					Row->SetStringField(TEXT("sound_wave"), Assets[0]->GetPathName());
					ImportedAudio.Add(MakeShared<FJsonValueString>(Assets[0]->GetPathName()));
				}
			}
			else
			{
				TSharedPtr<FJsonObject> Issue = MakeShared<FJsonObject>();
				Issue->SetStringField(TEXT("id"), Row->GetStringField(TEXT("id")));
				Issue->SetStringField(TEXT("issue"), FString::Printf(TEXT("Audio file not found: %s"), *AudioFile));
				Issues.Add(MakeShared<FJsonValueObject>(Issue));
			}
		}

		Rows.Add(MakeShared<FJsonValueObject>(Row));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("destination_path"), DestinationPath);
	Data->SetArrayField(TEXT("rows"), Rows);
	Data->SetArrayField(TEXT("imported_audio"), ImportedAudio);
	Data->SetArrayField(TEXT("issues"), Issues);
	Data->SetNumberField(TEXT("row_count"), Rows.Num());
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleApplyLightingPreset(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
	}

	FString Preset = TEXT("night_horror");
	Params->TryGetStringField(TEXT("preset"), Preset);
	const FString LowerPreset = Preset.ToLower();

	ADirectionalLight* DirectionalLight = nullptr;
	for (TActorIterator<ADirectionalLight> It(World); It; ++It)
	{
		DirectionalLight = *It;
		break;
	}
	if (!DirectionalLight)
	{
		DirectionalLight = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), FVector(0, 0, 600), FRotator(-35, -120, 0));
	}

	ASkyLight* SkyLight = nullptr;
	for (TActorIterator<ASkyLight> It(World); It; ++It)
	{
		SkyLight = *It;
		break;
	}
	if (!SkyLight)
	{
		SkyLight = World->SpawnActor<ASkyLight>(ASkyLight::StaticClass(), FVector(0, 0, 400), FRotator::ZeroRotator);
	}

	AExponentialHeightFog* Fog = nullptr;
	for (TActorIterator<AExponentialHeightFog> It(World); It; ++It)
	{
		Fog = *It;
		break;
	}
	if (!Fog)
	{
		Fog = World->SpawnActor<AExponentialHeightFog>(AExponentialHeightFog::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
	}

	APostProcessVolume* PostProcess = nullptr;
	for (TActorIterator<APostProcessVolume> It(World); It; ++It)
	{
		PostProcess = *It;
		break;
	}
	if (!PostProcess)
	{
		PostProcess = World->SpawnActor<APostProcessVolume>(APostProcessVolume::StaticClass(), FVector::ZeroVector, FRotator::ZeroRotator);
		PostProcess->bUnbound = true;
	}

	if (UClass* SkyAtmosphereClass = LoadClassFromCandidates({TEXT("/Script/Engine.SkyAtmosphere"), TEXT("SkyAtmosphere")}))
	{
		bool bHasSkyAtmosphere = false;
		for (TActorIterator<AActor> It(World, SkyAtmosphereClass); It; ++It)
		{
			bHasSkyAtmosphere = true;
			break;
		}
		if (!bHasSkyAtmosphere)
		{
			World->SpawnActor<AActor>(SkyAtmosphereClass, FVector::ZeroVector, FRotator::ZeroRotator);
		}
	}

	if (LowerPreset.Contains(TEXT("horror")) || LowerPreset.Contains(TEXT("terror")) || LowerPreset.Contains(TEXT("night")) || LowerPreset.Contains(TEXT("noite")))
	{
		DirectionalLight->SetActorRotation(FRotator(-8.0f, -135.0f, 0.0f));
		DirectionalLight->GetLightComponent()->SetIntensity(0.15f);
		DirectionalLight->GetLightComponent()->SetLightColor(FLinearColor(0.45f, 0.52f, 0.8f));
		SkyLight->GetLightComponent()->SetIntensity(0.08f);
		Fog->GetComponent()->SetFogDensity(0.075f);
		Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.05f, 0.07f, 0.11f));
		PostProcess->Settings.bOverride_ColorSaturation = true;
		PostProcess->Settings.ColorSaturation = FVector4(0.55f, 0.55f, 0.62f, 1.0f);
		PostProcess->Settings.bOverride_ColorContrast = true;
		PostProcess->Settings.ColorContrast = FVector4(1.28f, 1.28f, 1.28f, 1.0f);
		PostProcess->Settings.bOverride_FilmGrainIntensity = true;
		PostProcess->Settings.FilmGrainIntensity = 0.35f;
	}
	else if (LowerPreset.Contains(TEXT("sunset")) || LowerPreset.Contains(TEXT("por do sol")) || LowerPreset.Contains(TEXT("dusk")))
	{
		DirectionalLight->SetActorRotation(FRotator(-12.0f, 45.0f, 0.0f));
		DirectionalLight->GetLightComponent()->SetIntensity(1.35f);
		DirectionalLight->GetLightComponent()->SetLightColor(FLinearColor(1.0f, 0.58f, 0.32f));
		SkyLight->GetLightComponent()->SetIntensity(0.55f);
		Fog->GetComponent()->SetFogDensity(0.02f);
		Fog->GetComponent()->SetFogInscatteringColor(FLinearColor(0.95f, 0.45f, 0.25f));
		PostProcess->Settings.bOverride_ColorContrast = true;
		PostProcess->Settings.ColorContrast = FVector4(1.12f, 1.08f, 1.02f, 1.0f);
	}
	else
	{
		DirectionalLight->SetActorRotation(FRotator(-45.0f, -45.0f, 0.0f));
		DirectionalLight->GetLightComponent()->SetIntensity(3.0f);
		DirectionalLight->GetLightComponent()->SetLightColor(FLinearColor::White);
		SkyLight->GetLightComponent()->SetIntensity(1.0f);
		Fog->GetComponent()->SetFogDensity(0.01f);
		PostProcess->Settings.bOverride_ColorSaturation = false;
		PostProcess->Settings.bOverride_ColorContrast = false;
		PostProcess->Settings.bOverride_FilmGrainIntensity = false;
	}

	TArray<TSharedPtr<FJsonValue>> Changed;
	if (DirectionalLight) Changed.Add(MakeShared<FJsonValueString>(DirectionalLight->GetActorLabel()));
	if (SkyLight) Changed.Add(MakeShared<FJsonValueString>(SkyLight->GetActorLabel()));
	if (Fog) Changed.Add(MakeShared<FJsonValueString>(Fog->GetActorLabel()));
	if (PostProcess) Changed.Add(MakeShared<FJsonValueString>(PostProcess->GetActorLabel()));

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("preset"), Preset);
	Data->SetArrayField(TEXT("changed_actors"), Changed);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleAuditOptimization(const TSharedPtr<FJsonObject>& Params)
{
	FString Path = TEXT("/Game");
	Params->TryGetStringField(TEXT("path"), Path);
	double MaxTextureMegapixels = 16.0;
	Params->TryGetNumberField(TEXT("max_texture_megapixels"), MaxTextureMegapixels);

	FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
	TArray<FAssetData> Assets;
	AssetRegistryModule.Get().GetAssetsByPath(FName(*Path), Assets, true);

	TArray<TSharedPtr<FJsonValue>> Issues;
	int32 StaticMeshCount = 0;
	int32 TextureCount = 0;

	for (const FAssetData& AssetData : Assets)
	{
		const FName ClassName = AssetData.AssetClassPath.GetAssetName();
		if (ClassName == FName(TEXT("StaticMesh")))
		{
			StaticMeshCount++;
			UObject* Object = AssetData.GetAsset();
			bool bNaniteEnabled = false;
			const bool bCanReadNanite = GetBoolInsideStructProperty(Object, TEXT("NaniteSettings"), {TEXT("bEnabled"), TEXT("Enabled")}, bNaniteEnabled);
			if (bCanReadNanite && !bNaniteEnabled)
			{
				TSharedPtr<FJsonObject> Issue = MakeShared<FJsonObject>();
				Issue->SetStringField(TEXT("severity"), TEXT("warning"));
				Issue->SetStringField(TEXT("type"), TEXT("nanite_disabled"));
				Issue->SetStringField(TEXT("asset"), AssetData.GetSoftObjectPath().ToString());
				Issue->SetStringField(TEXT("message"), TEXT("Static Mesh has Nanite disabled"));
				Issues.Add(MakeShared<FJsonValueObject>(Issue));
			}
		}
		else if (ClassName == FName(TEXT("Texture2D")))
		{
			TextureCount++;
			if (UTexture2D* Texture = Cast<UTexture2D>(AssetData.GetAsset()))
			{
				const double Megapixels = (double)Texture->GetSizeX() * (double)Texture->GetSizeY() / 1000000.0;
				if (Megapixels > MaxTextureMegapixels)
				{
					TSharedPtr<FJsonObject> Issue = MakeShared<FJsonObject>();
					Issue->SetStringField(TEXT("severity"), TEXT("warning"));
					Issue->SetStringField(TEXT("type"), TEXT("large_texture"));
					Issue->SetStringField(TEXT("asset"), AssetData.GetSoftObjectPath().ToString());
					Issue->SetNumberField(TEXT("megapixels"), Megapixels);
					Issue->SetStringField(TEXT("message"), TEXT("Texture is larger than the configured budget"));
					Issues.Add(MakeShared<FJsonValueObject>(Issue));
				}
			}
		}
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("path"), Path);
	Data->SetNumberField(TEXT("static_mesh_count"), StaticMeshCount);
	Data->SetNumberField(TEXT("texture_count"), TextureCount);
	Data->SetNumberField(TEXT("issue_count"), Issues.Num());
	Data->SetArrayField(TEXT("issues"), Issues);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleSetNaniteEnabled(const TSharedPtr<FJsonObject>& Params)
{
	bool bEnabled = true;
	Params->TryGetBoolField(TEXT("enabled"), bEnabled);
	TArray<FString> AssetPaths = GetStringArray(Params, TEXT("asset_paths"));
	FString Path;
	Params->TryGetStringField(TEXT("path"), Path);

	if (AssetPaths.IsEmpty() && !Path.IsEmpty())
	{
		FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(TEXT("AssetRegistry"));
		TArray<FAssetData> Assets;
		AssetRegistryModule.Get().GetAssetsByPath(FName(*Path), Assets, true);
		for (const FAssetData& AssetData : Assets)
		{
			if (AssetData.AssetClassPath.GetAssetName() == FName(TEXT("StaticMesh")))
			{
				AssetPaths.Add(AssetData.GetSoftObjectPath().ToString());
			}
		}
	}

	TArray<TSharedPtr<FJsonValue>> Updated;
	TArray<TSharedPtr<FJsonValue>> Failed;
	for (const FString& AssetPath : AssetPaths)
	{
		UObject* Asset = LoadAssetObject(AssetPath);
		if (Asset && SetBoolInsideStructProperty(Asset, TEXT("NaniteSettings"), {TEXT("bEnabled"), TEXT("Enabled")}, bEnabled))
		{
			Asset->Modify();
			Asset->MarkPackageDirty();
			Asset->PostEditChange();
			Updated.Add(MakeShared<FJsonValueString>(Asset->GetPathName()));
		}
		else
		{
			Failed.Add(MakeShared<FJsonValueString>(AssetPath));
		}
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetBoolField(TEXT("enabled"), bEnabled);
	Data->SetArrayField(TEXT("updated"), Updated);
	Data->SetArrayField(TEXT("failed"), Failed);
	Data->SetNumberField(TEXT("updated_count"), Updated.Num());
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleGenerateHLODs(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World || !GEditor)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
	}

	const bool bResult = GEditor->Exec(World, TEXT("BuildHLODs"));
	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetBoolField(TEXT("command_sent"), bResult);
	Data->SetStringField(TEXT("command"), TEXT("BuildHLODs"));
	Data->SetStringField(TEXT("note"), TEXT("HLOD generation is editor-build-system dependent; check Output Log for detailed results."));
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCaptureProfileSnapshot(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World || !GEditor)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
	}

	TArray<FString> Commands = GetStringArray(Params, TEXT("stat_commands"));
	if (Commands.IsEmpty())
	{
		Commands = {TEXT("stat unit"), TEXT("stat scenerendering"), TEXT("stat gpu")};
	}

	TArray<TSharedPtr<FJsonValue>> Sent;
	for (const FString& Command : Commands)
	{
		GEditor->Exec(World, *Command);
		Sent.Add(MakeShared<FJsonValueString>(Command));
	}

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetArrayField(TEXT("commands_sent"), Sent);
	Data->SetStringField(TEXT("note"), TEXT("Profile stat commands were toggled in the editor. Use get_output_log or Unreal Insights for captured details."));
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleTriggerLiveCodingCompile(const TSharedPtr<FJsonObject>& Params)
{
	UWorld* World = GetEditorWorld();
	if (!World || !GEditor)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
	}

	const bool bResult = GEditor->Exec(World, TEXT("LiveCoding.Compile"));
	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetBoolField(TEXT("command_sent"), bResult);
	Data->SetStringField(TEXT("command"), TEXT("LiveCoding.Compile"));
	Data->SetStringField(TEXT("note"), TEXT("Live Coding must be enabled in the editor for this command to compile changes."));
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleSetBlueprintVariableReplication(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintName;
	FString VariableName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) || !Params->TryGetStringField(TEXT("variable_name"), VariableName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'variable_name' parameter"));
	}

	UBlueprint* Blueprint = Cast<UBlueprint>(LoadAssetObject(BlueprintName));
	if (!Blueprint)
	{
		Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
	}
	if (!Blueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
	}

	FString Mode = TEXT("Replicated");
	Params->TryGetStringField(TEXT("replication"), Mode);
	FString NotifyFunction = VariableName + TEXT("_OnRep");
	Params->TryGetStringField(TEXT("notify_function"), NotifyFunction);

	bool bFound = false;
	for (FBPVariableDescription& Variable : Blueprint->NewVariables)
	{
		if (Variable.VarName.ToString() == VariableName)
		{
			bFound = true;
			Variable.PropertyFlags |= CPF_Net;
			if (Mode.Equals(TEXT("RepNotify"), ESearchCase::IgnoreCase) || Mode.Equals(TEXT("ReplicatedUsing"), ESearchCase::IgnoreCase))
			{
				Variable.RepNotifyFunc = FName(*NotifyFunction);
			}
			else
			{
				Variable.RepNotifyFunc = NAME_None;
			}
			break;
		}
	}

	if (!bFound)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Variable not found on Blueprint: %s"), *VariableName));
	}

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	Blueprint->MarkPackageDirty();

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("blueprint_name"), Blueprint->GetName());
	Data->SetStringField(TEXT("variable_name"), VariableName);
	Data->SetStringField(TEXT("replication"), Mode);
	Data->SetStringField(TEXT("notify_function"), NotifyFunction);
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}

TSharedPtr<FJsonObject> FUnrealMCPExtendedCommands::HandleCreateBlueprintRPCEvent(const TSharedPtr<FJsonObject>& Params)
{
	FString BlueprintName;
	FString EventName;
	if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) || !Params->TryGetStringField(TEXT("event_name"), EventName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'event_name' parameter"));
	}

	UBlueprint* Blueprint = Cast<UBlueprint>(LoadAssetObject(BlueprintName));
	if (!Blueprint)
	{
		Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
	}
	if (!Blueprint)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
	}

	UEdGraph* Graph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
	if (!Graph)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
	}

	FString RPCType = TEXT("RunOnServer");
	Params->TryGetStringField(TEXT("rpc_type"), RPCType);
	bool bReliable = true;
	Params->TryGetBoolField(TEXT("reliable"), bReliable);
	const FVector2D Position = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("position"));

	UK2Node_CustomEvent* EventNode = NewObject<UK2Node_CustomEvent>(Graph);
	EventNode->CustomFunctionName = FName(*EventName);
	EventNode->NodePosX = (int32)Position.X;
	EventNode->NodePosY = (int32)Position.Y;
	EventNode->FunctionFlags = FUNC_BlueprintCallable | FUNC_BlueprintEvent | FUNC_Net;
	if (RPCType.Equals(TEXT("RunOnServer"), ESearchCase::IgnoreCase) || RPCType.Equals(TEXT("Server"), ESearchCase::IgnoreCase))
	{
		EventNode->FunctionFlags |= FUNC_NetServer;
	}
	else if (RPCType.Equals(TEXT("Multicast"), ESearchCase::IgnoreCase) || RPCType.Equals(TEXT("NetMulticast"), ESearchCase::IgnoreCase))
	{
		EventNode->FunctionFlags |= FUNC_NetMulticast;
	}
	else
	{
		EventNode->FunctionFlags |= FUNC_NetClient;
	}
	if (bReliable)
	{
		EventNode->FunctionFlags |= FUNC_NetReliable;
	}

	Graph->AddNode(EventNode, true);
	EventNode->CreateNewGuid();
	EventNode->PostPlacedNewNode();
	EventNode->AllocateDefaultPins();

	FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
	FKismetEditorUtilities::CompileBlueprint(Blueprint);
	Blueprint->MarkPackageDirty();

	TSharedPtr<FJsonObject> Data = MakeShared<FJsonObject>();
	Data->SetStringField(TEXT("blueprint_name"), Blueprint->GetName());
	Data->SetStringField(TEXT("event_name"), EventName);
	Data->SetStringField(TEXT("rpc_type"), RPCType);
	Data->SetBoolField(TEXT("reliable"), bReliable);
	Data->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
	return FUnrealMCPCommonUtils::CreateSuccessResponse(Data);
}
