"""
Schema Tools for Unreal MCP.

Phase 3: UserDefinedStruct creation, field management, and Data Table schema generation.
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


def register_schema_tools(mcp: FastMCP):
    """Register schema/struct tools with the MCP server."""

    @mcp.tool()
    def create_user_defined_struct(
        ctx: Context,
        name: str,
        path: str = "/Game/Data"
    ) -> dict:
        """Create a new UserDefinedStruct asset.

        Args:
            name: Name of the struct (e.g. 'S_ItemData').
            path: Content path to create the struct in.
        """
        try:
            return _send("create_user_defined_struct", {"name": name, "path": path})
        except Exception as e:
            logger.error(f"Error creating UserDefinedStruct: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_struct_field(
        ctx: Context,
        struct_path: str,
        field_name: str,
        field_type: str,
        default_value: str = ""
    ) -> dict:
        """Add a field to a UserDefinedStruct.

        Args:
            struct_path: Content path of the struct asset.
            field_name: Name for the new field.
            field_type: UE type name: Boolean, Integer, Float, String, Name,
                        Text, Vector, Rotator, Transform, Object, etc.
            default_value: Optional default value as a string.
        """
        try:
            params = {
                "struct_path": struct_path,
                "field_name": field_name,
                "field_type": field_type
            }
            if default_value:
                params["default_value"] = default_value
            return _send("add_struct_field", params)
        except Exception as e:
            logger.error(f"Error adding struct field: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def remove_struct_field(
        ctx: Context,
        struct_path: str,
        field_name: str
    ) -> dict:
        """Remove a field from a UserDefinedStruct.

        Args:
            struct_path: Content path of the struct asset.
            field_name: Name of the field to remove.
        """
        try:
            return _send("remove_struct_field", {
                "struct_path": struct_path,
                "field_name": field_name
            })
        except Exception as e:
            logger.error(f"Error removing struct field: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_struct_fields(
        ctx: Context,
        struct_path: str
    ) -> dict:
        """List all fields in a UserDefinedStruct.

        Args:
            struct_path: Content path of the struct asset.
        """
        try:
            return _send("get_struct_fields", {"struct_path": struct_path})
        except Exception as e:
            logger.error(f"Error getting struct fields: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def recompile_struct(
        ctx: Context,
        struct_path: str
    ) -> dict:
        """Recompile a UserDefinedStruct and update dependent Data Tables.

        Args:
            struct_path: Content path of the struct asset.
        """
        try:
            return _send("recompile_struct", {"struct_path": struct_path})
        except Exception as e:
            logger.error(f"Error recompiling struct: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_data_table_from_struct(
        ctx: Context,
        name: str,
        struct_path: str,
        path: str = "/Game/Data"
    ) -> dict:
        """Create a Data Table using a UserDefinedStruct as its row type.

        Args:
            name: Name for the new Data Table.
            struct_path: Content path of the UserDefinedStruct to use.
            path: Content path to create the Data Table in.
        """
        try:
            return _send("create_data_table_from_struct", {
                "name": name,
                "struct_path": struct_path,
                "path": path
            })
        except Exception as e:
            logger.error(f"Error creating data table from struct: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def generate_schema(
        ctx: Context,
        schema_type: str,
        name: str,
        path: str = "/Game/Data",
        custom_fields: list = None
    ) -> dict:
        """Generate a complete data schema (struct + Data Table) for common game data.

        Args:
            schema_type: One of 'item', 'quest', 'dialogue', 'save_data'.
                         Each generates appropriate default fields.
            name: Base name for the generated assets.
            path: Content path for the assets.
            custom_fields: Optional list of dicts with 'name' and 'type' to add
                           additional fields beyond the defaults.
        """
        try:
            return _send("generate_schema", {
                "schema_type": schema_type,
                "name": name,
                "path": path,
                "custom_fields": custom_fields or []
            })
        except Exception as e:
            logger.error(f"Error generating schema: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Schema tools registered successfully")
