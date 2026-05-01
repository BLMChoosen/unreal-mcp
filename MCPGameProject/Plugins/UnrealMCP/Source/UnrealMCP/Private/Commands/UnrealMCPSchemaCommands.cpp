#include "Commands/UnrealMCPSchemaCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "AssetToolsModule.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphSchema_K2.h"
#include "Engine/DataTable.h"
#include "Engine/UserDefinedStruct.h"
#include "Factories/DataTableFactory.h"
#include "Factories/StructureFactory.h"
#include "Kismet2/StructureEditorUtils.h"
#include "Modules/ModuleManager.h"
#include "UserDefinedStructure/UserDefinedStructEditorData.h"
#include "UObject/Package.h"
#include "UObject/SavePackage.h"
#include "Misc/PackageName.h"

namespace
{
    bool MapTypeStringToPinType(const FString& TypeName, FEdGraphPinType& OutPin, FString& OutError)
    {
        const FString T = TypeName;

        if (T == TEXT("Boolean") || T == TEXT("bool"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Boolean;
            return true;
        }
        if (T == TEXT("Integer") || T == TEXT("Int") || T == TEXT("int"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Int;
            return true;
        }
        if (T == TEXT("Float") || T == TEXT("float"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Float;
            return true;
        }
        if (T == TEXT("String") || T == TEXT("string"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_String;
            return true;
        }
        if (T == TEXT("Name"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Name;
            return true;
        }
        if (T == TEXT("Text"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Text;
            return true;
        }
        if (T == TEXT("Byte"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Byte;
            return true;
        }
        if (T == TEXT("Vector"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Struct;
            OutPin.PinSubCategoryObject = TBaseStructure<FVector>::Get();
            return true;
        }
        if (T == TEXT("Rotator"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Struct;
            OutPin.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
            return true;
        }
        if (T == TEXT("Transform"))
        {
            OutPin.PinCategory = UEdGraphSchema_K2::PC_Struct;
            OutPin.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
            return true;
        }

        OutError = FString::Printf(TEXT("Unsupported field type: %s"), *TypeName);
        return false;
    }

    FString PinTypeToTypeString(const FEdGraphPinType& Pin)
    {
        if (Pin.PinCategory == UEdGraphSchema_K2::PC_Boolean) return TEXT("Boolean");
        if (Pin.PinCategory == UEdGraphSchema_K2::PC_Int)     return TEXT("Integer");
        if (Pin.PinCategory == UEdGraphSchema_K2::PC_Float)   return TEXT("Float");
        if (Pin.PinCategory == UEdGraphSchema_K2::PC_String)  return TEXT("String");
        if (Pin.PinCategory == UEdGraphSchema_K2::PC_Name)    return TEXT("Name");
        if (Pin.PinCategory == UEdGraphSchema_K2::PC_Text)    return TEXT("Text");
        if (Pin.PinCategory == UEdGraphSchema_K2::PC_Byte)    return TEXT("Byte");
        if (Pin.PinCategory == UEdGraphSchema_K2::PC_Struct && Pin.PinSubCategoryObject.IsValid())
        {
            return Pin.PinSubCategoryObject->GetName();
        }
        return Pin.PinCategory.ToString();
    }

    UUserDefinedStruct* LoadUserStructByPath(const FString& StructPath)
    {
        // Accept "/Game/Folder/S_Foo" or "/Game/Folder/S_Foo.S_Foo" form.
        if (UUserDefinedStruct* S = LoadObject<UUserDefinedStruct>(nullptr, *StructPath))
        {
            return S;
        }
        int32 LastSlash = INDEX_NONE;
        if (StructPath.FindLastChar(TEXT('/'), LastSlash))
        {
            const FString AssetName = StructPath.Mid(LastSlash + 1);
            int32 DotIdx = INDEX_NONE;
            const FString BareAsset = AssetName.FindChar(TEXT('.'), DotIdx)
                ? AssetName.Left(DotIdx) : AssetName;
            const FString FullObjectPath = StructPath.Left(LastSlash + 1) + BareAsset + TEXT(".") + BareAsset;
            return LoadObject<UUserDefinedStruct>(nullptr, *FullObjectPath);
        }
        return nullptr;
    }

    bool FindFieldByName(UUserDefinedStruct* Struct, const FString& FieldName, FStructVariableDescription& OutDesc)
    {
        const TArray<FStructVariableDescription>& Vars = FStructureEditorUtils::GetVarDesc(Struct);
        for (const FStructVariableDescription& Var : Vars)
        {
            // FStructureEditorUtils renames variables to "<DisplayName>_GUID". Compare display name.
            if (Var.VarName.ToString().StartsWith(FieldName + TEXT("_")) ||
                Var.VarName.ToString() == FieldName)
            {
                OutDesc = Var;
                return true;
            }
        }
        return false;
    }
}

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

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleCreateUserDefinedStruct(const TSharedPtr<FJsonObject>& Params)
{
    FString StructName;
    if (!Params->TryGetStringField(TEXT("name"), StructName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }
    FString PackagePath = TEXT("/Game/Data");
    Params->TryGetStringField(TEXT("path"), PackagePath);

    FAssetToolsModule& AssetToolsModule =
        FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    UStructureFactory* Factory = NewObject<UStructureFactory>();
    UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
        StructName, PackagePath, UUserDefinedStruct::StaticClass(), Factory);

    UUserDefinedStruct* NewStruct = Cast<UUserDefinedStruct>(NewAsset);
    if (!NewStruct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Failed to create UserDefinedStruct '%s' at '%s'"), *StructName, *PackagePath));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("name"), StructName);
    Result->SetStringField(TEXT("path"), NewStruct->GetPathName());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleAddStructField(const TSharedPtr<FJsonObject>& Params)
{
    FString StructPath;
    if (!Params->TryGetStringField(TEXT("struct_path"), StructPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_path' parameter"));
    }
    FString FieldName;
    if (!Params->TryGetStringField(TEXT("field_name"), FieldName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'field_name' parameter"));
    }
    FString FieldType;
    if (!Params->TryGetStringField(TEXT("field_type"), FieldType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'field_type' parameter"));
    }

    UUserDefinedStruct* Struct = LoadUserStructByPath(StructPath);
    if (!Struct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("UserDefinedStruct not found: %s"), *StructPath));
    }

    FEdGraphPinType PinType;
    FString MapErr;
    if (!MapTypeStringToPinType(FieldType, PinType, MapErr))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(MapErr);
    }

    // Snapshot existing GUIDs so we can identify the newly added variable and rename it.
    TSet<FGuid> Before;
    for (const FStructVariableDescription& Var : FStructureEditorUtils::GetVarDesc(Struct))
    {
        Before.Add(Var.VarGuid);
    }

    if (!FStructureEditorUtils::AddVariable(Struct, PinType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("AddVariable failed"));
    }

    FGuid NewGuid;
    for (const FStructVariableDescription& Var : FStructureEditorUtils::GetVarDesc(Struct))
    {
        if (!Before.Contains(Var.VarGuid))
        {
            NewGuid = Var.VarGuid;
            break;
        }
    }
    if (NewGuid.IsValid())
    {
        FStructureEditorUtils::RenameVariable(Struct, NewGuid, FieldName);
    }

    FString DefaultValue;
    if (Params->TryGetStringField(TEXT("default_value"), DefaultValue) && NewGuid.IsValid() && !DefaultValue.IsEmpty())
    {
        FStructureEditorUtils::ChangeVariableDefaultValue(Struct, NewGuid, DefaultValue);
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("struct_path"), StructPath);
    Result->SetStringField(TEXT("field_name"), FieldName);
    Result->SetStringField(TEXT("field_type"), FieldType);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleRemoveStructField(const TSharedPtr<FJsonObject>& Params)
{
    FString StructPath;
    if (!Params->TryGetStringField(TEXT("struct_path"), StructPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_path' parameter"));
    }
    FString FieldName;
    if (!Params->TryGetStringField(TEXT("field_name"), FieldName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'field_name' parameter"));
    }

    UUserDefinedStruct* Struct = LoadUserStructByPath(StructPath);
    if (!Struct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("UserDefinedStruct not found: %s"), *StructPath));
    }

    FStructVariableDescription Var;
    if (!FindFieldByName(Struct, FieldName, Var))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Field not found: %s"), *FieldName));
    }

    if (!FStructureEditorUtils::RemoveVariable(Struct, Var.VarGuid))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("RemoveVariable failed"));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("struct_path"), StructPath);
    Result->SetStringField(TEXT("field_name"), FieldName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleGetStructFields(const TSharedPtr<FJsonObject>& Params)
{
    FString StructPath;
    if (!Params->TryGetStringField(TEXT("struct_path"), StructPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_path' parameter"));
    }

    UUserDefinedStruct* Struct = LoadUserStructByPath(StructPath);
    if (!Struct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("UserDefinedStruct not found: %s"), *StructPath));
    }

    TArray<TSharedPtr<FJsonValue>> Fields;
    for (const FStructVariableDescription& Var : FStructureEditorUtils::GetVarDesc(Struct))
    {
        TSharedPtr<FJsonObject> FieldObj = MakeShared<FJsonObject>();
        FieldObj->SetStringField(TEXT("name"), Var.VarName.ToString());
        FieldObj->SetStringField(TEXT("display_name"), Var.FriendlyName);
        FieldObj->SetStringField(TEXT("guid"), Var.VarGuid.ToString());
        FEdGraphPinType PinType = Var.ToPinType();
        FieldObj->SetStringField(TEXT("type"), PinTypeToTypeString(PinType));
        FieldObj->SetStringField(TEXT("default_value"), Var.DefaultValue);
        Fields.Add(MakeShared<FJsonValueObject>(FieldObj));
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("struct_path"), StructPath);
    Result->SetNumberField(TEXT("count"), Fields.Num());
    Result->SetArrayField(TEXT("fields"), Fields);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleRecompileStruct(const TSharedPtr<FJsonObject>& Params)
{
    FString StructPath;
    if (!Params->TryGetStringField(TEXT("struct_path"), StructPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_path' parameter"));
    }

    UUserDefinedStruct* Struct = LoadUserStructByPath(StructPath);
    if (!Struct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("UserDefinedStruct not found: %s"), *StructPath));
    }

    FStructureEditorUtils::CompileStructure(Struct);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("struct_path"), StructPath);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleCreateDataTableFromStruct(const TSharedPtr<FJsonObject>& Params)
{
    FString Name;
    if (!Params->TryGetStringField(TEXT("name"), Name))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
    }

    FString StructPath;
    if (!Params->TryGetStringField(TEXT("struct_path"), StructPath))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'struct_path' parameter"));
    }

    FString PackagePath = TEXT("/Game/Data");
    Params->TryGetStringField(TEXT("path"), PackagePath);

    UUserDefinedStruct* Struct = LoadUserStructByPath(StructPath);
    if (!Struct)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("UserDefinedStruct not found: %s"), *StructPath));
    }

    FStructureEditorUtils::CompileStructure(Struct);

    UDataTableFactory* Factory = NewObject<UDataTableFactory>();
    Factory->Struct = Struct;

    FAssetToolsModule& AssetToolsModule =
        FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    UObject* NewAsset = AssetToolsModule.Get().CreateAsset(
        Name, PackagePath, UDataTable::StaticClass(), Factory);

    UDataTable* DataTable = Cast<UDataTable>(NewAsset);
    if (!DataTable)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Failed to create Data Table '%s' at '%s'"), *Name, *PackagePath));
    }

    DataTable->RowStruct = Struct;
    FAssetRegistryModule::AssetCreated(DataTable);
    DataTable->MarkPackageDirty();
    DataTable->PostEditChange();

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("name"), Name);
    Result->SetStringField(TEXT("path"), DataTable->GetPathName());
    Result->SetStringField(TEXT("row_struct"), Struct->GetPathName());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPSchemaCommands::HandleGenerateSchema(const TSharedPtr<FJsonObject>& Params)
{
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented"));
}
