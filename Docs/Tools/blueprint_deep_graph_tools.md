# Deep Blueprint Graph Tools

These tools extend Blueprint graph authoring beyond basic node placement and pin links. They are intended for autonomous graph creation, refactoring, debugging, and semantic inspection.

## Control Flow and Core Nodes

- `add_blueprint_control_node(blueprint_name, node_type, ...)` - Add Branch, Sequence, loops, Gate, DoOnce, and Switch nodes.
- `add_blueprint_cast_node(blueprint_name, cast_to_class, ...)` - Add Cast To nodes. Optional auto-wiring params: `next_node_id_on_success` / `next_node_id_on_fail` (and `next_pin_name_on_success` / `next_pin_name_on_fail`) to connect the `then` and `CastFailed` exec pins, and `prev_node_id` / `prev_pin_name` (default `then`) to connect an upstream node's exec output into the cast's `execute` input. Response includes `as_class_pin` (the output object pin name like `AsPlayerController`) and connection flags.
- `add_blueprint_math_node(blueprint_name, operation, ...)` - Add common Kismet math calls with aliases such as Add, Multiply, Clamp, and vector variants.
- `add_spawn_actor_node(blueprint_name, actor_class, ...)` - Add SpawnActorFromClass with Spawn Transform support.
- `add_math_expression_node(blueprint_name, expression, ...)` - Add a Math Expression node from an expression string.

## Variables, Pins, and Data

- `set_variable_default_value(blueprint_name, variable_name, default_value)`
- `set_variable_container_type(blueprint_name, variable_name, container_type)` - Single, Array, Set, or Map.
- `add_blueprint_variable_get_node(...)` / `add_blueprint_variable_set_node(...)`
- `split_struct_pin(blueprint_name, node_id, pin_name)` / `recombine_struct_pin(...)`
- `resolve_wildcard_pin(blueprint_name, node_id, pin_name, pin_type, ...)`
- `add_dynamic_pin_to_node(blueprint_name, node_id, count=1)`
- `add_break_struct_node(blueprint_name, struct_type, ...)` / `add_make_struct_node(...)`
- `add_class_reference_node(blueprint_name, class_path, ...)`
- `add_collection_node(blueprint_name, collection_type, operation, ...)`

## Refactor and Organization

- `delete_blueprint_node(blueprint_name, node_id)`
- `disconnect_blueprint_pin(blueprint_name, node_id, pin_name, ...)`
- `move_blueprint_node(blueprint_name, node_id, x, y)`
- `format_blueprint_nodes(blueprint_name, node_ids, ...)`
- `add_comment_box(blueprint_name, text, node_ids, ...)`
- `collapse_to_function(blueprint_name, graph_name, node_ids, ...)`
- `collapse_to_macro(blueprint_name, graph_name, node_ids, ...)`
- `promote_to_variable(blueprint_name, node_id, pin_name, variable_name)`

## Functions, Interfaces, Delegates, and Events

- `add_custom_event_node(blueprint_name, event_name, inputs, ...)`
- `add_function_entry_node(blueprint_name, function_name, inputs, outputs)`
- `add_local_variable(blueprint_name, function_name, variable_name, variable_type)`
- `set_function_flags(blueprint_name, function_name, pure, call_in_editor)`
- `create_blueprint_interface(interface_name, path, functions)`
- `add_interface_function(interface_name, function_name, inputs, outputs)`
- `implement_interface(blueprint_name, interface_name)`
- `add_interface_message_node(blueprint_name, interface_name, function_name, ...)`
- `create_event_dispatcher(blueprint_name, dispatcher_name, parameters)`
- `add_event_dispatcher_node(blueprint_name, dispatcher_name, action, ...)`
- `add_bind_event_node(blueprint_name, dispatcher_name, ...)`
- `add_create_event_node(blueprint_name, function_name, event_name, ...)`

## Intelligent and Semantic Helpers

- `add_format_text_node(blueprint_name, format, arguments, ...)` - Creates pins from `{Argument}` placeholders.
- `add_switch_node(blueprint_name, switch_type, cases, ...)`
- `add_async_action_node(blueprint_name, proxy_class, factory_function, ...)`
- `add_global_getter(blueprint_name, getter, actor_class, ...)`
- `edit_construction_script(blueprint_name, node_class, ...)`
- `read_graph_logic_flow(blueprint_name, graph_name, graph_type)` - Returns a readable execution-flow story.
- `inspect_node_pins(blueprint_name, node_id)` - Returns pin names, types, direction, defaults, and links.
- `export_nodes_as_text(blueprint_name, node_ids)` / `paste_nodes_from_text(blueprint_name, text, ...)`
- `get_specific_node_error(blueprint_name, node_id)` - Reports node-local missing required input pins.
