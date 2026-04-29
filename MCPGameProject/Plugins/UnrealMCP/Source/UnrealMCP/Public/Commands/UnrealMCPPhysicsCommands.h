#pragma once

#include "CoreMinimal.h"

class FJsonObject;

/**
 * Handler for physics constraint and Chaos destruction commands.
 */
class FUnrealMCPPhysicsCommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	static TSharedPtr<FJsonObject> HandleCreatePhysicsConstraint(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleSetupRagdoll(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleEnableChaosDestruction(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleSetPhysicsConstraintProperties(const TSharedPtr<FJsonObject>& Params);
};
