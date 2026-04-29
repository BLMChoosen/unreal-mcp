#pragma once

#include "CoreMinimal.h"

class FJsonObject;

class FUnrealMCPAICommands
{
public:
	static TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
	// Behavior Tree
	static TSharedPtr<FJsonObject> HandleCreateBehaviorTree(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddBehaviorTreeNode(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleConnectBehaviorTreeNodes(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddBehaviorTreeDecorator(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetBehaviorTreeNodes(const TSharedPtr<FJsonObject>& Params);
	
	// Blackboard
	static TSharedPtr<FJsonObject> HandleCreateBlackboard(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleAddBlackboardKey(const TSharedPtr<FJsonObject>& Params);
	static TSharedPtr<FJsonObject> HandleGetBlackboardKeys(const TSharedPtr<FJsonObject>& Params);
	
	// NavMesh
	static TSharedPtr<FJsonObject> HandleBuildNavMesh(const TSharedPtr<FJsonObject>& Params);
};
