#pragma once

#include "CoreMinimal.h"

class FJsonObject;

/**
 * Handler for asset validation and automation test result commands.
 */
class FUnrealMCPValidationCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	static TSharedPtr<FJsonObject> HandleFindUnusedAssets(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleFindBrokenReferences(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleValidateNamingConventions(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleValidateDataTableAgainstSchema(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGenerateValidationReport(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetAutomationTestResults(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetAutomationTestList(const TSharedPtr<FJsonObject>& Params);
};
