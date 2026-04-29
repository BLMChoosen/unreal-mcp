#include "Commands/UnrealMCPSchemaCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_user_defined_struct")) return HandleCreateUserDefinedStruct(Params);
    if (CommandType == TEXT("add_struct_field")) return HandleAddStructField(Params);
    if (CommandType == TEXT("remove_struct_field")) return HandleRemoveStructField(Params);
    if (CommandType == TEXT("get_struct_fields")) return HandleGetStructFields(Params);
    if (CommandType == TEXT("recompile_struct")) return HandleRecompileStruct(Params);
    if (CommandType == TEXT("create_data_table_from_struct")) return HandleCreateDataTableFromStruct(Params);
    if (CommandType == TEXT("generate_schema")) return HandleGenerateSchema(Params);
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Unknown schema command"));
}

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleCreateUserDefinedStruct(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleAddStructField(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleRemoveStructField(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleGetStructFields(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleRecompileStruct(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleCreateDataTableFromStruct(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleGenerateSchema(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
