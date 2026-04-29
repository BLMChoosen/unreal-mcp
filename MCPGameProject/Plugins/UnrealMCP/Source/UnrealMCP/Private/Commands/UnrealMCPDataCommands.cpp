#include "Commands/UnrealMCPDataCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "AssetToolsModule.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "DataTableUtils.h"
#include "Engine/DataAsset.h"
#include "Engine/DataTable.h"
#include "Factories/DataAssetFactory.h"
#include "Factories/DataTableFactory.h"
#include "Factories/StringTableFactory.h"
#include "Internationalization/StringTable.h"
#include "Internationalization/StringTableCore.h"
#include "Internationalization/TextKey.h"
#include "JsonObjectConverter.h"
#include "Misc/PackageName.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"
#include "UObject/Field.h"
#include "UObject/UnrealType.h"

namespace
{
	FString ResolveDataObjectPath(const FString& AssetPath)
	{
		if (AssetPath.Contains(TEXT(".")) || AssetPath.StartsWith(TEXT("/Script/")))
		{
			return AssetPath;
		}

		const FString AssetName = FPackageName::GetShortName(AssetPath);
		return AssetPath + TEXT(".") + AssetName;
	}

	template <typename TObjectType>
	TObjectType* LoadAssetObject(const FString& AssetPath)
	{
		return LoadObject<TObjectType>(nullptr, *ResolveDataObjectPath(AssetPath));
	}

	UScriptStruct* LoadRowStruct(const FString& StructPath)
	{
		if (UScriptStruct* RowStruct = LoadObject<UScriptStruct>(nullptr, *ResolveDataObjectPath(StructPath)))
		{
			return RowStruct;
		}

		if (UScriptStruct* RowStruct = LoadObject<UScriptStruct>(nullptr, *StructPath))
		{
			return RowStruct;
		}

		return FindFirstObjectSafe<UScriptStruct>(*FPackageName::GetShortName(StructPath));
	}

	UClass* LoadClassByPath(const FString& ClassPath)
	{
		if (UClass* LoadedClass = LoadClass<UObject>(nullptr, *ClassPath))
		{
			return LoadedClass;
		}

		if (UClass* LoadedClass = LoadObject<UClass>(nullptr, *ClassPath))
		{
			return LoadedClass;
		}

		const FString ShortName = FPackageName::GetShortName(ClassPath);
		return FindFirstObjectSafe<UClass>(*ShortName);
	}

	bool TryGetJsonObjectField(const TSharedPtr<FJsonObject>& JsonObject, const FString& FieldName, TSharedPtr<FJsonObject>& OutObject)
	{
		if (!JsonObject.IsValid() || !JsonObject->HasTypedField(FieldName, EJson::Object))
		{
			return false;
		}

		OutObject = JsonObject->GetObjectField(FieldName);
		return OutObject.IsValid();
	}

	TSharedPtr<FJsonObject> RowToJsonObject(UDataTable* DataTable, const uint8* RowData)
	{
		if (!DataTable || !DataTable->RowStruct || !RowData)
		{
			return nullptr;
		}

		TSharedRef<FJsonObject> RowObject = MakeShared<FJsonObject>();
		if (!FJsonObjectConverter::UStructToJsonObject(
			DataTable->RowStruct,
			RowData,
			RowObject,
			0,
			CPF_Transient))
		{
			return nullptr;
		}

		return RowObject;
	}

	bool JsonObjectToRow(UDataTable* DataTable, const TSharedPtr<FJsonObject>& RowObject, uint8* RowData, FString& OutError)
	{
		if (!DataTable || !DataTable->RowStruct)
		{
			OutError = TEXT("Data Table has no row struct");
			return false;
		}
		if (!RowObject.IsValid())
		{
			OutError = TEXT("'row_data' must be a JSON object");
			return false;
		}
		if (!RowData)
		{
			OutError = TEXT("Invalid row memory");
			return false;
		}

		FText FailReason;
		if (!FJsonObjectConverter::JsonObjectToUStruct(
			RowObject.ToSharedRef(),
			DataTable->RowStruct,
			RowData,
			0,
			CPF_Transient,
			false,
			&FailReason))
		{
			OutError = FailReason.IsEmpty()
				? TEXT("Failed to convert JSON object to row struct")
				: FailReason.ToString();
			return false;
		}

		return true;
	}

	TSharedPtr<FJsonObject> LoadDataTableFromParams(const TSharedPtr<FJsonObject>& Params, UDataTable*& OutDataTable)
	{
		FString DataTablePath;
		if (!Params->TryGetStringField(TEXT("data_table"), DataTablePath))
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'data_table' parameter"));
		}

		OutDataTable = LoadAssetObject<UDataTable>(DataTablePath);
		if (!OutDataTable)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Data Table not found: %s"), *DataTablePath));
		}

		if (!OutDataTable->RowStruct)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Data Table has no row struct"));
		}

		return nullptr;
	}

	TSharedPtr<FJsonObject> LoadStringTableFromParams(const TSharedPtr<FJsonObject>& Params, UStringTable*& OutStringTable)
	{
		FString StringTablePath;
		if (!Params->TryGetStringField(TEXT("string_table"), StringTablePath))
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'string_table' parameter"));
		}

		OutStringTable = LoadAssetObject<UStringTable>(StringTablePath);
		if (!OutStringTable)
		{
			return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("String Table not found: %s"), *StringTablePath));
		}

		return nullptr;
	}

	void MarkAssetChanged(UObject* Asset)
	{
		if (!Asset)
		{
			return;
		}

		Asset->Modify();
		Asset->MarkPackageDirty();
		Asset->PostEditChange();
	}

	TSharedPtr<FJsonObject> ExportEditableProperties(UObject* Object)
	{
		TSharedPtr<FJsonObject> PropertiesObject = MakeShared<FJsonObject>();
		if (!Object)
		{
			return PropertiesObject;
		}

		for (TFieldIterator<FProperty> PropIt(Object->GetClass()); PropIt; ++PropIt)
		{
			FProperty* Property = *PropIt;
			if (!Property || Property->HasAnyPropertyFlags(CPF_Transient))
			{
				continue;
			}

			if (!Property->HasAnyPropertyFlags(CPF_Edit | CPF_BlueprintVisible))
			{
				continue;
			}

			const void* PropertyValue = Property->ContainerPtrToValuePtr<void>(Object);
			TSharedPtr<FJsonValue> JsonValue = FJsonObjectConverter::UPropertyToJsonValue(
				Property,
				PropertyValue,
				0,
				CPF_Transient);
			if (JsonValue.IsValid())
			{
				PropertiesObject->SetField(Property->GetName(), JsonValue);
			}
		}

		return PropertiesObject;
	}

	bool SetEditableProperties(UObject* Object, const TSharedPtr<FJsonObject>& PropertiesObject, TArray<FString>& OutErrors)
	{
		if (!Object)
		{
			OutErrors.Add(TEXT("Invalid object"));
			return false;
		}
		if (!PropertiesObject.IsValid())
		{
			OutErrors.Add(TEXT("'properties' must be a JSON object"));
			return false;
		}

		bool bAnyPropertySet = false;
		for (const TPair<FString, TSharedPtr<FJsonValue>>& Pair : PropertiesObject->Values)
		{
			FProperty* Property = Object->GetClass()->FindPropertyByName(FName(*Pair.Key));
			if (!Property)
			{
				OutErrors.Add(FString::Printf(TEXT("Property not found: %s"), *Pair.Key));
				continue;
			}

			if (Property->HasAnyPropertyFlags(CPF_Transient))
			{
				OutErrors.Add(FString::Printf(TEXT("Property is transient and cannot be set: %s"), *Pair.Key));
				continue;
			}

			void* PropertyValue = Property->ContainerPtrToValuePtr<void>(Object);
			FText FailReason;
			if (!FJsonObjectConverter::JsonValueToUProperty(
				Pair.Value,
				Property,
				PropertyValue,
				0,
				CPF_Transient,
				false,
				&FailReason))
			{
				OutErrors.Add(FString::Printf(
					TEXT("Failed to set property '%s': %s"),
					*Pair.Key,
					FailReason.IsEmpty() ? TEXT("invalid value") : *FailReason.ToString()));
				continue;
			}

			bAnyPropertySet = true;
		}

		return bAnyPropertySet && OutErrors.Num() == 0;
	}
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
	if (CommandType == TEXT("create_data_table"))
	{
		return HandleCreateDataTable(Params);
	}
	if (CommandType == TEXT("import_data_table"))
	{
		return HandleImportDataTable(Params);
	}
	if (CommandType == TEXT("export_data_table"))
	{
		return HandleExportDataTable(Params);
	}
	if (CommandType == TEXT("add_data_table_row"))
	{
		return HandleAddDataTableRow(Params);
	}
	if (CommandType == TEXT("edit_data_table_row"))
	{
		return HandleEditDataTableRow(Params);
	}
	if (CommandType == TEXT("get_data_table_row"))
	{
		return HandleGetDataTableRow(Params);
	}
	if (CommandType == TEXT("list_data_table_rows"))
	{
		return HandleListDataTableRows(Params);
	}
	if (CommandType == TEXT("create_data_asset"))
	{
		return HandleCreateDataAsset(Params);
	}
	if (CommandType == TEXT("set_data_asset_properties"))
	{
		return HandleSetDataAssetProperties(Params);
	}
	if (CommandType == TEXT("get_data_asset_properties"))
	{
		return HandleGetDataAssetProperties(Params);
	}
	if (CommandType == TEXT("create_string_table"))
	{
		return HandleCreateStringTable(Params);
	}
	if (CommandType == TEXT("add_string_table_entry"))
	{
		return HandleAddStringTableEntry(Params);
	}
	if (CommandType == TEXT("get_string_table_entry"))
	{
		return HandleGetStringTableEntry(Params);
	}
	if (CommandType == TEXT("remove_string_table_entry"))
	{
		return HandleRemoveStringTableEntry(Params);
	}
	if (CommandType == TEXT("list_string_table_entries"))
	{
		return HandleListStringTableEntries(Params);
	}
	if (CommandType == TEXT("export_string_table"))
	{
		return HandleExportStringTable(Params);
	}

	return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown data command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleCreateDataTable(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/Data");
	Params->TryGetStringField(TEXT("path"), Path);

	FString RowStructPath;
	if (!Params->TryGetStringField(TEXT("row_struct"), RowStructPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'row_struct' parameter"));
	}

	UScriptStruct* RowStruct = LoadRowStruct(RowStructPath);
	if (!RowStruct)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Row struct not found: %s"), *RowStructPath));
	}

	UDataTableFactory* Factory = NewObject<UDataTableFactory>();
	Factory->Struct = RowStruct;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, UDataTable::StaticClass(), Factory);
	UDataTable* DataTable = Cast<UDataTable>(NewAsset);
	if (!DataTable)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Data Table asset"));
	}

	DataTable->RowStruct = RowStruct;
	FAssetRegistryModule::AssetCreated(DataTable);
	MarkAssetChanged(DataTable);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("name"), Name);
	Result->SetStringField(TEXT("path"), FString::Printf(TEXT("%s/%s"), *Path, *Name));
	Result->SetStringField(TEXT("row_struct"), RowStruct->GetPathName());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleImportDataTable(const TSharedPtr<FJsonObject>& Params)
{
	UDataTable* DataTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadDataTableFromParams(Params, DataTable))
	{
		return Error;
	}

	FString Data;
	if (!Params->TryGetStringField(TEXT("data"), Data))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'data' parameter"));
	}

	FString Format = TEXT("json");
	Params->TryGetStringField(TEXT("format"), Format);

	TArray<FString> Problems;
	if (Format.Equals(TEXT("json"), ESearchCase::IgnoreCase))
	{
		Problems = DataTable->CreateTableFromJSONString(Data);
	}
	else if (Format.Equals(TEXT("csv"), ESearchCase::IgnoreCase))
	{
		Problems = DataTable->CreateTableFromCSVString(Data);
	}
	else
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported Data Table format: %s"), *Format));
	}

	if (Problems.Num() > 0)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Join(Problems, TEXT("; ")));
	}

	MarkAssetChanged(DataTable);
	DataTable->HandleDataTableChanged();

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("data_table"), DataTable->GetPathName());
	Result->SetStringField(TEXT("format"), Format.ToLower());
	Result->SetNumberField(TEXT("row_count"), DataTable->GetRowMap().Num());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleExportDataTable(const TSharedPtr<FJsonObject>& Params)
{
	UDataTable* DataTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadDataTableFromParams(Params, DataTable))
	{
		return Error;
	}

	FString Format = TEXT("json");
	Params->TryGetStringField(TEXT("format"), Format);

	FString ExportedData;
	if (Format.Equals(TEXT("json"), ESearchCase::IgnoreCase))
	{
		ExportedData = DataTable->GetTableAsJSON(EDataTableExportFlags::UseJsonObjectsForStructs);
	}
	else if (Format.Equals(TEXT("csv"), ESearchCase::IgnoreCase))
	{
		ExportedData = DataTable->GetTableAsCSV();
	}
	else
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported Data Table format: %s"), *Format));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("data_table"), DataTable->GetPathName());
	Result->SetStringField(TEXT("format"), Format.ToLower());
	Result->SetStringField(TEXT("data"), ExportedData);
	Result->SetNumberField(TEXT("row_count"), DataTable->GetRowMap().Num());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleAddDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	UDataTable* DataTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadDataTableFromParams(Params, DataTable))
	{
		return Error;
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'row_name' parameter"));
	}

	if (DataTable->FindRowUnchecked(FName(*RowName)))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Data Table row already exists: %s"), *RowName));
	}

	TSharedPtr<FJsonObject> RowObject;
	if (!TryGetJsonObjectField(Params, TEXT("row_data"), RowObject))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'row_data' object parameter"));
	}

	TArray<uint8> RowMemory;
	RowMemory.SetNumZeroed(DataTable->RowStruct->GetStructureSize());
	DataTable->RowStruct->InitializeStruct(RowMemory.GetData());

	FString ErrorMessage;
	if (!JsonObjectToRow(DataTable, RowObject, RowMemory.GetData(), ErrorMessage))
	{
		DataTable->RowStruct->DestroyStruct(RowMemory.GetData());
		return FUnrealMCPCommonUtils::CreateErrorResponse(ErrorMessage);
	}

	DataTable->Modify();
	DataTable->AddRow(FName(*RowName), RowMemory.GetData(), DataTable->RowStruct);
	DataTable->RowStruct->DestroyStruct(RowMemory.GetData());
	DataTable->MarkPackageDirty();
	DataTable->HandleDataTableChanged(FName(*RowName));

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("data_table"), DataTable->GetPathName());
	Result->SetStringField(TEXT("row_name"), RowName);
	Result->SetObjectField(TEXT("row_data"), RowObject);
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleEditDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	UDataTable* DataTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadDataTableFromParams(Params, DataTable))
	{
		return Error;
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'row_name' parameter"));
	}

	uint8* ExistingRow = DataTable->FindRowUnchecked(FName(*RowName));
	if (!ExistingRow)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Data Table row not found: %s"), *RowName));
	}

	TSharedPtr<FJsonObject> RowObject;
	if (!TryGetJsonObjectField(Params, TEXT("row_data"), RowObject))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'row_data' object parameter"));
	}

	FString ErrorMessage;
	DataTable->Modify();
	if (!JsonObjectToRow(DataTable, RowObject, ExistingRow, ErrorMessage))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(ErrorMessage);
	}

	DataTable->MarkPackageDirty();
	DataTable->HandleDataTableChanged(FName(*RowName));

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("data_table"), DataTable->GetPathName());
	Result->SetStringField(TEXT("row_name"), RowName);
	TSharedPtr<FJsonObject> ExportedRow = RowToJsonObject(DataTable, ExistingRow);
	if (ExportedRow.IsValid())
	{
		Result->SetObjectField(TEXT("row_data"), ExportedRow);
	}
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleGetDataTableRow(const TSharedPtr<FJsonObject>& Params)
{
	UDataTable* DataTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadDataTableFromParams(Params, DataTable))
	{
		return Error;
	}

	FString RowName;
	if (!Params->TryGetStringField(TEXT("row_name"), RowName))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'row_name' parameter"));
	}

	const uint8* RowData = DataTable->FindRowUnchecked(FName(*RowName));
	if (!RowData)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Data Table row not found: %s"), *RowName));
	}

	TSharedPtr<FJsonObject> RowObject = RowToJsonObject(DataTable, RowData);
	if (!RowObject.IsValid())
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to export row data"));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("data_table"), DataTable->GetPathName());
	Result->SetStringField(TEXT("row_name"), RowName);
	Result->SetObjectField(TEXT("row_data"), RowObject);
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleListDataTableRows(const TSharedPtr<FJsonObject>& Params)
{
	UDataTable* DataTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadDataTableFromParams(Params, DataTable))
	{
		return Error;
	}

	TArray<TSharedPtr<FJsonValue>> Rows;
	for (const TPair<FName, uint8*>& RowPair : DataTable->GetRowMap())
	{
		Rows.Add(MakeShared<FJsonValueString>(RowPair.Key.ToString()));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("data_table"), DataTable->GetPathName());
	Result->SetArrayField(TEXT("rows"), Rows);
	Result->SetNumberField(TEXT("count"), Rows.Num());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleCreateDataAsset(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/Data");
	Params->TryGetStringField(TEXT("path"), Path);

	FString AssetClassPath;
	if (!Params->TryGetStringField(TEXT("asset_class"), AssetClassPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'asset_class' parameter"));
	}

	UClass* DataAssetClass = LoadClassByPath(AssetClassPath);
	if (!DataAssetClass || !DataAssetClass->IsChildOf(UDataAsset::StaticClass()))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Class is not a DataAsset class: %s"), *AssetClassPath));
	}

	UDataAssetFactory* Factory = NewObject<UDataAssetFactory>();
	Factory->DataAssetClass = DataAssetClass;

	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, DataAssetClass, Factory);
	UDataAsset* DataAsset = Cast<UDataAsset>(NewAsset);
	if (!DataAsset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Data Asset"));
	}

	FAssetRegistryModule::AssetCreated(DataAsset);
	MarkAssetChanged(DataAsset);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("name"), Name);
	Result->SetStringField(TEXT("path"), FString::Printf(TEXT("%s/%s"), *Path, *Name));
	Result->SetStringField(TEXT("asset_class"), DataAssetClass->GetPathName());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleSetDataAssetProperties(const TSharedPtr<FJsonObject>& Params)
{
	FString DataAssetPath;
	if (!Params->TryGetStringField(TEXT("data_asset"), DataAssetPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'data_asset' parameter"));
	}

	UObject* DataAsset = LoadObject<UObject>(nullptr, *ResolveDataObjectPath(DataAssetPath));
	if (!DataAsset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Data Asset not found: %s"), *DataAssetPath));
	}

	TSharedPtr<FJsonObject> PropertiesObject;
	if (!TryGetJsonObjectField(Params, TEXT("properties"), PropertiesObject))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'properties' object parameter"));
	}

	TArray<FString> Errors;
	DataAsset->Modify();
	if (!SetEditableProperties(DataAsset, PropertiesObject, Errors))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(
			Errors.Num() > 0 ? FString::Join(Errors, TEXT("; ")) : TEXT("No properties were set"));
	}

	MarkAssetChanged(DataAsset);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("data_asset"), DataAsset->GetPathName());
	Result->SetObjectField(TEXT("properties"), ExportEditableProperties(DataAsset));
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleGetDataAssetProperties(const TSharedPtr<FJsonObject>& Params)
{
	FString DataAssetPath;
	if (!Params->TryGetStringField(TEXT("data_asset"), DataAssetPath))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'data_asset' parameter"));
	}

	UObject* DataAsset = LoadObject<UObject>(nullptr, *ResolveDataObjectPath(DataAssetPath));
	if (!DataAsset)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Data Asset not found: %s"), *DataAssetPath));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("data_asset"), DataAsset->GetPathName());
	Result->SetStringField(TEXT("asset_class"), DataAsset->GetClass()->GetPathName());
	Result->SetObjectField(TEXT("properties"), ExportEditableProperties(DataAsset));
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleCreateStringTable(const TSharedPtr<FJsonObject>& Params)
{
	FString Name;
	if (!Params->TryGetStringField(TEXT("name"), Name))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'name' parameter"));
	}

	FString Path = TEXT("/Game/Localization");
	Params->TryGetStringField(TEXT("path"), Path);

	UStringTableFactory* Factory = NewObject<UStringTableFactory>();
	FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
	UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, Path, UStringTable::StaticClass(), Factory);
	UStringTable* StringTable = Cast<UStringTable>(NewAsset);
	if (!StringTable)
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create String Table"));
	}

	FAssetRegistryModule::AssetCreated(StringTable);
	MarkAssetChanged(StringTable);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("name"), Name);
	Result->SetStringField(TEXT("path"), FString::Printf(TEXT("%s/%s"), *Path, *Name));
	Result->SetStringField(TEXT("string_table_id"), StringTable->GetStringTableId().ToString());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleAddStringTableEntry(const TSharedPtr<FJsonObject>& Params)
{
	UStringTable* StringTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadStringTableFromParams(Params, StringTable))
	{
		return Error;
	}

	FString Key;
	if (!Params->TryGetStringField(TEXT("key"), Key))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'key' parameter"));
	}

	FString SourceString;
	if (!Params->TryGetStringField(TEXT("source_string"), SourceString))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_string' parameter"));
	}

	StringTable->GetMutableStringTable()->SetSourceString(FTextKey(Key), SourceString);
	MarkAssetChanged(StringTable);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("string_table"), StringTable->GetPathName());
	Result->SetStringField(TEXT("key"), Key);
	Result->SetStringField(TEXT("source_string"), SourceString);
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleGetStringTableEntry(const TSharedPtr<FJsonObject>& Params)
{
	UStringTable* StringTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadStringTableFromParams(Params, StringTable))
	{
		return Error;
	}

	FString Key;
	if (!Params->TryGetStringField(TEXT("key"), Key))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'key' parameter"));
	}

	FString SourceString;
	if (!StringTable->GetStringTable()->GetSourceString(FTextKey(Key), SourceString))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("String Table entry not found: %s"), *Key));
	}

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("string_table"), StringTable->GetPathName());
	Result->SetStringField(TEXT("key"), Key);
	Result->SetStringField(TEXT("source_string"), SourceString);
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleRemoveStringTableEntry(const TSharedPtr<FJsonObject>& Params)
{
	UStringTable* StringTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadStringTableFromParams(Params, StringTable))
	{
		return Error;
	}

	FString Key;
	if (!Params->TryGetStringField(TEXT("key"), Key))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'key' parameter"));
	}

	FString ExistingSourceString;
	if (!StringTable->GetStringTable()->GetSourceString(FTextKey(Key), ExistingSourceString))
	{
		return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("String Table entry not found: %s"), *Key));
	}

	StringTable->GetMutableStringTable()->RemoveSourceString(FTextKey(Key));
	MarkAssetChanged(StringTable);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("string_table"), StringTable->GetPathName());
	Result->SetStringField(TEXT("key"), Key);
	Result->SetBoolField(TEXT("removed"), true);
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleListStringTableEntries(const TSharedPtr<FJsonObject>& Params)
{
	UStringTable* StringTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadStringTableFromParams(Params, StringTable))
	{
		return Error;
	}

	TArray<TSharedPtr<FJsonValue>> Entries;
	StringTable->GetStringTable()->EnumerateKeysAndSourceStrings(
		[&Entries](const FTextKey& Key, const FString& SourceString)
		{
			TSharedPtr<FJsonObject> EntryObject = MakeShared<FJsonObject>();
			EntryObject->SetStringField(TEXT("key"), Key.ToString());
			EntryObject->SetStringField(TEXT("source_string"), SourceString);
			Entries.Add(MakeShared<FJsonValueObject>(EntryObject));
			return true;
		});

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("string_table"), StringTable->GetPathName());
	Result->SetArrayField(TEXT("entries"), Entries);
	Result->SetNumberField(TEXT("count"), Entries.Num());
	return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPDataCommands::HandleExportStringTable(const TSharedPtr<FJsonObject>& Params)
{
	UStringTable* StringTable = nullptr;
	if (TSharedPtr<FJsonObject> Error = LoadStringTableFromParams(Params, StringTable))
	{
		return Error;
	}

	TSharedPtr<FJsonObject> EntriesObject = MakeShared<FJsonObject>();
	StringTable->GetStringTable()->EnumerateKeysAndSourceStrings(
		[&EntriesObject](const FTextKey& Key, const FString& SourceString)
		{
			EntriesObject->SetStringField(Key.ToString(), SourceString);
			return true;
		});

	FString ExportedData;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ExportedData);
	FJsonSerializer::Serialize(EntriesObject.ToSharedRef(), Writer);

	TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
	Result->SetStringField(TEXT("string_table"), StringTable->GetPathName());
	Result->SetObjectField(TEXT("entries"), EntriesObject);
	Result->SetStringField(TEXT("data"), ExportedData);
	Result->SetNumberField(TEXT("count"), EntriesObject->Values.Num());
	return Result;
}
