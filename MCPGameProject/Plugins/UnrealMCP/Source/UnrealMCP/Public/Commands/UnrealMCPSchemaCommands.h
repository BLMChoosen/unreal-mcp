#pragma once

#include "CoreMinimal.h"

class FJsonObject;

/**
 * Handler for schema/struct management commands.
 * Handles UserDefinedStruct creation, field management, and schema generation.
 */
class FUnrealMCPSchemaCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	static TSharedPtr<FJsonObject> HandleCreateUserDefinedStruct(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddStructField(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleRemoveStructField(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetStructFields(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleRecompileStruct(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleCreateDataTableFromStruct(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGenerateSchema(const TSharedPtr<FJsonObject>& Params);
};
