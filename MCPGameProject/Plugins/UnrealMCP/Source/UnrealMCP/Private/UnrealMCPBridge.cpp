#include "UnrealMCPBridge.h"
#include "MCPServerRunnable.h"
#include "Sockets.h"
#include "SocketSubsystem.h"
#include "HAL/RunnableThread.h"
#include "Interfaces/IPv4/IPv4Address.h"
#include "Interfaces/IPv4/IPv4Endpoint.h"
#include "Dom/JsonObject.h"
#include "Dom/JsonValue.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonReader.h"
#include "Serialization/JsonWriter.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/DirectionalLight.h"
#include "Engine/PointLight.h"
#include "Engine/SpotLight.h"
#include "Camera/CameraActor.h"
#include "EditorAssetLibrary.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "JsonObjectConverter.h"
#include "GameFramework/Actor.h"
#include "Engine/Selection.h"
#include "Kismet/GameplayStatics.h"
#include "Async/Async.h"
// Add Blueprint related includes
#include "Engine/Blueprint.h"
#include "Engine/BlueprintGeneratedClass.h"
#include "Factories/BlueprintFactory.h"
#include "EdGraphSchema_K2.h"
#include "K2Node_Event.h"
#include "K2Node_VariableGet.h"
#include "K2Node_VariableSet.h"
#include "Components/StaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "Components/SphereComponent.h"
#include "Kismet2/BlueprintEditorUtils.h"
#include "Kismet2/KismetEditorUtilities.h"
// UE5.5 correct includes
#include "Engine/SimpleConstructionScript.h"
#include "Engine/SCS_Node.h"
#include "UObject/Field.h"
#include "UObject/FieldPath.h"
// Blueprint Graph specific includes
#include "EdGraph/EdGraph.h"
#include "EdGraph/EdGraphNode.h"
#include "EdGraph/EdGraphPin.h"
#include "K2Node_CallFunction.h"
#include "K2Node_InputAction.h"
#include "K2Node_Self.h"
#include "GameFramework/InputSettings.h"
#include "EditorSubsystem.h"
#include "Subsystems/EditorActorSubsystem.h"
// Include our new command handler classes
#include "Commands/UnrealMCPEditorCommands.h"
#include "Commands/UnrealMCPBlueprintCommands.h"
#include "Commands/UnrealMCPBlueprintNodeCommands.h"
#include "Commands/UnrealMCPProjectCommands.h"
#include "Commands/UnrealMCPCommonUtils.h"
#include "Commands/UnrealMCPUMGCommands.h"
#include "Commands/UnrealMCPAICommands.h"
#include "Commands/UnrealMCPDataCommands.h"
#include "Commands/UnrealMCPAdvancedCommands.h"

// Default settings
#define MCP_SERVER_HOST "127.0.0.1"
#define MCP_SERVER_PORT 55557

UUnrealMCPBridge::UUnrealMCPBridge()
{
    EditorCommands = MakeShared<FUnrealMCPEditorCommands>();
    BlueprintCommands = MakeShared<FUnrealMCPBlueprintCommands>();
    BlueprintNodeCommands = MakeShared<FUnrealMCPBlueprintNodeCommands>();
    ProjectCommands = MakeShared<FUnrealMCPProjectCommands>();
    UMGCommands = MakeShared<FUnrealMCPUMGCommands>();
    MaterialCommands = MakeShared<FUnrealMCPMaterialCommands>();
    LevelCommands = MakeShared<FUnrealMCPLevelCommands>();
    AssetCommands = MakeShared<FUnrealMCPAssetCommands>();
    SequencerCommands = MakeShared<FUnrealMCPSequencerCommands>();
}

UUnrealMCPBridge::~UUnrealMCPBridge()
{
    EditorCommands.Reset();
    BlueprintCommands.Reset();
    BlueprintNodeCommands.Reset();
    ProjectCommands.Reset();
    UMGCommands.Reset();
}

// Initialize subsystem
void UUnrealMCPBridge::Initialize(FSubsystemCollectionBase& Collection)
{
    UE_LOG(LogTemp, Display, TEXT("UnrealMCPBridge: Initializing"));
    
    bIsRunning = false;
    ListenerSocket = nullptr;
    ConnectionSocket = nullptr;
    ServerThread = nullptr;
    Port = MCP_SERVER_PORT;
    FIPv4Address::Parse(MCP_SERVER_HOST, ServerAddress);

    // Start the server automatically
    StartServer();
}

// Clean up resources when subsystem is destroyed
void UUnrealMCPBridge::Deinitialize()
{
    UE_LOG(LogTemp, Display, TEXT("UnrealMCPBridge: Shutting down"));
    StopServer();
}

// Start the MCP server
void UUnrealMCPBridge::StartServer()
{
    if (bIsRunning)
    {
        UE_LOG(LogTemp, Warning, TEXT("UnrealMCPBridge: Server is already running"));
        return;
    }

    // Create socket subsystem
    ISocketSubsystem* SocketSubsystem = ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM);
    if (!SocketSubsystem)
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCPBridge: Failed to get socket subsystem"));
        return;
    }

    // Create listener socket
    TSharedPtr<FSocket> NewListenerSocket = MakeShareable(SocketSubsystem->CreateSocket(NAME_Stream, TEXT("UnrealMCPListener"), false));
    if (!NewListenerSocket.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCPBridge: Failed to create listener socket"));
        return;
    }

    // Allow address reuse for quick restarts
    NewListenerSocket->SetReuseAddr(true);
    NewListenerSocket->SetNonBlocking(true);

    // Bind to address
    FIPv4Endpoint Endpoint(ServerAddress, Port);
    if (!NewListenerSocket->Bind(*Endpoint.ToInternetAddr()))
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCPBridge: Failed to bind listener socket to %s:%d"), *ServerAddress.ToString(), Port);
        return;
    }

    // Start listening
    if (!NewListenerSocket->Listen(5))
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCPBridge: Failed to start listening"));
        return;
    }

    ListenerSocket = NewListenerSocket;
    bIsRunning = true;
    UE_LOG(LogTemp, Display, TEXT("UnrealMCPBridge: Server started on %s:%d"), *ServerAddress.ToString(), Port);

    // Start server thread
    ServerThread = FRunnableThread::Create(
        new FMCPServerRunnable(this, ListenerSocket),
        TEXT("UnrealMCPServerThread"),
        0, TPri_Normal
    );

    if (!ServerThread)
    {
        UE_LOG(LogTemp, Error, TEXT("UnrealMCPBridge: Failed to create server thread"));
        StopServer();
        return;
    }
}

// Stop the MCP server
void UUnrealMCPBridge::StopServer()
{
    if (!bIsRunning)
    {
        return;
    }

    bIsRunning = false;

    // Clean up thread
    if (ServerThread)
    {
        ServerThread->Kill(true);
        delete ServerThread;
        ServerThread = nullptr;
    }

    // Close sockets
    if (ConnectionSocket.IsValid())
    {
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ConnectionSocket.Get());
        ConnectionSocket.Reset();
    }

    if (ListenerSocket.IsValid())
    {
        ISocketSubsystem::Get(PLATFORM_SOCKETSUBSYSTEM)->DestroySocket(ListenerSocket.Get());
        ListenerSocket.Reset();
    }

    UE_LOG(LogTemp, Display, TEXT("UnrealMCPBridge: Server stopped"));
}

// Execute a command received from a client
FString UUnrealMCPBridge::ExecuteCommand(const FString& CommandType, const TSharedPtr<FJsonObject>& Params)
{
    UE_LOG(LogTemp, Display, TEXT("UnrealMCPBridge: Executing command: %s"), *CommandType);
    
    // Create a promise to wait for the result
    TPromise<FString> Promise;
    TFuture<FString> Future = Promise.GetFuture();
    
    // Queue execution on Game Thread
    AsyncTask(ENamedThreads::GameThread, [this, CommandType, Params, Promise = MoveTemp(Promise)]() mutable
    {
        TSharedPtr<FJsonObject> ResponseJson = MakeShareable(new FJsonObject);
        
        try
        {
            TSharedPtr<FJsonObject> ResultJson;
            
            if (CommandType == TEXT("ping"))
            {
                ResultJson = MakeShareable(new FJsonObject);
                ResultJson->SetStringField(TEXT("message"), TEXT("pong"));
            }
            // Editor Commands (including actor manipulation, PIE, batch ops, viewport modes)
            else if (CommandType == TEXT("get_actors_in_level") ||
                     CommandType == TEXT("find_actors_by_name") ||
                     CommandType == TEXT("spawn_actor") ||
                     CommandType == TEXT("create_actor") ||
                     CommandType == TEXT("delete_actor") ||
                     CommandType == TEXT("set_actor_transform") ||
                     CommandType == TEXT("get_actor_properties") ||
                     CommandType == TEXT("set_actor_property") ||
                     CommandType == TEXT("spawn_blueprint_actor") ||
                     CommandType == TEXT("focus_viewport") ||
                     CommandType == TEXT("take_screenshot") ||
                     CommandType == TEXT("start_play_in_editor") ||
                     CommandType == TEXT("stop_play_in_editor") ||
                     CommandType == TEXT("get_play_in_editor_status") ||
                     CommandType == TEXT("get_actors_by_tag") ||
                     CommandType == TEXT("set_actors_transform_by_tag") ||
                     CommandType == TEXT("add_tag_to_actors_by_name") ||
                     CommandType == TEXT("set_actor_property_batch") ||
                     CommandType == TEXT("select_actors_by_tag") ||
                     CommandType == TEXT("set_viewport_display_mode") ||
                     CommandType == TEXT("get_viewport_display_mode") ||
                     CommandType == TEXT("set_viewport_show_flags") ||
                     CommandType == TEXT("undo_last_action") ||
                     CommandType == TEXT("redo_last_action") ||
                     CommandType == TEXT("bulk_set_actor_transform") ||
                     CommandType == TEXT("export_level_to_json") ||
                     CommandType == TEXT("duplicate_actor") ||
                     CommandType == TEXT("set_actor_material"))
            {
                ResultJson = EditorCommands->HandleCommand(CommandType, Params);
            }
            // Blueprint Commands
            else if (CommandType == TEXT("create_blueprint") || 
                     CommandType == TEXT("add_component_to_blueprint") || 
                     CommandType == TEXT("set_component_property") || 
                     CommandType == TEXT("set_physics_properties") || 
                     CommandType == TEXT("compile_blueprint") || 
                     CommandType == TEXT("set_blueprint_property") || 
                     CommandType == TEXT("set_static_mesh_properties") ||
                     CommandType == TEXT("set_pawn_properties") ||
                     CommandType == TEXT("hot_reload_blueprint"))
            {
                ResultJson = BlueprintCommands->HandleCommand(CommandType, Params);
            }
            // Blueprint Node Commands
            else if (CommandType == TEXT("connect_blueprint_nodes") ||
                     CommandType == TEXT("add_blueprint_get_self_component_reference") ||
                     CommandType == TEXT("add_blueprint_self_reference") ||
                     CommandType == TEXT("find_blueprint_nodes") ||
                     CommandType == TEXT("add_blueprint_event_node") ||
                     CommandType == TEXT("add_blueprint_input_action_node") ||
                     CommandType == TEXT("add_blueprint_function_node") ||
                     CommandType == TEXT("add_blueprint_get_component_node") ||
                     CommandType == TEXT("add_blueprint_variable") ||
                     CommandType == TEXT("get_blueprint_variable") ||
                     CommandType == TEXT("add_blueprint_variable_get_node") ||
                     CommandType == TEXT("add_blueprint_variable_set_node") ||
                     CommandType == TEXT("add_branch_node") ||
                     CommandType == TEXT("add_math_node") ||
                     CommandType == TEXT("add_cast_node") ||
                     CommandType == TEXT("add_sequence_node") ||
                     CommandType == TEXT("add_for_each_loop_node") ||
                     CommandType == TEXT("add_while_loop_node") ||
                     CommandType == TEXT("add_timeline_node") ||
                     CommandType == TEXT("add_timeline_float_track") ||
                     CommandType == TEXT("add_timeline_keyframe") ||
                     CommandType == TEXT("create_blueprint_function") ||
                     CommandType == TEXT("create_blueprint_macro") ||
                     CommandType == TEXT("create_blueprint_custom_event_graph") ||
                     CommandType == TEXT("add_node_by_class") ||
                     CommandType == TEXT("connect_pins_validated") ||
                     CommandType == TEXT("set_pin_default_value") ||
                     CommandType == TEXT("add_custom_event_node") ||
                     CommandType == TEXT("list_blueprint_functions") ||
                     CommandType == TEXT("get_node_pins") ||
                     CommandType == TEXT("delete_blueprint_node"))
            {
                ResultJson = BlueprintNodeCommands->HandleCommand(CommandType, Params);
            }
            // Project Commands
            else if (CommandType == TEXT("create_input_mapping"))
            {
                ResultJson = ProjectCommands->HandleCommand(CommandType, Params);
            }
            // UMG Commands
            else if (CommandType == TEXT("create_umg_widget_blueprint") ||
                     CommandType == TEXT("add_text_block_to_widget") ||
                     CommandType == TEXT("add_button_to_widget") ||
                     CommandType == TEXT("bind_widget_event") ||
                     CommandType == TEXT("set_text_block_binding") ||
                     CommandType == TEXT("add_widget_to_viewport"))
            {
                ResultJson = UMGCommands->HandleCommand(CommandType, Params);
            }
            // Material Commands
            else if (CommandType == TEXT("create_material") ||
                     CommandType == TEXT("create_material_instance") ||
                     CommandType == TEXT("set_material_scalar_parameter") ||
                     CommandType == TEXT("set_material_vector_parameter") ||
                     CommandType == TEXT("apply_material_to_actor"))
            {
                ResultJson = MaterialCommands->HandleCommand(CommandType, Params);
            }
            // Level Commands
            else if (CommandType == TEXT("save_current_level") ||
                     CommandType == TEXT("open_level") ||
                     CommandType == TEXT("get_current_level_name") ||
                     CommandType == TEXT("create_new_level") ||
                     CommandType == TEXT("add_sublevel") ||
                     CommandType == TEXT("remove_sublevel") ||
                     CommandType == TEXT("get_sublevels"))
            {
                ResultJson = LevelCommands->HandleCommand(CommandType, Params);
            }
            // Asset Commands
            else if (CommandType == TEXT("list_assets") ||
                     CommandType == TEXT("duplicate_asset") ||
                     CommandType == TEXT("move_asset") ||
                     CommandType == TEXT("delete_asset") ||
                     CommandType == TEXT("get_asset_info"))
            {
                ResultJson = AssetCommands->HandleCommand(CommandType, Params);
            }
            // Sequencer Commands
            else if (CommandType == TEXT("create_level_sequence") ||
                     CommandType == TEXT("add_actor_to_sequence") ||
                     CommandType == TEXT("add_transform_track") ||
                     CommandType == TEXT("add_transform_keyframe") ||
                     CommandType == TEXT("get_sequences_in_level") ||
                     CommandType == TEXT("add_camera_cut_track") ||
                     CommandType == TEXT("add_camera_cut"))
            {
                ResultJson = SequencerCommands->HandleCommand(CommandType, Params);
            }
            // AI Commands
            else if (CommandType == TEXT("create_behavior_tree") ||
                     CommandType == TEXT("add_behavior_tree_node") ||
                     CommandType == TEXT("connect_behavior_tree_nodes") ||
                     CommandType == TEXT("add_behavior_tree_decorator") ||
                     CommandType == TEXT("get_behavior_tree_nodes") ||
                     CommandType == TEXT("create_blackboard") ||
                     CommandType == TEXT("add_blackboard_key") ||
                     CommandType == TEXT("get_blackboard_keys") ||
                     CommandType == TEXT("build_nav_mesh"))
            {
                ResultJson = FUnrealMCPAICommands::HandleCommand(CommandType, Params);
            }
            // Data Commands
            else if (CommandType == TEXT("create_data_table") ||
                     CommandType == TEXT("import_data_table") ||
                     CommandType == TEXT("export_data_table") ||
                     CommandType == TEXT("add_data_table_row") ||
                     CommandType == TEXT("edit_data_table_row") ||
                     CommandType == TEXT("get_data_table_row") ||
                     CommandType == TEXT("list_data_table_rows") ||
                     CommandType == TEXT("create_data_asset") ||
                     CommandType == TEXT("set_data_asset_properties") ||
                     CommandType == TEXT("get_data_asset_properties") ||
                     CommandType == TEXT("create_string_table") ||
                     CommandType == TEXT("add_string_table_entry") ||
                     CommandType == TEXT("get_string_table_entry") ||
                     CommandType == TEXT("remove_string_table_entry") ||
                     CommandType == TEXT("list_string_table_entries") ||
                     CommandType == TEXT("export_string_table"))
            {
                ResultJson = FUnrealMCPDataCommands::HandleCommand(CommandType, Params);
            }
            // Advanced AI/Data/World tooling
            else if (CommandType == TEXT("create_ai_controller_blueprint") ||
                     CommandType == TEXT("add_ai_perception_component") ||
                     CommandType == TEXT("configure_ai_perception") ||
                     CommandType == TEXT("configure_pawn_ai") ||
                     CommandType == TEXT("create_env_query") ||
                     CommandType == TEXT("add_env_query_generator") ||
                     CommandType == TEXT("add_env_query_test") ||
                     CommandType == TEXT("run_env_query") ||
                     CommandType == TEXT("get_data_table_schema") ||
                     CommandType == TEXT("validate_data_table_row") ||
                     CommandType == TEXT("import_string_table_entries") ||
                     CommandType == TEXT("add_gameplay_tag") ||
                     CommandType == TEXT("list_gameplay_tags") ||
                     CommandType == TEXT("apply_gameplay_tags") ||
                     CommandType == TEXT("create_input_action") ||
                     CommandType == TEXT("create_input_mapping_context") ||
                     CommandType == TEXT("add_input_mapping") ||
                     CommandType == TEXT("create_niagara_system") ||
                     CommandType == TEXT("create_niagara_emitter") ||
                     CommandType == TEXT("spawn_niagara_system") ||
                     CommandType == TEXT("create_sound_cue") ||
                     CommandType == TEXT("spawn_sound") ||
                     CommandType == TEXT("create_collision_profile") ||
                     CommandType == TEXT("create_collision_channel") ||
                     CommandType == TEXT("create_functional_test") ||
                     CommandType == TEXT("run_automation_tests") ||
                     CommandType == TEXT("line_trace") ||
                     CommandType == TEXT("find_actors_in_radius") ||
                     CommandType == TEXT("find_actors_by_tag") ||
                     CommandType == TEXT("get_actor_distance") ||
                     CommandType == TEXT("get_asset_referencers") ||
                     CommandType == TEXT("validate_assets") ||
                     CommandType == TEXT("validate_blueprint_graph") ||
                     CommandType == TEXT("create_save_game_blueprint") ||
                     // Phase 3: Extended BT/EQS commands
                     CommandType == TEXT("create_bt_task_blueprint") ||
                     CommandType == TEXT("create_bt_service_blueprint") ||
                     CommandType == TEXT("add_bt_run_eqs_query") ||
                     CommandType == TEXT("set_bt_node_blackboard_key") ||
                     CommandType == TEXT("create_full_ai_setup") ||
                     // Phase 5: Extended SaveGame/Input/Niagara/Audio
                     CommandType == TEXT("create_save_game_with_variables") ||
                     CommandType == TEXT("generate_save_load_functions") ||
                     CommandType == TEXT("setup_enhanced_input_runtime") ||
                     CommandType == TEXT("bind_input_action_to_event") ||
                     CommandType == TEXT("create_player_controller_with_input") ||
                     CommandType == TEXT("add_niagara_user_parameter") ||
                     CommandType == TEXT("add_niagara_emitter_to_system") ||
                     CommandType == TEXT("create_vfx_preset") ||
                     CommandType == TEXT("create_sound_attenuation") ||
                     CommandType == TEXT("create_sound_mix") ||
                     CommandType == TEXT("configure_sound_cue_nodes") ||
                     CommandType == TEXT("attach_sound_to_actor") ||
                     // Phase 6: Recipe commands
                     CommandType == TEXT("create_enemy_ai_archetype") ||
                     CommandType == TEXT("create_interactable_archetype") ||
                     CommandType == TEXT("create_pickup_item_archetype") ||
                     // Phase 6: Diff/Preview commands
                     CommandType == TEXT("preview_operation") ||
                     CommandType == TEXT("get_undo_history"))
            {
                ResultJson = FUnrealMCPAdvancedCommands::HandleCommand(CommandType, Params);
            }
            // Schema commands (Phase 3)
            else if (CommandType == TEXT("create_user_defined_struct") ||
                     CommandType == TEXT("add_struct_field") ||
                     CommandType == TEXT("remove_struct_field") ||
                     CommandType == TEXT("get_struct_fields") ||
                     CommandType == TEXT("recompile_struct") ||
                     CommandType == TEXT("create_data_table_from_struct") ||
                     CommandType == TEXT("generate_schema"))
            {
                ResultJson = FUnrealMCPSchemaCommands::HandleCommand(CommandType, Params);
            }
            // Validation commands (Phase 4)
            else if (CommandType == TEXT("find_unused_assets") ||
                     CommandType == TEXT("find_broken_references") ||
                     CommandType == TEXT("validate_naming_conventions") ||
                     CommandType == TEXT("validate_data_table_against_schema") ||
                     CommandType == TEXT("generate_validation_report") ||
                     CommandType == TEXT("get_automation_test_results") ||
                     CommandType == TEXT("get_automation_test_list"))
            {
                ResultJson = FUnrealMCPValidationCommands::HandleCommand(CommandType, Params);
            }
            // Level Design commands (Phase 5)
            else if (CommandType == TEXT("snap_actor_to_grid") ||
                     CommandType == TEXT("align_actors") ||
                     CommandType == TEXT("procedural_scatter") ||
                     CommandType == TEXT("get_actor_bounds") ||
                     CommandType == TEXT("find_overlapping_actors") ||
                     CommandType == TEXT("create_volume"))
            {
                ResultJson = FUnrealMCPLevelDesignCommands::HandleCommand(CommandType, Params);
            }
            // Physics commands (Phase 5)
            else if (CommandType == TEXT("create_physics_constraint") ||
                     CommandType == TEXT("setup_ragdoll") ||
                     CommandType == TEXT("enable_chaos_destruction") ||
                     CommandType == TEXT("set_physics_constraint_properties"))
            {
                ResultJson = FUnrealMCPPhysicsCommands::HandleCommand(CommandType, Params);
            }
            // Diagnostics commands (Phase 6)
            else if (CommandType == TEXT("get_blueprint_compile_errors") ||
                     CommandType == TEXT("get_message_log") ||
                     CommandType == TEXT("get_output_log") ||
                     CommandType == TEXT("get_editor_warnings") ||
                     CommandType == TEXT("get_map_check_errors"))
            {
                ResultJson = FUnrealMCPDiagnosticsCommands::HandleCommand(CommandType, Params);
            }
            // Policy commands (Phase 6)
            else if (CommandType == TEXT("validate_naming_policy") ||
                     CommandType == TEXT("auto_organize_assets") ||
                     CommandType == TEXT("batch_rename_assets") ||
                     CommandType == TEXT("fix_redirectors"))
            {
                ResultJson = FUnrealMCPPolicyCommands::HandleCommand(CommandType, Params);
            }
            else
            {
                ResponseJson->SetStringField(TEXT("status"), TEXT("error"));
                ResponseJson->SetStringField(TEXT("error"), FString::Printf(TEXT("Unknown command: %s"), *CommandType));
                
                FString ResultString;
                TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
                FJsonSerializer::Serialize(ResponseJson.ToSharedRef(), Writer);
                Promise.SetValue(ResultString);
                return;
            }
            
            // Check if the result contains an error
            bool bSuccess = true;
            FString ErrorMessage;
            
            if (ResultJson->HasField(TEXT("success")))
            {
                bSuccess = ResultJson->GetBoolField(TEXT("success"));
                if (!bSuccess && ResultJson->HasField(TEXT("error")))
                {
                    ErrorMessage = ResultJson->GetStringField(TEXT("error"));
                }
            }
            
            if (bSuccess)
            {
                // Set success status and include the result
                ResponseJson->SetStringField(TEXT("status"), TEXT("success"));
                ResponseJson->SetObjectField(TEXT("result"), ResultJson);
            }
            else
            {
                // Set error status and include the error message
                ResponseJson->SetStringField(TEXT("status"), TEXT("error"));
                ResponseJson->SetStringField(TEXT("error"), ErrorMessage);
            }
        }
        catch (const std::exception& e)
        {
            ResponseJson->SetStringField(TEXT("status"), TEXT("error"));
            ResponseJson->SetStringField(TEXT("error"), UTF8_TO_TCHAR(e.what()));
        }
        
        FString ResultString;
        TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&ResultString);
        FJsonSerializer::Serialize(ResponseJson.ToSharedRef(), Writer);
        Promise.SetValue(ResultString);
    });
    
    return Future.Get();
}
