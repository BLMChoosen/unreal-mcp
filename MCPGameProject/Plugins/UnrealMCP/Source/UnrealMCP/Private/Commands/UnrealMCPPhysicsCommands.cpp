#include "Commands/UnrealMCPPhysicsCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"

TSharedPtr<FJsonObject> FUnrealMCPPhysicsCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("create_physics_constraint")) return HandleCreatePhysicsConstraint(Params);
    if (CommandType == TEXT("setup_ragdoll")) return HandleSetupRagdoll(Params);
    if (CommandType == TEXT("enable_chaos_destruction")) return HandleEnableChaosDestruction(Params);
    if (CommandType == TEXT("set_physics_constraint_properties")) return HandleSetPhysicsConstraintProperties(Params);
    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Unknown physics command"));
}

TSharedPtr<FJsonObject> FUnrealMCPPhysicsCommands::HandleCreatePhysicsConstraint(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPPhysicsCommands::HandleSetupRagdoll(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPPhysicsCommands::HandleEnableChaosDestruction(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
TSharedPtr<FJsonObject> FUnrealMCPPhysicsCommands::HandleSetPhysicsConstraintProperties(const TSharedPtr<FJsonObject>& Params) { return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Not implemented")); }
