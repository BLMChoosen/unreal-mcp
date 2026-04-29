#include "Commands/UnrealMCPMaterialCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Engine/StaticMeshActor.h"
#include "Components/StaticMeshComponent.h"
#include "Materials/Material.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceConstant.h"
#include "Materials/MaterialExpressionScalarParameter.h"
#include "Materials/MaterialExpressionVectorParameter.h"
#include "Factories/MaterialFactoryNew.h"
#include "Factories/MaterialInstanceConstantFactoryNew.h"
#include "MaterialEditingLibrary.h"
#include "UObject/Package.h"
#include "Kismet/GameplayStatics.h"
#include "EngineUtils.h"

FUnrealMCPMaterialCommands::FUnrealMCPMaterialCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_material"))
    {
        return HandleCreateMaterial(Params);
    }
    else if (CommandType == TEXT("create_material_instance"))
    {
        return HandleCreateMaterialInstance(Params);
    }
    else if (CommandType == TEXT("set_material_scalar_parameter"))
    {
        return HandleSetMaterialScalarParameter(Params);
    }
    else if (CommandType == TEXT("set_material_vector_parameter"))
    {
        return HandleSetMaterialVectorParameter(Params);
    }
    else if (CommandType == TEXT("apply_material_to_actor"))
    {
        return HandleApplyMaterialToActor(Params);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown material command: %s"), *CommandType));
}

namespace
{
    // Locate a material/instance by name. Searches /Game/Materials first then any subpath.
    UMaterialInterface* FindMaterialByName(const FString& Name)
    {
        // Try direct paths
        const TArray<FString> Roots = { TEXT("/Game/Materials/"), TEXT("/Game/") };
        for (const FString& Root : Roots)
        {
            FString FullPath = Root + Name + TEXT(".") + Name;
            UMaterialInterface* Mat = LoadObject<UMaterialInterface>(nullptr, *FullPath);
            if (Mat)
            {
                return Mat;
            }
        }

        // Fall back to AssetRegistry scan
        FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
        TArray<FAssetData> Assets;
        AssetRegistryModule.Get().GetAssetsByPackageName(FName(*FString::Printf(TEXT("/Game/Materials/%s"), *Name)), Assets);
        for (const FAssetData& Data : Assets)
        {
            if (UObject* Obj = Data.GetAsset())
            {
                if (UMaterialInterface* Mat = Cast<UMaterialInterface>(Obj))
                {
                    return Mat;
                }
            }
        }
        return nullptr;
    }
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleCreateMaterial(const TSharedPtr<FJsonObject>& Params)
{
    FString Name;
    if (!Params->TryGetStringField(TEXT("name"), Name))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    FString Path = TEXT("/Game/Materials");
    Params->TryGetStringField(TEXT("path"), Path);

    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    UMaterialFactoryNew* Factory = NewObject<UMaterialFactoryNew>();

    UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, UMaterial::StaticClass(), Factory);
    if (!NewAsset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create material '%s' at '%s'"), *Name, *Path));
    }

    UPackage* Package = NewAsset->GetOutermost();
    Package->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(NewAsset);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("name"), Name);
    Result->SetStringField(TEXT("path"), FString::Printf(TEXT("%s/%s"), *Path, *Name));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleCreateMaterialInstance(const TSharedPtr<FJsonObject>& Params)
{
    FString InstanceName;
    if (!Params->TryGetStringField(TEXT("instance_name"), InstanceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'instance_name' parameter"));
    }

    FString ParentName;
    if (!Params->TryGetStringField(TEXT("parent_material_name"), ParentName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parent_material_name' parameter"));
    }

    FString Path = TEXT("/Game/Materials");
    Params->TryGetStringField(TEXT("path"), Path);

    UMaterialInterface* Parent = FindMaterialByName(ParentName);
    if (!Parent)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Parent material not found: %s"), *ParentName));
    }

    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    UMaterialInstanceConstantFactoryNew* Factory = NewObject<UMaterialInstanceConstantFactoryNew>();
    Factory->InitialParent = Parent;

    UObject* NewAsset = AssetToolsModule.Get().CreateAsset(InstanceName, Path, UMaterialInstanceConstant::StaticClass(), Factory);
    if (!NewAsset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create material instance '%s'"), *InstanceName));
    }

    NewAsset->GetOutermost()->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(NewAsset);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("name"), InstanceName);
    Result->SetStringField(TEXT("path"), FString::Printf(TEXT("%s/%s"), *Path, *InstanceName));
    Result->SetStringField(TEXT("parent"), ParentName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleSetMaterialScalarParameter(const TSharedPtr<FJsonObject>& Params)
{
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    FString ParameterName;
    if (!Params->TryGetStringField(TEXT("parameter_name"), ParameterName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parameter_name' parameter"));
    }

    double Value = 0.0;
    if (!Params->TryGetNumberField(TEXT("value"), Value))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'value' parameter"));
    }

    UMaterialInterface* MaterialAsset = FindMaterialByName(MaterialName);
    if (!MaterialAsset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialName));
    }

    if (UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(MaterialAsset))
    {
        UMaterialEditingLibrary::SetMaterialInstanceScalarParameterValue(MIC, FName(*ParameterName), static_cast<float>(Value));
        UMaterialEditingLibrary::UpdateMaterialInstance(MIC);
        MIC->GetOutermost()->MarkPackageDirty();
    }
    else if (UMaterial* Material = Cast<UMaterial>(MaterialAsset))
    {
        bool bFound = false;
        for (UMaterialExpression* Expr : Material->GetExpressions())
        {
            if (UMaterialExpressionScalarParameter* ScalarExpr = Cast<UMaterialExpressionScalarParameter>(Expr))
            {
                if (ScalarExpr->ParameterName == FName(*ParameterName))
                {
                    ScalarExpr->DefaultValue = static_cast<float>(Value);
                    bFound = true;
                }
            }
        }

        if (!bFound)
        {
            // Create the parameter expression
            UMaterialExpressionScalarParameter* NewExpr = NewObject<UMaterialExpressionScalarParameter>(Material);
            NewExpr->ParameterName = FName(*ParameterName);
            NewExpr->DefaultValue = static_cast<float>(Value);
            Material->GetExpressionCollection().AddExpression(NewExpr);
        }

        UMaterialEditingLibrary::RecompileMaterial(Material);
        Material->GetOutermost()->MarkPackageDirty();
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported material type for: %s"), *MaterialName));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("material_name"), MaterialName);
    Result->SetStringField(TEXT("parameter_name"), ParameterName);
    Result->SetNumberField(TEXT("value"), Value);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleSetMaterialVectorParameter(const TSharedPtr<FJsonObject>& Params)
{
    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    FString ParameterName;
    if (!Params->TryGetStringField(TEXT("parameter_name"), ParameterName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'parameter_name' parameter"));
    }

    TArray<float> Values;
    FUnrealMCPCommonUtils::GetFloatArrayFromJson(Params, TEXT("value"), Values);
    if (Values.Num() < 3)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("'value' must be an array of 3 or 4 floats [R, G, B, A]"));
    }

    FLinearColor Color(Values[0], Values[1], Values[2], Values.Num() > 3 ? Values[3] : 1.0f);

    UMaterialInterface* MaterialAsset = FindMaterialByName(MaterialName);
    if (!MaterialAsset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialName));
    }

    if (UMaterialInstanceConstant* MIC = Cast<UMaterialInstanceConstant>(MaterialAsset))
    {
        UMaterialEditingLibrary::SetMaterialInstanceVectorParameterValue(MIC, FName(*ParameterName), Color);
        UMaterialEditingLibrary::UpdateMaterialInstance(MIC);
        MIC->GetOutermost()->MarkPackageDirty();
    }
    else if (UMaterial* Material = Cast<UMaterial>(MaterialAsset))
    {
        bool bFound = false;
        for (UMaterialExpression* Expr : Material->GetExpressions())
        {
            if (UMaterialExpressionVectorParameter* VecExpr = Cast<UMaterialExpressionVectorParameter>(Expr))
            {
                if (VecExpr->ParameterName == FName(*ParameterName))
                {
                    VecExpr->DefaultValue = Color;
                    bFound = true;
                }
            }
        }

        if (!bFound)
        {
            UMaterialExpressionVectorParameter* NewExpr = NewObject<UMaterialExpressionVectorParameter>(Material);
            NewExpr->ParameterName = FName(*ParameterName);
            NewExpr->DefaultValue = Color;
            Material->GetExpressionCollection().AddExpression(NewExpr);
        }

        UMaterialEditingLibrary::RecompileMaterial(Material);
        Material->GetOutermost()->MarkPackageDirty();
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported material type for: %s"), *MaterialName));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("material_name"), MaterialName);
    Result->SetStringField(TEXT("parameter_name"), ParameterName);
    TArray<TSharedPtr<FJsonValue>> ColorArray;
    ColorArray.Add(MakeShared<FJsonValueNumber>(Color.R));
    ColorArray.Add(MakeShared<FJsonValueNumber>(Color.G));
    ColorArray.Add(MakeShared<FJsonValueNumber>(Color.B));
    ColorArray.Add(MakeShared<FJsonValueNumber>(Color.A));
    Result->SetArrayField(TEXT("value"), ColorArray);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPMaterialCommands::HandleApplyMaterialToActor(const TSharedPtr<FJsonObject>& Params)
{
    FString ActorName;
    if (!Params->TryGetStringField(TEXT("actor_name"), ActorName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'actor_name' parameter"));
    }

    FString MaterialName;
    if (!Params->TryGetStringField(TEXT("material_name"), MaterialName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'material_name' parameter"));
    }

    int32 SlotIndex = 0;
    Params->TryGetNumberField(TEXT("slot_index"), SlotIndex);

    UMaterialInterface* MaterialAsset = FindMaterialByName(MaterialName);
    if (!MaterialAsset)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Material not found: %s"), *MaterialName));
    }

    UWorld* World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
    if (!World)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("No active editor world"));
    }

    AActor* TargetActor = nullptr;
    for (TActorIterator<AActor> It(World); It; ++It)
    {
        if (It->GetName() == ActorName || It->GetActorLabel() == ActorName)
        {
            TargetActor = *It;
            break;
        }
    }

    if (!TargetActor)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor not found: %s"), *ActorName));
    }

    TArray<UStaticMeshComponent*> MeshComponents;
    TargetActor->GetComponents<UStaticMeshComponent>(MeshComponents);
    if (MeshComponents.Num() == 0)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Actor '%s' has no StaticMeshComponent"), *ActorName));
    }

    UStaticMeshComponent* MeshComp = MeshComponents[0];
    if (SlotIndex < 0 || SlotIndex >= MeshComp->GetNumMaterials())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Invalid slot_index %d (mesh has %d slots)"), SlotIndex, MeshComp->GetNumMaterials()));
    }

    MeshComp->SetMaterial(SlotIndex, MaterialAsset);
    MeshComp->MarkRenderStateDirty();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("actor_name"), ActorName);
    Result->SetStringField(TEXT("material_name"), MaterialName);
    Result->SetNumberField(TEXT("slot_index"), SlotIndex);
    return Result;
}
