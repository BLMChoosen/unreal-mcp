"""
Unreal Engine MCP Server

A simple MCP server for interacting with Unreal Engine.
"""

import logging
import socket
import sys
import json
from contextlib import asynccontextmanager
from typing import AsyncIterator, Dict, Any, Optional
from mcp.server.fastmcp import FastMCP

# Configure logging with more detailed format
logging.basicConfig(
    level=logging.DEBUG,  # Change to DEBUG level for more details
    format='%(asctime)s - %(name)s - %(levelname)s - [%(filename)s:%(lineno)d] - %(message)s',
    handlers=[
        logging.FileHandler('unreal_mcp.log'),
        # logging.StreamHandler(sys.stdout) # Remove this handler to unexpected non-whitespace characters in JSON
    ]
)
logger = logging.getLogger("UnrealMCP")

# Configuration
UNREAL_HOST = "127.0.0.1"
UNREAL_PORT = 55557
UNREAL_CONNECT_TIMEOUT_SECONDS = 5
UNREAL_COMMAND_TIMEOUT_SECONDS = 30

class UnrealConnection:
    """Connection to an Unreal Engine instance."""
    
    def __init__(self):
        """Initialize the connection."""
        self.socket = None
        self.connected = False
    
    def connect(self) -> bool:
        """Connect to the Unreal Engine instance."""
        try:
            # Close any existing socket
            if self.socket:
                try:
                    self.socket.close()
                except:
                    pass
                self.socket = None
            
            logger.info(f"Connecting to Unreal at {UNREAL_HOST}:{UNREAL_PORT}...")
            self.socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
            self.socket.settimeout(UNREAL_CONNECT_TIMEOUT_SECONDS)
            
            # Set socket options for better stability
            self.socket.setsockopt(socket.IPPROTO_TCP, socket.TCP_NODELAY, 1)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_KEEPALIVE, 1)
            
            # Set larger buffer sizes
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_RCVBUF, 65536)
            self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_SNDBUF, 65536)
            
            self.socket.connect((UNREAL_HOST, UNREAL_PORT))
            self.connected = True
            logger.info("Connected to Unreal Engine")
            return True
            
        except Exception as e:
            logger.error(f"Failed to connect to Unreal: {e}")
            self.connected = False
            return False
    
    def disconnect(self):
        """Disconnect from the Unreal Engine instance."""
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
        self.socket = None
        self.connected = False

    def receive_full_response(self, sock, buffer_size=4096) -> bytes:
        """Receive a complete response from Unreal, handling chunked data."""
        chunks = []
        sock.settimeout(UNREAL_COMMAND_TIMEOUT_SECONDS)
        try:
            while True:
                chunk = sock.recv(buffer_size)
                if not chunk:
                    if not chunks:
                        raise Exception("Connection closed before receiving data")
                    break
                chunks.append(chunk)
                
                # Process the data received so far
                data = b''.join(chunks)
                decoded_data = data.decode('utf-8')
                
                # Try to parse as JSON to check if complete
                try:
                    json.loads(decoded_data)
                    logger.info(f"Received complete response ({len(data)} bytes)")
                    return data
                except json.JSONDecodeError:
                    # Not complete JSON yet, continue reading
                    logger.debug(f"Received partial response, waiting for more data...")
                    continue
                except Exception as e:
                    logger.warning(f"Error processing response chunk: {str(e)}")
                    continue
        except socket.timeout:
            logger.warning("Socket timeout during receive")
            if chunks:
                # If we have some data already, try to use it
                data = b''.join(chunks)
                try:
                    json.loads(data.decode('utf-8'))
                    logger.info(f"Using partial response after timeout ({len(data)} bytes)")
                    return data
                except:
                    pass
            raise Exception("Timeout receiving Unreal response")
        except Exception as e:
            logger.error(f"Error during receive: {str(e)}")
            raise
    
    def send_command(self, command: str, params: Dict[str, Any] = None) -> Optional[Dict[str, Any]]:
        """Send a command to Unreal Engine and get the response."""
        # Always reconnect for each command, since Unreal closes the connection after each command
        # This is different from Unity which keeps connections alive
        if self.socket:
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
            self.connected = False
        
        if not self.connect():
            logger.error("Failed to connect to Unreal Engine for command")
            return None
        
        try:
            # Match Unity's command format exactly
            command_obj = {
                "type": command,  # Use "type" instead of "command"
                "params": params or {}  # Use Unity's params or {} pattern
            }
            
            # Send without newline, exactly like Unity
            command_json = json.dumps(command_obj)
            logger.info(f"Sending command: {command_json}")
            self.socket.sendall(command_json.encode('utf-8'))
            
            # Read response using improved handler
            response_data = self.receive_full_response(self.socket)
            response = json.loads(response_data.decode('utf-8'))
            
            # Log complete response for debugging
            logger.info(f"Complete response from Unreal: {response}")
            
            # Check for both error formats: {"status": "error", ...} and {"success": false, ...}
            if response.get("status") == "error":
                error_message = response.get("error") or response.get("message", "Unknown Unreal error")
                logger.error(f"Unreal error (status=error): {error_message}")
                # We want to preserve the original error structure but ensure error is accessible
                if "error" not in response:
                    response["error"] = error_message
            elif response.get("success") is False:
                # This format uses {"success": false, "error": "message"} or {"success": false, "message": "message"}
                error_message = response.get("error") or response.get("message", "Unknown Unreal error")
                logger.error(f"Unreal error (success=false): {error_message}")
                # Convert to the standard format expected by higher layers
                response = {
                    "status": "error",
                    "error": error_message
                }
            
            # Always close the connection after command is complete
            # since Unreal will close it on its side anyway
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
            self.connected = False
            
            return response
            
        except Exception as e:
            logger.error(f"Error sending command: {e}")
            # Always reset connection state on any error
            self.connected = False
            try:
                self.socket.close()
            except:
                pass
            self.socket = None
            return {
                "status": "error",
                "error": str(e)
            }

# Global connection state
_unreal_connection: UnrealConnection = None

def get_unreal_connection() -> Optional[UnrealConnection]:
    """Get the connection to Unreal Engine."""
    global _unreal_connection
    try:
        if _unreal_connection is None:
            _unreal_connection = UnrealConnection()
            if not _unreal_connection.connect():
                logger.warning("Could not connect to Unreal Engine")
                _unreal_connection = None
        else:
            # Verify connection is still valid with a ping-like test
            try:
                # Simple test by sending an empty buffer to check if socket is still connected
                _unreal_connection.socket.sendall(b'\x00')
                logger.debug("Connection verified with ping test")
            except Exception as e:
                logger.warning(f"Existing connection failed: {e}")
                _unreal_connection.disconnect()
                _unreal_connection = None
                # Try to reconnect
                _unreal_connection = UnrealConnection()
                if not _unreal_connection.connect():
                    logger.warning("Could not reconnect to Unreal Engine")
                    _unreal_connection = None
                else:
                    logger.info("Successfully reconnected to Unreal Engine")
        
        return _unreal_connection
    except Exception as e:
        logger.error(f"Error getting Unreal connection: {e}")
        return None

@asynccontextmanager
async def server_lifespan(server: FastMCP) -> AsyncIterator[Dict[str, Any]]:
    """Handle server startup and shutdown."""
    global _unreal_connection
    logger.info("UnrealMCP server starting up")
    try:
        _unreal_connection = get_unreal_connection()
        if _unreal_connection:
            logger.info("Connected to Unreal Engine on startup")
        else:
            logger.warning("Could not connect to Unreal Engine on startup")
    except Exception as e:
        logger.error(f"Error connecting to Unreal Engine on startup: {e}")
        _unreal_connection = None
    
    try:
        yield {}
    finally:
        if _unreal_connection:
            _unreal_connection.disconnect()
            _unreal_connection = None
        logger.info("Unreal MCP server shut down")

# Initialize server
mcp = FastMCP(
    "UnrealMCP",
    description="Unreal Engine integration via Model Context Protocol",
    lifespan=server_lifespan
)

# Import and register tools
from tools.editor_tools import register_editor_tools
from tools.blueprint_tools import register_blueprint_tools
from tools.node_tools import register_blueprint_node_tools
from tools.blueprint_deep_graph_tools import register_blueprint_deep_graph_tools
from tools.project_tools import register_project_tools
from tools.umg_tools import register_umg_tools
from tools.material_tools import register_material_tools
from tools.level_tools import register_level_tools
from tools.asset_tools import register_asset_tools
from tools.sequencer_tools import register_sequencer_tools
from tools.ai_tools import register_ai_tools
from tools.behavior_tree_tools import register_behavior_tree_tools
from tools.data_tools import register_data_tools
from tools.advanced_tools import register_advanced_tools
from tools.schema_tools import register_schema_tools
from tools.validation_tools import register_validation_tools
from tools.level_design_tools import register_level_design_tools
from tools.physics_tools import register_physics_tools
from tools.recipe_tools import register_recipe_tools
from tools.diff_tools import register_diff_tools
from tools.policy_tools import register_policy_tools
from tools.diagnostics_tools import register_diagnostics_tools
from tools.pcg_landscape_tools import register_pcg_landscape_tools
from tools.animation_tools import register_animation_tools
from tools.metasound_tools import register_metasound_tools
from tools.art_direction_tools import register_art_direction_tools
from tools.optimization_tools import register_optimization_tools
from tools.source_control_tools import register_source_control_tools
from tools.cpp_tools import register_cpp_tools
from tools.multiplayer_tools import register_multiplayer_tools

# Register tools
register_editor_tools(mcp)
register_blueprint_tools(mcp)
register_blueprint_node_tools(mcp)
register_blueprint_deep_graph_tools(mcp)
register_project_tools(mcp)
register_umg_tools(mcp)
register_material_tools(mcp)
register_level_tools(mcp)
register_asset_tools(mcp)
register_sequencer_tools(mcp)
register_ai_tools(mcp)
register_behavior_tree_tools(mcp)
register_data_tools(mcp)
register_advanced_tools(mcp)
register_schema_tools(mcp)
register_validation_tools(mcp)
register_level_design_tools(mcp)
register_physics_tools(mcp)
register_recipe_tools(mcp)
register_diff_tools(mcp)
register_policy_tools(mcp)
register_diagnostics_tools(mcp)
register_pcg_landscape_tools(mcp)
register_animation_tools(mcp)
register_metasound_tools(mcp)
register_art_direction_tools(mcp)
register_optimization_tools(mcp)
register_source_control_tools(mcp)
register_cpp_tools(mcp)
register_multiplayer_tools(mcp)


@mcp.prompt()
def info():
    """Information about available Unreal MCP tools and best practices."""
    return """
    # Unreal MCP Server Tools and Best Practices
    
    ## UMG (Widget Blueprint) Tools
    - `create_umg_widget_blueprint(widget_name, parent_class="UserWidget", path="/Game/UI")` 
      Create a new UMG Widget Blueprint
    - `add_text_block_to_widget(widget_name, text_block_name, text="", position=[0,0], size=[200,50], font_size=12, color=[1,1,1,1])`
      Add a Text Block widget with customizable properties
    - `add_button_to_widget(widget_name, button_name, text="", position=[0,0], size=[200,50], font_size=12, color=[1,1,1,1], background_color=[0.1,0.1,0.1,1])`
      Add a Button widget with text and styling
    - `bind_widget_event(widget_name, widget_component_name, event_name, function_name="")`
      Bind events like OnClicked to functions
    - `add_widget_to_viewport(widget_name, z_order=0)`
      Add widget instance to game viewport
    - `set_text_block_binding(widget_name, text_block_name, binding_property, binding_type="Text")`
      Set up dynamic property binding for text blocks

    ## Editor Tools
    ### Viewport and Screenshots
    - `focus_viewport(target, location, distance, orientation)` - Focus viewport
    - `take_screenshot(filename, show_ui, resolution)` - Capture screenshots

    ### Actor Management
    - `get_actors_in_level()` - List all actors in current level
    - `find_actors_by_name(pattern)` - Find actors by name pattern
    - `spawn_actor(name, type, location=[0,0,0], rotation=[0,0,0], scale=[1,1,1])` - Create actors
    - `delete_actor(name)` - Remove actors
    - `set_actor_transform(name, location, rotation, scale)` - Modify actor transform
    - `get_actor_properties(name)` - Get actor properties
    
    ## Blueprint Management
    - `create_blueprint(name, parent_class)` - Create new Blueprint classes
    - `add_component_to_blueprint(blueprint_name, component_type, component_name)` - Add components
    - `set_static_mesh_properties(blueprint_name, component_name, static_mesh)` - Configure meshes
    - `set_physics_properties(blueprint_name, component_name)` - Configure physics
    - `compile_blueprint(blueprint_name)` - Compile Blueprint changes
    - `set_blueprint_property(blueprint_name, property_name, property_value)` - Set properties
    - `set_pawn_properties(blueprint_name)` - Configure Pawn settings
    - `spawn_blueprint_actor(blueprint_name, actor_name)` - Spawn Blueprint actors
    
    ## Blueprint Node Management
    - `add_blueprint_event_node(blueprint_name, event_type)` - Add event nodes
    - `add_blueprint_input_action_node(blueprint_name, action_name)` - Add input nodes
    - `add_blueprint_function_node(blueprint_name, target, function_name)` - Add function nodes
    - `connect_blueprint_nodes(blueprint_name, source_node_id, source_pin, target_node_id, target_pin)` - Connect nodes
    - `add_blueprint_variable(blueprint_name, variable_name, variable_type)` - Add variables
    - `add_blueprint_get_self_component_reference(blueprint_name, component_name)` - Add component refs
    - `add_blueprint_self_reference(blueprint_name)` - Add self references
    - `find_blueprint_nodes(blueprint_name, node_type, event_type)` - Find nodes

    ## Deep Blueprint Graph Authoring
    - `add_blueprint_control_node`, `add_blueprint_cast_node`, `add_blueprint_math_node` - Add common control, cast, and math nodes
    - `set_variable_default_value`, `set_variable_container_type`, `add_local_variable` - Manage defaults, containers, and function-local data
    - `split_struct_pin`, `recombine_struct_pin`, `resolve_wildcard_pin`, `add_dynamic_pin_to_node` - Manipulate advanced pins
    - `delete_blueprint_node`, `disconnect_blueprint_pin`, `move_blueprint_node`, `format_blueprint_nodes` - Refactor and organize graphs
    - `add_custom_event_node`, `add_function_entry_node`, `set_function_flags`, `edit_construction_script` - Author events and functions
    - `create_blueprint_interface`, `implement_interface`, `add_interface_message_node` - Interface-based communication
    - `create_event_dispatcher`, `add_event_dispatcher_node`, `add_bind_event_node`, `add_create_event_node` - Delegate and dispatcher workflows
    - `add_collection_node`, `add_format_text_node`, `add_switch_node`, `add_async_action_node` - Dynamic intelligent nodes
    - `add_spawn_actor_node`, `add_break_struct_node`, `add_make_struct_node`, `add_class_reference_node`, `add_global_getter` - Gameplay data and world access
    - `export_nodes_as_text`, `paste_nodes_from_text`, `read_graph_logic_flow`, `inspect_node_pins`, `get_specific_node_error` - Clipboard, semantic reading, and debug helpers
    
    ## Project Tools
    - `create_input_mapping(action_name, key, input_type)` - Create input mappings

    ## AI Tools
    - `create_blackboard(name, path)` - Create Blackboard assets
    - `add_blackboard_key(blackboard_path, key_name, key_type)` - Add Blackboard keys
    - `get_blackboard_keys(blackboard_path)` - List Blackboard keys
    - `create_behavior_tree(name, path, blackboard_path)` - Create Behavior Tree assets
    - `add_behavior_tree_node(behavior_tree_path, node_type)` - Add Selector, Sequence, or Task nodes
    - `connect_behavior_tree_nodes(behavior_tree_path, parent_node_id, child_node_id)` - Link Behavior Tree nodes
    - `add_behavior_tree_decorator(behavior_tree_path, parent_node_id, child_node_id, decorator_type)` - Add decorators
    - `build_nav_mesh(extent_x, extent_y, extent_z, location)` - Spawn or expand NavMeshBoundsVolume
    - `create_ai_controller_blueprint(name, path, add_perception)` - Create AIController Blueprints
    - `add_ai_perception_component(blueprint_name, component_name)` - Add AI perception to Blueprints
    - `configure_ai_perception(blueprint_name, component_name, sight, hearing, damage)` - Configure Sight/Hearing/Damage senses
    - `configure_pawn_ai(blueprint_name, ai_controller_class, auto_possess_ai)` - Assign Pawn AI controller settings
    - `create_env_query(name, path)` - Create EQS EnvQuery assets
    - `add_env_query_generator(env_query_path, generator_class, properties)` - Add EQS generators
    - `add_env_query_test(env_query_path, test_class, option_index, properties)` - Add EQS tests
    - `run_env_query(env_query_path, querier, run_mode)` - Start an EQS query in the editor world

    ## Data Tools
    - `create_data_table(name, row_struct, path)` - Create a Data Table asset
    - `import_data_table(data_table_path, data, data_format)` - Import CSV or JSON into a Data Table
    - `export_data_table(data_table_path, data_format)` - Export Data Table CSV or JSON
    - `add_data_table_row(data_table_path, row_name, row_data)` - Add Data Table rows
    - `get_data_table_row(data_table_path, row_name)` - Read a Data Table row
    - `edit_data_table_row(data_table_path, row_name, row_data)` - Update a Data Table row
    - `list_data_table_rows(data_table_path)` - List Data Table row names
    - `create_data_asset(name, path, asset_class)` - Create Data Asset instances
    - `set_data_asset_properties(data_asset_path, properties)` - Set Data Asset fields
    - `create_string_table(name, path)` - Create String Table assets
    - `add_string_table_entry(string_table_path, key, source_string)` - Add or update localized strings
    - `get_string_table_entry(string_table_path, key)` - Read a localized string
    - `remove_string_table_entry(string_table_path, key)` - Remove a localized string
    - `list_string_table_entries(string_table_path)` - List String Table entries
    - `export_string_table(string_table_path)` - Export String Table entries as JSON

    ## Advanced Design Tools
    - `get_data_table_schema(data_table_path)` - Inspect Data Table row struct fields
    - `validate_data_table_row(data_table_path, row_data)` - Validate JSON against a Data Table schema
    - `validate_item_balance(items)` - Validate item rarity/stat/price balance data
    - `build_dialogue_rows(dialogues)` - Normalize dialogue trees into row dictionaries
    - `build_quest_rows(quests)` - Normalize quests/objectives/rewards into row dictionaries
    - `import_string_table_entries(string_table_path, entries)` - Bulk import localization entries
    - `import_string_table_csv(string_table_path, csv_data)` - Import localization CSV
    - `export_string_table_csv(string_table_path)` - Export localization CSV
    - `add_gameplay_tag(tag, comment)` - Add Gameplay Tags to project config
    - `list_gameplay_tags()` - List Gameplay Tags from project config
    - `apply_gameplay_tags(tags, actor, asset_path)` - Apply tags to actors or GameplayTagContainer assets
    - `create_input_action(name, path, value_type)` - Create Enhanced Input Actions
    - `create_input_mapping_context(name, path)` - Create Enhanced Input Mapping Contexts
    - `add_input_mapping(mapping_context_path, input_action_path, key)` - Map keys to Input Actions
    - `create_niagara_system(name, path)` - Create Niagara Systems
    - `create_niagara_emitter(name, path)` - Create Niagara Emitters
    - `spawn_niagara_system(system_path, location, parameters)` - Spawn Niagara VFX
    - `create_sound_cue(name, path)` - Create Sound Cues
    - `spawn_sound(sound_path, location, volume, pitch)` - Play sounds in the editor world
    - `create_collision_profile(name, object_type, collision_enabled)` - Add collision profiles
    - `create_collision_channel(name, default_response, trace_type)` - Add collision channels
    - `create_functional_test(name, location)` - Spawn Functional Test actors
    - `run_automation_tests(filter)` - Start editor automation tests
    - `line_trace(start, end, channel)` - Query scene hits in the editor world
    - `find_actors_in_radius(location, radius, class_filter)` - Spatial actor query
    - `find_actors_by_tag(tag)` - Find actors by tag
    - `get_actor_distance(actor_a, actor_b)` - Measure actor distance
    - `get_asset_referencers(asset_path)` - List package referencers for an asset
    - `validate_assets(path)` - Load and report asset validation issues
    - `validate_blueprint_graph(blueprint_path)` - Detect orphan nodes and loose pins
    - `create_save_game_blueprint(name, path)` - Create SaveGame Blueprints

    ## Unreal 5 Production Workflow Tools
    - `create_pcg_graph(name, path)` / `configure_pcg_graph(graph_path, nodes, edges)` - Create PCG Graph assets and store graph node specs
    - `create_pcg_volume(name, graph_path, location, extent)` - Spawn PCG Volumes when the PCG plugin is enabled
    - `create_biome_from_prompt(prompt, asset_paths, density, bounds_min, bounds_max)` - Scatter biome meshes from project assets
    - `create_landscape(...)`, `apply_landscape_material(material_path)`, `create_road_spline(points)` - Terrain setup helpers
    - `create_anim_blueprint`, `create_blend_space`, `create_anim_montage_from_sequence`, `add_anim_notify` - Animation asset setup
    - `create_ik_rig`, `create_ik_retargeter` - IK retargeting setup
    - `create_metasound_source`, `configure_metasound_graph`, `import_dialogue_batch` - MetaSound and dialogue audio workflows
    - `apply_lighting_preset` / `apply_art_direction_prompt` - Lighting, fog, sky, and post-process presets
    - `audit_optimization`, `set_nanite_enabled`, `generate_hlods`, `capture_profile_snapshot` - Optimization and profiling automation
    - `git_status`, `git_diff`, `git_commit_changes`, `p4_status`, `p4_submit`, `explain_uasset_conflict` - Source control helpers
    - `create_unreal_cpp_class`, `trigger_live_coding_compile` - C++ class wizard and Live Coding
    - `set_blueprint_variable_replication`, `create_blueprint_rpc_event` - Multiplayer replication helpers
    
    ## Best Practices
    
    ### UMG Widget Development
    - Create widgets with descriptive names that reflect their purpose
    - Use consistent naming conventions for widget components
    - Organize widget hierarchy logically
    - Set appropriate anchors and alignment for responsive layouts
    - Use property bindings for dynamic updates instead of direct setting
    - Handle widget events appropriately with meaningful function names
    - Clean up widgets when no longer needed
    - Test widget layouts at different resolutions
    
    ### Editor and Actor Management
    - Use unique names for actors to avoid conflicts
    - Clean up temporary actors
    - Validate transforms before applying
    - Check actor existence before modifications
    - Take regular viewport screenshots during development
    - Keep the viewport focused on relevant actors during operations
    
    ### Blueprint Development
    - Compile Blueprints after changes
    - Use meaningful names for variables and functions
    - Organize nodes logically
    - Test functionality in isolation
    - Consider performance implications
    - Document complex setups
    
    ### Error Handling
    - Check command responses for success
    - Handle errors gracefully
    - Log important operations
    - Validate parameters
    - Clean up resources on errors
    """

# Run the server
if __name__ == "__main__":
    logger.info("Starting MCP server with stdio transport")
    mcp.run(transport='stdio') 
