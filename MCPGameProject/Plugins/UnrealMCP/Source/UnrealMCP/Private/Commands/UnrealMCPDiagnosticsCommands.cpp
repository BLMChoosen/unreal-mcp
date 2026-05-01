#include "Commands/UnrealMCPDiagnosticsCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Engine/Blueprint.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Kismet2/CompilerResultsLog.h"
#include "Logging/TokenizedMessage.h"
#include "MessageLogModule.h"
#include "IMessageLogListing.h"
#include "Modules/ModuleManager.h"

namespace
{
    FString SeverityToString(EMessageSeverity::Type Severity)
    {
        switch (Severity)
        {
        case EMessageSeverity::Error:   return TEXT("Error");
        case EMessageSeverity::Warning: return TEXT("Warning");
        case EMessageSeverity::Info:    return TEXT("Info");
        default:                        return TEXT("Unknown");
        }
    }

    TSharedPtr<FJsonObject> MessageToJson(const TSharedRef<FTokenizedMessage>& Message)
    {
        TSharedPtr<FJsonObject> Obj = MakeShared<FJsonObject>();
        Obj->SetStringField(TEXT("severity"), SeverityToString(Message->GetSeverity()));
        Obj->SetStringField(TEXT("text"), Message->ToText().ToString());
        return Obj;
    }

    TArray<TSharedPtr<FJsonValue>> CollectListingMessages(const FName& ListingName, int32 MaxEntries)
    {
        TArray<TSharedPtr<FJsonValue>> Result;
        FMessageLogModule& MessageLogModule =
            FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");
        if (!MessageLogModule.IsRegisteredLogListing(ListingName))
        {
            return Result;
        }
        TSharedRef<IMessageLogListing> Listing = MessageLogModule.GetLogListing(ListingName);
        const TArray<TSharedRef<FTokenizedMessage>> Messages = Listing->GetFilteredMessages();
        const int32 Take = (MaxEntries > 0) ? FMath::Min(MaxEntries, Messages.Num()) : Messages.Num();
        // Most-recent-first.
        for (int32 i = Messages.Num() - 1; i >= 0 && Result.Num() < Take; --i)
        {
            TSharedPtr<FJsonObject> MsgObj = MessageToJson(Messages[i]);
            MsgObj->SetStringField(TEXT("category"), ListingName.ToString());
            Result.Add(MakeShared<FJsonValueObject>(MsgObj));
        }
        return Result;
    }
}

TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("get_blueprint_compile_errors")) return HandleGetBlueprintCompileErrors(Params);
    if (CommandType == TEXT("get_message_log")) return HandleGetMessageLog(Params);
    if (CommandType == TEXT("get_output_log")) return HandleGetOutputLog(Params);
    if (CommandType == TEXT("get_editor_warnings")) return HandleGetEditorWarnings(Params);
    if (CommandType == TEXT("get_map_check_errors")) return HandleGetMapCheckErrors(Params);
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Unknown diagnostics command"));
}

TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetBlueprintCompileErrors(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    FCompilerResultsLog Results;
    FKismetEditorUtilities::CompileBlueprint(Blueprint, EBlueprintCompileOptions::SkipGarbageCollection, &Results);

    TArray<TSharedPtr<FJsonValue>> Errors;
    TArray<TSharedPtr<FJsonValue>> Warnings;
    TArray<TSharedPtr<FJsonValue>> Notes;

    for (const TSharedRef<FTokenizedMessage>& Msg : Results.Messages)
    {
        const TSharedPtr<FJsonObject> MsgObj = MessageToJson(Msg);
        switch (Msg->GetSeverity())
        {
        case EMessageSeverity::Error:
            Errors.Add(MakeShared<FJsonValueObject>(MsgObj));
            break;
        case EMessageSeverity::Warning:
            Warnings.Add(MakeShared<FJsonValueObject>(MsgObj));
            break;
        default:
            Notes.Add(MakeShared<FJsonValueObject>(MsgObj));
            break;
        }
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("blueprint"), Blueprint->GetPathName());
    Result->SetNumberField(TEXT("error_count"), Results.NumErrors);
    Result->SetNumberField(TEXT("warning_count"), Results.NumWarnings);
    Result->SetArrayField(TEXT("errors"), Errors);
    Result->SetArrayField(TEXT("warnings"), Warnings);
    Result->SetArrayField(TEXT("notes"), Notes);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetMessageLog(const TSharedPtr<FJsonObject>& Params)
{
    FString Category = TEXT("BlueprintLog");
    Params->TryGetStringField(TEXT("category"), Category);

    int32 MaxEntries = 50;
    if (Params->HasField(TEXT("max_entries")))
    {
        MaxEntries = static_cast<int32>(Params->GetNumberField(TEXT("max_entries")));
    }

    TArray<TSharedPtr<FJsonValue>> Messages = CollectListingMessages(FName(*Category), MaxEntries);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("category"), Category);
    Result->SetNumberField(TEXT("count"), Messages.Num());
    Result->SetArrayField(TEXT("messages"), Messages);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetOutputLog(const TSharedPtr<FJsonObject>& Params)
{
    // Capturing the Output Log requires installing a custom FOutputDevice sink at module
    // startup; that wiring lives elsewhere and is out of scope for this diagnostics handler.
    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), false);
    Result->SetStringField(TEXT("error"),
        TEXT("get_output_log requires an FOutputDevice capture sink that is not yet installed."));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetEditorWarnings(const TSharedPtr<FJsonObject>& Params)
{
    static const FName Channels[] = {
        TEXT("BlueprintLog"),
        TEXT("MapCheck"),
        TEXT("LoadErrors"),
        TEXT("AssetCheck"),
        TEXT("PIE"),
    };

    TArray<TSharedPtr<FJsonValue>> Warnings;
    TArray<TSharedPtr<FJsonValue>> Errors;

    for (const FName& Channel : Channels)
    {
        TArray<TSharedPtr<FJsonValue>> Channelmessages = CollectListingMessages(Channel, 0);
        for (const TSharedPtr<FJsonValue>& Val : Channelmessages)
        {
            const TSharedPtr<FJsonObject>& Obj = Val->AsObject();
            if (!Obj.IsValid()) continue;
            const FString Sev = Obj->GetStringField(TEXT("severity"));
            if (Sev == TEXT("Error"))
            {
                Errors.Add(Val);
            }
            else if (Sev == TEXT("Warning"))
            {
                Warnings.Add(Val);
            }
        }
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetNumberField(TEXT("warning_count"), Warnings.Num());
    Result->SetNumberField(TEXT("error_count"), Errors.Num());
    Result->SetArrayField(TEXT("warnings"), Warnings);
    Result->SetArrayField(TEXT("errors"), Errors);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDiagnosticsCommands::HandleGetMapCheckErrors(const TSharedPtr<FJsonObject>& Params)
{
    TArray<TSharedPtr<FJsonValue>> Messages = CollectListingMessages(TEXT("MapCheck"), 0);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("category"), TEXT("MapCheck"));
    Result->SetNumberField(TEXT("count"), Messages.Num());
    Result->SetArrayField(TEXT("messages"), Messages);
    return Result;
}
