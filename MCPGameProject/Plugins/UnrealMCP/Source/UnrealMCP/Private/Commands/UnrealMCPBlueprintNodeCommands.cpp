#include "Commands/UnrealMCPBlueprintNodeCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "AssetToolsModule.h"
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraphNode_Comment.h"
#include "EdGraph/EdGraphPin.h"
#include "EdGraphUtilities.h"
#include "Factories/BlueprintInterfaceFactory.h"
#include "K2Node_AddDelegate.h"
#include "K2Node_AddPinInterface.h"
#include "K2Node_AsyncAction.h"
#include "K2Node_BaseAsyncTask.h"
#include "K2Node_BreakStruct.h"
#include "K2Node_CallDelegate.h"
#include "K2Node_Event.h"
#include "K2Node_CallFunction.h"
#include "K2Node_ClearDelegate.h"
#include "K2Node_CreateDelegate.h"
#include "K2Node_CustomEvent.h"
#include "K2Node_EditablePinBase.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "K2Node_IfThenElse.h"
#include "K2Node_InputAction.h"
#include "K2Node_Self.h"
#include "K2Node_DynamicCast.h"
#include "K2Node_ExecutionSequence.h"
#include "K2Node_FormatText.h"
#include "K2Node_FunctionEntry.h"
#include "K2Node_FunctionResult.h"
#include "K2Node_Literal.h"
#include "K2Node_MakeArray.h"
#include "K2Node_MakeMap.h"
#include "K2Node_MakeSet.h"
#include "K2Node_MakeStruct.h"
#include "K2Node_MathExpression.h"
#include "K2Node_RemoveDelegate.h"
#include "K2Node_SpawnActorFromClass.h"
#include "K2Node_SwitchEnum.h"
#include "K2Node_SwitchInteger.h"
#include "K2Node_SwitchName.h"
#include "K2Node_SwitchString.h"
#include "K2Node_Timeline.h"
#include "K2Node_MacroInstance.h"
#include "Engine/TimelineTemplate.h"
#include "Curves/CurveFloat.h"
#include "Curves/CurveVector.h"
#include "Curves/CurveLinearColor.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "KismetCompiler.h"
#include "GameFramework/InputSettings.h"
#include "Camera/CameraActor.h"
#include "Kismet/BlueprintMapLibrary.h"
#include "Kismet/BlueprintSetLibrary.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetArrayLibrary.h"
#include "Kismet/KismetMathLibrary.h"
#include "EdGraphSchema_K2.h"
#include "UObject/SavePackage.h"

// Declare the log category
DEFINE_LOG_CATEGORY_STATIC(LogUnrealMCP, Log, All);

FUnrealMCPBlueprintNodeCommands::FUnrealMCPBlueprintNodeCommands()
{
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    if (CommandType == TEXT("connect_blueprint_nodes"))
    {
        return HandleConnectBlueprintNodes(Params);
    }
    else if (CommandType == TEXT("add_blueprint_get_self_component_reference"))
    {
        return HandleAddBlueprintGetSelfComponentReference(Params);
    }
    else if (CommandType == TEXT("add_blueprint_event_node"))
    {
        return HandleAddBlueprintEvent(Params);
    }
    else if (CommandType == TEXT("add_blueprint_function_node"))
    {
        return HandleAddBlueprintFunctionCall(Params);
    }
    else if (CommandType == TEXT("add_blueprint_variable"))
    {
        return HandleAddBlueprintVariable(Params);
    }
    else if (CommandType == TEXT("add_blueprint_input_action_node"))
    {
        return HandleAddBlueprintInputActionNode(Params);
    }
    else if (CommandType == TEXT("add_blueprint_self_reference"))
    {
        return HandleAddBlueprintSelfReference(Params);
    }
    else if (CommandType == TEXT("find_blueprint_nodes"))
    {
        return HandleFindBlueprintNodes(Params);
    }
    else if (CommandType == TEXT("get_blueprint_variable"))
    {
        return HandleGetBlueprintVariable(Params);
    }
    else if (CommandType == TEXT("add_blueprint_variable_get_node"))
    {
        return HandleAddBlueprintVariableGetNode(Params);
    }
    else if (CommandType == TEXT("add_blueprint_variable_set_node"))
    {
        return HandleAddBlueprintVariableSetNode(Params);
    }
    else if (CommandType == TEXT("add_branch_node"))
    {
        return HandleAddBranchNode(Params);
    }
    else if (CommandType == TEXT("add_math_node"))
    {
        return HandleAddMathNode(Params);
    }
    else if (CommandType == TEXT("add_cast_node"))
    {
        return HandleAddCastNode(Params);
    }
    else if (CommandType == TEXT("add_sequence_node"))
    {
        return HandleAddSequenceNode(Params);
    }
    else if (CommandType == TEXT("add_foreach_loop_node"))
    {
        return HandleAddForEachLoopNode(Params);
    }
    else if (CommandType == TEXT("add_while_loop_node"))
    {
        return HandleAddWhileLoopNode(Params);
    }
    else if (CommandType == TEXT("add_timeline_node"))
    {
        return HandleAddTimelineNode(Params);
    }
    else if (CommandType == TEXT("add_timeline_float_track"))
    {
        return HandleAddTimelineFloatTrack(Params);
    }
    else if (CommandType == TEXT("add_timeline_keyframe"))
    {
        return HandleAddTimelineKeyframe(Params);
    }
    else if (CommandType == TEXT("add_timeline_vector_track"))
    {
        return HandleAddTimelineVectorTrack(Params);
    }
    else if (CommandType == TEXT("add_timeline_vector_keyframe"))
    {
        return HandleAddTimelineVectorKeyframe(Params);
    }
    else if (CommandType == TEXT("add_timeline_linear_color_track"))
    {
        return HandleAddTimelineLinearColorTrack(Params);
    }
    else if (CommandType == TEXT("add_timeline_linear_color_keyframe"))
    {
        return HandleAddTimelineLinearColorKeyframe(Params);
    }
    else if (CommandType == TEXT("add_timeline_event_track"))
    {
        return HandleAddTimelineEventTrack(Params);
    }
    else if (CommandType == TEXT("create_blueprint_function"))
    {
        return HandleCreateBlueprintFunction(Params);
    }
    else if (CommandType == TEXT("create_blueprint_macro"))
    {
        return HandleCreateBlueprintMacro(Params);
    }
    else if (CommandType == TEXT("create_blueprint_custom_event_graph"))
    {
        return HandleCreateBlueprintCustomEventGraph(Params);
    }
    else if (CommandType == TEXT("add_node_by_class"))
    {
        return HandleAddNodeByClass(Params);
    }
    else if (CommandType == TEXT("connect_pins_validated"))
    {
        return HandleConnectPinsValidated(Params);
    }
    else if (CommandType == TEXT("set_pin_default_value"))
    {
        return HandleSetPinDefaultValue(Params);
    }
    else if (CommandType == TEXT("add_custom_event_node"))
    {
        return HandleAddCustomEventNode(Params);
    }
    else if (CommandType == TEXT("list_blueprint_functions"))
    {
        return HandleListBlueprintFunctions(Params);
    }
    else if (CommandType == TEXT("get_node_pins"))
    {
        return HandleGetNodePins(Params);
    }
    else if (CommandType == TEXT("delete_blueprint_node"))
    {
        return HandleDeleteBlueprintNode(Params);
    }
    else if (CommandType == TEXT("add_blueprint_control_node"))
    {
        return HandleAddBlueprintControlNode(Params);
    }
    else if (CommandType == TEXT("add_blueprint_cast_node"))
    {
        return HandleAddCastNode(Params);
    }
    else if (CommandType == TEXT("add_blueprint_math_node"))
    {
        return HandleAddMathNode(Params);
    }
    else if (CommandType == TEXT("set_variable_default_value"))
    {
        return HandleSetVariableDefaultValue(Params);
    }
    else if (CommandType == TEXT("split_struct_pin"))
    {
        return HandleSplitStructPin(Params);
    }
    else if (CommandType == TEXT("recombine_struct_pin"))
    {
        return HandleRecombineStructPin(Params);
    }
    else if (CommandType == TEXT("add_dynamic_pin_to_node"))
    {
        return HandleAddDynamicPinToNode(Params);
    }
    else if (CommandType == TEXT("disconnect_blueprint_pin"))
    {
        return HandleDisconnectBlueprintPin(Params);
    }
    else if (CommandType == TEXT("move_blueprint_node"))
    {
        return HandleMoveBlueprintNode(Params);
    }
    else if (CommandType == TEXT("format_blueprint_nodes"))
    {
        return HandleFormatBlueprintNodes(Params);
    }
    else if (CommandType == TEXT("add_function_entry_node"))
    {
        return HandleAddFunctionEntryNode(Params);
    }
    else if (CommandType == TEXT("add_spawn_actor_node"))
    {
        return HandleAddSpawnActorNode(Params);
    }
    else if (CommandType == TEXT("create_blueprint_interface"))
    {
        return HandleCreateBlueprintInterface(Params);
    }
    else if (CommandType == TEXT("add_interface_function"))
    {
        return HandleAddInterfaceFunction(Params);
    }
    else if (CommandType == TEXT("implement_interface"))
    {
        return HandleImplementInterface(Params);
    }
    else if (CommandType == TEXT("add_interface_message_node"))
    {
        return HandleAddInterfaceMessageNode(Params);
    }
    else if (CommandType == TEXT("create_event_dispatcher"))
    {
        return HandleCreateEventDispatcher(Params);
    }
    else if (CommandType == TEXT("add_event_dispatcher_node"))
    {
        return HandleAddEventDispatcherNode(Params);
    }
    else if (CommandType == TEXT("set_variable_container_type"))
    {
        return HandleSetVariableContainerType(Params);
    }
    else if (CommandType == TEXT("add_collection_node") || CommandType == TEXT("add_collection_nodes"))
    {
        return HandleAddCollectionNode(Params);
    }
    else if (CommandType == TEXT("export_nodes_as_text"))
    {
        return HandleExportNodesAsText(Params);
    }
    else if (CommandType == TEXT("paste_nodes_from_text"))
    {
        return HandlePasteNodesFromText(Params);
    }
    else if (CommandType == TEXT("add_local_variable"))
    {
        return HandleAddLocalVariable(Params);
    }
    else if (CommandType == TEXT("set_function_flags"))
    {
        return HandleSetFunctionFlags(Params);
    }
    else if (CommandType == TEXT("add_format_text_node"))
    {
        return HandleAddFormatTextNode(Params);
    }
    else if (CommandType == TEXT("add_switch_node"))
    {
        return HandleAddSwitchNode(Params);
    }
    else if (CommandType == TEXT("add_async_action_node"))
    {
        return HandleAddAsyncActionNode(Params);
    }
    else if (CommandType == TEXT("add_comment_box"))
    {
        return HandleAddCommentBox(Params);
    }
    else if (CommandType == TEXT("collapse_to_function"))
    {
        return HandleCollapseToGraph(Params, false);
    }
    else if (CommandType == TEXT("collapse_to_macro"))
    {
        return HandleCollapseToGraph(Params, true);
    }
    else if (CommandType == TEXT("promote_to_variable"))
    {
        return HandlePromoteToVariable(Params);
    }
    else if (CommandType == TEXT("edit_construction_script"))
    {
        return HandleEditConstructionScript(Params);
    }
    else if (CommandType == TEXT("read_graph_logic_flow"))
    {
        return HandleReadGraphLogicFlow(Params);
    }
    else if (CommandType == TEXT("inspect_node_pins"))
    {
        return HandleInspectNodePins(Params);
    }
    else if (CommandType == TEXT("add_break_struct_node"))
    {
        return HandleAddBreakStructNode(Params);
    }
    else if (CommandType == TEXT("add_make_struct_node"))
    {
        return HandleAddMakeStructNode(Params);
    }
    else if (CommandType == TEXT("add_class_reference_node"))
    {
        return HandleAddClassReferenceNode(Params);
    }
    else if (CommandType == TEXT("add_bind_event_node"))
    {
        return HandleAddBindEventNode(Params);
    }
    else if (CommandType == TEXT("add_create_event_node"))
    {
        return HandleAddCreateEventNode(Params);
    }
    else if (CommandType == TEXT("add_global_getter"))
    {
        return HandleAddGlobalGetter(Params);
    }
    else if (CommandType == TEXT("add_math_expression_node"))
    {
        return HandleAddMathExpressionNode(Params);
    }
    else if (CommandType == TEXT("get_specific_node_error"))
    {
        return HandleGetSpecificNodeError(Params);
    }
    else if (CommandType == TEXT("resolve_wildcard_pin"))
    {
        return HandleResolveWildcardPin(Params);
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unknown blueprint node command: %s"), *CommandType));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleConnectBlueprintNodes(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString SourceNodeId;
    if (!Params->TryGetStringField(TEXT("source_node_id"), SourceNodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_node_id' parameter"));
    }

    FString TargetNodeId;
    if (!Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'target_node_id' parameter"));
    }

    FString SourcePinName;
    if (!Params->TryGetStringField(TEXT("source_pin"), SourcePinName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'source_pin' parameter"));
    }

    FString TargetPinName;
    if (!Params->TryGetStringField(TEXT("target_pin"), TargetPinName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'target_pin' parameter"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Find the nodes
    UEdGraphNode* SourceNode = nullptr;
    UEdGraphNode* TargetNode = nullptr;
    for (UEdGraphNode* Node : EventGraph->Nodes)
    {
        if (Node->NodeGuid.ToString() == SourceNodeId)
        {
            SourceNode = Node;
        }
        else if (Node->NodeGuid.ToString() == TargetNodeId)
        {
            TargetNode = Node;
        }
    }

    if (!SourceNode || !TargetNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Source or target node not found"));
    }

    // Connect the nodes
    if (FUnrealMCPCommonUtils::ConnectGraphNodes(EventGraph, SourceNode, SourcePinName, TargetNode, TargetPinName))
    {
        // Mark the blueprint as modified
        FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

        TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
        ResultObj->SetStringField(TEXT("source_node_id"), SourceNodeId);
        ResultObj->SetStringField(TEXT("target_node_id"), TargetNodeId);
        return ResultObj;
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to connect nodes"));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintGetSelfComponentReference(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ComponentName;
    if (!Params->TryGetStringField(TEXT("component_name"), ComponentName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'component_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }
    
    // We'll skip component verification since the GetAllNodes API may have changed in UE5.5
    
    // Create the variable get node directly
    UK2Node_VariableGet* GetComponentNode = NewObject<UK2Node_VariableGet>(EventGraph);
    if (!GetComponentNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create get component node"));
    }
    
    // Set up the variable reference properly for UE5.5
    FMemberReference& VarRef = GetComponentNode->VariableReference;
    VarRef.SetSelfMember(FName(*ComponentName));
    
    // Set node position
    GetComponentNode->NodePosX = NodePosition.X;
    GetComponentNode->NodePosY = NodePosition.Y;
    
    // Add to graph
    EventGraph->AddNode(GetComponentNode);
    GetComponentNode->CreateNewGuid();
    GetComponentNode->PostPlacedNewNode();
    GetComponentNode->AllocateDefaultPins();
    
    // Explicitly reconstruct node for UE5.5
    GetComponentNode->ReconstructNode();
    
    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), GetComponentNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintEvent(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString EventName;
    if (!Params->TryGetStringField(TEXT("event_name"), EventName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'event_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create the event node
    UK2Node_Event* EventNode = FUnrealMCPCommonUtils::CreateEventNode(EventGraph, EventName, NodePosition);
    if (!EventNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create event node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), EventNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintFunctionCall(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString FunctionName;
    if (!Params->TryGetStringField(TEXT("function_name"), FunctionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'function_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Check for target parameter (optional)
    FString Target;
    Params->TryGetStringField(TEXT("target"), Target);

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Find the function
    UFunction* Function = nullptr;
    UK2Node_CallFunction* FunctionNode = nullptr;
    
    // Add extensive logging for debugging
    UE_LOG(LogTemp, Display, TEXT("Looking for function '%s' in target '%s'"), 
           *FunctionName, Target.IsEmpty() ? TEXT("Blueprint") : *Target);
    
    // Check if we have a target class specified
    if (!Target.IsEmpty())
    {
        // Try to find the target class
        UClass* TargetClass = nullptr;
        
        // First try without a prefix
        TargetClass = FindFirstObjectSafe<UClass>(*Target);
        UE_LOG(LogTemp, Display, TEXT("Tried to find class '%s': %s"), 
               *Target, TargetClass ? TEXT("Found") : TEXT("Not found"));
        
        // If not found, try with U prefix (common convention for UE classes)
        if (!TargetClass && !Target.StartsWith(TEXT("U")))
        {
            FString TargetWithPrefix = FString(TEXT("U")) + Target;
            TargetClass = FindFirstObjectSafe<UClass>(*TargetWithPrefix);
            UE_LOG(LogTemp, Display, TEXT("Tried to find class '%s': %s"), 
                   *TargetWithPrefix, TargetClass ? TEXT("Found") : TEXT("Not found"));
        }
        
        // If still not found, try with common component names
        if (!TargetClass)
        {
            // Try some common component class names
            TArray<FString> PossibleClassNames;
            PossibleClassNames.Add(FString(TEXT("U")) + Target + TEXT("Component"));
            PossibleClassNames.Add(Target + TEXT("Component"));
            
            for (const FString& ClassName : PossibleClassNames)
            {
                TargetClass = FindFirstObjectSafe<UClass>(*ClassName);
                if (TargetClass)
                {
                    UE_LOG(LogTemp, Display, TEXT("Found class using alternative name '%s'"), *ClassName);
                    break;
                }
            }
        }
        
        // Special case handling for common classes like UGameplayStatics
        if (!TargetClass && Target == TEXT("UGameplayStatics"))
        {
            // For UGameplayStatics, use a direct reference to known class
            TargetClass = FindFirstObjectSafe<UClass>(TEXT("UGameplayStatics"));
            if (!TargetClass)
            {
                // Try loading it from its known package
                TargetClass = LoadObject<UClass>(nullptr, TEXT("/Script/Engine.GameplayStatics"));
                UE_LOG(LogTemp, Display, TEXT("Explicitly loading GameplayStatics: %s"), 
                       TargetClass ? TEXT("Success") : TEXT("Failed"));
            }
        }
        
        // If we found a target class, look for the function there
        if (TargetClass)
        {
            UE_LOG(LogTemp, Display, TEXT("Looking for function '%s' in class '%s'"), 
                   *FunctionName, *TargetClass->GetName());
                   
            // First try exact name
            Function = TargetClass->FindFunctionByName(*FunctionName);
            
            // If not found, try class hierarchy
            UClass* CurrentClass = TargetClass;
            while (!Function && CurrentClass)
            {
                UE_LOG(LogTemp, Display, TEXT("Searching in class: %s"), *CurrentClass->GetName());
                
                // Try exact match
                Function = CurrentClass->FindFunctionByName(*FunctionName);
                
                // Try case-insensitive match
                if (!Function)
                {
                    for (TFieldIterator<UFunction> FuncIt(CurrentClass); FuncIt; ++FuncIt)
                    {
                        UFunction* AvailableFunc = *FuncIt;
                        UE_LOG(LogTemp, Display, TEXT("  - Available function: %s"), *AvailableFunc->GetName());
                        
                        if (AvailableFunc->GetName().Equals(FunctionName, ESearchCase::IgnoreCase))
                        {
                            UE_LOG(LogTemp, Display, TEXT("  - Found case-insensitive match: %s"), *AvailableFunc->GetName());
                            Function = AvailableFunc;
                            break;
                        }
                    }
                }
                
                // Move to parent class
                CurrentClass = CurrentClass->GetSuperClass();
            }
            
            // Special handling for known functions
            if (!Function)
            {
                if (TargetClass->GetName() == TEXT("GameplayStatics") && 
                    (FunctionName == TEXT("GetActorOfClass") || FunctionName.Equals(TEXT("GetActorOfClass"), ESearchCase::IgnoreCase)))
                {
                    UE_LOG(LogTemp, Display, TEXT("Using special case handling for GameplayStatics::GetActorOfClass"));
                    
                    // Create the function node directly
                    FunctionNode = NewObject<UK2Node_CallFunction>(EventGraph);
                    if (FunctionNode)
                    {
                        // Direct setup for known function
                        FunctionNode->FunctionReference.SetExternalMember(
                            FName(TEXT("GetActorOfClass")), 
                            TargetClass
                        );
                        
                        FunctionNode->NodePosX = NodePosition.X;
                        FunctionNode->NodePosY = NodePosition.Y;
                        EventGraph->AddNode(FunctionNode);
                        FunctionNode->CreateNewGuid();
                        FunctionNode->PostPlacedNewNode();
                        FunctionNode->AllocateDefaultPins();
                        
                        UE_LOG(LogTemp, Display, TEXT("Created GetActorOfClass node directly"));
                        
                        // List all pins
                        for (UEdGraphPin* Pin : FunctionNode->Pins)
                        {
                            UE_LOG(LogTemp, Display, TEXT("  - Pin: %s, Direction: %d, Category: %s"), 
                                   *Pin->PinName.ToString(), (int32)Pin->Direction, *Pin->PinType.PinCategory.ToString());
                        }
                    }
                }
            }
        }
    }
    
    // If we still haven't found the function, try in the blueprint's class
    if (!Function && !FunctionNode)
    {
        UE_LOG(LogTemp, Display, TEXT("Trying to find function in blueprint class"));
        Function = Blueprint->GeneratedClass->FindFunctionByName(*FunctionName);
    }
    
    // Create the function call node if we found the function
    if (Function && !FunctionNode)
    {
        FunctionNode = FUnrealMCPCommonUtils::CreateFunctionCallNode(EventGraph, Function, NodePosition);
    }
    
    if (!FunctionNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Function not found: %s in target %s"), *FunctionName, Target.IsEmpty() ? TEXT("Blueprint") : *Target));
    }

    // Set parameters if provided
    if (Params->HasField(TEXT("params")))
    {
        const TSharedPtr<FJsonObject>* ParamsObj;
        if (Params->TryGetObjectField(TEXT("params"), ParamsObj))
        {
            // Process parameters
            for (const TPair<FString, TSharedPtr<FJsonValue>>& Param : (*ParamsObj)->Values)
            {
                const FString& ParamName = Param.Key;
                const TSharedPtr<FJsonValue>& ParamValue = Param.Value;
                
                // Find the parameter pin
                UEdGraphPin* ParamPin = FUnrealMCPCommonUtils::FindPin(FunctionNode, ParamName, EGPD_Input);
                if (ParamPin)
                {
                    UE_LOG(LogTemp, Display, TEXT("Found parameter pin '%s' of category '%s'"), 
                           *ParamName, *ParamPin->PinType.PinCategory.ToString());
                    UE_LOG(LogTemp, Display, TEXT("  Current default value: '%s'"), *ParamPin->DefaultValue);
                    if (ParamPin->PinType.PinSubCategoryObject.IsValid())
                    {
                        UE_LOG(LogTemp, Display, TEXT("  Pin subcategory: '%s'"), 
                               *ParamPin->PinType.PinSubCategoryObject->GetName());
                    }
                    
                    // Set parameter based on type
                    if (ParamValue->Type == EJson::String)
                    {
                        FString StringVal = ParamValue->AsString();
                        UE_LOG(LogTemp, Display, TEXT("  Setting string parameter '%s' to: '%s'"), 
                               *ParamName, *StringVal);
                        
                        // Handle class reference parameters (e.g., ActorClass in GetActorOfClass)
                        if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Class)
                        {
                            // For class references, we require the exact class name with proper prefix
                            // - Actor classes must start with 'A' (e.g., ACameraActor)
                            // - Non-actor classes must start with 'U' (e.g., UObject)
                            const FString& ClassName = StringVal;
                            
                            // TODO: This likely won't work in UE5.5+, so don't rely on it.
                            UClass* Class = FindFirstObjectSafe<UClass>(*ClassName);

                            if (!Class)
                            {
                                Class = LoadObject<UClass>(nullptr, *ClassName);
                                UE_LOG(LogUnrealMCP, Display, TEXT("FindObject<UClass> failed. Assuming soft path  path: %s"), *ClassName);
                            }
                            
                            // If not found, try with Engine module path
                            if (!Class)
                            {
                                FString EngineClassName = FString::Printf(TEXT("/Script/Engine.%s"), *ClassName);
                                Class = LoadObject<UClass>(nullptr, *EngineClassName);
                                UE_LOG(LogUnrealMCP, Display, TEXT("Trying Engine module path: %s"), *EngineClassName);
                            }
                            
                            if (!Class)
                            {
                                UE_LOG(LogUnrealMCP, Error, TEXT("Failed to find class '%s'. Make sure to use the exact class name with proper prefix (A for actors, U for non-actors)"), *ClassName);
                                return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to find class '%s'"), *ClassName));
                            }

                            const UEdGraphSchema_K2* K2Schema = Cast<const UEdGraphSchema_K2>(EventGraph->GetSchema());
                            if (!K2Schema)
                            {
                                UE_LOG(LogUnrealMCP, Error, TEXT("Failed to get K2Schema"));
                                return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get K2Schema"));
                            }

                            K2Schema->TrySetDefaultObject(*ParamPin, Class);
                            if (ParamPin->DefaultObject != Class)
                            {
                                UE_LOG(LogUnrealMCP, Error, TEXT("Failed to set class reference for pin '%s' to '%s'"), *ParamPin->PinName.ToString(), *ClassName);
                                return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to set class reference for pin '%s'"), *ParamPin->PinName.ToString()));
                            }

                            UE_LOG(LogUnrealMCP, Log, TEXT("Successfully set class reference for pin '%s' to '%s'"), *ParamPin->PinName.ToString(), *ClassName);
                            continue;
                        }
                        else if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Int)
                        {
                            // Ensure we're using an integer value (no decimal)
                            int32 IntValue = FMath::RoundToInt(ParamValue->AsNumber());
                            ParamPin->DefaultValue = FString::FromInt(IntValue);
                            UE_LOG(LogTemp, Display, TEXT("  Set integer parameter '%s' to: %d (string: '%s')"), 
                                   *ParamName, IntValue, *ParamPin->DefaultValue);
                        }
                        else if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Float)
                        {
                            // For other numeric types
                            float FloatValue = ParamValue->AsNumber();
                            ParamPin->DefaultValue = FString::SanitizeFloat(FloatValue);
                            UE_LOG(LogTemp, Display, TEXT("  Set float parameter '%s' to: %f (string: '%s')"), 
                                   *ParamName, FloatValue, *ParamPin->DefaultValue);
                        }
                        else if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Boolean)
                        {
                            bool BoolValue = ParamValue->AsBool();
                            ParamPin->DefaultValue = BoolValue ? TEXT("true") : TEXT("false");
                            UE_LOG(LogTemp, Display, TEXT("  Set boolean parameter '%s' to: %s"), 
                                   *ParamName, *ParamPin->DefaultValue);
                        }
                        else if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct && ParamPin->PinType.PinSubCategoryObject == TBaseStructure<FVector>::Get())
                        {
                            // Handle array parameters - like Vector parameters
                            const TArray<TSharedPtr<FJsonValue>>* ArrayValue;
                            if (ParamValue->TryGetArray(ArrayValue))
                            {
                                // Check if this could be a vector (array of 3 numbers)
                                if (ArrayValue->Num() == 3)
                                {
                                    // Create a proper vector string: (X=0.0,Y=0.0,Z=1000.0)
                                    float X = (*ArrayValue)[0]->AsNumber();
                                    float Y = (*ArrayValue)[1]->AsNumber();
                                    float Z = (*ArrayValue)[2]->AsNumber();
                                    
                                    FString VectorString = FString::Printf(TEXT("(X=%f,Y=%f,Z=%f)"), X, Y, Z);
                                    ParamPin->DefaultValue = VectorString;
                                    
                                    UE_LOG(LogTemp, Display, TEXT("  Set vector parameter '%s' to: %s"), 
                                           *ParamName, *VectorString);
                                    UE_LOG(LogTemp, Display, TEXT("  Final pin value: '%s'"), 
                                           *ParamPin->DefaultValue);
                                }
                                else
                                {
                                    UE_LOG(LogTemp, Warning, TEXT("Array parameter type not fully supported yet"));
                                }
                            }
                        }
                    }
                    else if (ParamValue->Type == EJson::Number)
                    {
                        // Handle integer vs float parameters correctly
                        if (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Int)
                        {
                            // Ensure we're using an integer value (no decimal)
                            int32 IntValue = FMath::RoundToInt(ParamValue->AsNumber());
                            ParamPin->DefaultValue = FString::FromInt(IntValue);
                            UE_LOG(LogTemp, Display, TEXT("  Set integer parameter '%s' to: %d (string: '%s')"), 
                                   *ParamName, IntValue, *ParamPin->DefaultValue);
                        }
                        else
                        {
                            // For other numeric types
                            float FloatValue = ParamValue->AsNumber();
                            ParamPin->DefaultValue = FString::SanitizeFloat(FloatValue);
                            UE_LOG(LogTemp, Display, TEXT("  Set float parameter '%s' to: %f (string: '%s')"), 
                                   *ParamName, FloatValue, *ParamPin->DefaultValue);
                        }
                    }
                    else if (ParamValue->Type == EJson::Boolean)
                    {
                        bool BoolValue = ParamValue->AsBool();
                        ParamPin->DefaultValue = BoolValue ? TEXT("true") : TEXT("false");
                        UE_LOG(LogTemp, Display, TEXT("  Set boolean parameter '%s' to: %s"), 
                               *ParamName, *ParamPin->DefaultValue);
                    }
                    else if (ParamValue->Type == EJson::Array)
                    {
                        UE_LOG(LogTemp, Display, TEXT("  Processing array parameter '%s'"), *ParamName);
                        // Handle array parameters - like Vector parameters
                        const TArray<TSharedPtr<FJsonValue>>* ArrayValue;
                        if (ParamValue->TryGetArray(ArrayValue))
                        {
                            // Check if this could be a vector (array of 3 numbers)
                            if (ArrayValue->Num() == 3 && 
                                (ParamPin->PinType.PinCategory == UEdGraphSchema_K2::PC_Struct) &&
                                (ParamPin->PinType.PinSubCategoryObject == TBaseStructure<FVector>::Get()))
                            {
                                // Create a proper vector string: (X=0.0,Y=0.0,Z=1000.0)
                                float X = (*ArrayValue)[0]->AsNumber();
                                float Y = (*ArrayValue)[1]->AsNumber();
                                float Z = (*ArrayValue)[2]->AsNumber();
                                
                                FString VectorString = FString::Printf(TEXT("(X=%f,Y=%f,Z=%f)"), X, Y, Z);
                                ParamPin->DefaultValue = VectorString;
                                
                                UE_LOG(LogTemp, Display, TEXT("  Set vector parameter '%s' to: %s"), 
                                       *ParamName, *VectorString);
                                UE_LOG(LogTemp, Display, TEXT("  Final pin value: '%s'"), 
                                       *ParamPin->DefaultValue);
                            }
                            else
                            {
                                UE_LOG(LogTemp, Warning, TEXT("Array parameter type not fully supported yet"));
                            }
                        }
                    }
                    // Add handling for other types as needed
                }
                else
                {
                    UE_LOG(LogTemp, Warning, TEXT("Parameter pin '%s' not found"), *ParamName);
                }
            }
        }
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), FunctionNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintVariable(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString VariableName;
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }

    FString VariableType;
    if (!Params->TryGetStringField(TEXT("variable_type"), VariableType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_type' parameter"));
    }

    // Get optional parameters
    bool IsExposed = false;
    if (Params->HasField(TEXT("is_exposed")))
    {
        IsExposed = Params->GetBoolField(TEXT("is_exposed"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Create variable based on type
    FEdGraphPinType PinType;
    
    // Set up pin type based on variable_type string
    if (VariableType == TEXT("Boolean"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
    }
    else if (VariableType == TEXT("Integer") || VariableType == TEXT("Int"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
    }
    else if (VariableType == TEXT("Float"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Float;
    }
    else if (VariableType == TEXT("String"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_String;
    }
    else if (VariableType == TEXT("Vector"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
        PinType.PinSubCategoryObject = TBaseStructure<FVector>::Get();
    }
    else if (VariableType == TEXT("Rotator"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
        PinType.PinSubCategoryObject = TBaseStructure<FRotator>::Get();
    }
    else if (VariableType == TEXT("Transform"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
        PinType.PinSubCategoryObject = TBaseStructure<FTransform>::Get();
    }
    else if (VariableType == TEXT("Name"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Name;
    }
    else if (VariableType == TEXT("Text"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
    }
    else if (VariableType == TEXT("Byte"))
    {
        PinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported variable type: %s"), *VariableType));
    }

    // Create the variable
    FBlueprintEditorUtils::AddMemberVariable(Blueprint, FName(*VariableName), PinType);

    // Set variable properties
    FBPVariableDescription* NewVar = nullptr;
    for (FBPVariableDescription& Variable : Blueprint->NewVariables)
    {
        if (Variable.VarName == FName(*VariableName))
        {
            NewVar = &Variable;
            break;
        }
    }

    FString DefaultValueStr;
    const bool bHasDefault = Params->HasField(TEXT("default_value"));
    if (bHasDefault)
    {
        // Accept any JSON scalar (string/number/bool) and convert to its string form.
        TSharedPtr<FJsonValue> DV = Params->TryGetField(TEXT("default_value"));
        if (DV.IsValid())
        {
            switch (DV->Type)
            {
                case EJson::String: DefaultValueStr = DV->AsString(); break;
                case EJson::Number: DefaultValueStr = FString::SanitizeFloat(DV->AsNumber()); break;
                case EJson::Boolean: DefaultValueStr = DV->AsBool() ? TEXT("true") : TEXT("false"); break;
                default: DV->TryGetString(DefaultValueStr); break;
            }
        }
    }

    if (NewVar)
    {
        if (IsExposed)
        {
            NewVar->PropertyFlags |= CPF_Edit;
        }
        if (bHasDefault)
        {
            NewVar->DefaultValue = DefaultValueStr;
        }
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("variable_name"), VariableName);
    ResultObj->SetStringField(TEXT("variable_type"), VariableType);
    if (bHasDefault)
    {
        ResultObj->SetStringField(TEXT("default_value"), DefaultValueStr);
    }
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintInputActionNode(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString ActionName;
    if (!Params->TryGetStringField(TEXT("action_name"), ActionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'action_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create the input action node
    UK2Node_InputAction* InputActionNode = FUnrealMCPCommonUtils::CreateInputActionNode(EventGraph, ActionName, NodePosition);
    if (!InputActionNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create input action node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), InputActionNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintSelfReference(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    // Get position parameters (optional)
    FVector2D NodePosition(0.0f, 0.0f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create the self node
    UK2Node_Self* SelfNode = FUnrealMCPCommonUtils::CreateSelfReferenceNode(EventGraph, NodePosition);
    if (!SelfNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create self node"));
    }

    // Mark the blueprint as modified
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetStringField(TEXT("node_id"), SelfNode->NodeGuid.ToString());
    return ResultObj;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleFindBlueprintNodes(const TSharedPtr<FJsonObject>& Params)
{
    // Get required parameters
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString NodeType;
    Params->TryGetStringField(TEXT("node_type"), NodeType);

    // Find the blueprint
    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    // Get the event graph
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to get event graph"));
    }

    // Create JSON arrays for both the legacy GUID-only shape and richer node details.
    TArray<TSharedPtr<FJsonValue>> NodeGuidArray;
    TArray<TSharedPtr<FJsonValue>> NodeArray;

    FString EventName;
    Params->TryGetStringField(TEXT("event_name"), EventName);

    for (UEdGraphNode* Node : EventGraph->Nodes)
    {
        if (!Node)
        {
            continue;
        }

        FString ResolvedType = TEXT("Node");
        FString ResolvedName = Node->GetNodeTitle(ENodeTitleType::ListView).ToString();
        FString MemberName;

        if (UK2Node_Event* EventNode = Cast<UK2Node_Event>(Node))
        {
            ResolvedType = TEXT("Event");
            MemberName = EventNode->EventReference.GetMemberName().ToString();
            if (MemberName.IsEmpty())
            {
                MemberName = EventNode->CustomFunctionName.ToString();
            }
            ResolvedName = MemberName.IsEmpty() ? ResolvedName : MemberName;
        }
        else if (UK2Node_CallFunction* FunctionNode = Cast<UK2Node_CallFunction>(Node))
        {
            ResolvedType = TEXT("Function");
            MemberName = FunctionNode->FunctionReference.GetMemberName().ToString();
            ResolvedName = MemberName.IsEmpty() ? ResolvedName : MemberName;
        }
        else if (Cast<UK2Node_VariableGet>(Node))
        {
            ResolvedType = TEXT("VariableGet");
        }
        else if (Cast<UK2Node_VariableSet>(Node))
        {
            ResolvedType = TEXT("VariableSet");
        }

        if (!NodeType.IsEmpty() && !ResolvedType.Equals(NodeType, ESearchCase::IgnoreCase))
        {
            continue;
        }
        if (!EventName.IsEmpty())
        {
            const bool bEventNameMatches =
                ResolvedName.Equals(EventName, ESearchCase::IgnoreCase) ||
                ResolvedName.Equals(TEXT("Receive") + EventName, ESearchCase::IgnoreCase);
            if (!ResolvedType.Equals(TEXT("Event"), ESearchCase::IgnoreCase) ||
                !bEventNameMatches)
            {
                continue;
            }
        }

        TSharedPtr<FJsonObject> NodeObj = MakeShared<FJsonObject>();
        NodeObj->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
        NodeObj->SetStringField(TEXT("guid"), Node->NodeGuid.ToString());
        NodeObj->SetStringField(TEXT("type"), ResolvedType);
        NodeObj->SetStringField(TEXT("name"), ResolvedName);
        NodeObj->SetNumberField(TEXT("x"), Node->NodePosX);
        NodeObj->SetNumberField(TEXT("y"), Node->NodePosY);

        NodeGuidArray.Add(MakeShared<FJsonValueString>(Node->NodeGuid.ToString()));
        NodeArray.Add(MakeShared<FJsonValueObject>(NodeObj));
    }
    
    TSharedPtr<FJsonObject> ResultObj = MakeShared<FJsonObject>();
    ResultObj->SetBoolField(TEXT("success"), true);
    ResultObj->SetArrayField(TEXT("node_guids"), NodeGuidArray);
    ResultObj->SetArrayField(TEXT("nodes"), NodeArray);
    ResultObj->SetNumberField(TEXT("count"), NodeArray.Num());

    return ResultObj;
}

namespace
{
    FString PinTypeToString(const FEdGraphPinType& PinType)
    {
        const FName Cat = PinType.PinCategory;
        if (Cat == UEdGraphSchema_K2::PC_Boolean) return TEXT("Boolean");
        if (Cat == UEdGraphSchema_K2::PC_Int)     return TEXT("Integer");
        if (Cat == UEdGraphSchema_K2::PC_Float)   return TEXT("Float");
        if (Cat == UEdGraphSchema_K2::PC_String)  return TEXT("String");
        if (Cat == UEdGraphSchema_K2::PC_Name)    return TEXT("Name");
        if (Cat == UEdGraphSchema_K2::PC_Text)    return TEXT("Text");
        if (Cat == UEdGraphSchema_K2::PC_Byte)    return TEXT("Byte");
        if (Cat == UEdGraphSchema_K2::PC_Struct)
        {
            if (UScriptStruct* SS = Cast<UScriptStruct>(PinType.PinSubCategoryObject.Get()))
            {
                return SS->GetName();
            }
            return TEXT("Struct");
        }
        return Cat.ToString();
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleGetBlueprintVariable(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, VariableName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    for (const FBPVariableDescription& Var : Blueprint->NewVariables)
    {
        if (Var.VarName == FName(*VariableName))
        {
            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("name"), Var.VarName.ToString());
            Result->SetStringField(TEXT("type"), PinTypeToString(Var.VarType));
            Result->SetStringField(TEXT("default_value"), Var.DefaultValue);
            Result->SetBoolField(TEXT("is_exposed"), (Var.PropertyFlags & CPF_Edit) != 0);
            return Result;
        }
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Variable '%s' not found in blueprint '%s'"), *VariableName, *BlueprintName));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintVariableGetNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, VariableName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    UK2Node_VariableGet* GetNode = FUnrealMCPCommonUtils::CreateVariableGetNode(EventGraph, Blueprint, VariableName, NodePosition);
    if (!GetNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create Get node for variable '%s'"), *VariableName));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), GetNode->NodeGuid.ToString());
    Result->SetStringField(TEXT("variable_name"), VariableName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintVariableSetNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, VariableName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }
    if (!Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'variable_name' parameter"));
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    UK2Node_VariableSet* SetNode = FUnrealMCPCommonUtils::CreateVariableSetNode(EventGraph, Blueprint, VariableName, NodePosition);
    if (!SetNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create Set node for variable '%s'"), *VariableName));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), SetNode->NodeGuid.ToString());
    Result->SetStringField(TEXT("variable_name"), VariableName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBranchNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    UK2Node_IfThenElse* BranchNode = NewObject<UK2Node_IfThenElse>(EventGraph);
    BranchNode->CreateNewGuid();
    BranchNode->NodePosX = NodePosition.X;
    BranchNode->NodePosY = NodePosition.Y;
    EventGraph->AddNode(BranchNode, true, false);
    BranchNode->PostPlacedNewNode();
    BranchNode->AllocateDefaultPins();

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), BranchNode->NodeGuid.ToString());
    return Result;
}

// ---------------------------------------------------------------------------
// Math node  (UK2Node_CallFunction wrapping UKismetMathLibrary)
// ---------------------------------------------------------------------------

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddMathNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString Operation;
    if (!Params->TryGetStringField(TEXT("operation"), Operation))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'operation' parameter"));
    }

    const FString OperationAlias = Operation.Replace(TEXT(" "), TEXT("")).Replace(TEXT("_"), TEXT("")).ToLower();
    if (OperationAlias == TEXT("add") || OperationAlias == TEXT("addfloat"))
    {
        Operation = TEXT("Add_FloatFloat");
    }
    else if (OperationAlias == TEXT("multiply") || OperationAlias == TEXT("mul") || OperationAlias == TEXT("multiplyfloat"))
    {
        Operation = TEXT("Multiply_FloatFloat");
    }
    else if (OperationAlias == TEXT("subtract") || OperationAlias == TEXT("sub"))
    {
        Operation = TEXT("Subtract_FloatFloat");
    }
    else if (OperationAlias == TEXT("divide") || OperationAlias == TEXT("div"))
    {
        Operation = TEXT("Divide_FloatFloat");
    }
    else if (OperationAlias == TEXT("clamp") || OperationAlias == TEXT("clampfloat"))
    {
        Operation = TEXT("FClamp");
    }
    else if (OperationAlias == TEXT("addvector") || OperationAlias == TEXT("vectoradd"))
    {
        Operation = TEXT("Add_VectorVector");
    }
    else if (OperationAlias == TEXT("multiplyvector") || OperationAlias == TEXT("vectormultiply"))
    {
        Operation = TEXT("Multiply_VectorFloat");
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    UClass* MathLibClass = UKismetMathLibrary::StaticClass();
    UFunction* MathFunc = MathLibClass->FindFunctionByName(FName(*Operation));
    if (!MathFunc)
    {
        // Case-insensitive fallback
        for (TFieldIterator<UFunction> It(MathLibClass); It; ++It)
        {
            if (It->GetName().Equals(Operation, ESearchCase::IgnoreCase))
            {
                MathFunc = *It;
                break;
            }
        }
    }

    if (!MathFunc)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("KismetMathLibrary function not found: %s"), *Operation));
    }

    UK2Node_CallFunction* MathNode = FUnrealMCPCommonUtils::CreateFunctionCallNode(EventGraph, MathFunc, NodePosition);
    if (!MathNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create math function node"));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), MathNode->NodeGuid.ToString());
    Result->SetStringField(TEXT("operation"), Operation);
    return Result;
}

// ---------------------------------------------------------------------------
// Cast node  (UK2Node_DynamicCast)
// ---------------------------------------------------------------------------

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddCastNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString CastToClass;
    if (!Params->TryGetStringField(TEXT("cast_to_class"), CastToClass))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'cast_to_class' parameter"));
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    // Resolve target class (try with/without prefix)
    UClass* TargetClass = FindFirstObjectSafe<UClass>(*CastToClass);
    if (!TargetClass)
    {
        TargetClass = LoadObject<UClass>(nullptr, *FString::Printf(TEXT("/Script/Engine.%s"), *CastToClass));
    }
    if (!TargetClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Class not found: %s"), *CastToClass));
    }

    UK2Node_DynamicCast* CastNode = NewObject<UK2Node_DynamicCast>(EventGraph);
    CastNode->TargetType = TargetClass;
    CastNode->NodePosX = NodePosition.X;
    CastNode->NodePosY = NodePosition.Y;
    EventGraph->AddNode(CastNode, true, false);
    CastNode->CreateNewGuid();
    CastNode->PostPlacedNewNode();
    CastNode->AllocateDefaultPins();

    // Optional auto-wiring of exec pins to neighbouring nodes by GUID.
    // Pin names on UK2Node_DynamicCast: exec in = "execute", success then = "then",
    // failure then = "CastFailed". The output object pin is named "AsXxxx" where
    // Xxxx is the target class name (no prefix).
    auto FindNodeByGuid = [EventGraph](const FString& GuidStr) -> UEdGraphNode*
    {
        if (GuidStr.IsEmpty())
        {
            return nullptr;
        }
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            if (Node && Node->NodeGuid.ToString() == GuidStr)
            {
                return Node;
            }
        }
        return nullptr;
    };

    bool bConnectedSuccess = false;
    bool bConnectedFail = false;
    bool bConnectedPrev = false;

    FString NextSuccessGuid;
    Params->TryGetStringField(TEXT("next_node_id_on_success"), NextSuccessGuid);
    if (!NextSuccessGuid.IsEmpty())
    {
        if (UEdGraphNode* SuccessTarget = FindNodeByGuid(NextSuccessGuid))
        {
            FString TargetPin = TEXT("execute");
            Params->TryGetStringField(TEXT("next_pin_name_on_success"), TargetPin);
            bConnectedSuccess = FUnrealMCPCommonUtils::ConnectGraphNodes(EventGraph, CastNode, TEXT("then"), SuccessTarget, TargetPin);
        }
    }

    FString NextFailGuid;
    Params->TryGetStringField(TEXT("next_node_id_on_fail"), NextFailGuid);
    if (!NextFailGuid.IsEmpty())
    {
        if (UEdGraphNode* FailTarget = FindNodeByGuid(NextFailGuid))
        {
            FString TargetPin = TEXT("execute");
            Params->TryGetStringField(TEXT("next_pin_name_on_fail"), TargetPin);
            bConnectedFail = FUnrealMCPCommonUtils::ConnectGraphNodes(EventGraph, CastNode, TEXT("CastFailed"), FailTarget, TargetPin);
        }
    }

    FString PrevGuid;
    Params->TryGetStringField(TEXT("prev_node_id"), PrevGuid);
    if (!PrevGuid.IsEmpty())
    {
        if (UEdGraphNode* PrevNode = FindNodeByGuid(PrevGuid))
        {
            FString PrevPin = TEXT("then");
            Params->TryGetStringField(TEXT("prev_pin_name"), PrevPin);
            bConnectedPrev = FUnrealMCPCommonUtils::ConnectGraphNodes(EventGraph, PrevNode, PrevPin, CastNode, TEXT("execute"));
        }
    }

    // Compute the name of the output object pin ("AsClassName" — strips A/U prefix).
    FString AsClassPinName;
    {
        FString CleanClassName = TargetClass->GetName();
        if (CleanClassName.Len() > 1 && (CleanClassName[0] == TEXT('A') || CleanClassName[0] == TEXT('U')) && FChar::IsUpper(CleanClassName[1]))
        {
            CleanClassName = CleanClassName.Mid(1);
        }
        AsClassPinName = FString::Printf(TEXT("As%s"), *CleanClassName);
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), CastNode->NodeGuid.ToString());
    Result->SetStringField(TEXT("cast_to_class"), CastToClass);
    Result->SetStringField(TEXT("as_class_pin"), AsClassPinName);
    Result->SetStringField(TEXT("success_exec_pin"), TEXT("then"));
    Result->SetStringField(TEXT("fail_exec_pin"), TEXT("CastFailed"));
    Result->SetBoolField(TEXT("connected_on_success"), bConnectedSuccess);
    Result->SetBoolField(TEXT("connected_on_fail"), bConnectedFail);
    Result->SetBoolField(TEXT("connected_prev"), bConnectedPrev);
    return Result;
}

// ---------------------------------------------------------------------------
// Sequence node  (UK2Node_ExecutionSequence)
// ---------------------------------------------------------------------------

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddSequenceNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    int32 NumOutputs = 2;
    if (Params->HasField(TEXT("num_outputs")))
    {
        Params->TryGetNumberField(TEXT("num_outputs"), NumOutputs);
        NumOutputs = FMath::Clamp(NumOutputs, 2, 16);
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    UK2Node_ExecutionSequence* SeqNode = NewObject<UK2Node_ExecutionSequence>(EventGraph);
    SeqNode->NodePosX = NodePosition.X;
    SeqNode->NodePosY = NodePosition.Y;
    EventGraph->AddNode(SeqNode, true, false);
    SeqNode->CreateNewGuid();
    SeqNode->PostPlacedNewNode();
    SeqNode->AllocateDefaultPins();

    // AllocateDefaultPins creates 2 outputs; add extras as needed
    for (int32 i = 2; i < NumOutputs; ++i)
    {
        SeqNode->AddInputPin();
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), SeqNode->NodeGuid.ToString());
    Result->SetNumberField(TEXT("num_outputs"), NumOutputs);
    return Result;
}

// ---------------------------------------------------------------------------
// ForEach Loop  (UK2Node_MacroInstance – StandardMacros)
// ---------------------------------------------------------------------------

namespace
{
    UK2Node_MacroInstance* CreateStandardMacroNode(UEdGraph* EventGraph, const FString& MacroName, const FVector2D& Position)
    {
        const FString MacroLibPath = TEXT("/Engine/EditorBlueprintResources/StandardMacros.StandardMacros");
        UBlueprint* MacroLib = Cast<UBlueprint>(StaticLoadObject(UBlueprint::StaticClass(), nullptr, *MacroLibPath));
        if (!MacroLib)
        {
            return nullptr;
        }

        UEdGraph* TargetGraph = nullptr;
        for (UEdGraph* Graph : MacroLib->MacroGraphs)
        {
            if (Graph && Graph->GetName() == MacroName)
            {
                TargetGraph = Graph;
                break;
            }
        }
        if (!TargetGraph)
        {
            return nullptr;
        }

        UK2Node_MacroInstance* MacroNode = NewObject<UK2Node_MacroInstance>(EventGraph);
        MacroNode->SetMacroGraph(TargetGraph);
        MacroNode->NodePosX = Position.X;
        MacroNode->NodePosY = Position.Y;
        EventGraph->AddNode(MacroNode, true, false);
        MacroNode->CreateNewGuid();
        MacroNode->PostPlacedNewNode();
        MacroNode->AllocateDefaultPins();
        return MacroNode;
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddForEachLoopNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    UK2Node_MacroInstance* MacroNode = CreateStandardMacroNode(EventGraph, TEXT("ForEachLoop"), NodePosition);
    if (!MacroNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create ForEachLoop macro node"));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), MacroNode->NodeGuid.ToString());
    return Result;
}

// ---------------------------------------------------------------------------
// While Loop  (UK2Node_MacroInstance – StandardMacros)
// ---------------------------------------------------------------------------

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddWhileLoopNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    UK2Node_MacroInstance* MacroNode = CreateStandardMacroNode(EventGraph, TEXT("WhileLoop"), NodePosition);
    if (!MacroNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create WhileLoop macro node"));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), MacroNode->NodeGuid.ToString());
    return Result;
}

// ---------------------------------------------------------------------------
// Timeline node  (UK2Node_Timeline)
// ---------------------------------------------------------------------------

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddTimelineNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString TimelineName;
    if (!Params->TryGetStringField(TEXT("timeline_name"), TimelineName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'timeline_name' parameter"));
    }

    FVector2D NodePosition(0.f, 0.f);
    if (Params->HasField(TEXT("node_position")))
    {
        NodePosition = FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (!EventGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to find or create EventGraph"));
    }

    // Ensure the timeline name is unique within this Blueprint
    const FName TLName(*TimelineName);
    if (Blueprint->FindTimelineTemplateByVariableName(TLName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(
            FString::Printf(TEXT("Timeline '%s' already exists in blueprint"), *TimelineName));
    }

    UK2Node_Timeline* TimelineNode = NewObject<UK2Node_Timeline>(EventGraph);
    TimelineNode->TimelineName = TLName;
    TimelineNode->NodePosX = NodePosition.X;
    TimelineNode->NodePosY = NodePosition.Y;
    EventGraph->AddNode(TimelineNode, true, false);
    TimelineNode->CreateNewGuid();
    TimelineNode->PostPlacedNewNode();  // creates the FTimelineTemplate in the Blueprint
    TimelineNode->AllocateDefaultPins();

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("node_id"), TimelineNode->NodeGuid.ToString());
    Result->SetStringField(TEXT("timeline_name"), TimelineName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddTimelineFloatTrack(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString TimelineName;
    if (!Params->TryGetStringField(TEXT("timeline_name"), TimelineName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'timeline_name' parameter"));
    }

    FString TrackName;
    if (!Params->TryGetStringField(TEXT("track_name"), TrackName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'track_name' parameter"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UTimelineTemplate* TimelineTemplate = Blueprint->FindTimelineTemplateByVariableName(FName(*TimelineName));
    if (!TimelineTemplate)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Timeline '%s' not found in blueprint"), *TimelineName));
    }

    // Create a new Float curve
    UCurveFloat* NewCurve = NewObject<UCurveFloat>(Blueprint, UCurveFloat::StaticClass(), NAME_None, RF_NoFlags);
    
    FTTFloatTrack NewTrack;
    NewTrack.SetTrackName(FName(*TrackName), TimelineTemplate);
    NewTrack.CurveFloat = NewCurve;
    
    TimelineTemplate->FloatTracks.Add(NewTrack);
    
    // Update the node's pins to reflect the new track
    UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    if (EventGraph)
    {
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            if (UK2Node_Timeline* TLNode = Cast<UK2Node_Timeline>(Node))
            {
                if (TLNode->TimelineName == FName(*TimelineName))
                {
                    TLNode->ReconstructNode();
                    break;
                }
            }
        }
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddTimelineKeyframe(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString TimelineName;
    if (!Params->TryGetStringField(TEXT("timeline_name"), TimelineName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'timeline_name' parameter"));
    }

    FString TrackName;
    if (!Params->TryGetStringField(TEXT("track_name"), TrackName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'track_name' parameter"));
    }

    double Time = 0.0;
    if (!Params->TryGetNumberField(TEXT("time"), Time))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'time' parameter"));
    }

    double Value = 0.0;
    if (!Params->TryGetNumberField(TEXT("value"), Value))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'value' parameter"));
    }

    UBlueprint* Blueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UTimelineTemplate* TimelineTemplate = Blueprint->FindTimelineTemplateByVariableName(FName(*TimelineName));
    if (!TimelineTemplate)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Timeline '%s' not found in blueprint"), *TimelineName));
    }

    bool bFoundTrack = false;
    for (FTTFloatTrack& Track : TimelineTemplate->FloatTracks)
    {
        if (Track.GetTrackName() == FName(*TrackName) && Track.CurveFloat)
        {
            Track.CurveFloat->FloatCurve.UpdateOrAddKey(Time, Value);
            bFoundTrack = true;
            break;
        }
    }

    if (!bFoundTrack)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Track '%s' not found in timeline"), *TrackName));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    return Result;
}

// ---------------------------------------------------------------------------
// Timeline Vector / LinearColor / Event tracks (Fase 0 bug fix)
// ---------------------------------------------------------------------------

namespace
{
    UTimelineTemplate* FindTimelineTemplateOrError(
        const TSharedPtr<FJsonObject>& Params,
        UBlueprint*& OutBlueprint,
        FString& OutTimelineName,
        FString& OutTrackName,
        TSharedPtr<FJsonObject>& OutError)
    {
        FString BlueprintName;
        if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
        {
            OutError = FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
            return nullptr;
        }
        if (!Params->TryGetStringField(TEXT("timeline_name"), OutTimelineName))
        {
            OutError = FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'timeline_name' parameter"));
            return nullptr;
        }
        if (!Params->TryGetStringField(TEXT("track_name"), OutTrackName))
        {
            OutError = FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'track_name' parameter"));
            return nullptr;
        }

        OutBlueprint = FUnrealMCPCommonUtils::FindBlueprint(BlueprintName);
        if (!OutBlueprint)
        {
            OutError = FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
            return nullptr;
        }

        UTimelineTemplate* Template = OutBlueprint->FindTimelineTemplateByVariableName(FName(*OutTimelineName));
        if (!Template)
        {
            OutError = FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Timeline '%s' not found in blueprint"), *OutTimelineName));
            return nullptr;
        }
        return Template;
    }

    void ReconstructTimelineNode(UBlueprint* Blueprint, const FString& TimelineName)
    {
        UEdGraph* EventGraph = FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
        if (!EventGraph)
        {
            return;
        }
        for (UEdGraphNode* Node : EventGraph->Nodes)
        {
            if (UK2Node_Timeline* TLNode = Cast<UK2Node_Timeline>(Node))
            {
                if (TLNode->TimelineName == FName(*TimelineName))
                {
                    TLNode->ReconstructNode();
                    break;
                }
            }
        }
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddTimelineVectorTrack(const TSharedPtr<FJsonObject>& Params)
{
    UBlueprint* Blueprint = nullptr;
    FString TimelineName, TrackName;
    TSharedPtr<FJsonObject> Error;
    UTimelineTemplate* Template = FindTimelineTemplateOrError(Params, Blueprint, TimelineName, TrackName, Error);
    if (!Template)
    {
        return Error;
    }

    UCurveVector* NewCurve = NewObject<UCurveVector>(Blueprint, UCurveVector::StaticClass(), NAME_None, RF_NoFlags);

    FTTVectorTrack NewTrack;
    NewTrack.SetTrackName(FName(*TrackName), Template);
    NewTrack.CurveVector = NewCurve;
    Template->VectorTracks.Add(NewTrack);

    ReconstructTimelineNode(Blueprint, TimelineName);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("track_name"), TrackName);
    Result->SetStringField(TEXT("track_type"), TEXT("Vector"));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddTimelineVectorKeyframe(const TSharedPtr<FJsonObject>& Params)
{
    UBlueprint* Blueprint = nullptr;
    FString TimelineName, TrackName;
    TSharedPtr<FJsonObject> Error;
    UTimelineTemplate* Template = FindTimelineTemplateOrError(Params, Blueprint, TimelineName, TrackName, Error);
    if (!Template)
    {
        return Error;
    }

    double Time = 0.0;
    if (!Params->TryGetNumberField(TEXT("time"), Time))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'time' parameter"));
    }

    if (!Params->HasField(TEXT("value")))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'value' parameter (expected [x, y, z])"));
    }
    const FVector Value = FUnrealMCPCommonUtils::GetVectorFromJson(Params, TEXT("value"));

    bool bFound = false;
    for (FTTVectorTrack& Track : Template->VectorTracks)
    {
        if (Track.GetTrackName() == FName(*TrackName) && Track.CurveVector)
        {
            Track.CurveVector->FloatCurves[0].UpdateOrAddKey(Time, Value.X);
            Track.CurveVector->FloatCurves[1].UpdateOrAddKey(Time, Value.Y);
            Track.CurveVector->FloatCurves[2].UpdateOrAddKey(Time, Value.Z);
            bFound = true;
            break;
        }
    }
    if (!bFound)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Vector track '%s' not found in timeline"), *TrackName));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddTimelineLinearColorTrack(const TSharedPtr<FJsonObject>& Params)
{
    UBlueprint* Blueprint = nullptr;
    FString TimelineName, TrackName;
    TSharedPtr<FJsonObject> Error;
    UTimelineTemplate* Template = FindTimelineTemplateOrError(Params, Blueprint, TimelineName, TrackName, Error);
    if (!Template)
    {
        return Error;
    }

    UCurveLinearColor* NewCurve = NewObject<UCurveLinearColor>(Blueprint, UCurveLinearColor::StaticClass(), NAME_None, RF_NoFlags);

    FTTLinearColorTrack NewTrack;
    NewTrack.SetTrackName(FName(*TrackName), Template);
    NewTrack.CurveLinearColor = NewCurve;
    Template->LinearColorTracks.Add(NewTrack);

    ReconstructTimelineNode(Blueprint, TimelineName);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("track_name"), TrackName);
    Result->SetStringField(TEXT("track_type"), TEXT("LinearColor"));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddTimelineLinearColorKeyframe(const TSharedPtr<FJsonObject>& Params)
{
    UBlueprint* Blueprint = nullptr;
    FString TimelineName, TrackName;
    TSharedPtr<FJsonObject> Error;
    UTimelineTemplate* Template = FindTimelineTemplateOrError(Params, Blueprint, TimelineName, TrackName, Error);
    if (!Template)
    {
        return Error;
    }

    double Time = 0.0;
    if (!Params->TryGetNumberField(TEXT("time"), Time))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'time' parameter"));
    }

    if (!Params->HasField(TEXT("value")))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'value' parameter (expected [r, g, b, a])"));
    }
    const TArray<TSharedPtr<FJsonValue>>* RawArray = nullptr;
    if (!Params->TryGetArrayField(TEXT("value"), RawArray) || RawArray->Num() < 3)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("'value' must be an array of [r, g, b] or [r, g, b, a]"));
    }
    const float R = static_cast<float>((*RawArray)[0]->AsNumber());
    const float G = static_cast<float>((*RawArray)[1]->AsNumber());
    const float B = static_cast<float>((*RawArray)[2]->AsNumber());
    const float A = RawArray->Num() >= 4 ? static_cast<float>((*RawArray)[3]->AsNumber()) : 1.0f;

    bool bFound = false;
    for (FTTLinearColorTrack& Track : Template->LinearColorTracks)
    {
        if (Track.GetTrackName() == FName(*TrackName) && Track.CurveLinearColor)
        {
            Track.CurveLinearColor->FloatCurves[0].UpdateOrAddKey(Time, R);
            Track.CurveLinearColor->FloatCurves[1].UpdateOrAddKey(Time, G);
            Track.CurveLinearColor->FloatCurves[2].UpdateOrAddKey(Time, B);
            Track.CurveLinearColor->FloatCurves[3].UpdateOrAddKey(Time, A);
            bFound = true;
            break;
        }
    }
    if (!bFound)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Linear color track '%s' not found in timeline"), *TrackName));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddTimelineEventTrack(const TSharedPtr<FJsonObject>& Params)
{
    UBlueprint* Blueprint = nullptr;
    FString TimelineName, TrackName;
    TSharedPtr<FJsonObject> Error;
    UTimelineTemplate* Template = FindTimelineTemplateOrError(Params, Blueprint, TimelineName, TrackName, Error);
    if (!Template)
    {
        return Error;
    }

    // Event tracks store keys directly on a UCurveFloat, but the curve isn't sampled — only the keys' Time values trigger the exec pin.
    UCurveFloat* NewCurve = NewObject<UCurveFloat>(Blueprint, UCurveFloat::StaticClass(), NAME_None, RF_NoFlags);

    FTTEventTrack NewTrack;
    NewTrack.SetTrackName(FName(*TrackName), Template);
    NewTrack.CurveKeys = NewCurve;

    // Optional: pre-populate event times.
    const TArray<TSharedPtr<FJsonValue>>* TimesArray = nullptr;
    if (Params->TryGetArrayField(TEXT("event_times"), TimesArray))
    {
        for (const TSharedPtr<FJsonValue>& Item : *TimesArray)
        {
            const double T = Item->AsNumber();
            NewCurve->FloatCurve.UpdateOrAddKey(T, 0.0f);
        }
    }

    Template->EventTracks.Add(NewTrack);

    ReconstructTimelineNode(Blueprint, TimelineName);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("success"), true);
    Result->SetStringField(TEXT("track_name"), TrackName);
    Result->SetStringField(TEXT("track_type"), TEXT("Event"));
    return Result;
}


// ===== Phase 2: Extended Blueprint Graph Authoring =====
namespace
{
    FString ReadStringOrDefault(const TSharedPtr<FJsonObject>& Params, const FString& FieldName, const FString& DefaultValue = TEXT(""))
    {
        FString Value;
        return Params->TryGetStringField(FieldName, Value) ? Value : DefaultValue;
    }

    FVector2D ReadNodePosition(const TSharedPtr<FJsonObject>& Params)
    {
        return Params->HasField(TEXT("node_position"))
            ? FUnrealMCPCommonUtils::GetVector2DFromJson(Params, TEXT("node_position"))
            : FVector2D(0.f, 0.f);
    }

    UBlueprint* ResolveBlueprintForGraphTool(const FString& BlueprintNameOrPath)
    {
        if (BlueprintNameOrPath.StartsWith(TEXT("/")))
        {
            if (UBlueprint* LoadedBlueprint = LoadObject<UBlueprint>(nullptr, *BlueprintNameOrPath))
            {
                return LoadedBlueprint;
            }

            const FString AssetName = FPackageName::GetShortName(BlueprintNameOrPath);
            const FString ObjectPath = FString::Printf(TEXT("%s.%s"), *BlueprintNameOrPath, *AssetName);
            if (UBlueprint* LoadedBlueprint = LoadObject<UBlueprint>(nullptr, *ObjectPath))
            {
                return LoadedBlueprint;
            }
        }

        return FUnrealMCPCommonUtils::FindBlueprint(BlueprintNameOrPath);
    }

    void CollectBlueprintGraphs(UBlueprint* Blueprint, TArray<UEdGraph*>& OutGraphs)
    {
        if (!Blueprint)
        {
            return;
        }

        for (UEdGraph* Graph : Blueprint->UbergraphPages)
        {
            if (Graph) OutGraphs.Add(Graph);
        }
        for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        {
            if (Graph) OutGraphs.Add(Graph);
        }
        for (UEdGraph* Graph : Blueprint->MacroGraphs)
        {
            if (Graph) OutGraphs.Add(Graph);
        }
        for (UEdGraph* Graph : Blueprint->DelegateSignatureGraphs)
        {
            if (Graph) OutGraphs.Add(Graph);
        }
    }

    UEdGraph* FindGraphForParams(UBlueprint* Blueprint, const TSharedPtr<FJsonObject>& Params)
    {
        if (!Blueprint)
        {
            return nullptr;
        }

        const FString GraphType = ReadStringOrDefault(Params, TEXT("graph_type")).ToLower();
        FString GraphName = ReadStringOrDefault(Params, TEXT("graph_name"));

        if (GraphType == TEXT("construction_script") || GraphName.Equals(TEXT("ConstructionScript"), ESearchCase::IgnoreCase) || GraphName.Equals(TEXT("UserConstructionScript"), ESearchCase::IgnoreCase))
        {
            if (UEdGraph* ConstructionGraph = FBlueprintEditorUtils::FindUserConstructionScript(Blueprint))
            {
                return ConstructionGraph;
            }
        }

        TArray<UEdGraph*> Graphs;
        CollectBlueprintGraphs(Blueprint, Graphs);
        if (!GraphName.IsEmpty())
        {
            for (UEdGraph* Graph : Graphs)
            {
                if (Graph && Graph->GetName().Equals(GraphName, ESearchCase::IgnoreCase))
                {
                    return Graph;
                }
            }
        }

        if (GraphType == TEXT("function"))
        {
            return Blueprint->FunctionGraphs.Num() > 0 ? Blueprint->FunctionGraphs[0] : nullptr;
        }
        if (GraphType == TEXT("macro"))
        {
            return Blueprint->MacroGraphs.Num() > 0 ? Blueprint->MacroGraphs[0] : nullptr;
        }

        return FUnrealMCPCommonUtils::FindOrCreateEventGraph(Blueprint);
    }

    UEdGraphNode* FindNodeById(UBlueprint* Blueprint, const FString& NodeId, UEdGraph** OutGraph = nullptr)
    {
        TArray<UEdGraph*> Graphs;
        CollectBlueprintGraphs(Blueprint, Graphs);
        for (UEdGraph* Graph : Graphs)
        {
            if (!Graph)
            {
                continue;
            }

            for (UEdGraphNode* Node : Graph->Nodes)
            {
                if (Node && (Node->NodeGuid.ToString() == NodeId || Node->GetName() == NodeId))
                {
                    if (OutGraph)
                    {
                        *OutGraph = Graph;
                    }
                    return Node;
                }
            }
        }
        return nullptr;
    }

    UEdGraphPin* FindPinOnNode(UEdGraphNode* Node, const FString& PinNameOrId, EEdGraphPinDirection Direction = EGPD_MAX)
    {
        if (!Node)
        {
            return nullptr;
        }

        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (!Pin)
            {
                continue;
            }

            if (Direction != EGPD_MAX && Pin->Direction != Direction)
            {
                continue;
            }

            if (Pin->PinName.ToString().Equals(PinNameOrId, ESearchCase::IgnoreCase) ||
                Pin->PinId.ToString() == PinNameOrId ||
                Pin->GetDisplayName().ToString().Equals(PinNameOrId, ESearchCase::IgnoreCase))
            {
                return Pin;
            }
        }

        return nullptr;
    }

    UEdGraphPin* FindPinFromParams(UBlueprint* Blueprint, const TSharedPtr<FJsonObject>& Params, UEdGraphNode** OutNode = nullptr, UEdGraph** OutGraph = nullptr)
    {
        FString NodeId;
        if (!Params->TryGetStringField(TEXT("node_id"), NodeId) && !Params->TryGetStringField(TEXT("pin_node_id"), NodeId))
        {
            return nullptr;
        }

        UEdGraph* Graph = nullptr;
        UEdGraphNode* Node = FindNodeById(Blueprint, NodeId, &Graph);
        if (!Node)
        {
            return nullptr;
        }

        FString PinName;
        if (!Params->TryGetStringField(TEXT("pin_name"), PinName) && !Params->TryGetStringField(TEXT("pin_id"), PinName))
        {
            return nullptr;
        }

        if (OutNode)
        {
            *OutNode = Node;
        }
        if (OutGraph)
        {
            *OutGraph = Graph;
        }

        return FindPinOnNode(Node, PinName);
    }

    UClass* ResolveClassForGraphTool(const FString& ClassNameOrPath)
    {
        if (ClassNameOrPath.IsEmpty())
        {
            return nullptr;
        }

        if (UClass* FoundClass = FindFirstObjectSafe<UClass>(*ClassNameOrPath))
        {
            return FoundClass;
        }
        if (UClass* LoadedClass = LoadObject<UClass>(nullptr, *ClassNameOrPath))
        {
            return LoadedClass;
        }

        const TArray<FString> CandidatePaths = {
            FString::Printf(TEXT("/Script/Engine.%s"), *ClassNameOrPath),
            FString::Printf(TEXT("/Script/CoreUObject.%s"), *ClassNameOrPath),
            FString::Printf(TEXT("/Script/BlueprintGraph.%s"), *ClassNameOrPath),
            FString::Printf(TEXT("/Script/UMG.%s"), *ClassNameOrPath)
        };

        for (const FString& CandidatePath : CandidatePaths)
        {
            if (UClass* LoadedClass = LoadObject<UClass>(nullptr, *CandidatePath))
            {
                return LoadedClass;
            }
        }

        if (UBlueprint* BlueprintAsset = ResolveBlueprintForGraphTool(ClassNameOrPath))
        {
            return BlueprintAsset->GeneratedClass ? BlueprintAsset->GeneratedClass.Get() : BlueprintAsset->SkeletonGeneratedClass.Get();
        }

        return nullptr;
    }

    UScriptStruct* ResolveStructForGraphTool(const FString& StructNameOrPath)
    {
        if (StructNameOrPath.Equals(TEXT("Vector"), ESearchCase::IgnoreCase))
        {
            return TBaseStructure<FVector>::Get();
        }
        if (StructNameOrPath.Equals(TEXT("Rotator"), ESearchCase::IgnoreCase))
        {
            return TBaseStructure<FRotator>::Get();
        }
        if (StructNameOrPath.Equals(TEXT("Transform"), ESearchCase::IgnoreCase))
        {
            return TBaseStructure<FTransform>::Get();
        }
        if (StructNameOrPath.Equals(TEXT("LinearColor"), ESearchCase::IgnoreCase) || StructNameOrPath.Equals(TEXT("Color"), ESearchCase::IgnoreCase))
        {
            return TBaseStructure<FLinearColor>::Get();
        }

        if (UScriptStruct* FoundStruct = FindFirstObjectSafe<UScriptStruct>(*StructNameOrPath))
        {
            return FoundStruct;
        }
        return LoadObject<UScriptStruct>(nullptr, *StructNameOrPath);
    }

    UFunction* FindFunctionByNameLoose(UClass* Class, const FString& FunctionName)
    {
        if (!Class)
        {
            return nullptr;
        }

        if (UFunction* Function = Class->FindFunctionByName(FName(*FunctionName)))
        {
            return Function;
        }

        for (TFieldIterator<UFunction> It(Class, EFieldIteratorFlags::IncludeSuper); It; ++It)
        {
            if (It->GetName().Equals(FunctionName, ESearchCase::IgnoreCase))
            {
                return *It;
            }
        }

        return nullptr;
    }

    void ApplyContainerType(FEdGraphPinType& PinType, const FString& ContainerType)
    {
        if (ContainerType.Equals(TEXT("Array"), ESearchCase::IgnoreCase))
        {
            PinType.ContainerType = EPinContainerType::Array;
        }
        else if (ContainerType.Equals(TEXT("Set"), ESearchCase::IgnoreCase))
        {
            PinType.ContainerType = EPinContainerType::Set;
        }
        else if (ContainerType.Equals(TEXT("Map"), ESearchCase::IgnoreCase))
        {
            PinType.ContainerType = EPinContainerType::Map;
        }
        else
        {
            PinType.ContainerType = EPinContainerType::None;
        }
    }

    FEdGraphPinType PinTypeFromStrings(const FString& TypeName, const FString& ObjectPath = TEXT(""), const FString& ContainerType = TEXT(""))
    {
        FEdGraphPinType PinType;
        const FString Type = TypeName.TrimStartAndEnd();

        if (Type.Equals(TEXT("Boolean"), ESearchCase::IgnoreCase) || Type.Equals(TEXT("Bool"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Boolean;
        }
        else if (Type.Equals(TEXT("Integer"), ESearchCase::IgnoreCase) || Type.Equals(TEXT("Int"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Int;
        }
        else if (Type.Equals(TEXT("Float"), ESearchCase::IgnoreCase) || Type.Equals(TEXT("Double"), ESearchCase::IgnoreCase) || Type.Equals(TEXT("Real"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Float;
        }
        else if (Type.Equals(TEXT("String"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_String;
        }
        else if (Type.Equals(TEXT("Name"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Name;
        }
        else if (Type.Equals(TEXT("Text"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Text;
        }
        else if (Type.Equals(TEXT("Byte"), ESearchCase::IgnoreCase))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Byte;
        }
        else if (UScriptStruct* StructType = ResolveStructForGraphTool(Type))
        {
            PinType.PinCategory = UEdGraphSchema_K2::PC_Struct;
            PinType.PinSubCategoryObject = StructType;
        }
        else if (Type.Equals(TEXT("Class"), ESearchCase::IgnoreCase) || Type.StartsWith(TEXT("Class:"), ESearchCase::IgnoreCase))
        {
            FString ClassObjectPath = ObjectPath;
            if (Type.StartsWith(TEXT("Class:"), ESearchCase::IgnoreCase))
            {
                ClassObjectPath = Type.RightChop(6);
            }
            PinType.PinCategory = UEdGraphSchema_K2::PC_Class;
            PinType.PinSubCategoryObject = ResolveClassForGraphTool(ClassObjectPath.IsEmpty() ? TEXT("Object") : ClassObjectPath);
        }
        else
        {
            FString ClassObjectPath = ObjectPath.IsEmpty() ? Type : ObjectPath;
            if (Type.StartsWith(TEXT("Object:"), ESearchCase::IgnoreCase))
            {
                ClassObjectPath = Type.RightChop(7);
            }
            PinType.PinCategory = UEdGraphSchema_K2::PC_Object;
            PinType.PinSubCategoryObject = ResolveClassForGraphTool(ClassObjectPath.IsEmpty() ? TEXT("Object") : ClassObjectPath);
        }

        ApplyContainerType(PinType, ContainerType);
        return PinType;
    }

    FEdGraphPinType PinTypeFromJson(const TSharedPtr<FJsonObject>& Json)
    {
        FString TypeName = TEXT("Wildcard");
        Json->TryGetStringField(TEXT("type"), TypeName);
        Json->TryGetStringField(TEXT("pin_type"), TypeName);

        FString ObjectPath;
        Json->TryGetStringField(TEXT("object_path"), ObjectPath);

        FString ContainerType;
        Json->TryGetStringField(TEXT("container_type"), ContainerType);

        FEdGraphPinType PinType = PinTypeFromStrings(TypeName, ObjectPath, ContainerType);

        if (PinType.ContainerType == EPinContainerType::Map)
        {
            const TSharedPtr<FJsonObject>* ValueTypeObject = nullptr;
            if (Json->TryGetObjectField(TEXT("value_type"), ValueTypeObject) && ValueTypeObject && ValueTypeObject->IsValid())
            {
                const FEdGraphPinType ValueType = PinTypeFromJson(*ValueTypeObject);
                PinType.PinValueType.TerminalCategory = ValueType.PinCategory;
                PinType.PinValueType.TerminalSubCategory = ValueType.PinSubCategory;
                PinType.PinValueType.TerminalSubCategoryObject = ValueType.PinSubCategoryObject.Get();
            }
            else
            {
                FString ValueTypeName;
                if (Json->TryGetStringField(TEXT("value_type"), ValueTypeName))
                {
                    const FEdGraphPinType ValueType = PinTypeFromStrings(ValueTypeName);
                    PinType.PinValueType.TerminalCategory = ValueType.PinCategory;
                    PinType.PinValueType.TerminalSubCategory = ValueType.PinSubCategory;
                    PinType.PinValueType.TerminalSubCategoryObject = ValueType.PinSubCategoryObject.Get();
                }
            }
        }

        return PinType;
    }

    EEdGraphPinDirection DirectionFromString(const FString& Direction)
    {
        return Direction.Equals(TEXT("output"), ESearchCase::IgnoreCase) ? EGPD_Output : EGPD_Input;
    }

    TSharedPtr<FJsonObject> PinToJsonObject(UEdGraphPin* Pin)
    {
        TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
        if (!Pin)
        {
            return Json;
        }

        Json->SetStringField(TEXT("pin_id"), Pin->PinId.ToString());
        Json->SetStringField(TEXT("name"), Pin->PinName.ToString());
        Json->SetStringField(TEXT("display_name"), Pin->GetDisplayName().ToString());
        Json->SetStringField(TEXT("direction"), Pin->Direction == EGPD_Input ? TEXT("input") : TEXT("output"));
        Json->SetStringField(TEXT("type"), PinTypeToString(Pin->PinType));
        Json->SetStringField(TEXT("category"), Pin->PinType.PinCategory.ToString());
        Json->SetStringField(TEXT("default_value"), Pin->DefaultValue);
        Json->SetBoolField(TEXT("is_linked"), Pin->LinkedTo.Num() > 0);
        Json->SetBoolField(TEXT("is_required"), Pin->Direction == EGPD_Input && Pin->LinkedTo.Num() == 0 && Pin->DefaultValue.IsEmpty() && Pin->DefaultObject == nullptr && Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec);

        TArray<TSharedPtr<FJsonValue>> Links;
        for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
        {
            if (!LinkedPin || !LinkedPin->GetOwningNode())
            {
                continue;
            }
            TSharedPtr<FJsonObject> LinkJson = MakeShared<FJsonObject>();
            LinkJson->SetStringField(TEXT("node_id"), LinkedPin->GetOwningNode()->NodeGuid.ToString());
            LinkJson->SetStringField(TEXT("pin_name"), LinkedPin->PinName.ToString());
            Links.Add(MakeShared<FJsonValueObject>(LinkJson));
        }
        Json->SetArrayField(TEXT("links"), Links);

        return Json;
    }

    TArray<TSharedPtr<FJsonValue>> PinsToJsonArray(UEdGraphNode* Node)
    {
        TArray<TSharedPtr<FJsonValue>> Pins;
        if (!Node)
        {
            return Pins;
        }
        for (UEdGraphPin* Pin : Node->Pins)
        {
            Pins.Add(MakeShared<FJsonValueObject>(PinToJsonObject(Pin)));
        }
        return Pins;
    }

    TSharedPtr<FJsonObject> NodeToJsonObject(UEdGraphNode* Node)
    {
        TSharedPtr<FJsonObject> Json = MakeShared<FJsonObject>();
        if (!Node)
        {
            return Json;
        }

        Json->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
        Json->SetStringField(TEXT("node_name"), Node->GetName());
        Json->SetStringField(TEXT("node_class"), Node->GetClass()->GetName());
        Json->SetStringField(TEXT("title"), Node->GetNodeTitle(ENodeTitleType::ListView).ToString());
        Json->SetNumberField(TEXT("x"), Node->NodePosX);
        Json->SetNumberField(TEXT("y"), Node->NodePosY);
        Json->SetArrayField(TEXT("pins"), PinsToJsonArray(Node));
        return Json;
    }

    template<typename NodeType>
    NodeType* AddTypedNodeToGraph(UEdGraph* Graph, const FVector2D& Position)
    {
        if (!Graph)
        {
            return nullptr;
        }

        NodeType* Node = NewObject<NodeType>(Graph);
        Node->NodePosX = Position.X;
        Node->NodePosY = Position.Y;
        Graph->AddNode(Node, true, false);
        Node->CreateNewGuid();
        Node->PostPlacedNewNode();
        Node->AllocateDefaultPins();
        return Node;
    }

    UEdGraphNode* AddNodeClassToGraph(UEdGraph* Graph, UClass* NodeClass, const FVector2D& Position)
    {
        if (!Graph || !NodeClass || !NodeClass->IsChildOf(UEdGraphNode::StaticClass()))
        {
            return nullptr;
        }

        UEdGraphNode* Node = NewObject<UEdGraphNode>(Graph, NodeClass);
        Node->NodePosX = Position.X;
        Node->NodePosY = Position.Y;
        Graph->AddNode(Node, true, false);
        Node->CreateNewGuid();
        Node->PostPlacedNewNode();
        Node->AllocateDefaultPins();
        Node->ReconstructNode();
        return Node;
    }

    UK2Node_FunctionEntry* FindFunctionEntryNode(UEdGraph* Graph)
    {
        if (!Graph)
        {
            return nullptr;
        }
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_FunctionEntry* Entry = Cast<UK2Node_FunctionEntry>(Node))
            {
                return Entry;
            }
        }
        return nullptr;
    }

    UK2Node_FunctionResult* FindFunctionResultNode(UEdGraph* Graph)
    {
        if (!Graph)
        {
            return nullptr;
        }
        for (UEdGraphNode* Node : Graph->Nodes)
        {
            if (UK2Node_FunctionResult* Result = Cast<UK2Node_FunctionResult>(Node))
            {
                return Result;
            }
        }
        return nullptr;
    }

    void AddEditablePinsFromJson(UK2Node_EditablePinBase* Node, const TArray<TSharedPtr<FJsonValue>>* Pins, EEdGraphPinDirection Direction)
    {
        if (!Node || !Pins)
        {
            return;
        }

        for (const TSharedPtr<FJsonValue>& PinValue : *Pins)
        {
            const TSharedPtr<FJsonObject>* PinObject = nullptr;
            if (!PinValue.IsValid() || !PinValue->TryGetObject(PinObject) || !PinObject || !PinObject->IsValid())
            {
                continue;
            }

            FString PinName;
            if (!(*PinObject)->TryGetStringField(TEXT("name"), PinName))
            {
                continue;
            }

            FString DirectionOverride;
            const EEdGraphPinDirection FinalDirection = (*PinObject)->TryGetStringField(TEXT("direction"), DirectionOverride)
                ? DirectionFromString(DirectionOverride)
                : Direction;

            Node->CreateUserDefinedPin(FName(*PinName), PinTypeFromJson(*PinObject), FinalDirection, false);
        }
        Node->ReconstructNode();
    }

    UEdGraph* FindOrCreateFunctionGraphFromParams(UBlueprint* Blueprint, const TSharedPtr<FJsonObject>& Params, bool bMacro, bool bUbergraph = false)
    {
        if (!Blueprint)
        {
            return nullptr;
        }

        FString FunctionName;
        if (!Params->TryGetStringField(TEXT("function_name"), FunctionName) &&
            !Params->TryGetStringField(TEXT("graph_name"), FunctionName) &&
            !Params->TryGetStringField(TEXT("name"), FunctionName))
        {
            return nullptr;
        }

        TArray<UEdGraph*> ExistingGraphs;
        CollectBlueprintGraphs(Blueprint, ExistingGraphs);
        for (UEdGraph* Graph : ExistingGraphs)
        {
            if (Graph && Graph->GetName().Equals(FunctionName, ESearchCase::IgnoreCase))
            {
                return Graph;
            }
        }

        UEdGraph* NewGraph = FBlueprintEditorUtils::CreateNewGraph(Blueprint, FName(*FunctionName), UEdGraph::StaticClass(), UEdGraphSchema_K2::StaticClass());
        if (!NewGraph)
        {
            return nullptr;
        }

        if (bUbergraph)
        {
            FBlueprintEditorUtils::AddUbergraphPage(Blueprint, NewGraph);
        }
        else if (bMacro)
        {
            FBlueprintEditorUtils::AddMacroGraph(Blueprint, NewGraph, true, nullptr);
        }
        else
        {
            FBlueprintEditorUtils::AddFunctionGraph<UFunction>(Blueprint, NewGraph, true, nullptr);
        }

        return NewGraph;
    }

    bool SetStructTypeOnStructNode(UEdGraphNode* Node, UScriptStruct* StructType)
    {
        if (!Node || !StructType)
        {
            return false;
        }

        if (FObjectProperty* StructProperty = FindFProperty<FObjectProperty>(Node->GetClass(), TEXT("StructType")))
        {
            StructProperty->SetObjectPropertyValue_InContainer(Node, StructType);
            return true;
        }

        return false;
    }

    TSharedPtr<FJsonObject> CreateNodeResult(UEdGraphNode* Node)
    {
        TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
        if (Node)
        {
            Result->SetStringField(TEXT("node_id"), Node->NodeGuid.ToString());
            Result->SetObjectField(TEXT("node"), NodeToJsonObject(Node));
        }
        return Result;
    }

    TArray<FString> ExtractFormatArguments(const FString& Format)
    {
        TArray<FString> Names;
        int32 Cursor = 0;
        while (Cursor < Format.Len())
        {
            int32 OpenIndex = INDEX_NONE;
            if (!Format.FindChar(TEXT('{'), OpenIndex) || OpenIndex < Cursor)
            {
                break;
            }

            int32 CloseIndex = INDEX_NONE;
            if (!Format.RightChop(OpenIndex + 1).FindChar(TEXT('}'), CloseIndex))
            {
                break;
            }
            CloseIndex += OpenIndex + 1;

            FString Name = Format.Mid(OpenIndex + 1, CloseIndex - OpenIndex - 1).TrimStartAndEnd();
            if (!Name.IsEmpty() && !Names.Contains(Name))
            {
                Names.Add(Name);
            }
            Cursor = CloseIndex + 1;
        }
        return Names;
    }
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCreateBlueprintFunction(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* Graph = FindOrCreateFunctionGraphFromParams(Blueprint, Params, false);
    if (!Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create function graph"));
    }

    const TArray<TSharedPtr<FJsonValue>>* Inputs = nullptr;
    const TArray<TSharedPtr<FJsonValue>>* Outputs = nullptr;
    Params->TryGetArrayField(TEXT("inputs"), Inputs);
    Params->TryGetArrayField(TEXT("outputs"), Outputs);
    AddEditablePinsFromJson(FindFunctionEntryNode(Graph), Inputs, EGPD_Output);
    AddEditablePinsFromJson(FindFunctionResultNode(Graph), Outputs, EGPD_Input);

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("graph_name"), Graph->GetName());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCreateBlueprintMacro(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* Graph = FindOrCreateFunctionGraphFromParams(Blueprint, Params, true);
    if (!Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create macro graph"));
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("graph_name"), Graph->GetName());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCreateBlueprintCustomEventGraph(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    UEdGraph* Graph = FindOrCreateFunctionGraphFromParams(Blueprint, Params, false, true);
    if (!Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create custom event graph"));
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("graph_name"), Graph->GetName());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddNodeByClass(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    FString NodeClassName;
    if (!Params->TryGetStringField(TEXT("node_class"), NodeClassName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'node_class' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    UClass* NodeClass = ResolveClassForGraphTool(NodeClassName);
    if (!Blueprint || !Graph || !NodeClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint, graph, or node class"));
    }

    UEdGraphNode* Node = AddNodeClassToGraph(Graph, NodeClass, ReadNodePosition(Params));
    if (!Node)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Failed to create node class: %s"), *NodeClassName));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(Node);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleConnectPinsValidated(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, SourceNodeId, TargetNodeId, SourcePinName, TargetPinName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("source_node_id"), SourceNodeId) ||
        !Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId) ||
        !Params->TryGetStringField(TEXT("source_pin"), SourcePinName) ||
        !Params->TryGetStringField(TEXT("target_pin"), TargetPinName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing required connection parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* SourceGraph = nullptr;
    UEdGraphNode* SourceNode = FindNodeById(Blueprint, SourceNodeId, &SourceGraph);
    UEdGraphNode* TargetNode = FindNodeById(Blueprint, TargetNodeId);
    UEdGraphPin* SourcePin = FindPinOnNode(SourceNode, SourcePinName, EGPD_Output);
    UEdGraphPin* TargetPin = FindPinOnNode(TargetNode, TargetPinName, EGPD_Input);
    if (!Blueprint || !SourceGraph || !SourcePin || !TargetPin)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Source or target pin not found"));
    }

    const UEdGraphSchema* Schema = SourceGraph->GetSchema();
    const bool bConnected = Schema && Schema->TryCreateConnection(SourcePin, TargetPin);
    if (!bConnected)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Schema rejected the pin connection"));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("connected"), true);
    Result->SetObjectField(TEXT("source_pin"), PinToJsonObject(SourcePin));
    Result->SetObjectField(TEXT("target_pin"), PinToJsonObject(TargetPin));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleSetPinDefaultValue(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphNode* Node = nullptr;
    UEdGraph* Graph = nullptr;
    UEdGraphPin* Pin = FindPinFromParams(Blueprint, Params, &Node, &Graph);
    if (!Blueprint || !Pin || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Pin not found"));
    }

    const UEdGraphSchema_K2* Schema = Cast<const UEdGraphSchema_K2>(Graph->GetSchema());
    if (!Schema)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Graph is not a K2 graph"));
    }

    FString ObjectPath;
    if (Params->TryGetStringField(TEXT("object_path"), ObjectPath))
    {
        UObject* Object = LoadObject<UObject>(nullptr, *ObjectPath);
        Schema->TrySetDefaultObject(*Pin, Object);
    }
    else if (Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Text)
    {
        Schema->TrySetDefaultText(*Pin, FText::FromString(ReadStringOrDefault(Params, TEXT("value"))));
    }
    else
    {
        Schema->TrySetDefaultValue(*Pin, ReadStringOrDefault(Params, TEXT("value")));
    }

    if (Node)
    {
        Node->PinDefaultValueChanged(Pin);
    }
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetObjectField(TEXT("pin"), PinToJsonObject(Pin));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddCustomEventNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, EventName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("event_name"), EventName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'event_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    UK2Node_CustomEvent* EventNode = AddTypedNodeToGraph<UK2Node_CustomEvent>(Graph, ReadNodePosition(Params));
    EventNode->CustomFunctionName = FName(*EventName);

    const TArray<TSharedPtr<FJsonValue>>* ParamsArray = nullptr;
    Params->TryGetArrayField(TEXT("parameters"), ParamsArray);
    AddEditablePinsFromJson(EventNode, ParamsArray, EGPD_Output);
    EventNode->ReconstructNode();

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);
    return CreateNodeResult(EventNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleListBlueprintFunctions(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    TArray<TSharedPtr<FJsonValue>> Functions;
    for (UEdGraph* Graph : Blueprint->FunctionGraphs)
    {
        if (Graph)
        {
            TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
            Entry->SetStringField(TEXT("name"), Graph->GetName());
            Entry->SetStringField(TEXT("type"), TEXT("function"));
            Functions.Add(MakeShared<FJsonValueObject>(Entry));
        }
    }
    for (UEdGraph* Graph : Blueprint->MacroGraphs)
    {
        if (Graph)
        {
            TSharedPtr<FJsonObject> Entry = MakeShared<FJsonObject>();
            Entry->SetStringField(TEXT("name"), Graph->GetName());
            Entry->SetStringField(TEXT("type"), TEXT("macro"));
            Functions.Add(MakeShared<FJsonValueObject>(Entry));
        }
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetArrayField(TEXT("graphs"), Functions);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleGetNodePins(const TSharedPtr<FJsonObject>& Params)
{
    return HandleInspectNodePins(Params);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleDeleteBlueprintNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, NodeId;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("node_id"), NodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'node_id' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphNode* Node = FindNodeById(Blueprint, NodeId);
    if (!Blueprint || !Node)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Node not found"));
    }

    const FString DeletedNodeId = Node->NodeGuid.ToString();
    Node->BreakAllNodeLinks();
    FBlueprintEditorUtils::RemoveNode(Blueprint, Node, true);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("deleted_node_id"), DeletedNodeId);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBlueprintControlNode(const TSharedPtr<FJsonObject>& Params)
{
    FString NodeType;
    if (!Params->TryGetStringField(TEXT("node_type"), NodeType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'node_type' parameter"));
    }

    if (NodeType.Equals(TEXT("Branch"), ESearchCase::IgnoreCase) || NodeType.Equals(TEXT("If"), ESearchCase::IgnoreCase))
    {
        return HandleAddBranchNode(Params);
    }
    if (NodeType.Equals(TEXT("Sequence"), ESearchCase::IgnoreCase))
    {
        return HandleAddSequenceNode(Params);
    }
    if (NodeType.Equals(TEXT("ForEachLoop"), ESearchCase::IgnoreCase))
    {
        return HandleAddForEachLoopNode(Params);
    }
    if (NodeType.Equals(TEXT("WhileLoop"), ESearchCase::IgnoreCase))
    {
        return HandleAddWhileLoopNode(Params);
    }
    if (NodeType.StartsWith(TEXT("Switch"), ESearchCase::IgnoreCase))
    {
        return HandleAddSwitchNode(Params);
    }

    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    FString MacroName = NodeType;
    if (NodeType.Equals(TEXT("ForLoop"), ESearchCase::IgnoreCase)) MacroName = TEXT("ForLoop");
    if (NodeType.Equals(TEXT("ForLoopWithBreak"), ESearchCase::IgnoreCase)) MacroName = TEXT("ForLoopWithBreak");
    if (NodeType.Equals(TEXT("DoOnce"), ESearchCase::IgnoreCase)) MacroName = TEXT("DoOnce");
    if (NodeType.Equals(TEXT("Gate"), ESearchCase::IgnoreCase)) MacroName = TEXT("Gate");

    UK2Node_MacroInstance* MacroNode = CreateStandardMacroNode(Graph, MacroName, ReadNodePosition(Params));
    if (!MacroNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Unsupported control node or failed macro lookup: %s"), *NodeType));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(MacroNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleSetVariableDefaultValue(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, VariableName, DefaultValue;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("variable_name"), VariableName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'variable_name' parameter"));
    }
    DefaultValue = ReadStringOrDefault(Params, TEXT("default_value"), ReadStringOrDefault(Params, TEXT("value")));

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Blueprint not found: %s"), *BlueprintName));
    }

    for (FBPVariableDescription& Variable : Blueprint->NewVariables)
    {
        if (Variable.VarName == FName(*VariableName))
        {
            Variable.DefaultValue = DefaultValue;
            FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("variable_name"), VariableName);
            Result->SetStringField(TEXT("default_value"), DefaultValue);
            return Result;
        }
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(FString::Printf(TEXT("Variable not found: %s"), *VariableName));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleSplitStructPin(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = nullptr;
    UEdGraphPin* Pin = FindPinFromParams(Blueprint, Params, nullptr, &Graph);
    const UEdGraphSchema_K2* Schema = Graph ? Cast<const UEdGraphSchema_K2>(Graph->GetSchema()) : nullptr;
    if (!Blueprint || !Pin || !Schema)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Struct pin not found"));
    }

    Schema->SplitPin(Pin);
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetObjectField(TEXT("pin"), PinToJsonObject(Pin));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleRecombineStructPin(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = nullptr;
    UEdGraphPin* Pin = FindPinFromParams(Blueprint, Params, nullptr, &Graph);
    const UEdGraphSchema_K2* Schema = Graph ? Cast<const UEdGraphSchema_K2>(Graph->GetSchema()) : nullptr;
    if (!Blueprint || !Pin || !Schema)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Struct pin not found"));
    }

    Schema->RecombinePin(Pin);
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetObjectField(TEXT("pin"), PinToJsonObject(Pin));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddDynamicPinToNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, NodeId;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("node_id"), NodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'node_id' parameter"));
    }

    int32 Count = 1;
    Params->TryGetNumberField(TEXT("count"), Count);
    Count = FMath::Clamp(Count, 1, 32);

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphNode* Node = FindNodeById(Blueprint, NodeId);
    if (!Blueprint || !Node)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Node not found"));
    }

    if (IK2Node_AddPinInterface* AddPinNode = Cast<IK2Node_AddPinInterface>(Node))
    {
        for (int32 Index = 0; Index < Count && AddPinNode->CanAddPin(); ++Index)
        {
            AddPinNode->AddInputPin();
        }
    }
    else if (UK2Node_FormatText* FormatTextNode = Cast<UK2Node_FormatText>(Node))
    {
        for (int32 Index = 0; Index < Count; ++Index)
        {
            FormatTextNode->AddArgumentPin();
        }
    }
    else if (UK2Node_Switch* SwitchNode = Cast<UK2Node_Switch>(Node))
    {
        for (int32 Index = 0; Index < Count; ++Index)
        {
            SwitchNode->AddPinToSwitchNode();
        }
    }
    else
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Node does not support dynamic pins"));
    }

    Node->ReconstructNode();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(Node);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleDisconnectBlueprintPin(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphPin* SourcePin = FindPinFromParams(Blueprint, Params);
    if (!Blueprint || !SourcePin)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Source pin not found"));
    }

    FString TargetNodeId, TargetPinName;
    if (Params->TryGetStringField(TEXT("target_node_id"), TargetNodeId) && Params->TryGetStringField(TEXT("target_pin"), TargetPinName))
    {
        UEdGraphNode* TargetNode = FindNodeById(Blueprint, TargetNodeId);
        if (UEdGraphPin* TargetPin = FindPinOnNode(TargetNode, TargetPinName))
        {
            SourcePin->BreakLinkTo(TargetPin);
        }
    }
    else
    {
        SourcePin->BreakAllPinLinks();
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetObjectField(TEXT("pin"), PinToJsonObject(SourcePin));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleMoveBlueprintNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, NodeId;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("node_id"), NodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'node_id' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphNode* Node = FindNodeById(Blueprint, NodeId);
    if (!Blueprint || !Node)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Node not found"));
    }

    double X = Node->NodePosX;
    double Y = Node->NodePosY;
    Params->TryGetNumberField(TEXT("x"), X);
    Params->TryGetNumberField(TEXT("y"), Y);
    Node->NodePosX = X;
    Node->NodePosY = Y;
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(Node);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleFormatBlueprintNodes(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint not found"));
    }

    const TArray<TSharedPtr<FJsonValue>>* NodeIds = nullptr;
    Params->TryGetArrayField(TEXT("node_ids"), NodeIds);
    if (!NodeIds)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'node_ids' array"));
    }

    FVector2D StartPosition = ReadNodePosition(Params);
    double ColumnSpacing = 360.0;
    double RowSpacing = 180.0;
    Params->TryGetNumberField(TEXT("column_spacing"), ColumnSpacing);
    Params->TryGetNumberField(TEXT("row_spacing"), RowSpacing);

    TArray<TSharedPtr<FJsonValue>> MovedNodes;
    for (int32 Index = 0; Index < NodeIds->Num(); ++Index)
    {
        const FString NodeId = (*NodeIds)[Index]->AsString();
        if (UEdGraphNode* Node = FindNodeById(Blueprint, NodeId))
        {
            Node->NodePosX = StartPosition.X + (Index * ColumnSpacing);
            Node->NodePosY = StartPosition.Y + ((Index % 2) * RowSpacing);
            MovedNodes.Add(MakeShared<FJsonValueObject>(NodeToJsonObject(Node)));
        }
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetArrayField(TEXT("nodes"), MovedNodes);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddFunctionEntryNode(const TSharedPtr<FJsonObject>& Params)
{
    return HandleCreateBlueprintFunction(Params);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddSpawnActorNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    UK2Node_SpawnActorFromClass* SpawnNode = AddTypedNodeToGraph<UK2Node_SpawnActorFromClass>(Graph, ReadNodePosition(Params));
    FString ClassName;
    if (Params->TryGetStringField(TEXT("actor_class"), ClassName) || Params->TryGetStringField(TEXT("class_path"), ClassName))
    {
        if (UClass* ActorClass = ResolveClassForGraphTool(ClassName))
        {
            if (UEdGraphPin* ClassPin = SpawnNode->GetClassPin())
            {
                if (const UEdGraphSchema_K2* Schema = Cast<const UEdGraphSchema_K2>(Graph->GetSchema()))
                {
                    Schema->TrySetDefaultObject(*ClassPin, ActorClass);
                }
            }
        }
    }
    SpawnNode->ReconstructNode();

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(SpawnNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCreateBlueprintInterface(const TSharedPtr<FJsonObject>& Params)
{
    FString InterfaceName;
    if (!Params->TryGetStringField(TEXT("interface_name"), InterfaceName) &&
        !Params->TryGetStringField(TEXT("name"), InterfaceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'interface_name' parameter"));
    }

    FString PackagePath = ReadStringOrDefault(Params, TEXT("path"), TEXT("/Game/Blueprints/Interfaces"));
    if (InterfaceName.StartsWith(TEXT("/")))
    {
        PackagePath = FPackageName::GetLongPackagePath(InterfaceName);
        InterfaceName = FPackageName::GetShortName(InterfaceName);
    }

    FAssetToolsModule& AssetTools = FModuleManager::LoadModuleChecked<FAssetToolsModule>(TEXT("AssetTools"));
    UBlueprintInterfaceFactory* Factory = NewObject<UBlueprintInterfaceFactory>();
    UObject* CreatedAsset = AssetTools.Get().CreateAsset(InterfaceName, PackagePath, UBlueprint::StaticClass(), Factory);
    UBlueprint* InterfaceBlueprint = Cast<UBlueprint>(CreatedAsset);
    if (!InterfaceBlueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Blueprint Interface asset"));
    }

    const TArray<TSharedPtr<FJsonValue>>* Functions = nullptr;
    if (Params->TryGetArrayField(TEXT("functions"), Functions))
    {
        for (const TSharedPtr<FJsonValue>& FunctionValue : *Functions)
        {
            const TSharedPtr<FJsonObject>* FunctionObject = nullptr;
            if (FunctionValue->TryGetObject(FunctionObject) && FunctionObject && FunctionObject->IsValid())
            {
                TSharedPtr<FJsonObject> FunctionParams = MakeShared<FJsonObject>(*(*FunctionObject));
                FunctionParams->SetStringField(TEXT("blueprint_name"), InterfaceBlueprint->GetPathName());
                HandleCreateBlueprintFunction(FunctionParams);
            }
        }
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("interface_path"), InterfaceBlueprint->GetPathName());
    Result->SetStringField(TEXT("interface_name"), InterfaceName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddInterfaceFunction(const TSharedPtr<FJsonObject>& Params)
{
    FString InterfaceName;
    if (!Params->TryGetStringField(TEXT("interface_name"), InterfaceName) &&
        !Params->TryGetStringField(TEXT("blueprint_name"), InterfaceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'interface_name' parameter"));
    }

    TSharedPtr<FJsonObject> FunctionParams = MakeShared<FJsonObject>(*Params);
    FunctionParams->SetStringField(TEXT("blueprint_name"), InterfaceName);
    return HandleCreateBlueprintFunction(FunctionParams);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleImplementInterface(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, InterfaceName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("interface_name"), InterfaceName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'interface_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UBlueprint* InterfaceBlueprint = ResolveBlueprintForGraphTool(InterfaceName);
    UClass* InterfaceClass = InterfaceBlueprint ? InterfaceBlueprint->GeneratedClass.Get() : ResolveClassForGraphTool(InterfaceName);
    if (!Blueprint || !InterfaceClass)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint or interface class"));
    }

    const bool bImplemented = FBlueprintEditorUtils::ImplementNewInterface(Blueprint, InterfaceClass->GetClassPathName());
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("implemented"), bImplemented);
    Result->SetStringField(TEXT("interface_class"), InterfaceClass->GetPathName());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddInterfaceMessageNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, InterfaceName, FunctionName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("interface_name"), InterfaceName) ||
        !Params->TryGetStringField(TEXT("function_name"), FunctionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing interface message parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UBlueprint* InterfaceBlueprint = ResolveBlueprintForGraphTool(InterfaceName);
    UClass* InterfaceClass = InterfaceBlueprint ? InterfaceBlueprint->GeneratedClass.Get() : ResolveClassForGraphTool(InterfaceName);
    UFunction* Function = FindFunctionByNameLoose(InterfaceClass, FunctionName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph || !InterfaceClass || !Function)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve interface message function"));
    }

    UK2Node_CallFunction* MessageNode = FUnrealMCPCommonUtils::CreateFunctionCallNode(Graph, Function, ReadNodePosition(Params));
    if (!MessageNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create interface message node"));
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(MessageNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCreateEventDispatcher(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, DispatcherName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("dispatcher_name"), DispatcherName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'dispatcher_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint not found"));
    }

    FEdGraphPinType DelegateType;
    DelegateType.PinCategory = UEdGraphSchema_K2::PC_MCDelegate;
    const bool bAdded = FBlueprintEditorUtils::AddMemberVariable(Blueprint, FName(*DispatcherName), DelegateType);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("added"), bAdded);
    Result->SetStringField(TEXT("dispatcher_name"), DispatcherName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddEventDispatcherNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, DispatcherName, Action;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("dispatcher_name"), DispatcherName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing dispatcher parameters"));
    }
    Action = ReadStringOrDefault(Params, TEXT("action"), TEXT("call"));

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    UK2Node_BaseMCDelegate* DelegateNode = nullptr;
    if (Action.Equals(TEXT("bind"), ESearchCase::IgnoreCase))
    {
        DelegateNode = AddTypedNodeToGraph<UK2Node_AddDelegate>(Graph, ReadNodePosition(Params));
    }
    else if (Action.Equals(TEXT("unbind"), ESearchCase::IgnoreCase))
    {
        DelegateNode = AddTypedNodeToGraph<UK2Node_RemoveDelegate>(Graph, ReadNodePosition(Params));
    }
    else if (Action.Equals(TEXT("clear"), ESearchCase::IgnoreCase))
    {
        DelegateNode = AddTypedNodeToGraph<UK2Node_ClearDelegate>(Graph, ReadNodePosition(Params));
    }
    else
    {
        DelegateNode = AddTypedNodeToGraph<UK2Node_CallDelegate>(Graph, ReadNodePosition(Params));
    }

    if (!DelegateNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create dispatcher node"));
    }

    DelegateNode->DelegateReference.SetSelfMember(FName(*DispatcherName));
    DelegateNode->ReconstructNode();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(DelegateNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleSetVariableContainerType(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, VariableName, ContainerType;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("variable_name"), VariableName) ||
        !Params->TryGetStringField(TEXT("container_type"), ContainerType))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing variable container parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint not found"));
    }

    for (FBPVariableDescription& Variable : Blueprint->NewVariables)
    {
        if (Variable.VarName == FName(*VariableName))
        {
            FEdGraphPinType NewType = Variable.VarType;
            ApplyContainerType(NewType, ContainerType);
            FBlueprintEditorUtils::ChangeMemberVariableType(Blueprint, FName(*VariableName), NewType);

            TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
            Result->SetStringField(TEXT("variable_name"), VariableName);
            Result->SetStringField(TEXT("container_type"), ContainerType);
            return Result;
        }
    }

    return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Variable not found"));
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddCollectionNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, CollectionType, Operation;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("collection_type"), CollectionType) ||
        !Params->TryGetStringField(TEXT("operation"), Operation))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing collection node parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    UEdGraphNode* Node = nullptr;
    if (Operation.Equals(TEXT("Make"), ESearchCase::IgnoreCase) || Operation.Equals(TEXT("MakeArray"), ESearchCase::IgnoreCase) || Operation.Equals(TEXT("MakeMap"), ESearchCase::IgnoreCase) || Operation.Equals(TEXT("MakeSet"), ESearchCase::IgnoreCase))
    {
        if (CollectionType.Equals(TEXT("Map"), ESearchCase::IgnoreCase))
        {
            Node = AddTypedNodeToGraph<UK2Node_MakeMap>(Graph, ReadNodePosition(Params));
        }
        else if (CollectionType.Equals(TEXT("Set"), ESearchCase::IgnoreCase))
        {
            Node = AddTypedNodeToGraph<UK2Node_MakeSet>(Graph, ReadNodePosition(Params));
        }
        else
        {
            Node = AddTypedNodeToGraph<UK2Node_MakeArray>(Graph, ReadNodePosition(Params));
        }
    }
    else
    {
        UClass* LibraryClass = UKismetArrayLibrary::StaticClass();
        FString FunctionName = TEXT("Array_") + Operation;
        if (CollectionType.Equals(TEXT("Map"), ESearchCase::IgnoreCase))
        {
            LibraryClass = UBlueprintMapLibrary::StaticClass();
            FunctionName = TEXT("Map_") + Operation;
        }
        else if (CollectionType.Equals(TEXT("Set"), ESearchCase::IgnoreCase))
        {
            LibraryClass = UBlueprintSetLibrary::StaticClass();
            FunctionName = TEXT("Set_") + Operation;
        }

        UFunction* Function = FindFunctionByNameLoose(LibraryClass, FunctionName);
        if (Function)
        {
            Node = FUnrealMCPCommonUtils::CreateFunctionCallNode(Graph, Function, ReadNodePosition(Params));
        }
    }

    if (!Node)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create collection node"));
    }

    int32 PinCount = 0;
    Params->TryGetNumberField(TEXT("pin_count"), PinCount);
    if (IK2Node_AddPinInterface* AddPinNode = Cast<IK2Node_AddPinInterface>(Node))
    {
        for (int32 Index = 1; Index < PinCount; ++Index)
        {
            AddPinNode->AddInputPin();
        }
        Node->ReconstructNode();
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(Node);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleExportNodesAsText(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    const TArray<TSharedPtr<FJsonValue>>* NodeIds = nullptr;
    Params->TryGetArrayField(TEXT("node_ids"), NodeIds);
    if (!Blueprint || !NodeIds)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint or node_ids not found"));
    }

    TSet<UObject*> NodesToExport;
    for (const TSharedPtr<FJsonValue>& NodeIdValue : *NodeIds)
    {
        if (UEdGraphNode* Node = FindNodeById(Blueprint, NodeIdValue->AsString()))
        {
            NodesToExport.Add(Node);
        }
    }

    FString ExportedText;
    FEdGraphUtilities::ExportNodesToText(NodesToExport, ExportedText);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("text"), ExportedText);
    Result->SetNumberField(TEXT("node_count"), NodesToExport.Num());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandlePasteNodesFromText(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, Text;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("text"), Text))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'text' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }
    if (!FEdGraphUtilities::CanImportNodesFromText(Graph, Text))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Graph cannot import provided node text"));
    }

    TSet<UEdGraphNode*> ImportedNodes;
    FEdGraphUtilities::ImportNodesFromText(Graph, Text, ImportedNodes);

    FVector2D Position = ReadNodePosition(Params);
    bool bMoveImportedNodes = Params->HasField(TEXT("node_position"));
    TArray<TSharedPtr<FJsonValue>> NodesJson;
    for (UEdGraphNode* Node : ImportedNodes)
    {
        if (Node)
        {
            if (bMoveImportedNodes)
            {
                Node->NodePosX += Position.X;
                Node->NodePosY += Position.Y;
            }
            NodesJson.Add(MakeShared<FJsonValueObject>(NodeToJsonObject(Node)));
        }
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetNumberField(TEXT("node_count"), ImportedNodes.Num());
    Result->SetArrayField(TEXT("nodes"), NodesJson);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddLocalVariable(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, FunctionName, VariableName, TypeName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("function_name"), FunctionName) ||
        !Params->TryGetStringField(TEXT("variable_name"), VariableName) ||
        !Params->TryGetStringField(TEXT("variable_type"), TypeName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing local variable parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    if (!Blueprint)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Blueprint not found"));
    }

    UEdGraph* TargetGraph = nullptr;
    for (UEdGraph* Graph : Blueprint->FunctionGraphs)
    {
        if (Graph && Graph->GetName().Equals(FunctionName, ESearchCase::IgnoreCase))
        {
            TargetGraph = Graph;
            break;
        }
    }
    if (!TargetGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Function graph not found"));
    }

    const FString DefaultValue = ReadStringOrDefault(Params, TEXT("default_value"));
    const bool bAdded = FBlueprintEditorUtils::AddLocalVariable(Blueprint, TargetGraph, FName(*VariableName), PinTypeFromStrings(TypeName), DefaultValue);
    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetBoolField(TEXT("added"), bAdded);
    Result->SetStringField(TEXT("variable_name"), VariableName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleSetFunctionFlags(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, FunctionName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("function_name"), FunctionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing function flag parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* FunctionGraph = nullptr;
    if (Blueprint)
    {
        for (UEdGraph* Graph : Blueprint->FunctionGraphs)
        {
            if (Graph && Graph->GetName().Equals(FunctionName, ESearchCase::IgnoreCase))
            {
                FunctionGraph = Graph;
                break;
            }
        }
    }

    UK2Node_FunctionEntry* Entry = FindFunctionEntryNode(FunctionGraph);
    if (!Blueprint || !Entry)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Function entry node not found"));
    }

    bool bPure = false;
    if (Params->TryGetBoolField(TEXT("pure"), bPure))
    {
        bPure ? Entry->AddExtraFlags(FUNC_BlueprintPure) : Entry->ClearExtraFlags(FUNC_BlueprintPure);
    }

    bool bCallInEditor = false;
    if (Params->TryGetBoolField(TEXT("call_in_editor"), bCallInEditor))
    {
        Entry->MetaData.bCallInEditor = bCallInEditor;
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetNumberField(TEXT("extra_flags"), Entry->GetExtraFlags());
    Result->SetBoolField(TEXT("call_in_editor"), Entry->MetaData.bCallInEditor);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddFormatTextNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, Format;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("format"), Format))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'format' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    UK2Node_FormatText* FormatNode = AddTypedNodeToGraph<UK2Node_FormatText>(Graph, ReadNodePosition(Params));
    if (UEdGraphPin* FormatPin = FormatNode->GetFormatPin())
    {
        if (const UEdGraphSchema_K2* Schema = Cast<const UEdGraphSchema_K2>(Graph->GetSchema()))
        {
            Schema->TrySetDefaultText(*FormatPin, FText::FromString(Format));
        }
    }

    TArray<FString> Arguments = ExtractFormatArguments(Format);
    const TArray<TSharedPtr<FJsonValue>>* ExplicitArguments = nullptr;
    if (Params->TryGetArrayField(TEXT("arguments"), ExplicitArguments))
    {
        Arguments.Reset();
        for (const TSharedPtr<FJsonValue>& ArgumentValue : *ExplicitArguments)
        {
            Arguments.Add(ArgumentValue->AsString());
        }
    }

    for (const FString& Argument : Arguments)
    {
        FormatNode->AddArgumentPin();
        FormatNode->SetArgumentName(FormatNode->GetArgumentCount() - 1, FName(*Argument));
    }
    FormatNode->ReconstructNode();

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(FormatNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddSwitchNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, SwitchType;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }
    SwitchType = ReadStringOrDefault(Params, TEXT("switch_type"), ReadStringOrDefault(Params, TEXT("node_type"), TEXT("String")));

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    const TArray<TSharedPtr<FJsonValue>>* Cases = nullptr;
    Params->TryGetArrayField(TEXT("cases"), Cases);

    UEdGraphNode* SwitchNode = nullptr;
    if (SwitchType.Contains(TEXT("Enum"), ESearchCase::IgnoreCase))
    {
        FString EnumPath;
        Params->TryGetStringField(TEXT("enum_path"), EnumPath);
        UEnum* Enum = FindFirstObjectSafe<UEnum>(*EnumPath);
        if (!Enum)
        {
            Enum = LoadObject<UEnum>(nullptr, *EnumPath);
        }
        UK2Node_SwitchEnum* EnumNode = NewObject<UK2Node_SwitchEnum>(Graph);
        EnumNode->SetEnum(Enum);
        EnumNode->NodePosX = ReadNodePosition(Params).X;
        EnumNode->NodePosY = ReadNodePosition(Params).Y;
        Graph->AddNode(EnumNode, true, false);
        EnumNode->CreateNewGuid();
        EnumNode->PostPlacedNewNode();
        EnumNode->AllocateDefaultPins();
        SwitchNode = EnumNode;
    }
    else if (SwitchType.Contains(TEXT("Name"), ESearchCase::IgnoreCase) || SwitchType.Contains(TEXT("GameplayTag"), ESearchCase::IgnoreCase))
    {
        UK2Node_SwitchName* NameNode = NewObject<UK2Node_SwitchName>(Graph);
        if (Cases)
        {
            for (const TSharedPtr<FJsonValue>& CaseValue : *Cases)
            {
                NameNode->PinNames.Add(FName(*CaseValue->AsString()));
            }
        }
        NameNode->NodePosX = ReadNodePosition(Params).X;
        NameNode->NodePosY = ReadNodePosition(Params).Y;
        Graph->AddNode(NameNode, true, false);
        NameNode->CreateNewGuid();
        NameNode->PostPlacedNewNode();
        NameNode->AllocateDefaultPins();
        SwitchNode = NameNode;
    }
    else if (SwitchType.Contains(TEXT("Int"), ESearchCase::IgnoreCase))
    {
        UK2Node_SwitchInteger* IntNode = AddTypedNodeToGraph<UK2Node_SwitchInteger>(Graph, ReadNodePosition(Params));
        int32 StartIndex = 0;
        Params->TryGetNumberField(TEXT("start_index"), StartIndex);
        IntNode->StartIndex = StartIndex;
        SwitchNode = IntNode;
    }
    else
    {
        UK2Node_SwitchString* StringNode = NewObject<UK2Node_SwitchString>(Graph);
        if (Cases)
        {
            for (const TSharedPtr<FJsonValue>& CaseValue : *Cases)
            {
                StringNode->PinNames.Add(FName(*CaseValue->AsString()));
            }
        }
        StringNode->NodePosX = ReadNodePosition(Params).X;
        StringNode->NodePosY = ReadNodePosition(Params).Y;
        Graph->AddNode(StringNode, true, false);
        StringNode->CreateNewGuid();
        StringNode->PostPlacedNewNode();
        StringNode->AllocateDefaultPins();
        SwitchNode = StringNode;
    }

    if (!SwitchNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create switch node"));
    }

    SwitchNode->ReconstructNode();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(SwitchNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddAsyncActionNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, ProxyClassName, FunctionName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("proxy_class"), ProxyClassName) ||
        !Params->TryGetStringField(TEXT("factory_function"), FunctionName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing async action parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    UClass* ProxyClass = ResolveClassForGraphTool(ProxyClassName);
    UFunction* FactoryFunction = FindFunctionByNameLoose(ProxyClass, FunctionName);
    if (!Blueprint || !Graph || !FactoryFunction)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve async factory function"));
    }

    UK2Node_AsyncAction* AsyncNode = AddTypedNodeToGraph<UK2Node_AsyncAction>(Graph, ReadNodePosition(Params));
    AsyncNode->InitializeProxyFromFunction(FactoryFunction);
    AsyncNode->ReconstructNode();

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(AsyncNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddCommentBox(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    UEdGraphNode_Comment* CommentNode = AddTypedNodeToGraph<UEdGraphNode_Comment>(Graph, ReadNodePosition(Params));
    CommentNode->NodeComment = ReadStringOrDefault(Params, TEXT("comment"), ReadStringOrDefault(Params, TEXT("text"), TEXT("Generated Logic")));
    CommentNode->CommentColor = FLinearColor(0.2f, 0.65f, 0.35f, 0.35f);
    CommentNode->MoveMode = ECommentBoxMode::GroupMovement;
    CommentNode->SetWidth(520.f);
    CommentNode->SetHeight(300.f);

    const TArray<TSharedPtr<FJsonValue>>* NodeIds = nullptr;
    if (Params->TryGetArrayField(TEXT("node_ids"), NodeIds) && NodeIds->Num() > 0)
    {
        int32 MinX = TNumericLimits<int32>::Max();
        int32 MinY = TNumericLimits<int32>::Max();
        int32 MaxX = TNumericLimits<int32>::Min();
        int32 MaxY = TNumericLimits<int32>::Min();
        for (const TSharedPtr<FJsonValue>& NodeIdValue : *NodeIds)
        {
            if (UEdGraphNode* Node = FindNodeById(Blueprint, NodeIdValue->AsString()))
            {
                MinX = FMath::Min(MinX, Node->NodePosX);
                MinY = FMath::Min(MinY, Node->NodePosY);
                MaxX = FMath::Max(MaxX, Node->NodePosX + FMath::Max(Node->NodeWidth, 220));
                MaxY = FMath::Max(MaxY, Node->NodePosY + FMath::Max(Node->NodeHeight, 120));
            }
        }
        if (MinX != TNumericLimits<int32>::Max())
        {
            CommentNode->NodePosX = MinX - 60;
            CommentNode->NodePosY = MinY - 80;
            CommentNode->SetWidth((MaxX - MinX) + 140);
            CommentNode->SetHeight((MaxY - MinY) + 160);
        }
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(CommentNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleCollapseToGraph(const TSharedPtr<FJsonObject>& Params, bool bMacro)
{
    FString BlueprintName, GraphName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("graph_name"), GraphName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing collapse graph parameters"));
    }

    TSharedPtr<FJsonObject> ExportResult = HandleExportNodesAsText(Params);
    if (!ExportResult.IsValid() || ExportResult->HasField(TEXT("error")))
    {
        return ExportResult;
    }

    FString ExportedText;
    ExportResult->TryGetStringField(TEXT("text"), ExportedText);

    TSharedPtr<FJsonObject> CreateParams = MakeShared<FJsonObject>(*Params);
    CreateParams->SetStringField(TEXT("name"), GraphName);
    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* NewGraph = FindOrCreateFunctionGraphFromParams(Blueprint, CreateParams, bMacro);
    if (!Blueprint || !NewGraph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create collapsed graph"));
    }

    TSet<UEdGraphNode*> ImportedNodes;
    FEdGraphUtilities::ImportNodesFromText(NewGraph, ExportedText, ImportedNodes);

    bool bDeleteOriginal = false;
    if (Params->TryGetBoolField(TEXT("delete_original"), bDeleteOriginal) && bDeleteOriginal)
    {
        const TArray<TSharedPtr<FJsonValue>>* NodeIds = nullptr;
        if (Params->TryGetArrayField(TEXT("node_ids"), NodeIds))
        {
            for (const TSharedPtr<FJsonValue>& NodeIdValue : *NodeIds)
            {
                if (UEdGraphNode* Node = FindNodeById(Blueprint, NodeIdValue->AsString()))
                {
                    Node->BreakAllNodeLinks();
                    FBlueprintEditorUtils::RemoveNode(Blueprint, Node, true);
                }
            }
        }
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("graph_name"), NewGraph->GetName());
    Result->SetStringField(TEXT("graph_type"), bMacro ? TEXT("macro") : TEXT("function"));
    Result->SetNumberField(TEXT("imported_node_count"), ImportedNodes.Num());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandlePromoteToVariable(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, VariableName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }
    VariableName = ReadStringOrDefault(Params, TEXT("variable_name"), TEXT("PromotedValue"));

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphNode* SourceNode = nullptr;
    UEdGraph* Graph = nullptr;
    UEdGraphPin* SourcePin = FindPinFromParams(Blueprint, Params, &SourceNode, &Graph);
    if (!Blueprint || !SourcePin || SourcePin->Direction != EGPD_Output)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Output pin not found"));
    }

    FBlueprintEditorUtils::AddMemberVariable(Blueprint, FName(*VariableName), SourcePin->PinType);
    UK2Node_VariableSet* SetNode = FUnrealMCPCommonUtils::CreateVariableSetNode(Graph, Blueprint, VariableName, FVector2D(SourceNode->NodePosX + 320, SourceNode->NodePosY));
    if (!SetNode)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to create Set variable node"));
    }

    if (UEdGraphPin* ValuePin = FindPinOnNode(SetNode, VariableName, EGPD_Input))
    {
        Graph->GetSchema()->TryCreateConnection(SourcePin, ValuePin);
    }

    FBlueprintEditorUtils::MarkBlueprintAsStructurallyModified(Blueprint);

    TSharedPtr<FJsonObject> Result = CreateNodeResult(SetNode);
    Result->SetStringField(TEXT("variable_name"), VariableName);
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleEditConstructionScript(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = Blueprint ? FBlueprintEditorUtils::FindUserConstructionScript(Blueprint) : nullptr;
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Construction Script not found or unsupported"));
    }

    FString NodeClassName;
    if (Params->TryGetStringField(TEXT("node_class"), NodeClassName))
    {
        TSharedPtr<FJsonObject> NodeParams = MakeShared<FJsonObject>(*Params);
        NodeParams->SetStringField(TEXT("graph_type"), TEXT("construction_script"));
        return HandleAddNodeByClass(NodeParams);
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("graph_name"), Graph->GetName());
    Result->SetNumberField(TEXT("node_count"), Graph->Nodes.Num());
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleReadGraphLogicFlow(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    TArray<FString> Lines;
    for (UEdGraphNode* Node : Graph->Nodes)
    {
        if (!Node)
        {
            continue;
        }

        TArray<FString> ExecLinks;
        for (UEdGraphPin* Pin : Node->Pins)
        {
            if (!Pin || Pin->Direction != EGPD_Output || Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec)
            {
                continue;
            }
            for (UEdGraphPin* LinkedPin : Pin->LinkedTo)
            {
                if (LinkedPin && LinkedPin->GetOwningNode())
                {
                    ExecLinks.Add(FString::Printf(TEXT("%s -> %s"),
                        *Pin->PinName.ToString(),
                        *LinkedPin->GetOwningNode()->GetNodeTitle(ENodeTitleType::ListView).ToString()));
                }
            }
            if (Pin->LinkedTo.Num() == 0)
            {
                ExecLinks.Add(FString::Printf(TEXT("%s -> desconectado"), *Pin->PinName.ToString()));
            }
        }

        if (ExecLinks.Num() > 0)
        {
            Lines.Add(FString::Printf(TEXT("%s (%s): %s"),
                *Node->GetNodeTitle(ENodeTitleType::ListView).ToString(),
                *Node->NodeGuid.ToString(),
                *FString::Join(ExecLinks, TEXT("; "))));
        }
    }

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetStringField(TEXT("graph_name"), Graph->GetName());
    Result->SetStringField(TEXT("story"), Lines.Num() > 0 ? FString::Join(Lines, TEXT("\n")) : TEXT("O grafo nao possui fluxo de execucao conectado."));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleInspectNodePins(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, NodeId;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("node_id"), NodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' or 'node_id' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphNode* Node = FindNodeById(Blueprint, NodeId);
    if (!Blueprint || !Node)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Node not found"));
    }

    return NodeToJsonObject(Node);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBreakStructNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, StructName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("struct_type"), StructName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing break struct parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    UScriptStruct* StructType = ResolveStructForGraphTool(StructName);
    if (!Blueprint || !Graph || !StructType)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve struct"));
    }

    UK2Node_BreakStruct* Node = AddTypedNodeToGraph<UK2Node_BreakStruct>(Graph, ReadNodePosition(Params));
    SetStructTypeOnStructNode(Node, StructType);
    Node->ReconstructNode();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(Node);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddMakeStructNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, StructName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("struct_type"), StructName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing make struct parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    UScriptStruct* StructType = ResolveStructForGraphTool(StructName);
    if (!Blueprint || !Graph || !StructType)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve struct"));
    }

    UK2Node_MakeStruct* Node = AddTypedNodeToGraph<UK2Node_MakeStruct>(Graph, ReadNodePosition(Params));
    SetStructTypeOnStructNode(Node, StructType);
    Node->ReconstructNode();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(Node);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddClassReferenceNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, ClassName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("class_path"), ClassName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing class reference parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    UClass* ClassRef = ResolveClassForGraphTool(ClassName);
    if (!Blueprint || !Graph || !ClassRef)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve class reference"));
    }

    UK2Node_Literal* LiteralNode = AddTypedNodeToGraph<UK2Node_Literal>(Graph, ReadNodePosition(Params));
    LiteralNode->SetObjectRef(ClassRef);
    LiteralNode->ReconstructNode();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(LiteralNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddBindEventNode(const TSharedPtr<FJsonObject>& Params)
{
    TSharedPtr<FJsonObject> BindParams = MakeShared<FJsonObject>(*Params);
    BindParams->SetStringField(TEXT("action"), TEXT("bind"));
    return HandleAddEventDispatcherNode(BindParams);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddCreateEventNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    UK2Node_CreateDelegate* CreateEventNode = AddTypedNodeToGraph<UK2Node_CreateDelegate>(Graph, ReadNodePosition(Params));
    FString FunctionName;
    if (Params->TryGetStringField(TEXT("function_name"), FunctionName) || Params->TryGetStringField(TEXT("event_name"), FunctionName))
    {
        CreateEventNode->SetFunction(FName(*FunctionName));
    }
    CreateEventNode->HandleAnyChange(true);
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(CreateEventNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddGlobalGetter(const TSharedPtr<FJsonObject>& Params)
{
    FString Getter = ReadStringOrDefault(Params, TEXT("getter"), TEXT("GetPlayerCharacter"));
    FString FunctionName = Getter.Replace(TEXT(" "), TEXT(""));
    if (FunctionName.Equals(TEXT("PlayerCharacter"), ESearchCase::IgnoreCase)) FunctionName = TEXT("GetPlayerCharacter");
    if (FunctionName.Equals(TEXT("PlayerController"), ESearchCase::IgnoreCase)) FunctionName = TEXT("GetPlayerController");
    if (FunctionName.Equals(TEXT("GameMode"), ESearchCase::IgnoreCase)) FunctionName = TEXT("GetGameMode");
    if (FunctionName.Equals(TEXT("GameInstance"), ESearchCase::IgnoreCase)) FunctionName = TEXT("GetGameInstance");
    if (FunctionName.Equals(TEXT("ActorOfClass"), ESearchCase::IgnoreCase)) FunctionName = TEXT("GetActorOfClass");

    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    UFunction* Function = FindFunctionByNameLoose(UGameplayStatics::StaticClass(), FunctionName);
    if (!Blueprint || !Graph || !Function)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve global getter"));
    }

    UK2Node_CallFunction* GetterNode = FUnrealMCPCommonUtils::CreateFunctionCallNode(Graph, Function, ReadNodePosition(Params));
    FString ClassName;
    if (Params->TryGetStringField(TEXT("actor_class"), ClassName) || Params->TryGetStringField(TEXT("class_path"), ClassName))
    {
        if (UEdGraphPin* ClassPin = FindPinOnNode(GetterNode, TEXT("ActorClass"), EGPD_Input))
        {
            if (const UEdGraphSchema_K2* Schema = Cast<const UEdGraphSchema_K2>(Graph->GetSchema()))
            {
                Schema->TrySetDefaultObject(*ClassPin, ResolveClassForGraphTool(ClassName));
            }
        }
    }

    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(GetterNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleAddMathExpressionNode(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, Expression;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("expression"), Expression))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing math expression parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraph* Graph = FindGraphForParams(Blueprint, Params);
    if (!Blueprint || !Graph)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Failed to resolve blueprint graph"));
    }

    UK2Node_MathExpression* MathNode = AddTypedNodeToGraph<UK2Node_MathExpression>(Graph, ReadNodePosition(Params));
    MathNode->Expression = Expression;
    MathNode->ReconstructNode();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);
    return CreateNodeResult(MathNode);
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleGetSpecificNodeError(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName, NodeId;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName) ||
        !Params->TryGetStringField(TEXT("node_id"), NodeId))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing node error parameters"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphNode* Node = FindNodeById(Blueprint, NodeId);
    if (!Blueprint || !Node)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Node not found"));
    }

    TArray<TSharedPtr<FJsonValue>> Issues;
    for (UEdGraphPin* Pin : Node->Pins)
    {
        if (!Pin || Pin->Direction != EGPD_Input)
        {
            continue;
        }
        const bool bMissingExec = Pin->PinType.PinCategory == UEdGraphSchema_K2::PC_Exec && Pin->LinkedTo.Num() == 0;
        const bool bMissingData = Pin->PinType.PinCategory != UEdGraphSchema_K2::PC_Exec && Pin->LinkedTo.Num() == 0 && Pin->DefaultValue.IsEmpty() && Pin->DefaultObject == nullptr;
        if (bMissingExec || bMissingData)
        {
            TSharedPtr<FJsonObject> Issue = MakeShared<FJsonObject>();
            Issue->SetStringField(TEXT("pin_name"), Pin->PinName.ToString());
            Issue->SetStringField(TEXT("message"), FString::Printf(TEXT("Pin '%s' is not connected and has no default value."), *Pin->PinName.ToString()));
            Issues.Add(MakeShared<FJsonValueObject>(Issue));
        }
    }

    TSharedPtr<FJsonObject> Result = NodeToJsonObject(Node);
    Result->SetArrayField(TEXT("issues"), Issues);
    Result->SetStringField(TEXT("summary"), Issues.Num() == 0 ? TEXT("No obvious node-local pin errors detected.") : FString::Printf(TEXT("%d possible node-local issue(s)."), Issues.Num()));
    return Result;
}

TSharedPtr<FJsonObject> FUnrealMCPBlueprintNodeCommands::HandleResolveWildcardPin(const TSharedPtr<FJsonObject>& Params)
{
    FString BlueprintName;
    if (!Params->TryGetStringField(TEXT("blueprint_name"), BlueprintName))
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Missing 'blueprint_name' parameter"));
    }

    UBlueprint* Blueprint = ResolveBlueprintForGraphTool(BlueprintName);
    UEdGraphNode* Node = nullptr;
    UEdGraphPin* Pin = FindPinFromParams(Blueprint, Params, &Node, nullptr);
    if (!Blueprint || !Pin || !Node)
    {
        return FUnrealMCPCommonUtils::CreateErrorResponse(TEXT("Pin not found"));
    }

    FString TypeName = ReadStringOrDefault(Params, TEXT("pin_type"), ReadStringOrDefault(Params, TEXT("type"), TEXT("Object")));
    Pin->Modify();
    Pin->PinType = PinTypeFromStrings(TypeName, ReadStringOrDefault(Params, TEXT("object_path")), ReadStringOrDefault(Params, TEXT("container_type")));
    Node->PinTypeChanged(Pin);
    Node->ReconstructNode();
    FBlueprintEditorUtils::MarkBlueprintAsModified(Blueprint);

    TSharedPtr<FJsonObject> Result = MakeShared<FJsonObject>();
    Result->SetObjectField(TEXT("pin"), PinToJsonObject(Pin));
    return Result;
}
