"""
Deep Blueprint graph authoring tools for Unreal MCP.

These wrappers expose higher-level Blueprint graph editing commands while the
Unreal plugin performs the actual graph mutation and validation.
"""

import logging
from typing import Any, Dict, List, Optional

from mcp.server.fastmcp import Context, FastMCP

logger = logging.getLogger("UnrealMCP")


def _clean(params: Dict[str, Any]) -> Dict[str, Any]:
    return {
        key: value
        for key, value in params.items()
        if key != "ctx" and value is not None
    }


def _send(command: str, params: Dict[str, Any]) -> Dict[str, Any]:
    from unreal_mcp_server import get_unreal_connection

    unreal = get_unreal_connection()
    if not unreal:
        return {"success": False, "message": "Failed to connect to Unreal Engine"}
    response = unreal.send_command(command, _clean(params))
    return response or {"success": False, "message": "No response from Unreal Engine"}


def register_blueprint_deep_graph_tools(mcp: FastMCP):
    """Register deep Blueprint graph tools."""

    @mcp.tool()
    def add_blueprint_control_node(
        ctx: Context,
        blueprint_name: str,
        node_type: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
        num_outputs: int = 2,
        switch_type: str = "",
        cases: Optional[List[str]] = None,
        enum_path: str = "",
    ) -> Dict[str, Any]:
        """Add Branch, Sequence, loop, Gate, DoOnce, or Switch control-flow nodes."""
        return _send("add_blueprint_control_node", locals())

    @mcp.tool()
    def add_blueprint_cast_node(
        ctx: Context,
        blueprint_name: str,
        cast_to_class: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a Cast To node."""
        return _send("add_blueprint_cast_node", locals())

    @mcp.tool()
    def add_blueprint_math_node(
        ctx: Context,
        blueprint_name: str,
        operation: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a Kismet math node by exact function name or alias such as Add, Multiply, or Clamp."""
        return _send("add_blueprint_math_node", locals())

    @mcp.tool()
    def set_variable_default_value(
        ctx: Context,
        blueprint_name: str,
        variable_name: str,
        default_value: str,
    ) -> Dict[str, Any]:
        """Set the default value string for an existing Blueprint variable."""
        return _send("set_variable_default_value", locals())

    @mcp.tool()
    def split_struct_pin(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
        pin_name: str,
    ) -> Dict[str, Any]:
        """Split a struct pin into child pins."""
        return _send("split_struct_pin", locals())

    @mcp.tool()
    def recombine_struct_pin(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
        pin_name: str,
    ) -> Dict[str, Any]:
        """Recombine a split struct pin."""
        return _send("recombine_struct_pin", locals())

    @mcp.tool()
    def add_dynamic_pin_to_node(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
        count: int = 1,
    ) -> Dict[str, Any]:
        """Add one or more dynamic pins to a node that supports Add Pin."""
        return _send("add_dynamic_pin_to_node", locals())

    @mcp.tool()
    def disconnect_blueprint_pin(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
        pin_name: str,
        target_node_id: str = "",
        target_pin: str = "",
    ) -> Dict[str, Any]:
        """Disconnect one pin from a specific target pin, or break all links from it."""
        return _send("disconnect_blueprint_pin", locals())

    @mcp.tool()
    def move_blueprint_node(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
        x: float,
        y: float,
    ) -> Dict[str, Any]:
        """Move a Blueprint node to a graph position."""
        return _send("move_blueprint_node", locals())

    @mcp.tool()
    def format_blueprint_nodes(
        ctx: Context,
        blueprint_name: str,
        node_ids: List[str],
        node_position: Optional[List[float]] = None,
        column_spacing: float = 360,
        row_spacing: float = 180,
    ) -> Dict[str, Any]:
        """Auto-arrange selected Blueprint nodes into a readable layout."""
        return _send("format_blueprint_nodes", locals())

    @mcp.tool()
    def add_function_entry_node(
        ctx: Context,
        blueprint_name: str,
        function_name: str,
        inputs: Optional[List[Dict[str, Any]]] = None,
        outputs: Optional[List[Dict[str, Any]]] = None,
    ) -> Dict[str, Any]:
        """Create or update a function graph signature."""
        return _send("add_function_entry_node", locals())

    @mcp.tool()
    def add_spawn_actor_node(
        ctx: Context,
        blueprint_name: str,
        actor_class: str = "",
        class_path: str = "",
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add SpawnActorFromClass and expose Spawn Transform pins."""
        return _send("add_spawn_actor_node", locals())

    @mcp.tool()
    def create_blueprint_interface(
        ctx: Context,
        interface_name: str,
        path: str = "/Game/Blueprints/Interfaces",
        functions: Optional[List[Dict[str, Any]]] = None,
    ) -> Dict[str, Any]:
        """Create a Blueprint Interface asset and optional function signatures."""
        return _send("create_blueprint_interface", locals())

    @mcp.tool()
    def add_interface_function(
        ctx: Context,
        interface_name: str,
        function_name: str,
        inputs: Optional[List[Dict[str, Any]]] = None,
        outputs: Optional[List[Dict[str, Any]]] = None,
    ) -> Dict[str, Any]:
        """Add a function signature to a Blueprint Interface."""
        return _send("add_interface_function", locals())

    @mcp.tool()
    def implement_interface(
        ctx: Context,
        blueprint_name: str,
        interface_name: str,
    ) -> Dict[str, Any]:
        """Make a Blueprint implement a Blueprint Interface."""
        return _send("implement_interface", locals())

    @mcp.tool()
    def add_interface_message_node(
        ctx: Context,
        blueprint_name: str,
        interface_name: str,
        function_name: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add an interface message call node."""
        return _send("add_interface_message_node", locals())

    @mcp.tool()
    def create_event_dispatcher(
        ctx: Context,
        blueprint_name: str,
        dispatcher_name: str,
        parameters: Optional[List[Dict[str, Any]]] = None,
    ) -> Dict[str, Any]:
        """Create an Event Dispatcher variable on a Blueprint."""
        return _send("create_event_dispatcher", locals())

    @mcp.tool()
    def add_event_dispatcher_node(
        ctx: Context,
        blueprint_name: str,
        dispatcher_name: str,
        action: str = "call",
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add Call, Bind, Unbind, or Clear node for an Event Dispatcher."""
        return _send("add_event_dispatcher_node", locals())

    @mcp.tool()
    def set_variable_container_type(
        ctx: Context,
        blueprint_name: str,
        variable_name: str,
        container_type: str,
    ) -> Dict[str, Any]:
        """Change a Blueprint variable between Single, Array, Set, and Map."""
        return _send("set_variable_container_type", locals())

    @mcp.tool()
    def add_collection_node(
        ctx: Context,
        blueprint_name: str,
        collection_type: str,
        operation: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
        pin_count: int = 0,
    ) -> Dict[str, Any]:
        """Add collection nodes such as Make Array, Add, Clear, Contains, or Find."""
        return _send("add_collection_node", locals())

    @mcp.tool()
    def add_collection_nodes(
        ctx: Context,
        blueprint_name: str,
        collection_type: str,
        operation: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
        pin_count: int = 0,
    ) -> Dict[str, Any]:
        """Alias for add_collection_node."""
        return _send("add_collection_nodes", locals())

    @mcp.tool()
    def export_nodes_as_text(
        ctx: Context,
        blueprint_name: str,
        node_ids: List[str],
    ) -> Dict[str, Any]:
        """Export Blueprint nodes as Unreal clipboard/T3D text."""
        return _send("export_nodes_as_text", locals())

    @mcp.tool()
    def paste_nodes_from_text(
        ctx: Context,
        blueprint_name: str,
        text: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Paste Unreal clipboard/T3D node text into a Blueprint graph."""
        return _send("paste_nodes_from_text", locals())

    @mcp.tool()
    def add_local_variable(
        ctx: Context,
        blueprint_name: str,
        function_name: str,
        variable_name: str,
        variable_type: str,
        default_value: str = "",
    ) -> Dict[str, Any]:
        """Add a local variable to a Blueprint function graph."""
        return _send("add_local_variable", locals())

    @mcp.tool()
    def set_function_flags(
        ctx: Context,
        blueprint_name: str,
        function_name: str,
        pure: Optional[bool] = None,
        call_in_editor: Optional[bool] = None,
    ) -> Dict[str, Any]:
        """Set function flags such as Pure and CallInEditor."""
        return _send("set_function_flags", locals())

    @mcp.tool()
    def add_format_text_node(
        ctx: Context,
        blueprint_name: str,
        format: str,
        arguments: Optional[List[str]] = None,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add Format Text and auto-create argument pins from placeholders."""
        return _send("add_format_text_node", locals())

    @mcp.tool()
    def add_switch_node(
        ctx: Context,
        blueprint_name: str,
        switch_type: str = "String",
        cases: Optional[List[str]] = None,
        enum_path: str = "",
        start_index: int = 0,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add Switch on String, Name, Int, Enum, or Gameplay Tag-like Name nodes."""
        return _send("add_switch_node", locals())

    @mcp.tool()
    def add_async_action_node(
        ctx: Context,
        blueprint_name: str,
        proxy_class: str,
        factory_function: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add an async action node from a proxy class factory function."""
        return _send("add_async_action_node", locals())

    @mcp.tool()
    def add_comment_box(
        ctx: Context,
        blueprint_name: str,
        text: str = "Generated Logic",
        node_ids: Optional[List[str]] = None,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a Blueprint comment box around selected nodes."""
        return _send("add_comment_box", locals())

    @mcp.tool()
    def collapse_to_function(
        ctx: Context,
        blueprint_name: str,
        graph_name: str,
        node_ids: List[str],
        delete_original: bool = False,
    ) -> Dict[str, Any]:
        """Export selected nodes into a new function graph."""
        return _send("collapse_to_function", locals())

    @mcp.tool()
    def collapse_to_macro(
        ctx: Context,
        blueprint_name: str,
        graph_name: str,
        node_ids: List[str],
        delete_original: bool = False,
    ) -> Dict[str, Any]:
        """Export selected nodes into a new macro graph."""
        return _send("collapse_to_macro", locals())

    @mcp.tool()
    def promote_to_variable(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
        pin_name: str,
        variable_name: str = "PromotedValue",
    ) -> Dict[str, Any]:
        """Promote an output pin to a Blueprint variable and connect a Set node."""
        return _send("promote_to_variable", locals())

    @mcp.tool()
    def edit_construction_script(
        ctx: Context,
        blueprint_name: str,
        node_class: str = "",
        node_position: Optional[List[float]] = None,
    ) -> Dict[str, Any]:
        """Target the User Construction Script for graph authoring."""
        return _send("edit_construction_script", locals())

    @mcp.tool()
    def read_graph_logic_flow(
        ctx: Context,
        blueprint_name: str,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Read a graph as a human-friendly execution-flow story."""
        return _send("read_graph_logic_flow", locals())

    @mcp.tool()
    def inspect_node_pins(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
    ) -> Dict[str, Any]:
        """Inspect node pins with types, directions, links, and required/default status."""
        return _send("inspect_node_pins", locals())

    @mcp.tool()
    def add_break_struct_node(
        ctx: Context,
        blueprint_name: str,
        struct_type: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a Break Struct node such as Break Hit Result or Break Transform."""
        return _send("add_break_struct_node", locals())

    @mcp.tool()
    def add_make_struct_node(
        ctx: Context,
        blueprint_name: str,
        struct_type: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a Make Struct node such as Make Transform."""
        return _send("add_make_struct_node", locals())

    @mcp.tool()
    def add_class_reference_node(
        ctx: Context,
        blueprint_name: str,
        class_path: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a literal class/object reference node."""
        return _send("add_class_reference_node", locals())

    @mcp.tool()
    def add_bind_event_node(
        ctx: Context,
        blueprint_name: str,
        dispatcher_name: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a Bind Event node for an Event Dispatcher."""
        return _send("add_bind_event_node", locals())

    @mcp.tool()
    def add_create_event_node(
        ctx: Context,
        blueprint_name: str,
        function_name: str = "",
        event_name: str = "",
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a Create Event delegate node."""
        return _send("add_create_event_node", locals())

    @mcp.tool()
    def add_global_getter(
        ctx: Context,
        blueprint_name: str,
        getter: str,
        actor_class: str = "",
        class_path: str = "",
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add global getters such as Get Player Character or Get Actor Of Class."""
        return _send("add_global_getter", locals())

    @mcp.tool()
    def add_math_expression_node(
        ctx: Context,
        blueprint_name: str,
        expression: str,
        node_position: Optional[List[float]] = None,
        graph_name: str = "",
        graph_type: str = "",
    ) -> Dict[str, Any]:
        """Add a Math Expression node and build input pins from the expression."""
        return _send("add_math_expression_node", locals())

    @mcp.tool()
    def get_specific_node_error(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
    ) -> Dict[str, Any]:
        """Return node-local missing-pin issues after a failed compile/debug loop."""
        return _send("get_specific_node_error", locals())

    @mcp.tool()
    def resolve_wildcard_pin(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
        pin_name: str,
        pin_type: str,
        object_path: str = "",
        container_type: str = "",
    ) -> Dict[str, Any]:
        """Force a wildcard pin to a specific type."""
        return _send("resolve_wildcard_pin", locals())
