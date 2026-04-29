#include "Commands/UnrealMCPAssetCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetRegistry/IAssetRegistry.h"
#include "AssetRegistry/AssetData.h"
#include "EditorAssetLibrary.h"

FUnrealMCPAssetCommands::FUnrealMCPAssetCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPAssetCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("list_assets"))
    {
        return HandleListAssets(Params);
    }
    else if (CommandType == TEXT("duplicate_asset"))
    {
        return HandleDuplicateAsset(Params);
    }
    else if (CommandType == TEXT("move_asset"))
    {
        return HandleMoveAsset(Params);
    }
    else if (CommandType == TEXT("delete_asset"))
    {
        return HandleDeleteAsset(Params);
    }
    else if (CommandType == TEXT("get_asset_info"))
    {
        return HandleGetAssetInfo(Params);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown asset command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPAssetCommands::HandleListAssets(const TSharedPtr<FJsonObject>& Params)
{
    FString Path = TEXT("/Game");
    Params->TryGetStringField(TEXT("path"), Path);

    bool bRecursive = true;
    if (Params->HasField(TEXT("recursive")))
    {
        bRecursive = Params->GetBoolField(TEXT("recursive"));
    }

    FString FilterType;
    Params->TryGetStringField(TEXT("filter_type"), FilterType);

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    TArray<FAssetData> Assets;
    AssetRegistry.GetAssetsByPath(FName(*Path), Assets, bRecursive);

    TArray<TSharedPtr<FJsonValue>> AssetArray;
    for (const FAssetData& Asset : Assets)
    {
        const FString AssetClass = Asset.AssetClassPath.GetAssetName().ToString();
        if (!FilterType.IsEmpty() && !AssetClass.Equals(FilterType, ESearchCase::IgnoreCase))
        {
            continue;
        }

        TSharedPtr<FJsonObject> AssetObj = MakeShared<FJsonObject>();
        AssetObj->SetStringField(TEXT("name"), Asset.AssetName.ToString());
        AssetObj->SetStringField(TEXT("path"), Asset.GetObjectPathString());
        AssetObj->SetStringField(TEXT("type"), AssetClass);
        AssetObj->SetStringField(TEXT("package_path"), Asset.PackagePath.ToString());
        AssetArray.Add(MakeShared<FJsonValueObject>(AssetObj));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetArrayField(TEXT("assets"), AssetArray);
    Result->SetNumberField(TEXT("count"), AssetArray.Num());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAssetCommands::HandleDuplicateAsset(const TSharedPtr<FJsonObject>& Params)
{
    FString SourcePath, DestPath;
    if (!Params->TryGetStringField(TEXT("source_path"), SourcePath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_path' parameter"));
    }
    if (!Params->TryGetStringField(TEXT("dest_path"), DestPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'dest_path' parameter"));
    }

    UObject* Duplicated = UEditorAssetLibrary::DuplicateAsset(SourcePath, DestPath);
    if (!Duplicated)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to duplicate '%s' to '%s'"), *SourcePath, *DestPath));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("source_path"), SourcePath);
    Result->SetStringField(TEXT("dest_path"), DestPath);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAssetCommands::HandleMoveAsset(const TSharedPtr<FJsonObject>& Params)
{
    FString SourcePath, DestPath;
    if (!Params->TryGetStringField(TEXT("source_path"), SourcePath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_path' parameter"));
    }
    if (!Params->TryGetStringField(TEXT("dest_path"), DestPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'dest_path' parameter"));
    }

    const bool bRenamed = UEditorAssetLibrary::RenameAsset(SourcePath, DestPath);
    if (!bRenamed)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to move '%s' to '%s'"), *SourcePath, *DestPath));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("source_path"), SourcePath);
    Result->SetStringField(TEXT("dest_path"), DestPath);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAssetCommands::HandleDeleteAsset(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetPath;
    if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'asset_path' parameter"));
    }

    const bool bDeleted = UEditorAssetLibrary::DeleteAsset(AssetPath);
    if (!bDeleted)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to delete asset: %s"), *AssetPath));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("asset_path"), AssetPath);
    Result->SetBoolField(TEXT("deleted"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPAssetCommands::HandleGetAssetInfo(const TSharedPtr<FJsonObject>& Params)
{
    FString AssetPath;
    if (!Params->TryGetStringField(TEXT("asset_path"), AssetPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'asset_path' parameter"));
    }

    FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>("AssetRegistry");
    IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

    FAssetData AssetData = AssetRegistry.GetAssetByObjectPath(FSoftObjectPath(AssetPath));
    if (!AssetData.IsValid())
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Asset not found: %s"), *AssetPath));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("name"), AssetData.AssetName.ToString());
    Result->SetStringField(TEXT("path"), AssetData.GetObjectPathString());
    Result->SetStringField(TEXT("type"), AssetData.AssetClassPath.GetAssetName().ToString());
    Result->SetStringField(TEXT("package_path"), AssetData.PackagePath.ToString());
    Result->SetStringField(TEXT("package_name"), AssetData.PackageName.ToString());
    return Result;
}
