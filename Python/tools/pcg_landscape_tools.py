"""
PCG, biome, landscape, and road spline tools for Unreal MCP.
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


def register_pcg_landscape_tools(mcp: FastMCP):
    """Register PCG and landscape tools with the MCP server."""

    @mcp.tool()
    def create_pcg_graph(ctx: Context, name: str, path: str = "/Game/PCG") -> dict:
        """Create a PCG Graph asset when the PCG editor plugin is available."""
        try:
            return _send("create_pcg_graph", {"name": name, "path": path})
        except Exception as e:
            logger.error(f"Error creating PCG graph: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def configure_pcg_graph(
        ctx: Context,
        graph_path: str,
        nodes: list,
        edges: list = None,
        description: str = "",
    ) -> dict:
        """Attach a PCG node graph spec to a PCG Graph asset.

        Example nodes: SurfaceSampler, DensityFilter, StaticMeshSpawner.
        """
        try:
            return _send(
                "configure_pcg_graph",
                {
                    "graph_path": graph_path,
                    "nodes": nodes,
                    "edges": edges or [],
                    "description": description,
                },
            )
        except Exception as e:
            logger.error(f"Error configuring PCG graph: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_pcg_volume(
        ctx: Context,
        name: str = "MCP_PCGVolume",
        graph_path: str = "",
        location: list = None,
        extent: list = None,
    ) -> dict:
        """Spawn a PCG Volume and optionally assign a PCG Graph."""
        try:
            params = {
                "name": name,
                "location": location or [0.0, 0.0, 0.0],
                "extent": extent or [2500.0, 2500.0, 1000.0],
            }
            if graph_path:
                params["graph_path"] = graph_path
            return _send("create_pcg_volume", params)
        except Exception as e:
            logger.error(f"Error creating PCG volume: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_biome_from_prompt(
        ctx: Context,
        prompt: str,
        name: str = "MCP_Biome",
        density: float = 0.75,
        count: int = 0,
        asset_paths: list = None,
        bounds_min: list = None,
        bounds_max: list = None,
        scale_min: float = 0.75,
        scale_max: float = 1.8,
        seed: int = 1337,
        create_pcg_volume: bool = True,
    ) -> dict:
        """Create an automated biome by selecting project Static Mesh assets and scattering them."""
        try:
            params = {
                "prompt": prompt,
                "name": name,
                "density": density,
                "asset_paths": asset_paths or ["/Game"],
                "bounds_min": bounds_min or [-2500.0, -2500.0, 0.0],
                "bounds_max": bounds_max or [2500.0, 2500.0, 0.0],
                "scale_min": scale_min,
                "scale_max": scale_max,
                "seed": seed,
                "create_pcg_volume": create_pcg_volume,
            }
            if count > 0:
                params["count"] = count
            return _send("create_biome_from_prompt", params)
        except Exception as e:
            logger.error(f"Error creating biome: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_landscape(
        ctx: Context,
        name: str = "MCP_Landscape",
        section_size: int = 63,
        component_count_x: int = 8,
        component_count_y: int = 8,
        material_path: str = "",
    ) -> dict:
        """Validate Landscape support and return a Landscape setup descriptor."""
        try:
            return _send(
                "create_landscape",
                {
                    "name": name,
                    "section_size": section_size,
                    "component_count_x": component_count_x,
                    "component_count_y": component_count_y,
                    "material_path": material_path,
                },
            )
        except Exception as e:
            logger.error(f"Error creating landscape: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def apply_landscape_material(
        ctx: Context,
        material_path: str,
        actor_name: str = "",
    ) -> dict:
        """Apply a Landscape Material to one or all Landscape actors."""
        try:
            return _send(
                "apply_landscape_material",
                {"material_path": material_path, "actor_name": actor_name},
            )
        except Exception as e:
            logger.error(f"Error applying landscape material: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_road_spline(
        ctx: Context,
        points: list,
        name: str = "MCP_RoadSpline",
        width: float = 600.0,
    ) -> dict:
        """Create a road spline actor from world-space points."""
        try:
            return _send("create_road_spline", {"points": points, "name": name, "width": width})
        except Exception as e:
            logger.error(f"Error creating road spline: {e}")
            return {"success": False, "message": str(e)}

    logger.info("PCG and landscape tools registered successfully")
