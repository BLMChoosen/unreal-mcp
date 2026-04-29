"""
Level Design Tools for Unreal MCP.

Phase 5: Snap/alignment, procedural scatter, bounds measurement,
overlap detection, and volume creation.
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


def register_level_design_tools(mcp: FastMCP):
    """Register level design tools with the MCP server."""

    @mcp.tool()
    def snap_actor_to_grid(
        ctx: Context,
        name: str,
        grid_size: float = 100.0
    ) -> dict:
        """Snap an actor's position to the nearest grid point.

        Args:
            name: Name of the actor to snap.
            grid_size: Grid size in Unreal units (default 100).
        """
        try:
            return _send("snap_actor_to_grid", {
                "name": name,
                "grid_size": grid_size
            })
        except Exception as e:
            logger.error(f"Error snapping actor: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def align_actors(
        ctx: Context,
        actor_names: list,
        axis: str = "X",
        mode: str = "min"
    ) -> dict:
        """Align multiple actors along an axis.

        Args:
            actor_names: List of actor names to align.
            axis: Axis to align along: 'X', 'Y', or 'Z'.
            mode: Alignment mode: 'min', 'max', 'center', or 'average'.
        """
        try:
            return _send("align_actors", {
                "actor_names": actor_names,
                "axis": axis,
                "mode": mode
            })
        except Exception as e:
            logger.error(f"Error aligning actors: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def procedural_scatter(
        ctx: Context,
        actor_type: str,
        count: int = 10,
        bounds_min: list = None,
        bounds_max: list = None,
        seed: int = -1,
        random_rotation: bool = True,
        random_scale_min: float = 1.0,
        random_scale_max: float = 1.0
    ) -> dict:
        """Scatter actors procedurally within a bounding region.

        Args:
            actor_type: Type of actor to scatter (e.g. 'StaticMeshActor').
            count: Number of actors to place.
            bounds_min: [X, Y, Z] minimum bounds.
            bounds_max: [X, Y, Z] maximum bounds.
            seed: Random seed (-1 for random).
            random_rotation: Randomize yaw rotation.
            random_scale_min: Minimum random scale.
            random_scale_max: Maximum random scale.
        """
        try:
            return _send("procedural_scatter", {
                "actor_type": actor_type,
                "count": count,
                "bounds_min": bounds_min or [-1000, -1000, 0],
                "bounds_max": bounds_max or [1000, 1000, 0],
                "seed": seed,
                "random_rotation": random_rotation,
                "random_scale_min": random_scale_min,
                "random_scale_max": random_scale_max
            })
        except Exception as e:
            logger.error(f"Error in procedural scatter: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_actor_bounds(ctx: Context, name: str) -> dict:
        """Get the bounding box of an actor.

        Args:
            name: Name of the actor.

        Returns:
            Dict with 'origin', 'extent', 'min', 'max' world-space vectors.
        """
        try:
            return _send("get_actor_bounds", {"name": name})
        except Exception as e:
            logger.error(f"Error getting bounds: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def find_overlapping_actors(ctx: Context, name: str) -> dict:
        """Find all actors whose bounds overlap with the target actor.

        Args:
            name: Name of the reference actor.
        """
        try:
            return _send("find_overlapping_actors", {"name": name})
        except Exception as e:
            logger.error(f"Error finding overlaps: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_volume(
        ctx: Context,
        volume_type: str,
        location: list = None,
        extent: list = None,
        name: str = ""
    ) -> dict:
        """Create a volume actor (Trigger, Nav Mesh, Blocking, etc.).

        Args:
            volume_type: One of 'TriggerVolume', 'NavMeshBoundsVolume',
                         'BlockingVolume', 'KillZVolume', 'PainCausingVolume',
                         'AudioVolume', 'PostProcessVolume'.
            location: [X, Y, Z] world position.
            extent: [X, Y, Z] half-extents of the volume brush.
            name: Optional custom name for the volume.
        """
        try:
            return _send("create_volume", {
                "volume_type": volume_type,
                "location": location or [0.0, 0.0, 0.0],
                "extent": extent or [200.0, 200.0, 200.0],
                "name": name
            })
        except Exception as e:
            logger.error(f"Error creating volume: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Level design tools registered successfully")
