"""
Optimization, Nanite, HLOD, and profiling tools for Unreal MCP.
"""

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


def register_optimization_tools(mcp: FastMCP):
    """Register optimization and profiling tools with the MCP server."""

    @mcp.tool()
    def audit_optimization(
        ctx: Context,
        path: str = "/Game",
        max_texture_megapixels: float = 16.0,
    ) -> dict:
        """Audit meshes/textures for common optimization issues."""
        try:
            return _send(
                "audit_optimization",
                {"path": path, "max_texture_megapixels": max_texture_megapixels},
            )
        except Exception as e:
            logger.error(f"Error auditing optimization: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_nanite_enabled(
        ctx: Context,
        enabled: bool = True,
        asset_paths: list = None,
        path: str = "",
    ) -> dict:
        """Enable or disable Nanite for Static Mesh assets by path or folder."""
        try:
            return _send(
                "set_nanite_enabled",
                {"enabled": enabled, "asset_paths": asset_paths or [], "path": path},
            )
        except Exception as e:
            logger.error(f"Error setting Nanite: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def generate_hlods(ctx: Context) -> dict:
        """Request HLOD generation in the editor."""
        try:
            return _send("generate_hlods", {})
        except Exception as e:
            logger.error(f"Error generating HLODs: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def capture_profile_snapshot(
        ctx: Context,
        stat_commands: list = None,
    ) -> dict:
        """Toggle profiling stat commands such as stat unit, stat gpu, and stat scenerendering."""
        try:
            return _send(
                "capture_profile_snapshot",
                {"stat_commands": stat_commands or ["stat unit", "stat scenerendering", "stat gpu"]},
            )
        except Exception as e:
            logger.error(f"Error capturing profile snapshot: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Optimization tools registered successfully")
