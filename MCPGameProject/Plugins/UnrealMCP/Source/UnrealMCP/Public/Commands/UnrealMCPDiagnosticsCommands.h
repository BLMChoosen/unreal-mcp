#pragma once

#include "CoreMinimal.h"

class FJsonObject;

/**
 * Handler for editor diagnostics commands.
 * Blueprint compile errors, message log, output log, warnings.
 */
class FUnrealMCPDiagnosticsCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	static TSharedPtr<FJsonObject> HandleGetBlueprintCompileErrors(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetMessageLog(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetOutputLog(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetEditorWarnings(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetMapCheckErrors(const TSharedPtr<FJsonObject>& Params);
};
