#pragma once

#include "CoreMinimal.h"

class FJsonObject;

/**
 * Handler for naming policy, asset organization, and batch rename commands.
 */
class FUnrealMCPPolicyCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	static TSharedPtr<FJsonObject> HandleValidateNamingPolicy(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAutoOrganizeAssets(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleBatchRenameAssets(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleFixRedirectors(const TSharedPtr<FJsonObject>& Params);
};
