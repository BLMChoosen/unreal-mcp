#pragma once

#include "CoreMinimal.h"
#include "Json.h"

/**
 * Handler class for Blueprint Node-related MCP commands
 */
class UNREALMCP_API FUnrealMCPBlueprintNodeCommands
{
public:
    FUnrealMCPBlueprintNodeCommands();

    // Handle blueprint node commands
    TSharedPtr<FJsonObject> HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params);

private:
    // Specific blueprint node command handlers
    TSharedPtr<FJsonObject> HandleConnectBlueprintNodes(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBlueprintGetSelfComponentReference(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBlueprintEvent(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBlueprintFunctionCall(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBlueprintVariable(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBlueprintInputActionNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBlueprintSelfReference(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleFindBlueprintNodes(const TSharedPtr<FJsonObject>& Params);

    // Variable inspection / node creation
    TSharedPtr<FJsonObject> HandleGetBlueprintVariable(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBlueprintVariableGetNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBlueprintVariableSetNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBranchNode(const TSharedPtr<FJsonObject>& Params);

    // Control flow / logic nodes
    TSharedPtr<FJsonObject> HandleAddMathNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddCastNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddSequenceNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddForEachLoopNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddWhileLoopNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTimelineNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTimelineFloatTrack(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTimelineKeyframe(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTimelineVectorTrack(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTimelineVectorKeyframe(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTimelineLinearColorTrack(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTimelineLinearColorKeyframe(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddTimelineEventTrack(const TSharedPtr<FJsonObject>& Params);

    // Phase 2: Extended Blueprint Graph Authoring
    TSharedPtr<FJsonObject> HandleCreateBlueprintFunction(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleCreateBlueprintMacro(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleCreateBlueprintCustomEventGraph(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddNodeByClass(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleConnectPinsValidated(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetPinDefaultValue(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddCustomEventNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleListBlueprintFunctions(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleGetNodePins(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleDeleteBlueprintNode(const TSharedPtr<FJsonObject>& Params);

    // Deep graph editing and semantic Blueprint authoring
    TSharedPtr<FJsonObject> HandleAddBlueprintControlNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetVariableDefaultValue(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSplitStructPin(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleRecombineStructPin(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddDynamicPinToNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleDisconnectBlueprintPin(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleMoveBlueprintNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleFormatBlueprintNodes(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddFunctionEntryNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddSpawnActorNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleCreateBlueprintInterface(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddInterfaceFunction(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleImplementInterface(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddInterfaceMessageNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleCreateEventDispatcher(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddEventDispatcherNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetVariableContainerType(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddCollectionNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleExportNodesAsText(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandlePasteNodesFromText(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddLocalVariable(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleSetFunctionFlags(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddFormatTextNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddSwitchNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddAsyncActionNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddCommentBox(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleCollapseToGraph(const TSharedPtr<FJsonObject>& Params, bool bMacro);
    TSharedPtr<FJsonObject> HandlePromoteToVariable(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleEditConstructionScript(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleReadGraphLogicFlow(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleInspectNodePins(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBreakStructNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddMakeStructNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddClassReferenceNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddBindEventNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddCreateEventNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddGlobalGetter(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleAddMathExpressionNode(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleGetSpecificNodeError(const TSharedPtr<FJsonObject>& Params);
    TSharedPtr<FJsonObject> HandleResolveWildcardPin(const TSharedPtr<FJsonObject>& Params);
};
