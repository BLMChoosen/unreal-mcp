"""
Data Tools for Unreal MCP.

Import/export Data Tables, manipulate rows, manage Data Assets,
and edit String Tables for localization.
"""

import logging
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def register_data_tools(mcp: FastMCP):
    """Register data tools with the MCP server."""

    @mcp.tool()
    def create_data_table(
        ctx: Context,
        name: str,
        row_struct: str,
        path: str = "/Game/Data"
    ) -> dict:
        """
        Create a new Data Table asset for the given row struct.

        Examples:
            create_data_table("DT_Items", "/Script/MyGame.ItemRow", "/Game/Data")
            create_data_table("DT_Items", "/Game/Data/ST_ItemRow.ST_ItemRow", "/Game/Data")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("create_data_table", {
                "name": name,
                "path": path,
                "row_struct": row_struct
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating data table: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def import_data_table(
        ctx: Context,
        data_table_path: str,
        data: str,
        data_format: str = "json"
    ) -> dict:
        """
        Import CSV or JSON content into an existing Data Table.

        Examples:
            import_data_table("/Game/Data/DT_Items.DT_Items", "{\"Rows\":{...}}", "json")
            import_data_table("/Game/Data/DT_Items.DT_Items", "Name,Damage\nSword,10", "csv")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("import_data_table", {
                "data_table": data_table_path,
                "data": data,
                "format": data_format
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error importing data table: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def export_data_table(
        ctx: Context,
        data_table_path: str,
        data_format: str = "json"
    ) -> dict:
        """
        Export a Data Table to CSV or JSON content.

        Examples:
            export_data_table("/Game/Data/DT_Items.DT_Items", "json")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("export_data_table", {
                "data_table": data_table_path,
                "format": data_format
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error exporting data table: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_data_table_row(
        ctx: Context,
        data_table_path: str,
        row_name: str,
        row_data: dict
    ) -> dict:
        """
        Add a row to a Data Table using JSON-like data.

        Examples:
            add_data_table_row("/Game/Data/DT_Items.DT_Items", "Sword", {"Damage": 10, "Price": 50})
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("add_data_table_row", {
                "data_table": data_table_path,
                "row_name": row_name,
                "row_data": row_data
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding data table row: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_data_table_row(
        ctx: Context,
        data_table_path: str,
        row_name: str
    ) -> dict:
        """
        Get a row from a Data Table.

        Examples:
            get_data_table_row("/Game/Data/DT_Items.DT_Items", "Sword")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("get_data_table_row", {
                "data_table": data_table_path,
                "row_name": row_name
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting data table row: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def edit_data_table_row(
        ctx: Context,
        data_table_path: str,
        row_name: str,
        row_data: dict
    ) -> dict:
        """
        Edit an existing row in a Data Table.

        Examples:
            edit_data_table_row("/Game/Data/DT_Items.DT_Items", "Sword", {"Damage": 12})
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("edit_data_table_row", {
                "data_table": data_table_path,
                "row_name": row_name,
                "row_data": row_data
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error editing data table row: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def list_data_table_rows(
        ctx: Context,
        data_table_path: str
    ) -> dict:
        """
        List row names in a Data Table.

        Examples:
            list_data_table_rows("/Game/Data/DT_Items.DT_Items")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("list_data_table_rows", {
                "data_table": data_table_path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error listing data table rows: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_data_asset(
        ctx: Context,
        name: str,
        path: str,
        asset_class: str
    ) -> dict:
        """
        Create a new Data Asset of the given class.

        Examples:
            create_data_asset("DA_Item", "/Game/Data", "/Script/Engine.DataAsset")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("create_data_asset", {
                "name": name,
                "path": path,
                "asset_class": asset_class
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating data asset: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_data_asset_properties(
        ctx: Context,
        data_asset_path: str,
        properties: dict
    ) -> dict:
        """
        Set properties on a Data Asset using JSON-like data.

        Examples:
            set_data_asset_properties("/Game/Data/DA_Item.DA_Item", {"DisplayName": "Sword"})
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("set_data_asset_properties", {
                "data_asset": data_asset_path,
                "properties": properties
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting data asset properties: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_data_asset_properties(
        ctx: Context,
        data_asset_path: str
    ) -> dict:
        """
        Get properties from a Data Asset as JSON-like data.

        Examples:
            get_data_asset_properties("/Game/Data/DA_Item.DA_Item")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("get_data_asset_properties", {
                "data_asset": data_asset_path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting data asset properties: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_string_table(
        ctx: Context,
        name: str,
        path: str = "/Game/Localization"
    ) -> dict:
        """
        Create a new String Table asset.

        Examples:
            create_string_table("ST_GameText", "/Game/Localization")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("create_string_table", {
                "name": name,
                "path": path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating string table: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_string_table_entry(
        ctx: Context,
        string_table_path: str,
        key: str,
        source_string: str
    ) -> dict:
        """
        Add or update a String Table entry.

        Examples:
            add_string_table_entry("/Game/Localization/ST_GameText.ST_GameText", "UI_Start", "Start")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("add_string_table_entry", {
                "string_table": string_table_path,
                "key": key,
                "source_string": source_string
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding string table entry: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_string_table_entry(
        ctx: Context,
        string_table_path: str,
        key: str
    ) -> dict:
        """
        Get a String Table entry by key.

        Examples:
            get_string_table_entry("/Game/Localization/ST_GameText.ST_GameText", "UI_Start")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("get_string_table_entry", {
                "string_table": string_table_path,
                "key": key
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error getting string table entry: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def remove_string_table_entry(
        ctx: Context,
        string_table_path: str,
        key: str
    ) -> dict:
        """
        Remove a String Table entry by key.

        Examples:
            remove_string_table_entry("/Game/Localization/ST_GameText.ST_GameText", "UI_Start")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("remove_string_table_entry", {
                "string_table": string_table_path,
                "key": key
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error removing string table entry: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def list_string_table_entries(
        ctx: Context,
        string_table_path: str
    ) -> dict:
        """
        List all entries in a String Table.

        Examples:
            list_string_table_entries("/Game/Localization/ST_GameText.ST_GameText")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("list_string_table_entries", {
                "string_table": string_table_path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error listing string table entries: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def export_string_table(
        ctx: Context,
        string_table_path: str
    ) -> dict:
        """
        Export a String Table as JSON.

        Examples:
            export_string_table("/Game/Localization/ST_GameText.ST_GameText")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("export_string_table", {
                "string_table": string_table_path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error exporting string table: {e}")
            return {"success": False, "message": str(e)}
