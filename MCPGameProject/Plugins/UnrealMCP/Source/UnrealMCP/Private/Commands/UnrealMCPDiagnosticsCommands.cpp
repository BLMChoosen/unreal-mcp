#include "Commands/UnrealMCPDiagnosticsCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"

TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("get_blueprint_compile_errors")) return HandleGetBlueprintCompileErrors(Params);
    if (CommandType == TEXT("get_message_log")) return HandleGetMessageLog(Params);
    if (CommandType == TEXT("get_output_log")) return HandleGetOutputLog(Params);
    if (CommandType == TEXT("get_editor_warnings")) return HandleGetEditorWarnings(Params);
    if (CommandType == TEXT("get_map_check_errors")) return HandleGetMapCheckErrors(Params);
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Unknown diagnostics command"));
}

TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetBlueprintCompileErrors(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetMessageLog(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetOutputLog(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetEditorWarnings(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetMapCheckErrors(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
