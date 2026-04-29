"""
Advanced Unreal MCP tools.

These tools cover higher-level AI setup, data validation, gameplay tags,
world queries, asset auditing, and framework Blueprint generation.
"""

import csv
import io
import logging
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def _send(command: str, params: dict) -> dict:
    from unreal_mcp_server import get_unreal_connection

    unreal = get_unreal_connection()
    if not unreal:
        return {"success": False, "message": "Failed to connect to Unreal Engine"}

    response = unreal.send_command(command, params)
    return response or {"success": False, "message": "No response"}


def register_advanced_tools(mcp: FastMCP):
    """Register advanced Unreal tools with the MCP server."""

    @mcp.tool()
    def create_ai_controller_blueprint(
        ctx: Context,
        name: str,
        path: str = "/Game/AI",
        add_perception: bool = True
    ) -> dict:
        """Create an AIController Blueprint, optionally with an AIPerception component."""
        try:
            return _send("create_ai_controller_blueprint", {
                "name": name,
                "path": path,
                "add_perception": add_perception
            })
        except Exception as e:
            logger.error(f"Error creating AI controller blueprint: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_ai_perception_component(
        ctx: Context,
        blueprint_name: str,
        component_name: str = "AIPerception"
    ) -> dict:
        """Add an AIPerception component to a Blueprint."""
        try:
            return _send("add_ai_perception_component", {
                "blueprint_name": blueprint_name,
                "component_name": component_name
            })
        except Exception as e:
            logger.error(f"Error adding AI perception component: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def configure_ai_perception(
        ctx: Context,
        blueprint_name: str,
        component_name: str = "AIPerception",
        sight: dict = None,
        hearing: dict = None,
        damage: dict = None
    ) -> dict:
        """Configure Sight/Hearing/Damage senses on an AIPerception component."""
        try:
            params = {
                "blueprint_name": blueprint_name,
                "component_name": component_name
            }
            if sight is not None:
                params["sight"] = sight
            if hearing is not None:
                params["hearing"] = hearing
            if damage is not None:
                params["damage"] = damage
            return _send("configure_ai_perception", params)
        except Exception as e:
            logger.error(f"Error configuring AI perception: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def configure_pawn_ai(
        ctx: Context,
        blueprint_name: str,
        ai_controller_class: str,
        auto_possess_ai: str = "PlacedInWorldOrSpawned"
    ) -> dict:
        """Assign an AIController class and Auto Possess AI mode to a Pawn Blueprint."""
        try:
            return _send("configure_pawn_ai", {
                "blueprint_name": blueprint_name,
                "ai_controller_class": ai_controller_class,
                "auto_possess_ai": auto_possess_ai
            })
        except Exception as e:
            logger.error(f"Error configuring pawn AI: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_env_query(ctx: Context, name: str, path: str = "/Game/AI/EQS") -> dict:
        """Create an Environment Query asset."""
        try:
            return _send("create_env_query", {"name": name, "path": path})
        except Exception as e:
            logger.error(f"Error creating EnvQuery: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_env_query_generator(
        ctx: Context,
        env_query_path: str,
        generator_class: str,
        properties: dict = None
    ) -> dict:
        """Add an EQS generator class to an EnvQuery option."""
        try:
            return _send("add_env_query_generator", {
                "env_query": env_query_path,
                "generator_class": generator_class,
                "properties": properties or {}
            })
        except Exception as e:
            logger.error(f"Error adding EnvQuery generator: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_env_query_test(
        ctx: Context,
        env_query_path: str,
        test_class: str,
        option_index: int = 0,
        properties: dict = None
    ) -> dict:
        """Add an EQS test to an EnvQuery option."""
        try:
            return _send("add_env_query_test", {
                "env_query": env_query_path,
                "test_class": test_class,
                "option_index": option_index,
                "properties": properties or {}
            })
        except Exception as e:
            logger.error(f"Error adding EnvQuery test: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def run_env_query(
        ctx: Context,
        env_query_path: str,
        querier: str = "",
        run_mode: str = "SingleResult"
    ) -> dict:
        """Run an EQS query in the editor world."""
        try:
            params = {"env_query": env_query_path, "run_mode": run_mode}
            if querier:
                params["querier"] = querier
            return _send("run_env_query", params)
        except Exception as e:
            logger.error(f"Error running EnvQuery: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_data_table_schema(
        ctx: Context,
        data_table_path: str
    ) -> dict:
        """Return the row struct fields for a Data Table."""
        try:
            return _send("get_data_table_schema", {
                "data_table": data_table_path
            })
        except Exception as e:
            logger.error(f"Error getting data table schema: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def validate_data_table_row(
        ctx: Context,
        data_table_path: str,
        row_data: dict
    ) -> dict:
        """Validate JSON row data against a Data Table row struct."""
        try:
            return _send("validate_data_table_row", {
                "data_table": data_table_path,
                "row_data": row_data
            })
        except Exception as e:
            logger.error(f"Error validating data table row: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def validate_item_balance(
        ctx: Context,
        items: list,
        required_fields: list = None
    ) -> dict:
        """Validate item rows for rarity, stat, price, and progression sanity."""
        try:
            required_fields = required_fields or ["id", "rarity", "price"]
            rarity_order = {"common": 1, "uncommon": 2, "rare": 3, "epic": 4, "legendary": 5}
            issues = []
            seen_ids = set()
            for index, item in enumerate(items):
                item_id = item.get("id") or item.get("name") or f"#{index}"
                for field in required_fields:
                    if field not in item:
                        issues.append({"item": item_id, "issue": f"Missing required field: {field}"})
                if item_id in seen_ids:
                    issues.append({"item": item_id, "issue": "Duplicate item id"})
                seen_ids.add(item_id)
                rarity = str(item.get("rarity", "")).lower()
                if rarity and rarity not in rarity_order:
                    issues.append({"item": item_id, "issue": f"Unknown rarity: {rarity}"})
                price = item.get("price", 0)
                if isinstance(price, (int, float)) and price < 0:
                    issues.append({"item": item_id, "issue": "Negative price"})
                stats = item.get("stats", {})
                if isinstance(stats, dict):
                    for stat_name, stat_value in stats.items():
                        if isinstance(stat_value, (int, float)) and stat_value < 0:
                            issues.append({"item": item_id, "issue": f"Negative stat: {stat_name}"})
            return {"success": True, "valid": len(issues) == 0, "item_count": len(items), "issues": issues}
        except Exception as e:
            logger.error(f"Error validating item balance: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def build_dialogue_rows(ctx: Context, dialogues: list) -> dict:
        """Normalize dialogue tree data into Data Table style rows."""
        try:
            rows = {}
            issues = []
            for dialogue in dialogues:
                node_id = dialogue.get("id")
                if not node_id:
                    issues.append({"issue": "Dialogue node missing id", "node": dialogue})
                    continue
                rows[node_id] = {
                    "Speaker": dialogue.get("speaker", ""),
                    "Text": dialogue.get("text", ""),
                    "Choices": dialogue.get("choices", []),
                    "Next": dialogue.get("next", "")
                }
            return {"success": True, "valid": len(issues) == 0, "rows": rows, "issues": issues}
        except Exception as e:
            logger.error(f"Error building dialogue rows: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def build_quest_rows(ctx: Context, quests: list) -> dict:
        """Normalize quest/objective/reward data into Data Table style rows."""
        try:
            rows = {}
            issues = []
            for quest in quests:
                quest_id = quest.get("id")
                if not quest_id:
                    issues.append({"issue": "Quest missing id", "quest": quest})
                    continue
                objectives = quest.get("objectives", [])
                if not objectives:
                    issues.append({"quest": quest_id, "issue": "Quest has no objectives"})
                rows[quest_id] = {
                    "Title": quest.get("title", ""),
                    "Description": quest.get("description", ""),
                    "Objectives": objectives,
                    "Rewards": quest.get("rewards", []),
                    "RequiredTags": quest.get("required_tags", [])
                }
            return {"success": True, "valid": len(issues) == 0, "rows": rows, "issues": issues}
        except Exception as e:
            logger.error(f"Error building quest rows: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def import_string_table_entries(
        ctx: Context,
        string_table_path: str,
        entries: dict
    ) -> dict:
        """Bulk import key/string entries into a String Table."""
        try:
            return _send("import_string_table_entries", {
                "string_table": string_table_path,
                "entries": entries
            })
        except Exception as e:
            logger.error(f"Error importing string table entries: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def import_string_table_csv(
        ctx: Context,
        string_table_path: str,
        csv_data: str,
        key_column: str = "key",
        text_column: str = "text"
    ) -> dict:
        """Import String Table entries from CSV content."""
        try:
            reader = csv.DictReader(io.StringIO(csv_data))
            entries = {}
            for row in reader:
                key = row.get(key_column, "")
                if key:
                    entries[key] = row.get(text_column, "")
            result = _send("import_string_table_entries", {
                "string_table": string_table_path,
                "entries": entries
            })
            result["parsed_count"] = len(entries)
            return result
        except Exception as e:
            logger.error(f"Error importing string table CSV: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def export_string_table_csv(
        ctx: Context,
        string_table_path: str,
        key_column: str = "key",
        text_column: str = "text"
    ) -> dict:
        """Export a String Table as CSV content."""
        try:
            result = _send("export_string_table", {"string_table": string_table_path})
            if not result.get("success"):
                return result
            output = io.StringIO()
            writer = csv.DictWriter(output, fieldnames=[key_column, text_column], lineterminator="\n")
            writer.writeheader()
            for key, text in result.get("entries", {}).items():
                writer.writerow({key_column: key, text_column: text})
            result["csv_data"] = output.getvalue()
            return result
        except Exception as e:
            logger.error(f"Error exporting string table CSV: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_gameplay_tag(
        ctx: Context,
        tag: str,
        comment: str = ""
    ) -> dict:
        """Add a Gameplay Tag to Config/DefaultGameplayTags.ini."""
        try:
            return _send("add_gameplay_tag", {
                "tag": tag,
                "comment": comment
            })
        except Exception as e:
            logger.error(f"Error adding gameplay tag: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def list_gameplay_tags(ctx: Context) -> dict:
        """List Gameplay Tags from Config/DefaultGameplayTags.ini."""
        try:
            return _send("list_gameplay_tags", {})
        except Exception as e:
            logger.error(f"Error listing gameplay tags: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def apply_gameplay_tags(
        ctx: Context,
        tags: list,
        actor: str = "",
        asset_path: str = ""
    ) -> dict:
        """Apply tags to an actor's Actor Tags or to an asset GameplayTagContainer property."""
        try:
            params = {"tags": tags}
            if actor:
                params["actor"] = actor
            if asset_path:
                params["asset_path"] = asset_path
            return _send("apply_gameplay_tags", params)
        except Exception as e:
            logger.error(f"Error applying gameplay tags: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_input_action(
        ctx: Context,
        name: str,
        path: str = "/Game/Input",
        value_type: str = "Boolean"
    ) -> dict:
        """Create an Enhanced Input Action asset."""
        try:
            return _send("create_input_action", {
                "name": name,
                "path": path,
                "value_type": value_type
            })
        except Exception as e:
            logger.error(f"Error creating input action: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_input_mapping_context(
        ctx: Context,
        name: str,
        path: str = "/Game/Input"
    ) -> dict:
        """Create an Enhanced Input Mapping Context asset."""
        try:
            return _send("create_input_mapping_context", {
                "name": name,
                "path": path
            })
        except Exception as e:
            logger.error(f"Error creating input mapping context: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_input_mapping(
        ctx: Context,
        mapping_context_path: str,
        input_action_path: str,
        key: str,
        properties: dict = None
    ) -> dict:
        """Map a key to an Input Action in an Input Mapping Context."""
        try:
            return _send("add_input_mapping", {
                "mapping_context": mapping_context_path,
                "input_action": input_action_path,
                "key": key,
                "properties": properties or {}
            })
        except Exception as e:
            logger.error(f"Error adding input mapping: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_niagara_system(ctx: Context, name: str, path: str = "/Game/VFX") -> dict:
        """Create a Niagara System asset."""
        try:
            return _send("create_niagara_system", {"name": name, "path": path})
        except Exception as e:
            logger.error(f"Error creating Niagara system: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_niagara_emitter(ctx: Context, name: str, path: str = "/Game/VFX") -> dict:
        """Create a Niagara Emitter asset."""
        try:
            return _send("create_niagara_emitter", {"name": name, "path": path})
        except Exception as e:
            logger.error(f"Error creating Niagara emitter: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def spawn_niagara_system(
        ctx: Context,
        system_path: str,
        location: list = None,
        parameters: dict = None
    ) -> dict:
        """Spawn a Niagara System in the editor world."""
        try:
            return _send("spawn_niagara_system", {
                "system": system_path,
                "location": location or [0.0, 0.0, 0.0],
                "parameters": parameters or {}
            })
        except Exception as e:
            logger.error(f"Error spawning Niagara system: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_sound_cue(ctx: Context, name: str, path: str = "/Game/Audio") -> dict:
        """Create a Sound Cue asset."""
        try:
            return _send("create_sound_cue", {"name": name, "path": path})
        except Exception as e:
            logger.error(f"Error creating sound cue: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def spawn_sound(
        ctx: Context,
        sound_path: str,
        location: list = None,
        volume: float = 1.0,
        pitch: float = 1.0
    ) -> dict:
        """Play a sound asset at a world location."""
        try:
            return _send("spawn_sound", {
                "sound": sound_path,
                "location": location or [0.0, 0.0, 0.0],
                "volume": volume,
                "pitch": pitch
            })
        except Exception as e:
            logger.error(f"Error spawning sound: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_collision_profile(
        ctx: Context,
        name: str,
        object_type: str = "WorldDynamic",
        collision_enabled: str = "QueryAndPhysics",
        default_response: str = "Block"
    ) -> dict:
        """Add a collision profile entry to DefaultEngine.ini."""
        try:
            return _send("create_collision_profile", {
                "name": name,
                "object_type": object_type,
                "collision_enabled": collision_enabled,
                "default_response": default_response
            })
        except Exception as e:
            logger.error(f"Error creating collision profile: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_collision_channel(
        ctx: Context,
        name: str,
        default_response: str = "Block",
        trace_type: bool = False
    ) -> dict:
        """Add a project collision channel entry to DefaultEngine.ini."""
        try:
            return _send("create_collision_channel", {
                "name": name,
                "default_response": default_response,
                "trace_type": trace_type
            })
        except Exception as e:
            logger.error(f"Error creating collision channel: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_functional_test(
        ctx: Context,
        name: str = "FunctionalTest",
        location: list = None
    ) -> dict:
        """Spawn a Functional Test actor in the editor world."""
        try:
            return _send("create_functional_test", {
                "name": name,
                "location": location or [0.0, 0.0, 0.0]
            })
        except Exception as e:
            logger.error(f"Error creating functional test: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def run_automation_tests(ctx: Context, filter: str = "Project") -> dict:
        """Start editor automation tests by filter."""
        try:
            return _send("run_automation_tests", {"filter": filter})
        except Exception as e:
            logger.error(f"Error running automation tests: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def line_trace(
        ctx: Context,
        start: list,
        end: list,
        channel: str = "Visibility"
    ) -> dict:
        """Run an editor-world line trace and return hit data."""
        try:
            return _send("line_trace", {
                "start": start,
                "end": end,
                "channel": channel
            })
        except Exception as e:
            logger.error(f"Error running line trace: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def find_actors_in_radius(
        ctx: Context,
        location: list,
        radius: float = 1000.0,
        class_filter: str = ""
    ) -> dict:
        """Find actors in the editor world within a radius."""
        try:
            return _send("find_actors_in_radius", {
                "location": location,
                "radius": radius,
                "class_filter": class_filter
            })
        except Exception as e:
            logger.error(f"Error finding actors in radius: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def find_actors_by_tag(ctx: Context, tag: str) -> dict:
        """Find actors with a given actor tag in the editor world."""
        try:
            return _send("find_actors_by_tag", {"tag": tag})
        except Exception as e:
            logger.error(f"Error finding actors by tag: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_actor_distance(
        ctx: Context,
        actor_a: str,
        actor_b: str
    ) -> dict:
        """Measure distance between two actors in the editor world."""
        try:
            return _send("get_actor_distance", {
                "actor_a": actor_a,
                "actor_b": actor_b
            })
        except Exception as e:
            logger.error(f"Error getting actor distance: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_asset_referencers(
        ctx: Context,
        asset_path: str
    ) -> dict:
        """List packages that reference an asset."""
        try:
            return _send("get_asset_referencers", {
                "asset_path": asset_path
            })
        except Exception as e:
            logger.error(f"Error getting asset referencers: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def validate_assets(
        ctx: Context,
        path: str = "/Game"
    ) -> dict:
        """Load and validate assets under a content path."""
        try:
            return _send("validate_assets", {
                "path": path
            })
        except Exception as e:
            logger.error(f"Error validating assets: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def validate_blueprint_graph(
        ctx: Context,
        blueprint_path: str
    ) -> dict:
        """Compile and inspect a Blueprint graph for orphan nodes and loose pins."""
        try:
            return _send("validate_blueprint_graph", {
                "blueprint": blueprint_path
            })
        except Exception as e:
            logger.error(f"Error validating blueprint graph: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_save_game_blueprint(
        ctx: Context,
        name: str,
        path: str = "/Game/Blueprints"
    ) -> dict:
        """Create a SaveGame Blueprint."""
        try:
            return _send("create_save_game_blueprint", {
                "name": name,
                "path": path
            })
        except Exception as e:
            logger.error(f"Error creating SaveGame blueprint: {e}")
            return {"success": False, "message": str(e)}

    # ===== Phase 5: Extended SaveGame, Enhanced Input, Niagara, Audio =====

    @mcp.tool()
    def create_save_game_with_variables(
        ctx: Context,
        name: str,
        path: str = "/Game/Blueprints",
        variables: list = None
    ) -> dict:
        """Create a SaveGame Blueprint with pre-defined variables.

        Args:
            name: Name for the SaveGame Blueprint.
            path: Content path for the asset.
            variables: List of dicts with 'name', 'type', and optional 'default'.
                       e.g. [{"name": "PlayerScore", "type": "Integer", "default": "0"}]
        """
        try:
            return _send("create_save_game_with_variables", {
                "name": name,
                "path": path,
                "variables": variables or []
            })
        except Exception as e:
            logger.error(f"Error creating SaveGame with variables: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def generate_save_load_functions(
        ctx: Context,
        blueprint_name: str,
        save_game_class: str,
        slot_name: str = "SaveSlot0"
    ) -> dict:
        """Generate SaveSlot, LoadSlot, and DoesSaveExist function nodes in a Blueprint.

        Args:
            blueprint_name: Target Blueprint (e.g. GameInstance or PlayerController).
            save_game_class: Path to the SaveGame Blueprint class.
            slot_name: Default save slot name.
        """
        try:
            return _send("generate_save_load_functions", {
                "blueprint_name": blueprint_name,
                "save_game_class": save_game_class,
                "slot_name": slot_name
            })
        except Exception as e:
            logger.error(f"Error generating save/load functions: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def setup_enhanced_input_runtime(
        ctx: Context,
        blueprint_name: str,
        mapping_context_path: str,
        priority: int = 0
    ) -> dict:
        """Generate Blueprint logic to add an Input Mapping Context on BeginPlay.

        Args:
            blueprint_name: Target Blueprint (e.g. PlayerController or Pawn).
            mapping_context_path: Content path of the Input Mapping Context.
            priority: Mapping context priority (higher = more priority).
        """
        try:
            return _send("setup_enhanced_input_runtime", {
                "blueprint_name": blueprint_name,
                "mapping_context_path": mapping_context_path,
                "priority": priority
            })
        except Exception as e:
            logger.error(f"Error setting up enhanced input runtime: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def bind_input_action_to_event(
        ctx: Context,
        blueprint_name: str,
        input_action_path: str,
        trigger_event: str = "Triggered",
        function_name: str = ""
    ) -> dict:
        """Bind an Enhanced Input Action to a Blueprint event.

        Args:
            blueprint_name: Target Blueprint.
            input_action_path: Content path of the Input Action.
            trigger_event: Trigger event type: 'Started', 'Triggered', 'Completed', 'Canceled'.
            function_name: Optional function to call (creates custom event if empty).
        """
        try:
            return _send("bind_input_action_to_event", {
                "blueprint_name": blueprint_name,
                "input_action_path": input_action_path,
                "trigger_event": trigger_event,
                "function_name": function_name
            })
        except Exception as e:
            logger.error(f"Error binding input action: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_player_controller_with_input(
        ctx: Context,
        name: str,
        path: str = "/Game/Blueprints",
        input_actions: list = None,
        mapping_context_name: str = ""
    ) -> dict:
        """Create a PlayerController Blueprint pre-configured with Enhanced Input.

        Args:
            name: Name for the PlayerController Blueprint.
            path: Content path for the assets.
            input_actions: List of input action configs, each a dict with
                           'name', 'key', 'value_type' (default: 'Boolean').
            mapping_context_name: Custom name for the mapping context.
        """
        try:
            return _send("create_player_controller_with_input", {
                "name": name,
                "path": path,
                "input_actions": input_actions or [],
                "mapping_context_name": mapping_context_name
            })
        except Exception as e:
            logger.error(f"Error creating PC with input: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_niagara_user_parameter(
        ctx: Context,
        system_path: str,
        parameter_name: str,
        parameter_type: str = "Float"
    ) -> dict:
        """Add a User Parameter to a Niagara System.

        Args:
            system_path: Content path of the Niagara System.
            parameter_name: Name for the user parameter.
            parameter_type: Parameter type: 'Float', 'Int32', 'Bool', 'Vector',
                            'LinearColor', 'Texture'.
        """
        try:
            return _send("add_niagara_user_parameter", {
                "system_path": system_path,
                "parameter_name": parameter_name,
                "parameter_type": parameter_type
            })
        except Exception as e:
            logger.error(f"Error adding Niagara user parameter: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_niagara_emitter_to_system(
        ctx: Context,
        system_path: str,
        emitter_path: str
    ) -> dict:
        """Add an emitter to a Niagara System.

        Args:
            system_path: Content path of the Niagara System.
            emitter_path: Content path of the Niagara Emitter to add.
        """
        try:
            return _send("add_niagara_emitter_to_system", {
                "system_path": system_path,
                "emitter_path": emitter_path
            })
        except Exception as e:
            logger.error(f"Error adding emitter to system: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_vfx_preset(
        ctx: Context,
        preset_type: str,
        name: str,
        path: str = "/Game/VFX"
    ) -> dict:
        """Create a common VFX preset Niagara System.

        Args:
            preset_type: One of 'impact', 'trail', 'aura', 'pickup', 'explosion'.
            name: Name for the Niagara System.
            path: Content path for the asset.
        """
        try:
            return _send("create_vfx_preset", {
                "preset_type": preset_type,
                "name": name,
                "path": path
            })
        except Exception as e:
            logger.error(f"Error creating VFX preset: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_sound_attenuation(
        ctx: Context,
        name: str,
        path: str = "/Game/Audio",
        inner_radius: float = 400.0,
        falloff_distance: float = 3600.0,
        attenuation_function: str = "NaturalSound"
    ) -> dict:
        """Create a Sound Attenuation asset.

        Args:
            name: Name for the attenuation settings.
            path: Content path for the asset.
            inner_radius: Inner radius (full volume).
            falloff_distance: Distance over which sound fades out.
            attenuation_function: 'Linear', 'Logarithmic', 'NaturalSound', 'Custom'.
        """
        try:
            return _send("create_sound_attenuation", {
                "name": name,
                "path": path,
                "inner_radius": inner_radius,
                "falloff_distance": falloff_distance,
                "attenuation_function": attenuation_function
            })
        except Exception as e:
            logger.error(f"Error creating sound attenuation: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_sound_mix(
        ctx: Context,
        name: str,
        path: str = "/Game/Audio"
    ) -> dict:
        """Create a Sound Mix / Sound Class setup.

        Args:
            name: Name for the Sound Mix asset.
            path: Content path for the asset.
        """
        try:
            return _send("create_sound_mix", {
                "name": name,
                "path": path
            })
        except Exception as e:
            logger.error(f"Error creating sound mix: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def configure_sound_cue_nodes(
        ctx: Context,
        sound_cue_path: str,
        sound_wave_paths: list = None,
        add_random: bool = False,
        add_modulator: bool = False,
        pitch_min: float = 0.9,
        pitch_max: float = 1.1
    ) -> dict:
        """Configure nodes inside a Sound Cue.

        Args:
            sound_cue_path: Content path of the Sound Cue.
            sound_wave_paths: List of Sound Wave asset paths to add.
            add_random: Add a Random node for variation.
            add_modulator: Add a Modulator node for pitch/volume variation.
            pitch_min: Minimum pitch multiplier (if modulator).
            pitch_max: Maximum pitch multiplier (if modulator).
        """
        try:
            return _send("configure_sound_cue_nodes", {
                "sound_cue_path": sound_cue_path,
                "sound_wave_paths": sound_wave_paths or [],
                "add_random": add_random,
                "add_modulator": add_modulator,
                "pitch_min": pitch_min,
                "pitch_max": pitch_max
            })
        except Exception as e:
            logger.error(f"Error configuring sound cue: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def attach_sound_to_actor(
        ctx: Context,
        actor_name: str,
        sound_path: str,
        auto_activate: bool = True,
        attenuation_path: str = ""
    ) -> dict:
        """Add an AudioComponent to an actor with a sound asset.

        Args:
            actor_name: Name of the target actor.
            sound_path: Content path of the sound asset.
            auto_activate: Whether the sound auto-plays.
            attenuation_path: Optional Sound Attenuation asset path.
        """
        try:
            return _send("attach_sound_to_actor", {
                "actor_name": actor_name,
                "sound_path": sound_path,
                "auto_activate": auto_activate,
                "attenuation_path": attenuation_path
            })
        except Exception as e:
            logger.error(f"Error attaching sound to actor: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Advanced tools registered successfully")

