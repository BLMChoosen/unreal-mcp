"""
Diagnostics Tools for Unreal MCP.

Phase 6: Blueprint compile errors, Message Log, Output Log capture,
and editor warning reporting.
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


def register_diagnostics_tools(mcp: FastMCP):
    """Register editor diagnostics tools with the MCP server."""

    @mcp.tool()
    def get_blueprint_compile_errors(
        ctx: Context,
        blueprint_name: str
    ) -> dict:
        """Compile a Blueprint and return any compile errors or warnings.

        Args:
            blueprint_name: Name or path of the Blueprint to compile.
        """
        try:
            return _send("get_blueprint_compile_errors", {
                "blueprint_name": blueprint_name
            })
        except Exception as e:
            logger.error(f"Error getting compile errors: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_message_log(
        ctx: Context,
        category: str = "BlueprintLog",
        max_entries: int = 50
    ) -> dict:
        """Capture entries from the Unreal Message Log.

        Args:
            category: Log category to retrieve (e.g. 'BlueprintLog',
                      'PIE', 'AssetCheck', 'MapCheck', 'LoadErrors').
            max_entries: Maximum number of entries to return.
        """
        try:
            return _send("get_message_log", {
                "category": category,
                "max_entries": max_entries
            })
        except Exception as e:
            logger.error(f"Error getting message log: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_output_log(
        ctx: Context,
        category: str = "",
        severity: str = "",
        max_entries: int = 100
    ) -> dict:
        """Capture recent Output Log entries, optionally filtered.

        Args:
            category: Optional log category filter (e.g. 'LogTemp', 'LogBlueprint').
            severity: Optional severity filter: 'Error', 'Warning', 'Display', 'Log'.
            max_entries: Maximum number of entries to return.
        """
        try:
            return _send("get_output_log", {
                "category": category,
                "severity": severity,
                "max_entries": max_entries
            })
        except Exception as e:
            logger.error(f"Error getting output log: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_editor_warnings(ctx: Context) -> dict:
        """Return current editor warnings relevant to MCP operations.

        Aggregates compile warnings, map check issues, and asset load errors.
        """
        try:
            return _send("get_editor_warnings", {})
        except Exception as e:
            logger.error(f"Error getting editor warnings: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_map_check_errors(ctx: Context) -> dict:
        """Run a Map Check and return all errors and warnings."""
        try:
            return _send("get_map_check_errors", {})
        except Exception as e:
            logger.error(f"Error running map check: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Diagnostics tools registered successfully")
