#include "Commands/UnrealMCPSequencerCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Editor.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "LevelSequence.h"
#include "MovieScene.h"
#include "Misc/PackageName.h"
#include "Tracks/MovieScene3DTransformTrack.h"
#include "Sections/MovieScene3DTransformSection.h"
#include "Tracks/MovieSceneCameraCutTrack.h"
#include "Sections/MovieSceneCameraCutSection.h"
#include "MovieSceneObjectBindingID.h"
#include "Channels/MovieSceneDoubleChannel.h"
#include "Channels/MovieSceneChannelProxy.h"

FUnrealMCPSequencerCommands::FUnrealMCPSequencerCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPSequencerCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_level_sequence"))
    {
        return HandleCreateLevelSequence(Params);
    }
    else if (CommandType == TEXT("add_actor_to_sequence"))
    {
        return HandleAddActorToSequence(Params);
    }
    else if (CommandType == TEXT("add_transform_track"))
    {
        return HandleAddTransformTrack(Params);
    }
    else if (CommandType == TEXT("add_transform_keyframe"))
    {
        return HandleAddTransformKeyframe(Params);
    }
    else if (CommandType == TEXT("get_sequences_in_level"))
    {
        return HandleGetSequencesInLevel(Params);
    }
    else if (CommandType == TEXT("add_camera_cut_track"))
    {
        return HandleAddCameraCutTrack(Params);
    }
    else if (CommandType == TEXT("add_camera_cut"))
    {
        return HandleAddCameraCut(Params);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown sequencer command: %s"), *CommandType));
}

namespace
{
    ULevelSequence* LoadLevelSequenceByName(const FString& Name)
    {
        const TArray<FString> Roots = { TEXT("/Game/Sequences/"), TEXT("/Game/") };
        for (const FString& Root : Roots)
        {
            FString FullPath = Root + Name + TEXT(".") + Name;
            if (ULevelSequence* Seq = LoadObject<ULevelSequence>(nullptr, *FullPath))
            {
                return Seq;
            }
        }

        // Try AssetRegistry by name
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        TArray<FAssetData> Assets;
        AssetRegistryModule.Get().GetAssetsByClass(ULevelSequence::StaticClass()->GetClassPathName(), Assets, true);
        for (const FAssetData& Data : Assets)
        {
            if (Data.AssetName.ToString() == Name)
            {
                return Cast<ULevelSequence>(Data.GetAsset());
            }
        }
        return nullptr;
    }

    AActor* FindActorByName(UWorld* World, const FString& Name)
    {
        if (!World) return nullptr;
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            if (It->GetName() == Name || It->GetActorLabel() == Name)
            {
                return *It;
            }
        }
        return nullptr;
    }
}

TSharedPtr<FJsonObject> FUnrealMCPSequencerCommands::HandleCreateLevelSequence(const TSharedPtr<FJsonObject>& Params)
{
    FString SequenceName;
    if (!Params->TryGetStringField(TEXT("sequence_name"), SequenceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'sequence_name' parameter"));
    }

    FString Path = TEXT("/Game/Sequences");
    Params->TryGetStringField(TEXT("path"), Path);

    double Duration = 5.0;
    Params->TryGetNumberField(TEXT("duration"), Duration);

    const FString PackageName = FString::Printf(TEXT("%s/%s"), *Path, *SequenceName);
    UPackage* Package = CreatePackage(*PackageName);
    ULevelSequence* Sequence = NewObject<ULevelSequence>(
        Package,
        ULevelSequence::StaticClass(),
        FName(*SequenceName),
        RF_Public | RF_Standalone | RF_Transactional);
    if (!Sequence)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create level sequence '%s'"), *SequenceName));
    }
    Sequence->Initialize();

    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (MovieScene)
    {
        const FFrameRate TickResolution = MovieScene->GetTickResolution();
        const FFrameNumber StartFrame = FFrameNumber(0);
        const FFrameNumber EndFrame = (Duration * TickResolution).RoundToFrame();
        MovieScene->SetPlaybackRange(StartFrame, (EndFrame - StartFrame).Value);
    }

    FAssetRegistryModule::AssetCreated(Sequence);
    Package->MarkPackageDirty();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("name"), SequenceName);
    Result->SetStringField(TEXT("path"), PackageName);
    Result->SetNumberField(TEXT("duration"), Duration);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSequencerCommands::HandleAddActorToSequence(const TSharedPtr<FJsonObject>& Params)
{
    FString SequenceName;
    if (!Params->TryGetStringField(TEXT("sequence_name"), SequenceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'sequence_name' parameter"));
    }

    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    ULevelSequence* Sequence = LoadLevelSequenceByName(SequenceName);
    if (!Sequence)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Sequence not found: %s"), *SequenceName));
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    AActor* Actor = FindActorByName(World, ActorName);
    if (!Actor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (!MovieScene)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Sequence has no MovieScene"));
    }

    const FGuid ObjectGuid = MovieScene->AddPossessable(Actor->GetActorLabel(), Actor->GetClass());
    Sequence->BindPossessableObject(ObjectGuid, *Actor, World);
    Sequence->GetOutermost()->MarkPackageDirty();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("sequence_name"), SequenceName);
    Result->SetStringField(TEXT("actor_name"), ActorName);
    Result->SetStringField(TEXT("binding_id"), ObjectGuid.ToString());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSequencerCommands::HandleAddTransformTrack(const TSharedPtr<FJsonObject>& Params)
{
    FString SequenceName;
    if (!Params->TryGetStringField(TEXT("sequence_name"), SequenceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'sequence_name' parameter"));
    }

    FString BindingIdStr;
    if (!Params->TryGetStringField(TEXT("binding_id"), BindingIdStr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'binding_id' parameter"));
    }

    FGuid BindingGuid;
    if (!FGuid::Parse(BindingIdStr, BindingGuid))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Invalid 'binding_id' GUID"));
    }

    ULevelSequence* Sequence = LoadLevelSequenceByName(SequenceName);
    if (!Sequence)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Sequence not found: %s"), *SequenceName));
    }

    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (!MovieScene)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Sequence has no MovieScene"));
    }

    UMovieScene3DTransformTrack* Track = MovieScene->AddTrack<UMovieScene3DTransformTrack>(BindingGuid);
    if (!Track)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to add transform track"));
    }

    UMovieSceneSection* NewSection = Track->CreateNewSection();
    NewSection->SetRange(MovieScene->GetPlaybackRange());
    Track->AddSection(*NewSection);
    Sequence->GetOutermost()->MarkPackageDirty();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("sequence_name"), SequenceName);
    Result->SetStringField(TEXT("binding_id"), BindingIdStr);
    Result->SetBoolField(TEXT("track_added"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSequencerCommands::HandleAddTransformKeyframe(const TSharedPtr<FJsonObject>& Params)
{
    FString SequenceName;
    if (!Params->TryGetStringField(TEXT("sequence_name"), SequenceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'sequence_name' parameter"));
    }

    FString BindingIdStr;
    if (!Params->TryGetStringField(TEXT("binding_id"), BindingIdStr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'binding_id' parameter"));
    }

    FGuid BindingGuid;
    if (!FGuid::Parse(BindingIdStr, BindingGuid))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Invalid 'binding_id' GUID"));
    }

    double Time = 0.0;
    if (!Params->TryGetNumberField(TEXT("time"), Time))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'time' parameter"));
    }

    FVector Location = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("location"));
    FRotator Rotation = FUnrealMCPCommonUtils::GetRotatorFromJson(Params, TEXT("rotation"));
    FVector Scale = FVector::OneVector;
    if (Params->HasField(TEXT("scale")))
    {
        Scale = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("scale"));
    }

    ULevelSequence* Sequence = LoadLevelSequenceByName(SequenceName);
    if (!Sequence)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Sequence not found: %s"), *SequenceName));
    }

    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (!MovieScene)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Sequence has no MovieScene"));
    }

    UMovieScene3DTransformTrack* Track = Cast<UMovieScene3DTransformTrack>(MovieScene->FindTrack(UMovieScene3DTransformTrack::StaticClass(), BindingGuid));
    if (!Track)
    {
        Track = MovieScene->AddTrack<UMovieScene3DTransformTrack>(BindingGuid);
        if (!Track)
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create transform track"));
        }
    }

    UMovieScene3DTransformSection* Section = nullptr;
    if (Track->GetAllSections().Num() > 0)
    {
        Section = Cast<UMovieScene3DTransformSection>(Track->GetAllSections()[0]);
    }
    if (!Section)
    {
        Section = Cast<UMovieScene3DTransformSection>(Track->CreateNewSection());
        if (!Section)
        {
            return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create transform section"));
        }
        Section->SetRange(MovieScene->GetPlaybackRange());
        Track->AddSection(*Section);
    }

    const FFrameRate TickResolution = MovieScene->GetTickResolution();
    const FFrameNumber Frame = (Time * TickResolution).RoundToFrame();

    TArrayView<FMovieSceneDoubleChannel*> DoubleChannels = Section->GetChannelProxy().GetChannels<FMovieSceneDoubleChannel>();
    if (DoubleChannels.Num() < 9)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Transform section does not expose 9 double channels"));
    }

    DoubleChannels[0]->AddCubicKey(Frame, Location.X);
    DoubleChannels[1]->AddCubicKey(Frame, Location.Y);
    DoubleChannels[2]->AddCubicKey(Frame, Location.Z);
    DoubleChannels[3]->AddCubicKey(Frame, Rotation.Roll);
    DoubleChannels[4]->AddCubicKey(Frame, Rotation.Pitch);
    DoubleChannels[5]->AddCubicKey(Frame, Rotation.Yaw);
    DoubleChannels[6]->AddCubicKey(Frame, Scale.X);
    DoubleChannels[7]->AddCubicKey(Frame, Scale.Y);
    DoubleChannels[8]->AddCubicKey(Frame, Scale.Z);

    Sequence->GetOutermost()->MarkPackageDirty();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("sequence_name"), SequenceName);
    Result->SetStringField(TEXT("binding_id"), BindingIdStr);
    Result->SetNumberField(TEXT("time"), Time);
    Result->SetNumberField(TEXT("frame"), Frame.Value);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSequencerCommands::HandleGetSequencesInLevel(const TSharedPtr<FJsonObject>& Params)
{
    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> Assets;
    AssetRegistry.GetAssetsByClass(ULevelSequence::StaticClass()->GetClassPathName(), Assets, true);

    TArray<TSharedPtr<FJsonValue>> SeqArray;
    for (const FAssetData& Data : Assets)
    {
        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetStringField(TEXT("name"), Data.AssetName.ToString());
        Obj->SetStringField(TEXT("path"), Data.GetObjectPathString());

        if (ULevelSequence* Seq = Cast<ULevelSequence>(Data.GetAsset()))
        {
            if (UMovieScene* MS = Seq->GetMovieScene())
            {
                const FFrameRate Tick = MS->GetTickResolution();
                const TRange<FFrameNumber> Range = MS->GetPlaybackRange();
                const double DurationSec = (Range.Size<FFrameNumber>().Value) / Tick.AsDecimal();
                Obj->SetNumberField(TEXT("duration"), DurationSec);
            }
        }
        SeqArray.Add(MakeShared<FJsonValueObject>(Obj));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetArrayField(TEXT("sequences"), SeqArray);
    Result->SetNumberField(TEXT("count"), SeqArray.Num());
    return Result;
}

// ---------------------------------------------------------------------------
// Camera Cut Track
// ---------------------------------------------------------------------------

TSharedPtr<FJsonObject> FUnrealMCPSequencerCommands::HandleAddCameraCutTrack(const TSharedPtr<FJsonObject>& Params)
{
    FString SequenceName;
    if (!Params->TryGetStringField(TEXT("sequence_name"), SequenceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'sequence_name' parameter"));
    }

    ULevelSequence* Sequence = LoadLevelSequenceByName(SequenceName);
    if (!Sequence)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Sequence not found: %s"), *SequenceName));
    }

    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (!MovieScene)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Sequence has no MovieScene"));
    }

    // Only one camera cut track is allowed per sequence
    UMovieSceneCameraCutTrack* CutTrack = Cast<UMovieSceneCameraCutTrack>(MovieScene->GetCameraCutTrack());
    if (!CutTrack)
    {
        CutTrack = Cast<UMovieSceneCameraCutTrack>(
            MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass()));
    }

    if (!CutTrack)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create camera cut track"));
    }

    Sequence->GetOutermost()->MarkPackageDirty();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("sequence_name"), SequenceName);
    Result->SetBoolField(TEXT("camera_cut_track_added"), true);
    return Result;
}

// ---------------------------------------------------------------------------
// Camera Cut Section (links a binding to a time range in the cut track)
// ---------------------------------------------------------------------------

TSharedPtr<FJsonObject> FUnrealMCPSequencerCommands::HandleAddCameraCut(const TSharedPtr<FJsonObject>& Params)
{
    FString SequenceName;
    if (!Params->TryGetStringField(TEXT("sequence_name"), SequenceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'sequence_name' parameter"));
    }

    FString CameraBindingIdStr;
    if (!Params->TryGetStringField(TEXT("camera_binding_id"), CameraBindingIdStr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'camera_binding_id' parameter"));
    }

    FGuid CameraBindingGuid;
    if (!FGuid::Parse(CameraBindingIdStr, CameraBindingGuid))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Invalid 'camera_binding_id' GUID"));
    }

    double StartTime = 0.0;
    Params->TryGetNumberField(TEXT("start_time"), StartTime);

    double EndTime = -1.0;
    Params->TryGetNumberField(TEXT("end_time"), EndTime);

    ULevelSequence* Sequence = LoadLevelSequenceByName(SequenceName);
    if (!Sequence)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Sequence not found: %s"), *SequenceName));
    }

    UMovieScene* MovieScene = Sequence->GetMovieScene();
    if (!MovieScene)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Sequence has no MovieScene"));
    }

    UMovieSceneCameraCutTrack* CutTrack = Cast<UMovieSceneCameraCutTrack>(MovieScene->GetCameraCutTrack());
    if (!CutTrack)
    {
        CutTrack = Cast<UMovieSceneCameraCutTrack>(
            MovieScene->AddCameraCutTrack(UMovieSceneCameraCutTrack::StaticClass()));
    }
    if (!CutTrack)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get or create camera cut track"));
    }

    const FFrameRate TickResolution = MovieScene->GetTickResolution();
    const FFrameNumber StartFrame = (StartTime * TickResolution).RoundToFrame();

    FFrameNumber EndFrame;
    if (EndTime >= 0.0)
    {
        EndFrame = (EndTime * TickResolution).RoundToFrame();
    }
    else
    {
        EndFrame = MovieScene->GetPlaybackRange().GetUpperBoundValue();
    }

    UMovieSceneCameraCutSection* CutSection = Cast<UMovieSceneCameraCutSection>(CutTrack->CreateNewSection());
    if (!CutSection)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create camera cut section"));
    }

    CutSection->SetRange(TRange<FFrameNumber>(StartFrame, EndFrame));

    FMovieSceneObjectBindingID BindingID;
    BindingID.SetGuid(CameraBindingGuid);
    CutSection->SetCameraBindingID(BindingID);

    CutTrack->AddSection(*CutSection);
    Sequence->GetOutermost()->MarkPackageDirty();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("sequence_name"), SequenceName);
    Result->SetStringField(TEXT("camera_binding_id"), CameraBindingIdStr);
    Result->SetNumberField(TEXT("start_time"), StartTime);
    Result->SetNumberField(TEXT("end_time"), EndTime >= 0.0 ? EndTime : -1.0);
    return Result;
}
