# Extended Unreal 5 Workflow Tools

These modules add production-oriented Unreal 5 workflows on top of the core actor, Blueprint, asset, and editor tools.

## PCG and Landscape

- `create_pcg_graph(name, path="/Game/PCG")`
- `configure_pcg_graph(graph_path, nodes, edges=[], description="")`
- `create_pcg_volume(name, graph_path="", location=[0,0,0], extent=[2500,2500,1000])`
- `create_biome_from_prompt(prompt, density=0.75, asset_paths=["/Game"], bounds_min, bounds_max)`
- `create_landscape(...)`
- `apply_landscape_material(material_path, actor_name="")`
- `create_road_spline(points, name="MCP_RoadSpline", width=600)`

PCG and MetaSound asset creation use reflection so the MCP plugin still compiles if optional editor plugins are disabled. When a required plugin is not available, the command returns a clear error.

## Animation and Retargeting

- `build_anim_state_machine_spec(states, transitions, blend_spaces)`
- `create_anim_blueprint(name, skeleton_path, path="/Game/Animation")`
- `create_blend_space(name, skeleton_path, axis_spec={})`
- `create_anim_montage_from_sequence(name, sequence_path)`
- `add_anim_notify(animation_path, notify_name, time_seconds, notify_class="")`
- `create_ik_rig(name, skeletal_mesh_path)`
- `create_ik_retargeter(name, source_ik_rig_path, target_ik_rig_path)`

## MetaSound and Dialogue

- `build_metasound_graph_spec(nodes, connections, inputs, outputs)`
- `create_metasound_source(name, path="/Game/Audio/MetaSounds")`
- `configure_metasound_graph(metasound_path, nodes, connections)`
- `import_dialogue_batch(dialogue, destination_path="/Game/Audio/Dialogues")`

Dialogue rows can include `id`, `speaker`, `text`, and `audio_file`.

## Art Direction

- `apply_lighting_preset(preset="night_horror")`
- `apply_art_direction_prompt(prompt)`

The preset command adjusts DirectionalLight, SkyLight, SkyAtmosphere, ExponentialHeightFog, and PostProcessVolume settings.

## Optimization and Profiling

- `audit_optimization(path="/Game", max_texture_megapixels=16)`
- `set_nanite_enabled(enabled=True, asset_paths=[], path="")`
- `generate_hlods()`
- `capture_profile_snapshot(stat_commands=["stat unit", "stat scenerendering", "stat gpu"])`

## Source Control and C++

- `git_status`, `git_diff`, `git_commit_changes`
- `p4_status`, `p4_submit`
- `explain_uasset_conflict(file_path)`
- `create_unreal_cpp_class(class_name, parent_class="Actor")`
- `trigger_live_coding_compile()`

## Multiplayer

- `build_replication_plan(variables, rpcs)`
- `set_blueprint_variable_replication(blueprint_name, variable_name, replication="Replicated")`
- `create_blueprint_rpc_event(blueprint_name, event_name, rpc_type="RunOnServer", reliable=True)`
