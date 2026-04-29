"""
Asset Tools for Unreal MCP.

Browse, duplicate, move, delete, and inspect assets in the Content Browser.
"""

import logging
from typing import Dict, Any, Optional
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def register_asset_tools(mcp: FastMCP):
    """Register Asset tools with the MCP server."""

    @mcp.tool()
    def list_assets(
        ctx: Context,
        path: str = "/Game",
        recursive: bool = True,
        filter_type: Optional[str] = None
    ) -> Dict[str, Any]:
        """List assets under a content path.

        Args:
            path: Root content path (e.g. /Game).
            recursive: Recurse into subfolders.
            filter_type: Optional asset class name (e.g. "Blueprint", "StaticMesh", "Material").
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            params = {"path": path, "recursive": recursive}
            if filter_type:
                params["filter_type"] = filter_type
            response = unreal.send_command("list_assets", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error listing assets: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def duplicate_asset(ctx: Context, source_path: str, dest_path: str) -> Dict[str, Any]:
        """Duplicate an asset to a new path."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("duplicate_asset", {
                "source_path": source_path, "dest_path": dest_path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error duplicating asset: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def move_asset(ctx: Context, source_path: str, dest_path: str) -> Dict[str, Any]:
        """Move (rename) an asset to a new path."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("move_asset", {
                "source_path": source_path, "dest_path": dest_path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error moving asset: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def delete_asset(ctx: Context, asset_path: str) -> Dict[str, Any]:
        """Delete an asset from the Content Browser."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("delete_asset", {"asset_path": asset_path})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error deleting asset: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_asset_info(ctx: Context, asset_path: str) -> Dict[str, Any]:
        """Return metadata about an asset (name, type, package path)."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_asset_info", {"asset_path": asset_path})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting asset info: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Asset tools registered successfully")
