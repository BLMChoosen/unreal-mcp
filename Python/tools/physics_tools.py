"""
Physics Tools for Unreal MCP.

Phase 5: Physics constraints, ragdoll setup, and Chaos destruction.
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


def register_physics_tools(mcp: FastMCP):
    """Register physics constraint and Chaos tools with the MCP server."""

    @mcp.tool()
    def create_physics_constraint(
        ctx: Context,
        name: str,
        actor1: str,
        actor2: str,
        constraint_type: str = "Fixed",
        location: list = None
    ) -> dict:
        """Create a physics constraint (joint) between two actors.

        Args:
            name: Name for the constraint actor.
            actor1: Name of the first constrained actor.
            actor2: Name of the second constrained actor.
            constraint_type: Type of constraint: 'Fixed', 'Hinge', 'Prismatic',
                             'BallSocket', 'Free'.
            location: [X, Y, Z] world position for the constraint.
        """
        try:
            return _send("create_physics_constraint", {
                "name": name,
                "actor1": actor1,
                "actor2": actor2,
                "constraint_type": constraint_type,
                "location": location or [0.0, 0.0, 0.0]
            })
        except Exception as e:
            logger.error(f"Error creating physics constraint: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def setup_ragdoll(
        ctx: Context,
        blueprint_name: str,
        skeletal_mesh_component: str = "Mesh"
    ) -> dict:
        """Configure ragdoll physics on a Skeletal Mesh component in a Blueprint.

        Args:
            blueprint_name: Name of the Blueprint containing the skeletal mesh.
            skeletal_mesh_component: Name of the SkeletalMeshComponent.
        """
        try:
            return _send("setup_ragdoll", {
                "blueprint_name": blueprint_name,
                "skeletal_mesh_component": skeletal_mesh_component
            })
        except Exception as e:
            logger.error(f"Error setting up ragdoll: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def enable_chaos_destruction(
        ctx: Context,
        actor_name: str,
        damage_threshold: float = 100.0,
        enable_clustering: bool = True
    ) -> dict:
        """Enable Chaos destruction on an actor.

        Args:
            actor_name: Name of the target actor.
            damage_threshold: Damage threshold for fracture.
            enable_clustering: Whether to enable proximity clustering.
        """
        try:
            return _send("enable_chaos_destruction", {
                "actor_name": actor_name,
                "damage_threshold": damage_threshold,
                "enable_clustering": enable_clustering
            })
        except Exception as e:
            logger.error(f"Error enabling Chaos destruction: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_physics_constraint_properties(
        ctx: Context,
        constraint_name: str,
        linear_limit: float = -1.0,
        swing1_limit: float = -1.0,
        swing2_limit: float = -1.0,
        twist_limit: float = -1.0,
        enable_projection: bool = True
    ) -> dict:
        """Configure properties on an existing physics constraint.

        Args:
            constraint_name: Name of the constraint actor.
            linear_limit: Linear movement limit (-1 = free).
            swing1_limit: Swing 1 angle limit in degrees (-1 = free).
            swing2_limit: Swing 2 angle limit in degrees (-1 = free).
            twist_limit: Twist angle limit in degrees (-1 = free).
            enable_projection: Enable projection for stability.
        """
        try:
            return _send("set_physics_constraint_properties", {
                "constraint_name": constraint_name,
                "linear_limit": linear_limit,
                "swing1_limit": swing1_limit,
                "swing2_limit": swing2_limit,
                "twist_limit": twist_limit,
                "enable_projection": enable_projection
            })
        except Exception as e:
            logger.error(f"Error setting constraint properties: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Physics tools registered successfully")
