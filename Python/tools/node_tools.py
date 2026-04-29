"""
Blueprint Node Tools for Unreal MCP.

This module provides tools for manipulating Blueprint graph nodes and connections.
"""

import logging
from typing import Dict, List, Any, Optional
from mcp.server.fastmcp import FastMCP, Context

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_blueprint_node_tools(mcp: FastMCP):
    """Register Blueprint node manipulation tools with the MCP server."""
    
    @mcp.tool()
    def add_blueprint_event_node(
        ctx: Context,
        blueprint_name: str,
        event_name: str,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add an event node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            event_name: Name of the event. Use 'Receive' prefix for standard events:
                       - 'ReceiveBeginPlay' for Begin Play
                       - 'ReceiveTick' for Tick
                       - etc.
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle default value within the method body
            if node_position is None:
                node_position = [0, 0]
            
            params = {
                "blueprint_name": blueprint_name,
                "event_name": event_name,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding event node '{event_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_event_node", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Event node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding event node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_blueprint_input_action_node(
        ctx: Context,
        blueprint_name: str,
        action_name: str,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add an input action event node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            action_name: Name of the input action to respond to
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle default value within the method body
            if node_position is None:
                node_position = [0, 0]
            
            params = {
                "blueprint_name": blueprint_name,
                "action_name": action_name,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding input action node for '{action_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_input_action_node", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Input action node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding input action node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_blueprint_function_node(
        ctx: Context,
        blueprint_name: str,
        target: str,
        function_name: str,
        params = None,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add a function call node to a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            target: Target object for the function (component name or self)
            function_name: Name of the function to call
            params: Optional parameters to set on the function node
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle default values within the method body
            if params is None:
                params = {}
            if node_position is None:
                node_position = [0, 0]
            
            command_params = {
                "blueprint_name": blueprint_name,
                "target": target,
                "function_name": function_name,
                "params": params,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding function node '{function_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_function_node", command_params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Function node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding function node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
            
    @mcp.tool()
    def connect_blueprint_nodes(
        ctx: Context,
        blueprint_name: str,
        source_node_id: str,
        source_pin: str,
        target_node_id: str,
        target_pin: str
    ) -> Dict[str, Any]:
        """
        Connect two nodes in a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            source_node_id: ID of the source node
            source_pin: Name of the output pin on the source node
            target_node_id: ID of the target node
            target_pin: Name of the input pin on the target node
            
        Returns:
            Response indicating success or failure
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "source_node_id": source_node_id,
                "source_pin": source_pin,
                "target_node_id": target_node_id,
                "target_pin": target_pin
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Connecting nodes in blueprint '{blueprint_name}'")
            response = unreal.send_command("connect_blueprint_nodes", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Node connection response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error connecting nodes: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_blueprint_variable(
        ctx: Context,
        blueprint_name: str,
        variable_name: str,
        variable_type: str,
        is_exposed: bool = False,
        default_value: Any = None
    ) -> Dict[str, Any]:
        """
        Add a variable to a Blueprint.

        Args:
            blueprint_name: Name of the target Blueprint
            variable_name: Name of the variable
            variable_type: Type of the variable (Boolean, Integer, Float, String, Name,
                           Text, Byte, Vector, Rotator, Transform).
            is_exposed: Whether to expose the variable to the editor.
            default_value: Optional default value (string/number/bool). Stored as a
                           string on FBPVariableDescription.DefaultValue.

        Returns:
            Response indicating success or failure.
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            params = {
                "blueprint_name": blueprint_name,
                "variable_name": variable_name,
                "variable_type": variable_type,
                "is_exposed": is_exposed,
            }
            if default_value is not None:
                params["default_value"] = default_value

            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            logger.info(f"Adding variable '{variable_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_variable", params)

            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}

            logger.info(f"Variable creation response: {response}")
            return response

        except Exception as e:
            error_msg = f"Error adding variable: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def get_blueprint_variable(
        ctx: Context,
        blueprint_name: str,
        variable_name: str
    ) -> Dict[str, Any]:
        """Return metadata about a Blueprint variable (type, default, exposure)."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_blueprint_variable", {
                "blueprint_name": blueprint_name,
                "variable_name": variable_name
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting variable: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_blueprint_variable_get_node(
        ctx: Context,
        blueprint_name: str,
        variable_name: str,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """Place a 'Get Variable' node in the Blueprint event graph."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_blueprint_variable_get_node", {
                "blueprint_name": blueprint_name,
                "variable_name": variable_name,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding variable get node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_blueprint_variable_set_node(
        ctx: Context,
        blueprint_name: str,
        variable_name: str,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """Place a 'Set Variable' node in the Blueprint event graph."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_blueprint_variable_set_node", {
                "blueprint_name": blueprint_name,
                "variable_name": variable_name,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding variable set node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_branch_node(
        ctx: Context,
        blueprint_name: str,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """Place a Branch (UK2Node_IfThenElse) node in the Blueprint event graph."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_branch_node", {
                "blueprint_name": blueprint_name,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding branch node: {e}")
            return {"success": False, "message": str(e)}
    
    @mcp.tool()
    def add_blueprint_get_self_component_reference(
        ctx: Context,
        blueprint_name: str,
        component_name: str,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add a node that gets a reference to a component owned by the current Blueprint.
        This creates a node similar to what you get when dragging a component from the Components panel.
        
        Args:
            blueprint_name: Name of the target Blueprint
            component_name: Name of the component to get a reference to
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            # Handle None case explicitly in the function
            if node_position is None:
                node_position = [0, 0]
            
            params = {
                "blueprint_name": blueprint_name,
                "component_name": component_name,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding self component reference node for '{component_name}' to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_get_self_component_reference", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Self component reference node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding self component reference node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_blueprint_self_reference(
        ctx: Context,
        blueprint_name: str,
        node_position = None
    ) -> Dict[str, Any]:
        """
        Add a 'Get Self' node to a Blueprint's event graph that returns a reference to this actor.
        
        Args:
            blueprint_name: Name of the target Blueprint
            node_position: Optional [X, Y] position in the graph
            
        Returns:
            Response containing the node ID and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            if node_position is None:
                node_position = [0, 0]
                
            params = {
                "blueprint_name": blueprint_name,
                "node_position": node_position
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Adding self reference node to blueprint '{blueprint_name}'")
            response = unreal.send_command("add_blueprint_self_reference", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Self reference node creation response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error adding self reference node: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def find_blueprint_nodes(
        ctx: Context,
        blueprint_name: str,
        node_type = None,
        event_type = None
    ) -> Dict[str, Any]:
        """
        Find nodes in a Blueprint's event graph.
        
        Args:
            blueprint_name: Name of the target Blueprint
            node_type: Optional type of node to find (Event, Function, Variable, etc.)
            event_type: Optional specific event type to find (BeginPlay, Tick, etc.)
            
        Returns:
            Response containing array of found node IDs and success status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            params = {
                "blueprint_name": blueprint_name,
                "node_type": node_type,
                "event_type": event_type
            }
            
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            logger.info(f"Finding nodes in blueprint '{blueprint_name}'")
            response = unreal.send_command("find_blueprint_nodes", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Node find response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error finding nodes: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def add_math_node(
        ctx: Context,
        blueprint_name: str,
        operation: str,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """
        Add a KismetMathLibrary function node to a Blueprint event graph.

        Args:
            blueprint_name: Name of the target Blueprint
            operation: Exact UKismetMathLibrary function name, e.g.:
                       Add_FloatFloat, Subtract_FloatFloat, Multiply_FloatFloat,
                       Divide_FloatFloat, Add_IntInt, Subtract_IntInt,
                       Multiply_IntInt, Divide_IntInt, Abs, Abs_Int, Sqrt, Square,
                       Lerp, FClamp, Clamp, FMin, FMax, Min, Max, Sin, Cos, Tan,
                       VSize, VInterpTo, etc.
            node_position: Optional [X, Y] position in the graph

        Returns:
            Response containing node_id and the resolved operation name
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_math_node", {
                "blueprint_name": blueprint_name,
                "operation": operation,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding math node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_cast_node(
        ctx: Context,
        blueprint_name: str,
        cast_to_class: str,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """
        Add a 'Cast To <Class>' node to a Blueprint event graph.

        Args:
            blueprint_name: Name of the target Blueprint
            cast_to_class: Class name to cast to, e.g. 'ACharacter', 'APawn',
                           'AStaticMeshActor'. Use the C++ class name with prefix.
            node_position: Optional [X, Y] position in the graph

        Returns:
            Response containing node_id and cast_to_class
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_cast_node", {
                "blueprint_name": blueprint_name,
                "cast_to_class": cast_to_class,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding cast node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_sequence_node(
        ctx: Context,
        blueprint_name: str,
        num_outputs: int = 2,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """
        Add a Sequence node that splits one exec pin into N sequential outputs.

        Args:
            blueprint_name: Name of the target Blueprint
            num_outputs: Number of output execution pins (2-16, default 2)
            node_position: Optional [X, Y] position in the graph

        Returns:
            Response containing node_id and num_outputs
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_sequence_node", {
                "blueprint_name": blueprint_name,
                "num_outputs": num_outputs,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding sequence node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_foreach_loop_node(
        ctx: Context,
        blueprint_name: str,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """
        Add a 'For Each Loop' macro node to a Blueprint event graph.

        Pins: Array (input), Loop Body (exec out), Array Element (out),
              Array Index (out), Completed (exec out).

        Args:
            blueprint_name: Name of the target Blueprint
            node_position: Optional [X, Y] position in the graph

        Returns:
            Response containing node_id
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_foreach_loop_node", {
                "blueprint_name": blueprint_name,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding foreach loop node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_while_loop_node(
        ctx: Context,
        blueprint_name: str,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """
        Add a 'While Loop' macro node to a Blueprint event graph.

        Pins: Condition (bool input), Loop Body (exec out), Completed (exec out).

        Args:
            blueprint_name: Name of the target Blueprint
            node_position: Optional [X, Y] position in the graph

        Returns:
            Response containing node_id
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_while_loop_node", {
                "blueprint_name": blueprint_name,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding while loop node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_timeline_node(
        ctx: Context,
        blueprint_name: str,
        timeline_name: str,
        node_position: List[float] = [0.0, 0.0]
    ) -> Dict[str, Any]:
        """
        Add a Timeline node to a Blueprint event graph.

        The Timeline template is created inside the Blueprint. Use the Unreal
        editor to add tracks (float, vector, event) to the timeline after creation.

        Args:
            blueprint_name: Name of the target Blueprint
            timeline_name: Unique name for the timeline within the Blueprint
            node_position: Optional [X, Y] position in the graph

        Returns:
            Response containing node_id and timeline_name
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_timeline_node", {
                "blueprint_name": blueprint_name,
                "timeline_name": timeline_name,
                "node_position": node_position
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding timeline node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_timeline_float_track(
        ctx: Context,
        blueprint_name: str,
        timeline_name: str,
        track_name: str
    ) -> Dict[str, Any]:
        """
        Add a float track to an existing Timeline node in a Blueprint.

        Args:
            blueprint_name: Name of the target Blueprint (e.g. 'BP_MyActor')
            timeline_name: Unique name of the timeline within the Blueprint (e.g. 'MyTimeline')
            track_name: Name of the float track to create (e.g. 'Alpha')

        Returns:
            Response containing success status
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            response = unreal.send_command("add_timeline_float_track", {
                "blueprint_name": blueprint_name,
                "timeline_name": timeline_name,
                "track_name": track_name
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding timeline float track: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_timeline_keyframe(
        ctx: Context,
        blueprint_name: str,
        timeline_name: str,
        track_name: str,
        time: float,
        value: float
    ) -> Dict[str, Any]:
        """
        Add a keyframe to a timeline float track.

        Args:
            blueprint_name: Name of the target Blueprint (e.g. 'BP_MyActor')
            timeline_name: Name of the timeline (e.g. 'MyTimeline')
            track_name: Name of the float track (e.g. 'Alpha')
            time: Time of the keyframe in seconds (e.g. 0.0)
            value: Value of the keyframe at the given time (e.g. 1.0)

        Returns:
            Response containing success status
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            response = unreal.send_command("add_timeline_keyframe", {
                "blueprint_name": blueprint_name,
                "timeline_name": timeline_name,
                "track_name": track_name,
                "time": time,
                "value": value
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding timeline keyframe: {e}")
            return {"success": False, "message": str(e)}

    # ===== Phase 2: Extended Blueprint Graph Authoring =====

    @mcp.tool()
    def create_blueprint_function(
        ctx: Context,
        blueprint_name: str,
        function_name: str,
        inputs: list = None,
        outputs: list = None
    ) -> Dict[str, Any]:
        """Create a new function graph in a Blueprint.

        Args:
            blueprint_name: Name of the target Blueprint.
            function_name: Name for the new function.
            inputs: Optional list of dicts with 'name' and 'type' for input params.
            outputs: Optional list of dicts with 'name' and 'type' for output params.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_blueprint_function", {
                "blueprint_name": blueprint_name,
                "function_name": function_name,
                "inputs": inputs or [],
                "outputs": outputs or []
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating function: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_blueprint_macro(
        ctx: Context,
        blueprint_name: str,
        macro_name: str,
        inputs: list = None,
        outputs: list = None
    ) -> Dict[str, Any]:
        """Create a new macro graph in a Blueprint.

        Args:
            blueprint_name: Name of the target Blueprint.
            macro_name: Name for the new macro.
            inputs: Optional list of dicts with 'name' and 'type' for input params.
            outputs: Optional list of dicts with 'name' and 'type' for output params.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_blueprint_macro", {
                "blueprint_name": blueprint_name,
                "macro_name": macro_name,
                "inputs": inputs or [],
                "outputs": outputs or []
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating macro: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_blueprint_custom_event_graph(
        ctx: Context,
        blueprint_name: str,
        graph_name: str
    ) -> Dict[str, Any]:
        """Create a custom event graph in a Blueprint.

        Args:
            blueprint_name: Name of the target Blueprint.
            graph_name: Name for the new event graph.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_blueprint_custom_event_graph", {
                "blueprint_name": blueprint_name,
                "graph_name": graph_name
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating event graph: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_node_by_class(
        ctx: Context,
        blueprint_name: str,
        node_class: str,
        node_position: List[float] = None
    ) -> Dict[str, Any]:
        """Add any K2Node by its class name to a Blueprint event graph.

        Args:
            blueprint_name: Name of the target Blueprint.
            node_class: Full or short class name of the node (e.g. 'K2Node_IfThenElse',
                        'K2Node_SpawnActorFromClass', 'K2Node_Delay').
            node_position: Optional [X, Y] position in the graph.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_node_by_class", {
                "blueprint_name": blueprint_name,
                "node_class": node_class,
                "node_position": node_position or [0, 0]
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding node by class: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def connect_pins_validated(
        ctx: Context,
        blueprint_name: str,
        source_node_id: str,
        source_pin: str,
        target_node_id: str,
        target_pin: str
    ) -> Dict[str, Any]:
        """Connect two pins with type validation, returning detailed errors on mismatch.

        Args:
            blueprint_name: Name of the target Blueprint.
            source_node_id: ID of the source node.
            source_pin: Name of the output pin.
            target_node_id: ID of the target node.
            target_pin: Name of the input pin.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("connect_pins_validated", {
                "blueprint_name": blueprint_name,
                "source_node_id": source_node_id,
                "source_pin": source_pin,
                "target_node_id": target_node_id,
                "target_pin": target_pin
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error connecting validated pins: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_pin_default_value(
        ctx: Context,
        blueprint_name: str,
        node_id: str,
        pin_name: str,
        default_value: str
    ) -> Dict[str, Any]:
        """Set the default value on a node's pin.

        Args:
            blueprint_name: Name of the target Blueprint.
            node_id: ID of the node containing the pin.
            pin_name: Name of the pin to set.
            default_value: Default value as a string representation.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("set_pin_default_value", {
                "blueprint_name": blueprint_name,
                "node_id": node_id,
                "pin_name": pin_name,
                "default_value": default_value
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting pin default: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_custom_event_node(
        ctx: Context,
        blueprint_name: str,
        event_name: str,
        parameters: list = None,
        node_position: List[float] = None
    ) -> Dict[str, Any]:
        """Add a Custom Event node with a configurable parameter list.

        Args:
            blueprint_name: Name of the target Blueprint.
            event_name: Name for the custom event.
            parameters: Optional list of dicts with 'name' and 'type' for event params.
            node_position: Optional [X, Y] position.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_custom_event_node", {
                "blueprint_name": blueprint_name,
                "event_name": event_name,
                "parameters": parameters or [],
                "node_position": node_position or [0, 0]
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding custom event: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def list_blueprint_functions(
        ctx: Context,
        blueprint_name: str
    ) -> Dict[str, Any]:
        """List all function and macro graphs in a Blueprint.

        Args:
            blueprint_name: Name of the target Blueprint.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("list_blueprint_functions", {
                "blueprint_name": blueprint_name
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error listing functions: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_node_pins(
        ctx: Context,
        blueprint_name: str,
        node_id: str
    ) -> Dict[str, Any]:
        """Return all pin names, types, and directions for a node.

        Args:
            blueprint_name: Name of the target Blueprint.
            node_id: ID of the node to inspect.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_node_pins", {
                "blueprint_name": blueprint_name,
                "node_id": node_id
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting node pins: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def delete_blueprint_node(
        ctx: Context,
        blueprint_name: str,
        node_id: str
    ) -> Dict[str, Any]:
        """Remove a node from a Blueprint graph.

        Args:
            blueprint_name: Name of the target Blueprint.
            node_id: ID of the node to remove.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("delete_blueprint_node", {
                "blueprint_name": blueprint_name,
                "node_id": node_id
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error deleting node: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Blueprint node tools registered successfully")