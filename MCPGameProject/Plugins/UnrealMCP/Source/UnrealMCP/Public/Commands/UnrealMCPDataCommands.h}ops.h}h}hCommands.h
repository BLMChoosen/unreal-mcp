#pragma once

#include "CoreMinimal.h"

class FJsonObject;

class FUnrealMCPDataCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	// Data Tables
	static TSharedPtr<FJsonObject> HandleImportDataTable(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleExportDataTable(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddDataTableRow(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetDataTableRow(const TSharedPtr<FJsonObject>& Params);
	
	// String Tables
	static TSharedPtr<FJsonObject> HandleCreateStringTable(const TSharedPtr<FJsonObject>& Params);
};
