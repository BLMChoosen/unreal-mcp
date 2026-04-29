#include "Commands/UnrealMCPEditorCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Editor.h"
#include "EditorViewportClient.h"
#include "LevelEditorViewport.h"
#include "ImageUtils.h"
#include "HighResScreenshot.h"
#include "Engine/GameViewportClient.h"
#include "Misc/FileHelper.h"
#include "GameFramework/Actor.h"
#include "Engine/Selection.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Camera/CameraActor.h"
#include "Components/StaticMeshComponent.h"
#include "EditorSubsystem.h"
#include "Subsystems/EditorActorSubsystem.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EngineUtils.h"
#include "Editor/UnrealEdEngine.h"
#include "UnrealEdGlobals.h"
#include "ShowFlags.h"

FUnrealMCPEditorCommands::FUnrealMCPEditorCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    // Actor manipulation commands
    if (CommandType == TEXT("get_actors_in_level"))
    {
        return HandleGetActorsInLevel(Params);
    }
    else if (CommandType == TEXT("find_actors_by_name"))
    {
        return HandleFindActorsByName(Params);
    }
    else if (CommandType == TEXT("spawn_actor") || CommandType == TEXT("create_actor"))
    {
        if (CommandType == TEXT("create_actor"))
        {
            UE_LOG(LogTemp, Warning, TEXT("'create_actor' command is deprecated and will be removed in a future version. Please use 'spawn_actor' instead."));
        }
        return HandleSpawnActor(Params);
    }
    else if (CommandType == TEXT("delete_actor"))
    {
        return HandleDeleteActor(Params);
    }
    else if (CommandType == TEXT("set_actor_transform"))
    {
        return HandleSetActorTransform(Params);
    }
    else if (CommandType == TEXT("get_actor_properties"))
    {
        return HandleGetActorProperties(Params);
    }
    else if (CommandType == TEXT("set_actor_property"))
    {
        return HandleSetActorProperty(Params);
    }
    // Blueprint actor spawning
    else if (CommandType == TEXT("spawn_blueprint_actor"))
    {
        return HandleSpawnBlueprintActor(Params);
    }
    // Editor viewport commands
    else if (CommandType == TEXT("focus_viewport"))
    {
        return HandleFocusViewport(Params);
    }
    else if (CommandType == TEXT("take_screenshot"))
    {
        return HandleTakeScreenshot(Params);
    }
    // Play In Editor
    else if (CommandType == TEXT("start_play_in_editor"))
    {
        return HandleStartPlayInEditor(Params);
    }
    else if (CommandType == TEXT("stop_play_in_editor"))
    {
        return HandleStopPlayInEditor(Params);
    }
    else if (CommandType == TEXT("get_play_in_editor_status"))
    {
        return HandleGetPlayInEditorStatus(Params);
    }
    // Batch actor operations
    else if (CommandType == TEXT("get_actors_by_tag"))
    {
        return HandleGetActorsByTag(Params);
    }
    else if (CommandType == TEXT("set_actors_transform_by_tag"))
    {
        return HandleSetActorsTransformByTag(Params);
    }
    else if (CommandType == TEXT("add_tag_to_actors_by_name"))
    {
        return HandleAddTagToActorsByName(Params);
    }
    else if (CommandType == TEXT("set_actor_property_batch"))
    {
        return HandleSetActorPropertyBatch(Params);
    }
    else if (CommandType == TEXT("select_actors_by_tag"))
    {
        return HandleSelectActorsByTag(Params);
    }
    // Viewport display mode
    else if (CommandType == TEXT("set_viewport_display_mode"))
    {
        return HandleSetViewportDisplayMode(Params);
    }
    else if (CommandType == TEXT("get_viewport_display_mode"))
    {
        return HandleGetViewportDisplayMode(Params);
    }
    else if (CommandType == TEXT("set_viewport_show_flags"))
    {
        return HandleSetViewportShowFlags(Params);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown editor command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleGetActorsInLevel(const TSharedPtr<FJsonObject>& Params)
{
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    TArray<TSharedPtr<FJsonValue>> ActorArray;
    for (AActor* Actor : AllActors)
    {
        if (Actor)
        {
            ActorArray.Add(FUnrealMCPCommonUtils::ActorToJson(Actor));
        }
    }
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("actors"), ActorArray);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleFindActorsByName(const TSharedPtr<FJsonObject>& Params)
{
    FString Pattern;
    if (!Params->TryGetStringField(TEXT("pattern"), Pattern))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'pattern' parameter"));
    }
    
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    TArray<TSharedPtr<FJsonValue>> MatchingActors;
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName().Contains(Pattern))
        {
            MatchingActors.Add(FUnrealMCPCommonUtils::ActorToJson(Actor));
        }
    }
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetArrayField(TEXT("actors"), MatchingActors);
    
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSpawnActor(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString ActorType;
    if (!Params->TryGetStringField(TEXT("type"), ActorType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'type' parameter"));
    }

    // Get actor name (required parameter)
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Get optional transform parameters
    FVector Location(0.0f, 0.0f, 0.0f);
    FRotator Rotation(0.0f, 0.0f, 0.0f);
    FVector Scale(1.0f, 1.0f, 1.0f);

    if (Params->HasField(TEXT("location")))
    {
        Location = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        Rotation = FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"));
    }
    if (Params->HasField(TEXT("scale")))
    {
        Scale = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale"));
    }

    // Create the actor based on type
    AActor* NewActor = nullptr;
    UWorld* World = GEditor->GetEditorWorldContext().World();

    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    // Check if an actor with this name already exists
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(World, AActor::StaticClass(), AllActors);
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor with name '%s' already exists"), *ActorName));
        }
    }

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = *ActorName;

    if (ActorType == TEXT("StaticMeshActor"))
    {
        NewActor = World->SpawnActor<AStaticMeshActor>(AStaticMeshActor::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("PointLight"))
    {
        NewActor = World->SpawnActor<APointLight>(APointLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("SpotLight"))
    {
        NewActor = World->SpawnActor<ASpotLight>(ASpotLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("DirectionalLight"))
    {
        NewActor = World->SpawnActor<ADirectionalLight>(ADirectionalLight::StaticClass(), Location, Rotation, SpawnParams);
    }
    else if (ActorType == TEXT("CameraActor"))
    {
        NewActor = World->SpawnActor<ACameraActor>(ACameraActor::StaticClass(), Location, Rotation, SpawnParams);
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown actor type: %s"), *ActorType));
    }

    if (NewActor)
    {
        // Set scale (since SpawnActor only takes location and rotation)
        FTransform Transform = NewActor->GetTransform();
        Transform.SetScale3D(Scale);
        NewActor->SetActorTransform(Transform);

        // Return the created actor's details
        return FUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create actor"));
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleDeleteActor(const TSharedPtr<FJsonObject>& Params)
{
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            // Store actor info before deletion for the response
            TSharedPtr<FJsonObject> ActorInfo = FUnrealMCPCommonUtils::ActorToJsonObject(Actor);
            
            // Delete the actor
            Actor->Destroy();
            
            TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
            ResultObj->SetObjectField(TEXT("deleted_actor"), ActorInfo);
            return ResultObj;
        }
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSetActorTransform(const TSharedPtr<FJsonObject>& Params)
{
    // Get actor name
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            TargetActor = Actor;
            break;
        }
    }

    if (!TargetActor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Get transform parameters
    FTransform NewTransform = TargetActor->GetTransform();

    if (Params->HasField(TEXT("location")))
    {
        NewTransform.SetLocation(FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location")));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        NewTransform.SetRotation(FQuat(FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"))));
    }
    if (Params->HasField(TEXT("scale")))
    {
        NewTransform.SetScale3D(FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale")));
    }

    // Set the new transform
    TargetActor->SetActorTransform(NewTransform);

    // Return updated actor info
    return FUnrealMCPCommonUtils::ActorToJsonObject(TargetActor, true);
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleGetActorProperties(const TSharedPtr<FJsonObject>& Params)
{
    // Get actor name
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            TargetActor = Actor;
            break;
        }
    }

    if (!TargetActor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Always return detailed properties for this command
    return FUnrealMCPCommonUtils::ActorToJsonObject(TargetActor, true);
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSetActorProperty(const TSharedPtr<FJsonObject>& Params)
{
    // Get actor name
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    // Find the actor
    AActor* TargetActor = nullptr;
    TArray<AActor*> AllActors;
    UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
    
    for (AActor* Actor : AllActors)
    {
        if (Actor && Actor->GetName() == ActorName)
        {
            TargetActor = Actor;
            break;
        }
    }

    if (!TargetActor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    // Get property name
    FString PropertyName;
    if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_name' parameter"));
    }

    // Get property value
    if (!Params->HasField(TEXT("property_value")))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_value' parameter"));
    }
    
    TSharedPtr<FJsonValue> PropertyValue = Params->Values.FindRef(TEXT("property_value"));
    
    // Set the property using our utility function
    FString ErrorMessage;
    if (FUnrealMCPCommonUtils::SetObjectProperty(TargetActor, PropertyName, PropertyValue, ErrorMessage))
    {
        // Property set successfully
        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("actor"), ActorName);
        ResultObj->SetStringField(TEXT("property"), PropertyName);
        ResultObj->SetBoolField(TEXT("success"), true);
        
        // Also include the full actor details
        ResultObj->SetObjectField(TEXT("actor_details"), FUnrealMCPCommonUtils::ActorToJsonObject(TargetActor, true));
        return ResultObj;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(ErrorMessage);
    }
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSpawnBlueprintActor(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    // Find the blueprint
    if (BlueprintName.IsEmpty())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint name is empty"));
    }

    FString Root      = TEXT("/Game/Blueprints/");
    FString AssetPath = Root + BlueprintName;

    if (!FPackageName::DoesPackageExist(AssetPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint '%s' not found – it must reside under /Game/Blueprints"), *BlueprintName));
    }

    UBlueprint* Blueprint = LoadObject<UBlueprint>(nullptr, *AssetPath);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get transform parameters
    FVector Location(0.0f, 0.0f, 0.0f);
    FRotator Rotation(0.0f, 0.0f, 0.0f);
    FVector Scale(1.0f, 1.0f, 1.0f);

    if (Params->HasField(TEXT("location")))
    {
        Location = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    }
    if (Params->HasField(TEXT("rotation")))
    {
        Rotation = FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"));
    }
    if (Params->HasField(TEXT("scale")))
    {
        Scale = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale"));
    }

    // Spawn the actor
    UWorld* World = GEditor->GetEditorWorldContext().World();
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get editor world"));
    }

    FTransform SpawnTransform;
    SpawnTransform.SetLocation(Location);
    SpawnTransform.SetRotation(FQuat(Rotation));
    SpawnTransform.SetScale3D(Scale);

    FActorSpawnParameters SpawnParams;
    SpawnParams.Name = *ActorName;

    AActor* NewActor = World->SpawnActor<AActor>(Blueprint->GeneratedClass, SpawnTransform, SpawnParams);
    if (NewActor)
    {
        return FUnrealMCPCommonUtils::ActorToJsonObject(NewActor, true);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to spawn blueprint actor"));
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleFocusViewport(const TSharedPtr<FJsonObject>& Params)
{
    // Get target actor name if provided
    FString TargetActorName;
    bool HasTargetActor = Params->TryGetStringField(TEXT("target"), TargetActorName);

    // Get location if provided
    FVector Location(0.0f, 0.0f, 0.0f);
    bool HasLocation = false;
    if (Params->HasField(TEXT("location")))
    {
        Location = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
        HasLocation = true;
    }

    // Get distance
    float Distance = 1000.0f;
    if (Params->HasField(TEXT("distance")))
    {
        Distance = Params->GetNumberField(TEXT("distance"));
    }

    // Get orientation if provided
    FRotator Orientation(0.0f, 0.0f, 0.0f);
    bool HasOrientation = false;
    if (Params->HasField(TEXT("orientation")))
    {
        Orientation = FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("orientation"));
        HasOrientation = true;
    }

    // Get the active viewport
    FLevelEditorViewportClient* ViewportClient = (FLevelEditorViewportClient*)GEditor->GetActiveViewport()->GetClient();
    if (!ViewportClient)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get active viewport"));
    }

    // If we have a target actor, focus on it
    if (HasTargetActor)
    {
        // Find the actor
        AActor* TargetActor = nullptr;
        TArray<AActor*> AllActors;
        UGameplayStatics::GetAllActorsOfClass(GWorld, AActor::StaticClass(), AllActors);
        
        for (AActor* Actor : AllActors)
        {
            if (Actor && Actor->GetName() == TargetActorName)
            {
                TargetActor = Actor;
                break;
            }
        }

        if (!TargetActor)
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *TargetActorName));
        }

        // Focus on the actor
        ViewportClient->SetViewLocation(TargetActor->GetActorLocation() - FVector(Distance, 0.0f, 0.0f));
    }
    // Otherwise use the provided location
    else if (HasLocation)
    {
        ViewportClient->SetViewLocation(Location - FVector(Distance, 0.0f, 0.0f));
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Either 'target' or 'location' must be provided"));
    }

    // Set orientation if provided
    if (HasOrientation)
    {
        ViewportClient->SetViewRotation(Orientation);
    }

    // Force viewport to redraw
    ViewportClient->Invalidate();

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleTakeScreenshot(const TSharedPtr<FJsonObject>& Params)
{
    // Get file path parameter
    FString FilePath;
    if (!Params->TryGetStringField(TEXT("filepath"), FilePath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'filepath' parameter"));
    }
    
    // Ensure the file path has a proper extension
    if (!FilePath.EndsWith(TEXT(".png")))
    {
        FilePath += TEXT(".png");
    }

    // Get the active viewport
    if (GEditor && GEditor->GetActiveViewport())
    {
        FViewport* Viewport = GEditor->GetActiveViewport();
        TArray<FColor> Bitmap;
        FIntRect ViewportRect(0, 0, Viewport->GetSizeXY().X, Viewport->GetSizeXY().Y);
        
        if (Viewport->ReadPixels(Bitmap, FReadSurfaceDataFlags(), ViewportRect))
        {
            TArray<uint8> CompressedBitmap;
            FImageUtils::CompressImageArray(Viewport->GetSizeXY().X, Viewport->GetSizeXY().Y, Bitmap, CompressedBitmap);
            
            if (FFileHelper::SaveArrayToFile(CompressedBitmap, *FilePath))
            {
                TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
                ResultObj->SetStringField(TEXT("filepath"), FilePath);
                return ResultObj;
            }
        }
    }
    
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to take screenshot"));
}

// ===== Play In Editor =====

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleStartPlayInEditor(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("GEditor unavailable"));
    }

    if (GEditor->PlayWorld != nullptr)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Play session already running"));
    }

    bool bMobilePreview = false;
    if (Params->HasField(TEXT("mobile_preview")))
    {
        bMobilePreview = Params->GetBoolField(TEXT("mobile_preview"));
    }
    bool bSimulate = false;
    if (Params->HasField(TEXT("simulate")))
    {
        bSimulate = Params->GetBoolField(TEXT("simulate"));
    }

    FRequestPlaySessionParams PlayParams;
    PlayParams.WorldType = bSimulate ? EPlaySessionWorldType::SimulateInEditor : EPlaySessionWorldType::PlayInEditor;
    if (bMobilePreview)
    {
        PlayParams.SessionPreviewTypeOverride = EPlaySessionPreviewType::MobilePreview;
    }
    GEditor->RequestPlaySession(PlayParams);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("requested"), true);
    Result->SetBoolField(TEXT("simulate"), bSimulate);
    Result->SetBoolField(TEXT("mobile_preview"), bMobilePreview);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleStopPlayInEditor(const TSharedPtr<FJsonObject>& Params)
{
    if (!GEditor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("GEditor unavailable"));
    }

    GEditor->RequestEndPlayMap();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("stopped"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleGetPlayInEditorStatus(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("is_playing"), GEditor && GEditor->PlayWorld != nullptr);
    return Result;
}

// ===== Batch actor operations =====

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleGetActorsByTag(const TSharedPtr<FJsonObject>& Params)
{
    FString Tag;
    if (!Params->TryGetStringField(TEXT("tag"), Tag))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'tag' parameter"));
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : GWorld;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world"));
    }

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsWithTag(World, FName(*Tag), Actors);

    TArray<TSharedPtr<FJsonValue>> ActorArray;
    for (AActor* Actor : Actors)
    {
        if (Actor)
        {
            ActorArray.Add(FUnrealMCPCommonUtils::ActorToJson(Actor));
        }
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetArrayField(TEXT("actors"), ActorArray);
    Result->SetNumberField(TEXT("count"), ActorArray.Num());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSetActorsTransformByTag(const TSharedPtr<FJsonObject>& Params)
{
    FString Tag;
    if (!Params->TryGetStringField(TEXT("tag"), Tag))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'tag' parameter"));
    }

    bool bRelative = false;
    if (Params->HasField(TEXT("relative")))
    {
        bRelative = Params->GetBoolField(TEXT("relative"));
    }

    const bool bHasLocation = Params->HasField(TEXT("location"));
    const bool bHasRotation = Params->HasField(TEXT("rotation"));
    const bool bHasScale = Params->HasField(TEXT("scale"));

    FVector Location = bHasLocation ? FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location")) : FVector::ZeroVector;
    FRotator Rotation = bHasRotation ? FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation")) : FRotator::ZeroRotator;
    FVector Scale = bHasScale ? FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale")) : FVector::OneVector;

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : GWorld;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world"));
    }

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsWithTag(World, FName(*Tag), Actors);

    int32 Count = 0;
    for (AActor* Actor : Actors)
    {
        if (!Actor) continue;
        if (bRelative)
        {
            if (bHasLocation) Actor->SetActorLocation(Actor->GetActorLocation() + Location);
            if (bHasRotation) Actor->SetActorRotation(Actor->GetActorRotation() + Rotation);
            if (bHasScale)    Actor->SetActorScale3D(Actor->GetActorScale3D() * Scale);
        }
        else
        {
            if (bHasLocation) Actor->SetActorLocation(Location);
            if (bHasRotation) Actor->SetActorRotation(Rotation);
            if (bHasScale)    Actor->SetActorScale3D(Scale);
        }
        ++Count;
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetNumberField(TEXT("count"), Count);
    Result->SetStringField(TEXT("tag"), Tag);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleAddTagToActorsByName(const TSharedPtr<FJsonObject>& Params)
{
    FString Pattern;
    if (!Params->TryGetStringField(TEXT("pattern"), Pattern))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'pattern' parameter"));
    }
    FString Tag;
    if (!Params->TryGetStringField(TEXT("tag"), Tag))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'tag' parameter"));
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : GWorld;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world"));
    }

    int32 Count = 0;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        AActor* Actor = *It;
        if (Actor && (Actor->GetName().Contains(Pattern) || Actor->GetActorLabel().Contains(Pattern)))
        {
            Actor->Tags.AddUnique(FName(*Tag));
            ++Count;
        }
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetNumberField(TEXT("count"), Count);
    Result->SetStringField(TEXT("pattern"), Pattern);
    Result->SetStringField(TEXT("tag"), Tag);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSetActorPropertyBatch(const TSharedPtr<FJsonObject>& Params)
{
    const TArray<TSharedPtr<FJsonValue>>* NamesArrayPtr = nullptr;
    if (!Params->TryGetArrayField(TEXT("actor_names"), NamesArrayPtr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_names' array"));
    }

    FString PropertyName;
    if (!Params->TryGetStringField(TEXT("property_name"), PropertyName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_name' parameter"));
    }

    TSharedPtr<FJsonValue> ValueField = Params->TryGetField(TEXT("property_value"));
    if (!ValueField.IsValid())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'property_value' parameter"));
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : GWorld;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active world"));
    }

    TArray<TSharedPtr<FJsonValue>> Results;
    for (const TSharedPtr<FJsonValue>& NameValue : *NamesArrayPtr)
    {
        const FString Name = NameValue->AsString();

        TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
        Entry->SetStringField(TEXT("actor_name"), Name);

        AActor* Found = nullptr;
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if (It->GetName() == Name || It->GetActorLabel() == Name)
            {
                Found = *It;
                break;
            }
        }

        if (!Found)
        {
            Entry->SetBoolField(TEXT("success"), false);
            Entry->SetStringField(TEXT("error"), TEXT("Actor not found"));
        }
        else
        {
            FString ErrorMessage;
            const bool bOk = FUnrealMCPCommonUtils::SetObjectProperty(Found, PropertyName, ValueField, ErrorMessage);
            Entry->SetBoolField(TEXT("success"), bOk);
            if (!bOk)
            {
                Entry->SetStringField(TEXT("error"), ErrorMessage);
            }
        }
        Results.Add(MakeShared<FJsonValueObject>(Entry));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetArrayField(TEXT("results"), Results);
    Result->SetNumberField(TEXT("count"), Results.Num());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSelectActorsByTag(const TSharedPtr<FJsonObject>& Params)
{
    FString Tag;
    if (!Params->TryGetStringField(TEXT("tag"), Tag))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'tag' parameter"));
    }

    bool bAddToSelection = false;
    if (Params->HasField(TEXT("add_to_selection")))
    {
        bAddToSelection = Params->GetBoolField(TEXT("add_to_selection"));
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : GWorld;
    if (!World || !GEditor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Editor unavailable"));
    }

    if (!bAddToSelection)
    {
        GEditor->SelectNone(true, true, false);
    }

    TArray<AActor*> Actors;
    UGameplayStatics::GetAllActorsWithTag(World, FName(*Tag), Actors);

    int32 Count = 0;
    for (AActor* Actor : Actors)
    {
        if (Actor)
        {
            GEditor->SelectActor(Actor, true, true);
            ++Count;
        }
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetNumberField(TEXT("selected_count"), Count);
    Result->SetStringField(TEXT("tag"), Tag);
    return Result;
}

// ===== Viewport display mode =====

namespace
{
    FLevelEditorViewportClient* GetActiveLevelViewportClient()
    {
        if (!GEditor) return nullptr;
        for (FLevelEditorViewportClient* VC : GEditor->GetLevelViewportClients())
        {
            if (VC && VC->IsPerspective())
            {
                return VC;
            }
        }
        if (GEditor->GetLevelViewportClients().Num() > 0)
        {
            return GEditor->GetLevelViewportClients()[0];
        }
        return nullptr;
    }

    bool ParseViewMode(const FString& Mode, EViewModeIndex& OutIndex)
    {
        static const TMap<FString, EViewModeIndex> Map = {
            { TEXT("Lit"),               VMI_Lit },
            { TEXT("Unlit"),             VMI_Unlit },
            { TEXT("Wireframe"),         VMI_BrushWireframe },
            { TEXT("DetailLighting"),    VMI_Lit_DetailLighting },
            { TEXT("LightingOnly"),      VMI_LightingOnly },
            { TEXT("ReflectionsOnly"),   VMI_ReflectionOverride },
            { TEXT("PathTracing"),       VMI_PathTracing }
        };
        if (const EViewModeIndex* Found = Map.Find(Mode))
        {
            OutIndex = *Found;
            return true;
        }
        return false;
    }

    FString ViewModeToString(EViewModeIndex Index)
    {
        switch (Index)
        {
            case VMI_Lit: return TEXT("Lit");
            case VMI_Unlit: return TEXT("Unlit");
            case VMI_BrushWireframe:
            case VMI_Wireframe: return TEXT("Wireframe");
            case VMI_Lit_DetailLighting: return TEXT("DetailLighting");
            case VMI_LightingOnly: return TEXT("LightingOnly");
            case VMI_ReflectionOverride: return TEXT("ReflectionsOnly");
            case VMI_PathTracing: return TEXT("PathTracing");
            default: return FString::Printf(TEXT("Unknown(%d)"), (int32)Index);
        }
    }
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSetViewportDisplayMode(const TSharedPtr<FJsonObject>& Params)
{
    FString Mode;
    if (!Params->TryGetStringField(TEXT("mode"), Mode))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'mode' parameter"));
    }

    EViewModeIndex Index;
    if (!ParseViewMode(Mode, Index))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown view mode: %s"), *Mode));
    }

    FLevelEditorViewportClient* VC = GetActiveLevelViewportClient();
    if (!VC)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active viewport client"));
    }

    VC->SetViewMode(Index);
    VC->Invalidate(true, true);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("mode"), Mode);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleGetViewportDisplayMode(const TSharedPtr<FJsonObject>& Params)
{
    FLevelEditorViewportClient* VC = GetActiveLevelViewportClient();
    if (!VC)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active viewport client"));
    }

    const EViewModeIndex Index = VC->GetViewMode();
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("mode"), ViewModeToString(Index));
    Result->SetNumberField(TEXT("mode_index"), (int32)Index);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPEditorCommands::HandleSetViewportShowFlags(const TSharedPtr<FJsonObject>& Params)
{
    FString Flag;
    if (!Params->TryGetStringField(TEXT("flag"), Flag))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'flag' parameter"));
    }

    bool bEnabled = true;
    if (Params->HasField(TEXT("enabled")))
    {
        bEnabled = Params->GetBoolField(TEXT("enabled"));
    }

    FLevelEditorViewportClient* VC = GetActiveLevelViewportClient();
    if (!VC)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active viewport client"));
    }

    FEngineShowFlags& Flags = VC->EngineShowFlags;
    if      (Flag.Equals(TEXT("Grid"),       ESearchCase::IgnoreCase)) Flags.SetGrid(bEnabled);
    else if (Flag.Equals(TEXT("Stats"),      ESearchCase::IgnoreCase)) VC->SetShowStats(bEnabled);
    else if (Flag.Equals(TEXT("Bounds"),     ESearchCase::IgnoreCase)) Flags.SetBounds(bEnabled);
    else if (Flag.Equals(TEXT("Collision"),  ESearchCase::IgnoreCase)) Flags.SetCollision(bEnabled);
    else if (Flag.Equals(TEXT("Navigation"), ESearchCase::IgnoreCase)) Flags.SetNavigation(bEnabled);
    else if (Flag.Equals(TEXT("Bloom"),      ESearchCase::IgnoreCase)) Flags.SetBloom(bEnabled);
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported flag: %s"), *Flag));
    }

    VC->Invalidate(true, true);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("flag"), Flag);
    Result->SetBoolField(TEXT("enabled"), bEnabled);
    return Result;
} 
