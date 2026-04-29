"""
Validation Tools for Unreal MCP.

Phase 4: Asset validation, naming conventions, broken references,
unused asset detection, and automation test result polling.
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


def register_validation_tools(mcp: FastMCP):
    """Register asset validation and automation tools with the MCP server."""

    @mcp.tool()
    def find_unused_assets(
        ctx: Context,
        path: str = "/Game",
        ignore_patterns: list = None
    ) -> dict:
        """Detect assets that have no referencers (potentially unused).

        Args:
            path: Content path to scan.
            ignore_patterns: Optional list of path patterns to ignore.
        """
        try:
            return _send("find_unused_assets", {
                "path": path,
                "ignore_patterns": ignore_patterns or []
            })
        except Exception as e:
            logger.error(f"Error finding unused assets: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def find_broken_references(
        ctx: Context,
        path: str = "/Game"
    ) -> dict:
        """Detect broken asset references under a content path.

        Args:
            path: Content path to scan for broken references.
        """
        try:
            return _send("find_broken_references", {"path": path})
        except Exception as e:
            logger.error(f"Error finding broken references: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def validate_naming_conventions(
        ctx: Context,
        path: str = "/Game",
        rules: dict = None
    ) -> dict:
        """Validate asset naming conventions by type.

        Args:
            path: Content path to scan.
            rules: Optional dict mapping asset types to expected prefixes,
                   e.g. {"Blueprint": "BP_", "BehaviorTree": "BT_"}.
                   Uses UE defaults if not provided.
        """
        try:
            return _send("validate_naming_conventions", {
                "path": path,
                "rules": rules or {}
            })
        except Exception as e:
            logger.error(f"Error validating naming: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def validate_data_table_against_schema(
        ctx: Context,
        data_table_path: str,
        expected_fields: list = None
    ) -> dict:
        """Deep-validate a Data Table against an expected schema.

        Args:
            data_table_path: Content path of the Data Table.
            expected_fields: Optional list of dicts with 'name', 'type',
                             'required' for each expected field.
        """
        try:
            return _send("validate_data_table_against_schema", {
                "data_table": data_table_path,
                "expected_fields": expected_fields or []
            })
        except Exception as e:
            logger.error(f"Error validating data table schema: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def generate_validation_report(
        ctx: Context,
        path: str = "/Game",
        check_naming: bool = True,
        check_references: bool = True,
        check_unused: bool = True,
        naming_rules: dict = None
    ) -> dict:
        """Generate a comprehensive validation report with severity and suggestions.

        Args:
            path: Content path to scan.
            check_naming: Include naming convention checks.
            check_references: Include broken reference checks.
            check_unused: Include unused asset detection.
            naming_rules: Optional naming convention rules.
        """
        try:
            return _send("generate_validation_report", {
                "path": path,
                "check_naming": check_naming,
                "check_references": check_references,
                "check_unused": check_unused,
                "naming_rules": naming_rules or {}
            })
        except Exception as e:
            logger.error(f"Error generating validation report: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_automation_test_results(
        ctx: Context,
        filter: str = "Project"
    ) -> dict:
        """Poll automation test results including status, failures, and logs.

        Call this after run_automation_tests() to retrieve results.

        Args:
            filter: Test filter that was used to run the tests.
        """
        try:
            return _send("get_automation_test_results", {"filter": filter})
        except Exception as e:
            logger.error(f"Error getting test results: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_automation_test_list(ctx: Context) -> dict:
        """List all available automation tests in the editor."""
        try:
            return _send("get_automation_test_list", {})
        except Exception as e:
            logger.error(f"Error listing tests: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Validation tools registered successfully")
