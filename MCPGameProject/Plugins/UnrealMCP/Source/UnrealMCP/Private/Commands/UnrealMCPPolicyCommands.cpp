#include "Commands/UnrealMCPPolicyCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"

TSharedPtr<FJsonObject> FUnrealMCPPolicyCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("validate_naming_policy")) return HandleValidateNamingPolicy(Params);
    if (CommandType == TEXT("auto_organize_assets")) return HandleAutoOrganizeAssets(Params);
    if (CommandType == TEXT("batch_rename_assets")) return HandleBatchRenameAssets(Params);
    if (CommandType == TEXT("fix_redirectors")) return HandleFixRedirectors(Params);
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Unknown policy command"));
}

TSharedPtr<FJsonObject> FUnrealMCPPolicyCommands::HandleValidateNamingPolicy(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPPolicyCommands::HandleAutoOrganizeAssets(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPPolicyCommands::HandleBatchRenameAssets(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPPolicyCommands::HandleFixRedirectors(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
