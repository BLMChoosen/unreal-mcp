"""
Diff / Preview Tools for Unreal MCP.

Phase 6: Dry-run previews, undo manifests, and operation diffs.
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


def register_diff_tools(mcp: FastMCP):
    """Register diff/preview tools with the MCP server."""

    @mcp.tool()
    def preview_operation(
        ctx: Context,
        operation: str,
        params: dict = None
    ) -> dict:
        """Dry-run an operation to see what it would create or modify.

        Returns a description of changes without actually executing them.

        Args:
            operation: The command name to preview (e.g. 'create_blueprint').
            params: The parameters that would be passed to the command.
        """
        try:
            return _send("preview_operation", {
                "operation": operation,
                "params": params or {}
            })
        except Exception as e:
            logger.error(f"Error previewing operation: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_undo_history(
        ctx: Context,
        count: int = 20
    ) -> dict:
        """Get the editor's undo history.

        Args:
            count: Maximum number of undo entries to return.
        """
        try:
            return _send("get_undo_history", {"count": count})
        except Exception as e:
            logger.error(f"Error getting undo history: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Diff/preview tools registered successfully")
