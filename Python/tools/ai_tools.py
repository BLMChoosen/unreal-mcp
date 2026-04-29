"""
AI Tools for Unreal MCP.

Create and manipulate AI assets like Behavior Trees and Blackboards,
and manage navigation mesh bounds.
"""

import logging
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def register_ai_tools(mcp: FastMCP):
    """Register AI tools with the MCP server."""

    @mcp.tool()
    def create_behavior_tree(
        ctx: Context,
        name: str,
        path: str = "/Game/AI",
        blackboard_path: str = None
    ) -> dict:
        """
        Create a new Behavior Tree asset.

        Examples:
            create_behavior_tree("BT_Enemy", "/Game/AI")
            create_behavior_tree("BT_Enemy", "/Game/AI", "/Game/AI/BB_Enemy.BB_Enemy")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {"name": name, "path": path}
            if blackboard_path:
                params["blackboard_path"] = blackboard_path

            response = unreal.send_command("create_behavior_tree", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating behavior tree: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_blackboard(
        ctx: Context,
        name: str,
        path: str = "/Game/AI"
    ) -> dict:
        """
        Create a new Blackboard asset.

        Examples:
            create_blackboard("BB_Enemy", "/Game/AI")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("create_blackboard", {
                "path": path,
                "name": name
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating blackboard: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_blackboard_key(
        ctx: Context,
        blackboard_path: str,
        key_name: str,
        key_type: str,
        base_class: str = None,
        enum_path: str = None
    ) -> dict:
        """
        Add a variable key to a Blackboard.

        Supported key_type values: Object, Vector, Bool, Float, Int, Enum, Name, String, Rotator, Class.

        Examples:
            add_blackboard_key("/Game/AI/BB_Enemy.BB_Enemy", "TargetActor", "Object", "/Script/Engine.Actor")
            add_blackboard_key("/Game/AI/BB_Enemy.BB_Enemy", "PatrolPoint", "Vector")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "blackboard": blackboard_path,
                "keyName": key_name,
                "keyType": key_type
            }
            if base_class:
                params["base_class"] = base_class
            if enum_path:
                params["enum_path"] = enum_path

            response = unreal.send_command("add_blackboard_key", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding blackboard key: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_blackboard_keys(
        ctx: Context,
        blackboard_path: str
    ) -> dict:
        """
        List keys in a Blackboard asset.

        Examples:
            get_blackboard_keys("/Game/AI/BB_Enemy.BB_Enemy")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("get_blackboard_keys", {
                "blackboard": blackboard_path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting blackboard keys: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def build_nav_mesh(
        ctx: Context,
        extent_x: float = 1000.0,
        extent_y: float = 1000.0,
        extent_z: float = 500.0,
        location: list = None,
        actor_name: str = None,
        expand_existing: bool = True,
        rebuild: bool = True
    ) -> dict:
        """
        Spawn or resize a NavMeshBoundsVolume and rebuild navigation.

        Examples:
            build_nav_mesh(2000.0, 2000.0, 600.0)
            build_nav_mesh(1500.0, 1500.0, 400.0, location=[0, 0, 0], actor_name="NavMeshBounds")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            if location is None:
                location = [0.0, 0.0, 0.0]

            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "extents": {"X": float(extent_x), "Y": float(extent_y), "Z": float(extent_z)},
                "location": location,
                "expand": expand_existing,
                "rebuild": rebuild
            }
            if actor_name:
                params["name"] = actor_name

            response = unreal.send_command("build_nav_mesh", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error building nav mesh: {e}")
            return {"success": False, "message": str(e)}
