"""
Material Tools for Unreal MCP.

Create UMaterial assets, material instances, set scalar/vector parameters,
and apply materials to actors in the editor.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def register_material_tools(mcp: FastMCP):
    """Register Material tools with the MCP server."""

    @mcp.tool()
    def create_material(
        ctx: Context,
        name: str,
        path: str = "/Game/Materials"
    ) -> Dict[str, Any]:
        """Create a new UMaterial asset.

        Args:
            name: Asset name (e.g. "M_RedPlastic").
            path: Content browser path. Defaults to /Game/Materials.

        Returns:
            { "name": str, "path": str } on success.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_material", {"name": name, "path": path})
            if not response:
                return {"success": False, "message": "No response from Unreal Engine"}
            logger.info(f"create_material response: {response}")
            return response
        except Exception as e:
            logger.error(f"Error creating material: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_material_instance(
        ctx: Context,
        instance_name: str,
        parent_material_name: str,
        path: str = "/Game/Materials"
    ) -> Dict[str, Any]:
        """Create a UMaterialInstanceConstant from a parent UMaterial.

        Args:
            instance_name: Name of the new material instance.
            parent_material_name: Name of the parent material asset.
            path: Content browser path.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_material_instance", {
                "instance_name": instance_name,
                "parent_material_name": parent_material_name,
                "path": path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating material instance: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_material_scalar_parameter(
        ctx: Context,
        material_name: str,
        parameter_name: str,
        value: float
    ) -> Dict[str, Any]:
        """Set a scalar parameter on a UMaterial or UMaterialInstanceConstant.

        For a parent UMaterial, this updates the matching ScalarParameter expression
        and recompiles. For a UMaterialInstanceConstant, this calls UMaterialEditingLibrary.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("set_material_scalar_parameter", {
                "material_name": material_name,
                "parameter_name": parameter_name,
                "value": value
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting scalar parameter: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_material_vector_parameter(
        ctx: Context,
        material_name: str,
        parameter_name: str,
        value: List[float]
    ) -> Dict[str, Any]:
        """Set a vector (color) parameter on a material.

        Args:
            value: [R, G, B] or [R, G, B, A] floats in 0..1 range.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("set_material_vector_parameter", {
                "material_name": material_name,
                "parameter_name": parameter_name,
                "value": value
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting vector parameter: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def apply_material_to_actor(
        ctx: Context,
        actor_name: str,
        material_name: str,
        slot_index: int = 0
    ) -> Dict[str, Any]:
        """Apply a material to a static-mesh actor at the given material slot."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("apply_material_to_actor", {
                "actor_name": actor_name,
                "material_name": material_name,
                "slot_index": slot_index
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error applying material: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Material tools registered successfully")
