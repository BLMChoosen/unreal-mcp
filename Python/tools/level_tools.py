"""
Level Tools for Unreal MCP.

Save / load / create levels and query the active level.
Includes streaming sub-level management.
"""

import logging
from typing import Dict, Any, List
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def register_level_tools(mcp: FastMCP):
    """Register Level tools with the MCP server."""

    @mcp.tool()
    def save_current_level(ctx: Context) -> Dict[str, Any]:
        """Save the currently open level."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("save_current_level", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error saving level: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def open_level(ctx: Context, level_path: str) -> Dict[str, Any]:
        """Open a level by content path (e.g. /Game/Maps/MyLevel)."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("open_level", {"level_path": level_path})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error opening level: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_current_level_name(ctx: Context) -> Dict[str, Any]:
        """Return the name and path of the currently active level."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_current_level_name", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting current level: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_new_level(ctx: Context, level_name: str, path: str = "/Game/Maps") -> Dict[str, Any]:
        """Create a new empty level at the given path."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_new_level", {"level_name": level_name, "path": path})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating level: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_sublevel(ctx: Context, level_path: str) -> Dict[str, Any]:
        """
        Add a streaming sub-level to the currently open world.

        The level asset must already exist in the project. After adding, the
        persistent level is saved automatically.

        Args:
            level_path: Content path of the sub-level, e.g. /Game/Maps/MySubLevel

        Returns:
            Response with level_path and added flag
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_sublevel", {"level_path": level_path})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding sublevel: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def remove_sublevel(ctx: Context, level_path: str) -> Dict[str, Any]:
        """
        Remove a streaming sub-level from the currently open world.

        Args:
            level_path: Content path of the sub-level to remove,
                        e.g. /Game/Maps/MySubLevel

        Returns:
            Response with level_path and removed flag
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("remove_sublevel", {"level_path": level_path})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error removing sublevel: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_sublevels(ctx: Context) -> Dict[str, Any]:
        """
        List all streaming sub-levels registered in the currently open world.

        Returns:
            Response with a 'sublevels' array. Each entry has:
              - package_name: content path of the sub-level
              - is_loaded: whether the level is currently loaded in memory
              - is_visible: whether the level is currently visible
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_sublevels", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting sublevels: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Level tools registered successfully")
