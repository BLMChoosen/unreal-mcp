#pragma once

#include "CoreMinimal.h"

class FJsonObject;

class FUnrealMCPDataCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	// Data Tables
	static TSharedPtr<FJsonObject> HandleCreateDataTable(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleImportDataTable(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleExportDataTable(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddDataTableRow(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleEditDataTableRow(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetDataTableRow(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleListDataTableRows(const TSharedPtr<FJsonObject>& Params);

	// Data Assets
	static TSharedPtr<FJsonObject> HandleCreateDataAsset(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleSetDataAssetProperties(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetDataAssetProperties(const TSharedPtr<FJsonObject>& Params);
	
	// String Tables
	static TSharedPtr<FJsonObject> HandleCreateStringTable(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddStringTableEntry(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetStringTableEntry(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleRemoveStringTableEntry(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleListStringTableEntries(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleExportStringTable(const TSharedPtr<FJsonObject>& Params);
};
