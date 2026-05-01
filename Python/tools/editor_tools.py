"""
Editor Tools for Unreal MCP.

This module provides tools for controlling the Unreal Editor viewport and other editor functionality.
"""

import logging
from typing import Dict, List, Any, Optional
from mcp.server.fastmcp import FastMCP, Context

# Get logger
logger = logging.getLogger("UnrealMCP")

def register_editor_tools(mcp: FastMCP):
    """Register editor tools with the MCP server."""
    
    @mcp.tool()
    def get_actors_in_level(ctx: Context) -> List[Dict[str, Any]]:
        """Get a list of all actors in the current level."""
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.warning("Failed to connect to Unreal Engine")
                return []
                
            response = unreal.send_command("get_actors_in_level", {})
            
            if not response:
                logger.warning("No response from Unreal Engine")
                return []
                
            # Log the complete response for debugging
            logger.info(f"Complete response from Unreal: {response}")
            
            # Check response format
            if "result" in response and "actors" in response["result"]:
                actors = response["result"]["actors"]
                logger.info(f"Found {len(actors)} actors in level")
                return actors
            elif "actors" in response:
                actors = response["actors"]
                logger.info(f"Found {len(actors)} actors in level")
                return actors
                
            logger.warning(f"Unexpected response format: {response}")
            return []
            
        except Exception as e:
            logger.error(f"Error getting actors: {e}")
            return []

    @mcp.tool()
    def find_actors_by_name(ctx: Context, pattern: str) -> List[Dict[str, Any]]:
        """Find actors by name pattern."""
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.warning("Failed to connect to Unreal Engine")
                return []
                
            response = unreal.send_command("find_actors_by_name", {
                "pattern": pattern
            })
            
            if not response:
                return []
                
            return response.get("actors", [])
            
        except Exception as e:
            logger.error(f"Error finding actors: {e}")
            return []
    
    @mcp.tool()
    def spawn_actor(
        ctx: Context,
        name: str,
        type: str,
        location: List[float] = [0.0, 0.0, 0.0],
        rotation: List[float] = [0.0, 0.0, 0.0]
    ) -> Dict[str, Any]:
        """Create a new actor in the current level.
        
        Args:
            ctx: The MCP context
            name: The name to give the new actor (must be unique)
            type: The type of actor to create (e.g. StaticMeshActor, PointLight)
            location: The [x, y, z] world location to spawn at
            rotation: The [pitch, yaw, roll] rotation in degrees
            
        Returns:
            Dict containing the created actor's properties
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            # Ensure all parameters are properly formatted
            params = {
                "name": name,
                "type": type.upper(),  # Make sure type is uppercase
                "location": location,
                "rotation": rotation
            }
            
            # Validate location and rotation formats
            for param_name in ["location", "rotation"]:
                param_value = params[param_name]
                if not isinstance(param_value, list) or len(param_value) != 3:
                    logger.error(f"Invalid {param_name} format: {param_value}. Must be a list of 3 float values.")
                    return {"success": False, "message": f"Invalid {param_name} format. Must be a list of 3 float values."}
                # Ensure all values are float
                params[param_name] = [float(val) for val in param_value]
            
            logger.info(f"Creating actor '{name}' of type '{type}' with params: {params}")
            response = unreal.send_command("spawn_actor", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            # Log the complete response for debugging
            logger.info(f"Actor creation response: {response}")
            
            # Handle error responses correctly
            if response.get("status") == "error":
                error_message = response.get("error", "Unknown error")
                logger.error(f"Error creating actor: {error_message}")
                return {"success": False, "message": error_message}
            
            return response
            
        except Exception as e:
            error_msg = f"Error creating actor: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}
    
    @mcp.tool()
    def delete_actor(ctx: Context, name: str) -> Dict[str, Any]:
        """Delete an actor by name."""
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            response = unreal.send_command("delete_actor", {
                "name": name
            })
            return response or {}
            
        except Exception as e:
            logger.error(f"Error deleting actor: {e}")
            return {}
    
    @mcp.tool()
    def set_actor_transform(
        ctx: Context,
        name: str,
        location: List[float]  = None,
        rotation: List[float]  = None,
        scale: List[float] = None
    ) -> Dict[str, Any]:
        """Set the transform of an actor."""
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            params = {"name": name}
            if location is not None:
                params["location"] = location
            if rotation is not None:
                params["rotation"] = rotation
            if scale is not None:
                params["scale"] = scale
                
            response = unreal.send_command("set_actor_transform", params)
            return response or {}
            
        except Exception as e:
            logger.error(f"Error setting transform: {e}")
            return {}
    
    @mcp.tool()
    def get_actor_properties(ctx: Context, name: str) -> Dict[str, Any]:
        """Get all properties of an actor."""
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            response = unreal.send_command("get_actor_properties", {
                "name": name
            })
            return response or {}
            
        except Exception as e:
            logger.error(f"Error getting properties: {e}")
            return {}

    @mcp.tool()
    def set_actor_property(
        ctx: Context,
        name: str,
        property_name: str,
        property_value,
    ) -> Dict[str, Any]:
        """
        Set a property on an actor.
        
        Args:
            name: Name of the actor
            property_name: Name of the property to set
            property_value: Value to set the property to
            
        Returns:
            Dict containing response from Unreal with operation status
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
                
            response = unreal.send_command("set_actor_property", {
                "name": name,
                "property_name": property_name,
                "property_value": property_value
            })
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Set actor property response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error setting actor property: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    @mcp.tool()
    def focus_viewport(
        ctx: Context,
        target: str = None,
        location: List[float] = None,
        distance: float = 1000.0,
        orientation: List[float] = None
    ) -> Dict[str, Any]:
        """
        Focus the viewport on a specific actor or location.

        Args:
            target: Name of the actor to focus on (if provided, location is ignored)
            location: [X, Y, Z] coordinates to focus on (used if target is None)
            distance: Distance from the target/location
            orientation: Optional [Pitch, Yaw, Roll] for the viewport camera

        Returns:
            Response from Unreal Engine
        """
        from unreal_mcp_server import get_unreal_connection

        if target is None and location is None:
            return {"success": False, "message": "Either 'target' or 'location' must be provided"}

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {}
            if target:
                params["target"] = target
            elif location:
                params["location"] = location

            if distance:
                params["distance"] = distance

            if orientation:
                params["orientation"] = orientation

            response = unreal.send_command("focus_viewport", params)
            return response or {}

        except Exception as e:
            logger.error(f"Error focusing viewport: {e}")
            return {"status": "error", "message": str(e)}

    @mcp.tool()
    def take_screenshot(
        ctx: Context,
        filepath: str
    ) -> Dict[str, Any]:
        """
        Capture a screenshot of the active editor viewport and save to disk as PNG.

        Args:
            filepath: Absolute path to write the PNG file. ".png" is appended if missing.

        Returns:
            Dict with the saved filepath on success, or an error message.
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {"filepath": filepath}
            response = unreal.send_command("take_screenshot", params)
            return response or {}

        except Exception as e:
            logger.error(f"Error taking screenshot: {e}")
            return {"status": "error", "message": str(e)}

    @mcp.tool()
    def spawn_blueprint_actor(
        ctx: Context,
        blueprint_name: str,
        actor_name: str,
        location: List[float] = [0.0, 0.0, 0.0],
        rotation: List[float] = [0.0, 0.0, 0.0]
    ) -> Dict[str, Any]:
        """Spawn an actor from a Blueprint.
        
        Args:
            ctx: The MCP context
            blueprint_name: Name of the Blueprint to spawn from
            actor_name: Name to give the spawned actor
            location: The [x, y, z] world location to spawn at
            rotation: The [pitch, yaw, roll] rotation in degrees
            
        Returns:
            Dict containing the spawned actor's properties
        """
        from unreal_mcp_server import get_unreal_connection
        
        try:
            unreal = get_unreal_connection()
            if not unreal:
                logger.error("Failed to connect to Unreal Engine")
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            
            # Ensure all parameters are properly formatted
            params = {
                "blueprint_name": blueprint_name,
                "actor_name": actor_name,
                "location": location or [0.0, 0.0, 0.0],
                "rotation": rotation or [0.0, 0.0, 0.0]
            }
            
            # Validate location and rotation formats
            for param_name in ["location", "rotation"]:
                param_value = params[param_name]
                if not isinstance(param_value, list) or len(param_value) != 3:
                    logger.error(f"Invalid {param_name} format: {param_value}. Must be a list of 3 float values.")
                    return {"success": False, "message": f"Invalid {param_name} format. Must be a list of 3 float values."}
                # Ensure all values are float
                params[param_name] = [float(val) for val in param_value]
            
            logger.info(f"Spawning blueprint actor with params: {params}")
            response = unreal.send_command("spawn_blueprint_actor", params)
            
            if not response:
                logger.error("No response from Unreal Engine")
                return {"success": False, "message": "No response from Unreal Engine"}
            
            logger.info(f"Spawn blueprint actor response: {response}")
            return response
            
        except Exception as e:
            error_msg = f"Error spawning blueprint actor: {e}"
            logger.error(error_msg)
            return {"success": False, "message": error_msg}

    # ===== Play In Editor =====

    @mcp.tool()
    def start_play_in_editor(
        ctx: Context,
        mobile_preview: bool = False,
        simulate: bool = False
    ) -> Dict[str, Any]:
        """Start a Play In Editor session.

        Args:
            mobile_preview: If True, starts in mobile preview mode.
            simulate: If True, runs Simulate-In-Editor instead of full PIE.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("start_play_in_editor", {
                "mobile_preview": mobile_preview,
                "simulate": simulate
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error starting PIE: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def stop_play_in_editor(ctx: Context) -> Dict[str, Any]:
        """Stop any running Play In Editor session."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("stop_play_in_editor", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error stopping PIE: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_play_in_editor_status(ctx: Context) -> Dict[str, Any]:
        """Return whether a PIE session is currently running."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_play_in_editor_status", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting PIE status: {e}")
            return {"success": False, "message": str(e)}

    # ===== Batch actor operations =====

    @mcp.tool()
    def get_actors_by_tag(ctx: Context, tag: str) -> Dict[str, Any]:
        """Return all actors that have a given tag."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_actors_by_tag", {"tag": tag})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting actors by tag: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_actors_transform_by_tag(
        ctx: Context,
        tag: str,
        location: List[float] = None,
        rotation: List[float] = None,
        scale: List[float] = None,
        relative: bool = False
    ) -> Dict[str, Any]:
        """Apply a transform to every actor with the given tag.

        Args:
            location: [X, Y, Z] (optional).
            rotation: [Pitch, Yaw, Roll] (optional).
            scale: [X, Y, Z] (optional).
            relative: If True, add to current transform instead of overwriting.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            params: Dict[str, Any] = {"tag": tag, "relative": relative}
            if location is not None: params["location"] = location
            if rotation is not None: params["rotation"] = rotation
            if scale is not None:    params["scale"] = scale
            response = unreal.send_command("set_actors_transform_by_tag", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting transforms by tag: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_tag_to_actors_by_name(ctx: Context, pattern: str, tag: str) -> Dict[str, Any]:
        """Add a tag to every actor whose name contains the given pattern."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_tag_to_actors_by_name", {
                "pattern": pattern, "tag": tag
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding tag: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_actor_property_batch(
        ctx: Context,
        actor_names: List[str],
        property_name: str,
        property_value: Any
    ) -> Dict[str, Any]:
        """Set a property on a batch of actors. Returns per-actor success/error."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("set_actor_property_batch", {
                "actor_names": actor_names,
                "property_name": property_name,
                "property_value": property_value
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error in batch property set: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def select_actors_by_tag(
        ctx: Context,
        tag: str,
        add_to_selection: bool = False
    ) -> Dict[str, Any]:
        """Select all actors with a given tag in the editor.

        Args:
            add_to_selection: If True, add to existing selection instead of replacing.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("select_actors_by_tag", {
                "tag": tag, "add_to_selection": add_to_selection
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error selecting by tag: {e}")
            return {"success": False, "message": str(e)}

    # ===== Viewport display mode =====

    @mcp.tool()
    def set_viewport_display_mode(ctx: Context, mode: str) -> Dict[str, Any]:
        """Set the active viewport's display mode.

        Args:
            mode: One of "Lit", "Unlit", "Wireframe", "DetailLighting",
                  "LightingOnly", "ReflectionsOnly", "PathTracing".
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("set_viewport_display_mode", {"mode": mode})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting display mode: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_viewport_display_mode(ctx: Context) -> Dict[str, Any]:
        """Get the active viewport's current display mode."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_viewport_display_mode", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting display mode: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_viewport_show_flags(
        ctx: Context,
        flag: str,
        enabled: bool = True
    ) -> Dict[str, Any]:
        """Toggle a viewport show flag.

        Args:
            flag: One of "Grid", "Stats", "Bounds", "Collision", "Navigation", "Bloom".
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("set_viewport_show_flags", {
                "flag": flag, "enabled": enabled
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting show flag: {e}")
            return {"success": False, "message": str(e)}

    # ===== Phase 1: Core Editor Enhancements =====

    @mcp.tool()
    def undo_last_action(ctx: Context) -> Dict[str, Any]:
        """Undo the last editor operation performed by the AI."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("undo_last_action", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error undoing action: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def redo_last_action(ctx: Context) -> Dict[str, Any]:
        """Redo the last undone editor operation."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("redo_last_action", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error redoing action: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def bulk_set_actor_transform(
        ctx: Context,
        actor_names: List[str],
        location: List[float] = None,
        rotation: List[float] = None,
        scale: List[float] = None,
        relative: bool = False
    ) -> Dict[str, Any]:
        """Set transform on multiple actors at once.

        Args:
            actor_names: List of actor names to transform.
            location: Optional [X, Y, Z] position.
            rotation: Optional [Pitch, Yaw, Roll] rotation.
            scale: Optional [X, Y, Z] scale.
            relative: If True, add to current transform instead of overwriting.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            params: Dict[str, Any] = {"actor_names": actor_names, "relative": relative}
            if location is not None: params["location"] = location
            if rotation is not None: params["rotation"] = rotation
            if scale is not None:    params["scale"] = scale
            response = unreal.send_command("bulk_set_actor_transform", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error in bulk transform: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def export_level_to_json(
        ctx: Context,
        include_transforms: bool = True,
        include_properties: bool = False,
        include_components: bool = False
    ) -> Dict[str, Any]:
        """Serialize the entire level state to JSON for analysis or backup.

        Args:
            include_transforms: Include actor transforms in the output.
            include_properties: Include detailed actor properties.
            include_components: Include actor component hierarchy.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("export_level_to_json", {
                "include_transforms": include_transforms,
                "include_properties": include_properties,
                "include_components": include_components
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error exporting level: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def duplicate_actor(
        ctx: Context,
        name: str,
        new_name: str = "",
        offset: List[float] = None
    ) -> Dict[str, Any]:
        """Duplicate an actor with a configurable offset.

        Args:
            name: Name of the actor to duplicate.
            new_name: Optional name for the duplicate. Auto-generated if empty.
            offset: Optional [X, Y, Z] offset from the original actor's position.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            params: Dict[str, Any] = {"name": name}
            if new_name:
                params["new_name"] = new_name
            if offset is not None:
                params["offset"] = [float(v) for v in offset]
            response = unreal.send_command("duplicate_actor", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error duplicating actor: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_actor_material(
        ctx: Context,
        name: str,
        material_path: str,
        slot_index: int = 0
    ) -> Dict[str, Any]:
        """Apply a material to an actor directly without opening Blueprint.

        Args:
            name: Name of the target actor.
            material_path: Content path to the material or material instance.
            slot_index: Material slot index (0-based).
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("set_actor_material", {
                "name": name,
                "material_path": material_path,
                "slot_index": slot_index
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting actor material: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Editor tools registered successfully")
