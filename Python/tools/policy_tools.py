"""
Policy Tools for Unreal MCP.

Phase 6: Naming/folder policy enforcement, batch rename, and auto-organize.
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


def register_policy_tools(mcp: FastMCP):
    """Register naming/folder policy tools with the MCP server."""

    @mcp.tool()
    def validate_naming_policy(
        ctx: Context,
        path: str = "/Game",
        rules: dict = None
    ) -> dict:
        """Check assets against naming convention rules.

        Default rules enforce UE conventions: BP_, BT_, BB_, EQS_, DT_, DA_,
        MI_, M_, T_, S_, WBP_, etc.

        Args:
            path: Content path to scan.
            rules: Optional dict mapping class names to prefix rules,
                   e.g. {"Blueprint": "BP_", "DataTable": "DT_"}.
        """
        try:
            return _send("validate_naming_policy", {
                "path": path,
                "rules": rules or {}
            })
        except Exception as e:
            logger.error(f"Error validating naming policy: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def auto_organize_assets(
        ctx: Context,
        path: str = "/Game",
        dry_run: bool = True,
        rules: dict = None
    ) -> dict:
        """Move assets to correct folders based on type.

        Args:
            path: Content path to scan and organize.
            dry_run: If True, only report what would be moved without acting.
            rules: Optional dict mapping class names to target folder names,
                   e.g. {"Blueprint": "Blueprints", "Material": "Materials"}.
        """
        try:
            return _send("auto_organize_assets", {
                "path": path,
                "dry_run": dry_run,
                "rules": rules or {}
            })
        except Exception as e:
            logger.error(f"Error organizing assets: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def batch_rename_assets(
        ctx: Context,
        path: str,
        search_pattern: str,
        replacement: str,
        asset_type: str = "",
        dry_run: bool = True
    ) -> dict:
        """Rename assets in batch with redirector support.

        Args:
            path: Content path to scan.
            search_pattern: Pattern to search for in asset names.
            replacement: Replacement string.
            asset_type: Optional filter by asset type class name.
            dry_run: If True, only report what would be renamed.
        """
        try:
            return _send("batch_rename_assets", {
                "path": path,
                "search_pattern": search_pattern,
                "replacement": replacement,
                "asset_type": asset_type,
                "dry_run": dry_run
            })
        except Exception as e:
            logger.error(f"Error in batch rename: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def fix_redirectors(
        ctx: Context,
        path: str = "/Game"
    ) -> dict:
        """Fix up all redirectors under a content path.

        Args:
            path: Content path to scan for redirectors.
        """
        try:
            return _send("fix_redirectors", {"path": path})
        except Exception as e:
            logger.error(f"Error fixing redirectors: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Policy tools registered successfully")
