<div align="center">

# Model Context Protocol for Unreal Engine
<span style="color: #555555">unreal-mcp</span>

[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](https://opensource.org/licenses/MIT)
[![Unreal Engine](https://img.shields.io/badge/Unreal%20Engine-5.5%2B-orange)](https://www.unrealengine.com)
[![Python](https://img.shields.io/badge/Python-3.12%2B-yellow)](https://www.python.org)
[![Status](https://img.shields.io/badge/Status-Experimental-red)](https://github.com/chongdashu/unreal-mcp)

</div>

This project enables AI assistant clients like Cursor, Windsurf and Claude Desktop to control Unreal Engine through natural language using the Model Context Protocol (MCP).

## ⚠️ Experimental Status

This project is currently in an **EXPERIMENTAL** state. The API, functionality, and implementation details are subject to significant changes. While we encourage testing and feedback, please be aware that:

- Breaking changes may occur without notice
- Features may be incomplete or unstable
- Documentation may be outdated or missing
- Production use is not recommended at this time

## 🌟 Overview

The Unreal MCP integration provides a comprehensive suite of tools for controlling Unreal Engine through natural language, organized into core editor modules plus Unreal 5 production workflow modules:

| Category | Capabilities |
|----------|-------------|
| **Actor Management** | Create/delete actors, set transforms, query properties, list level actors |
| **Blueprint Development** | Create Blueprint classes, add/configure components, compile, spawn Blueprint actors |
| **Blueprint Node Graph** | Add event/function nodes, connect pins, add variables, manage component references |
| **Editor Control** | Focus viewport, take screenshots, camera orientation |
| **UMG (UI Widgets)** | Create widget blueprints, add text/button widgets, bind events, add to viewport |
| **Material Tools** | Create and configure materials and material instances |
| **Level Tools** | Manage levels and streaming |
| **Asset Tools** | Browse and manage content browser assets |
| **Sequencer** | Create and edit cinematics with Sequencer |
| **AI & Behavior Trees** | Create Blackboards, Behavior Trees, AI Controllers, EQS queries, NavMesh |
| **Data Tools** | Create/edit Data Tables, Data Assets, String Tables with CSV/JSON support |
| **Advanced Design Tools** | Gameplay Tags, Enhanced Input, Niagara VFX, Sound Cues, collision config, world queries, asset validation |
| **Project Tools** | Create input mappings and configure project settings |
| **PCG & Landscape** | Create PCG graph assets/specs, spawn PCG volumes, scatter prompt-driven biomes, apply landscape materials, create road splines |
| **Animation & Retargeting** | Create AnimBPs, Blend Spaces, Montages, IK Rigs, IK Retargeters, notify specs, and state machine specs |
| **MetaSound & Dialogue** | Create MetaSound Source assets/specs and import dialogue JSON with optional Sound Wave imports |
| **Art Direction** | Apply lighting, fog, sky, and post-process presets from prompts |
| **Optimization & Profiling** | Audit Nanite/texture budgets, batch-toggle Nanite, request HLODs, toggle profiling stats |
| **Source Control** | Git/Perforce status, diff, commit/submit helpers, and binary `.uasset` conflict summaries |
| **C++ & Live Coding** | Generate Unreal C++ class boilerplate and trigger editor Live Coding compile |
| **Multiplayer** | Set Blueprint variable replication and create RunOnServer/Client/Multicast RPC custom events |

All capabilities are accessible through natural language commands via AI assistants, making it easy to automate and control Unreal Engine workflows.

## 🧩 Components

### Sample Project `MCPGameProject`
- Based off the Blank Project, but with the UnrealMCP plugin already added.

### Plugin `MCPGameProject/Plugins/UnrealMCP`
- Native TCP server for MCP communication (port 55557)
- Integrates with Unreal Editor subsystems
- Implements actor manipulation tools
- Handles command execution and response handling

### Python MCP Server `Python/unreal_mcp_server.py`
- Manages TCP socket connections to the C++ plugin
- Handles command serialization and response parsing
- Provides error handling and connection management
- Loads and registers tool modules from the `tools/` directory
- Uses the FastMCP library to implement the Model Context Protocol

## 📂 Directory Structure

```
unreal-mcp/
├── MCPGameProject/               # Example Unreal project
│   └── Plugins/UnrealMCP/        # C++ plugin source
│       ├── Source/UnrealMCP/     # Plugin source code
│       └── UnrealMCP.uplugin     # Plugin definition
│
├── Python/                       # Python server and tools
│   ├── tools/                    # Tool modules
│   │   ├── editor_tools.py       # Actor management & viewport control
│   │   ├── blueprint_tools.py    # Blueprint creation & configuration
│   │   ├── node_tools.py         # Blueprint node graph editing
│   │   ├── blueprint_deep_graph_tools.py # Advanced Blueprint graph authoring
│   │   ├── umg_tools.py          # UMG widget development
│   │   ├── material_tools.py     # Material & material instance tools
│   │   ├── level_tools.py        # Level management
│   │   ├── asset_tools.py        # Content browser asset tools
│   │   ├── sequencer_tools.py    # Cinematics & Sequencer
│   │   ├── ai_tools.py           # AI, Blackboard & Behavior Tree
│   │   ├── behavior_tree_tools.py# Behavior Tree node tools
│   │   ├── data_tools.py         # Data Table, Data Asset, String Table
│   │   ├── advanced_tools.py     # Gameplay Tags, EQS, Niagara, Audio, etc.
│   │   ├── project_tools.py      # Project settings & input mappings
│   │   ├── pcg_landscape_tools.py# PCG, biome, landscape, and spline helpers
│   │   ├── animation_tools.py    # AnimBP, Montage, Blend Space, IK setup
│   │   ├── metasound_tools.py    # MetaSound and dialogue audio
│   │   ├── art_direction_tools.py# Lighting/post-process presets
│   │   ├── optimization_tools.py # Nanite, HLOD, profiling audits
│   │   ├── source_control_tools.py# Git/Perforce helpers
│   │   ├── cpp_tools.py          # C++ class wizard and Live Coding
│   │   └── multiplayer_tools.py  # Replication and RPC helpers
│   ├── scripts/                  # Example scripts and demos
│   └── unreal_mcp_server.py      # MCP server entry point
│
└── Docs/                         # Documentation
    └── Tools/                    # Per-tool documentation
```

## 🚀 Quick Start Guide

### Prerequisites
- Unreal Engine 5.5+
- Python 3.12+
- MCP Client (e.g., Claude Desktop, Cursor, Windsurf)

### Sample Project

For getting started quickly, use the starter project in `MCPGameProject`. This is a UE 5.5 Blank Starter Project with the `UnrealMCP.uplugin` already configured.

1. **Prepare the project**
   - Right-click your `.uproject` file
   - Select **Generate Visual Studio project files**
2. **Build the project (including the plugin)**
   - Open the solution (`.sln`)
   - Choose `Development Editor` as your target
   - Build

### Plugin (Existing Project)

If you want to use the plugin in your existing project:

1. **Copy the plugin to your project**
   - Copy `MCPGameProject/Plugins/UnrealMCP` to your project's `Plugins/` folder

2. **Enable the plugin**
   - Go to **Edit → Plugins**
   - Find **UnrealMCP** in the Editor category
   - Enable the plugin and restart the editor when prompted

3. **Build the plugin**
   - Right-click your `.uproject` file → **Generate Visual Studio project files**
   - Open the solution (`.sln`) and build with your target platform

### Python Server Setup

1. Install [`uv`](https://astral.sh/uv) if you haven't already:
   ```bash
   # Windows (PowerShell)
   powershell -ExecutionPolicy ByPass -c "irm https://astral.sh/uv/install.ps1 | iex"

   # Unix/macOS
   curl -LsSf https://astral.sh/uv/install.sh | sh
   ```

2. Create and activate a virtual environment inside `Python/`:
   ```bash
   cd Python
   uv venv
   .venv\Scripts\activate   # Windows
   # or
   source .venv/bin/activate # Unix/macOS
   ```

3. Install dependencies:
   ```bash
   uv pip install -e .
   ```

See [Python/README.md](Python/README.md) for more details, including how to use the test scripts.

### Configuring your MCP Client

Use the following JSON for your MCP configuration:

```json
{
  "mcpServers": {
    "unrealMCP": {
      "command": "uv",
      "args": [
        "--directory",
        "<path/to/the/folder/PYTHON>",
        "run",
        "unreal_mcp_server.py"
      ]
    }
  }
}
```

An example is found in `mcp.json` at the project root.

### MCP Configuration Locations

| MCP Client | Configuration File Location | Notes |
|------------|------------------------------|-------|
| Claude Desktop | `~/.config/claude-desktop/mcp.json` | Windows: `%USERPROFILE%\.config\claude-desktop\mcp.json` |
| Cursor | `.cursor/mcp.json` | Located in your project root directory |
| Windsurf | `~/.config/windsurf/mcp.json` | Windows: `%USERPROFILE%\.config\windsurf\mcp.json` |

## 🛠️ Tool Reference

<details>
<summary><strong>Actor Management (editor_tools)</strong></summary>

- `get_actors_in_level()` — List all actors in the current level
- `find_actors_by_name(pattern)` — Find actors by name pattern
- `spawn_actor(name, type, location, rotation, scale)` — Create actors (cubes, spheres, lights, cameras, etc.)
- `delete_actor(name)` — Remove actors from the level
- `set_actor_transform(name, location, rotation, scale)` — Modify actor transform
- `get_actor_properties(name)` — Query actor properties
- `focus_viewport(target, location, distance, orientation)` — Focus the editor viewport
- `take_screenshot(filename, show_ui, resolution)` — Capture viewport screenshots
</details>

<details>
<summary><strong>Blueprint Development (blueprint_tools)</strong></summary>

- `create_blueprint(name, parent_class)` — Create new Blueprint classes
- `add_component_to_blueprint(blueprint_name, component_type, component_name)` — Add components
- `set_static_mesh_properties(blueprint_name, component_name, static_mesh)` — Configure meshes
- `set_physics_properties(blueprint_name, component_name)` — Configure physics
- `compile_blueprint(blueprint_name)` — Compile Blueprint changes
- `set_blueprint_property(blueprint_name, property_name, property_value)` — Set properties
- `set_pawn_properties(blueprint_name)` — Configure Pawn settings
- `spawn_blueprint_actor(blueprint_name, actor_name)` — Spawn Blueprint actors
</details>

<details>
<summary><strong>Blueprint Node Graph (node_tools)</strong></summary>

- `add_blueprint_event_node(blueprint_name, event_type)` — Add event nodes (BeginPlay, Tick, etc.)
- `add_blueprint_input_action_node(blueprint_name, action_name)` — Add input action nodes
- `add_blueprint_function_node(blueprint_name, target, function_name)` — Add function call nodes
- `connect_blueprint_nodes(blueprint_name, source_node_id, source_pin, target_node_id, target_pin)` — Connect nodes
- `add_blueprint_variable(blueprint_name, variable_name, variable_type)` — Add variables
- `add_blueprint_get_self_component_reference(blueprint_name, component_name)` — Add component refs
- `add_blueprint_self_reference(blueprint_name)` — Add self references
- `find_blueprint_nodes(blueprint_name, node_type, event_type)` — Find nodes in the graph
</details>

<details>
<summary><strong>UMG Widget Development (umg_tools)</strong></summary>

- `create_umg_widget_blueprint(widget_name, parent_class, path)` — Create Widget Blueprints
- `add_text_block_to_widget(widget_name, text_block_name, text, position, size, font_size, color)` — Add Text Blocks
- `add_button_to_widget(widget_name, button_name, text, position, size, font_size, color, background_color)` — Add Buttons
- `bind_widget_event(widget_name, widget_component_name, event_name, function_name)` — Bind events
- `add_widget_to_viewport(widget_name, z_order)` — Add widget to game viewport
- `set_text_block_binding(widget_name, text_block_name, binding_property, binding_type)` — Set dynamic bindings
</details>

<details>
<summary><strong>AI & Behavior Trees (ai_tools + behavior_tree_tools)</strong></summary>

- `create_blackboard(name, path)` — Create Blackboard assets
- `add_blackboard_key(blackboard_path, key_name, key_type)` — Add Blackboard keys
- `get_blackboard_keys(blackboard_path)` — List Blackboard keys
- `create_behavior_tree(name, path, blackboard_path)` — Create Behavior Tree assets
- `add_behavior_tree_node(behavior_tree_path, node_type)` — Add Selector, Sequence, or Task nodes
- `connect_behavior_tree_nodes(behavior_tree_path, parent_node_id, child_node_id)` — Link nodes
- `add_behavior_tree_decorator(behavior_tree_path, parent_node_id, child_node_id, decorator_type)` — Add decorators
- `build_nav_mesh(extent_x, extent_y, extent_z, location)` — Spawn or expand NavMeshBoundsVolume
- `create_ai_controller_blueprint(name, path, add_perception)` — Create AIController Blueprints
- `add_ai_perception_component(blueprint_name, component_name)` — Add AI Perception
- `configure_ai_perception(blueprint_name, component_name, sight, hearing, damage)` — Configure senses
- `configure_pawn_ai(blueprint_name, ai_controller_class, auto_possess_ai)` — Set Pawn AI controller
- `create_env_query(name, path)` — Create EQS EnvQuery assets
- `add_env_query_generator(env_query_path, generator_class, properties)` — Add EQS generators
- `add_env_query_test(env_query_path, test_class, option_index, properties)` — Add EQS tests
- `run_env_query(env_query_path, querier, run_mode)` — Run an EQS query
</details>

<details>
<summary><strong>Data Tools (data_tools)</strong></summary>

- `create_data_table(name, row_struct, path)` — Create Data Table assets
- `import_data_table(data_table_path, data, data_format)` — Import CSV or JSON into a Data Table
- `export_data_table(data_table_path, data_format)` — Export Data Table as CSV or JSON
- `add_data_table_row(data_table_path, row_name, row_data)` — Add rows
- `get_data_table_row(data_table_path, row_name)` — Read a row
- `edit_data_table_row(data_table_path, row_name, row_data)` — Update a row
- `list_data_table_rows(data_table_path)` — List row names
- `create_data_asset(name, path, asset_class)` — Create Data Asset instances
- `set_data_asset_properties(data_asset_path, properties)` — Set Data Asset fields
- `create_string_table(name, path)` — Create String Table assets
- `add_string_table_entry(string_table_path, key, source_string)` — Add/update localized strings
- `get_string_table_entry(string_table_path, key)` — Read a localized string
- `remove_string_table_entry(string_table_path, key)` — Remove a localized string
- `list_string_table_entries(string_table_path)` — List all entries
- `export_string_table(string_table_path)` — Export as JSON
</details>

<details>
<summary><strong>Advanced Design Tools (advanced_tools)</strong></summary>

**Data Validation**
- `get_data_table_schema(data_table_path)` — Inspect Data Table row struct fields
- `validate_data_table_row(data_table_path, row_data)` — Validate JSON against a schema
- `validate_item_balance(items)` — Validate item rarity/stat/price balance
- `build_dialogue_rows(dialogues)` — Normalize dialogue trees into Data Table rows
- `build_quest_rows(quests)` — Normalize quests/objectives/rewards into rows

**Localization**
- `import_string_table_entries(string_table_path, entries)` — Bulk import localization entries
- `import_string_table_csv(string_table_path, csv_data)` — Import from CSV
- `export_string_table_csv(string_table_path)` — Export as CSV

**Gameplay Tags**
- `add_gameplay_tag(tag, comment)` — Add tags to `DefaultGameplayTags.ini`
- `list_gameplay_tags()` — List all Gameplay Tags
- `apply_gameplay_tags(tags, actor, asset_path)` — Apply tags to actors or assets

**Enhanced Input**
- `create_input_action(name, path, value_type)` — Create Enhanced Input Actions
- `create_input_mapping_context(name, path)` — Create Input Mapping Contexts
- `add_input_mapping(mapping_context_path, input_action_path, key)` — Map keys to actions

**VFX & Audio**
- `create_niagara_system(name, path)` — Create Niagara Systems
- `create_niagara_emitter(name, path)` — Create Niagara Emitters
- `spawn_niagara_system(system_path, location, parameters)` — Spawn VFX in the editor world
- `create_sound_cue(name, path)` — Create Sound Cue assets
- `spawn_sound(sound_path, location, volume, pitch)` — Play sounds in the editor world

**Collision**
- `create_collision_profile(name, object_type, collision_enabled)` — Add collision profiles
- `create_collision_channel(name, default_response, trace_type)` — Add collision channels

**World Queries**
- `line_trace(start, end, channel)` — Line trace in the editor world
- `find_actors_in_radius(location, radius, class_filter)` — Spatial actor query
- `find_actors_by_tag(tag)` — Find actors by tag
- `get_actor_distance(actor_a, actor_b)` — Measure distance between actors

**Asset Auditing**
- `get_asset_referencers(asset_path)` — List package referencers
- `validate_assets(path)` — Load and report asset validation issues
- `validate_blueprint_graph(blueprint_path)` — Detect orphan nodes and loose pins

**Automation & Testing**
- `create_functional_test(name, location)` — Spawn Functional Test actors
- `run_automation_tests(filter)` — Start editor automation tests

**SaveGame**
- `create_save_game_blueprint(name, path)` — Create SaveGame Blueprints
</details>

<details>
<summary><strong>Material, Level, Asset & Sequencer Tools</strong></summary>

- **material_tools** — Create materials and material instances, configure material properties
- **level_tools** — Open, save, and manage levels and level streaming
- **asset_tools** — List, search, and manage content browser assets
- **sequencer_tools** — Create Sequences, add tracks, control cinematics timeline
</details>

<details>
<summary><strong>Validation & Diagnostics Tools (validation_tools + diagnostics_tools)</strong></summary>

- `find_unused_assets()` — Locate assets with zero references
- `validate_naming_conventions()` — Ensure assets follow standard UE naming conventions
- `get_blueprint_compile_errors()` — Retrieve compile errors from blueprints
- `get_editor_warnings()` — Retrieve the current editor message log warnings
</details>

<details>
<summary><strong>Level Design & Physics Tools (level_design_tools + physics_tools)</strong></summary>

- `snap_actor_to_grid(actor, grid_size)` — Snap actor transform to grid
- `procedural_scatter(mesh, area, density)` — Scatter meshes across a surface
- `create_physics_constraint(actor1, actor2)` — Connect actors via Physics Constraint
- `enable_chaos_destruction(actor)` — Convert static mesh to Geometry Collection
</details>

<details>
<summary><strong>Agent-Friendly & Meta Tools (recipe_tools + policy_tools + diff_tools)</strong></summary>

- `create_enemy_ai_archetype()` — High-level recipe to scaffold a complete AI character
- `create_interactable_archetype()` — High-level recipe to scaffold a door or chest
- `preview_operation()` — Get a JSON diff of what a command will do before applying
- `auto_organize_assets()` — Move assets into standard folders based on their type
</details>

## License
MIT

## Questions

For questions, you can reach me on X/Twitter: [@chongdashu](https://www.x.com/chongdashu)
