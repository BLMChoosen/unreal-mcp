"""
Recipe Tools for Unreal MCP.

Phase 6: High-level "archetype" tools that orchestrate multiple commands
to create complete gameplay archetypes in one call.
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


def register_recipe_tools(mcp: FastMCP):
    """Register archetype recipe tools with the MCP server."""

    @mcp.tool()
    def create_enemy_ai_archetype(
        ctx: Context,
        name: str,
        path: str = "/Game/AI",
        add_perception: bool = True,
        add_eqs: bool = True,
        add_navmesh: bool = True,
        pawn_parent_class: str = "Character",
        perception_senses: list = None,
        gameplay_tags: list = None
    ) -> dict:
        """Create a complete enemy AI archetype with all required assets.

        Creates: Pawn Blueprint, AIController, Blackboard, Behavior Tree,
        EQS query, tags, AI perception, and NavMesh volume.

        Args:
            name: Base name for all generated assets (e.g. 'EnemyGuard').
            path: Content path for the assets.
            add_perception: Add AIPerception component with senses.
            add_eqs: Create an EQS query asset.
            add_navmesh: Spawn a NavMeshBoundsVolume.
            pawn_parent_class: Parent class for the Pawn (default: Character).
            perception_senses: List of senses to add: ['sight', 'hearing', 'damage'].
            gameplay_tags: Optional gameplay tags to add to the project.
        """
        try:
            return _send("create_enemy_ai_archetype", {
                "name": name,
                "path": path,
                "add_perception": add_perception,
                "add_eqs": add_eqs,
                "add_navmesh": add_navmesh,
                "pawn_parent_class": pawn_parent_class,
                "perception_senses": perception_senses or ["sight", "hearing"],
                "gameplay_tags": gameplay_tags or []
            })
        except Exception as e:
            logger.error(f"Error creating enemy AI archetype: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_interactable_archetype(
        ctx: Context,
        name: str,
        path: str = "/Game/Blueprints",
        mesh_path: str = "",
        interaction_type: str = "overlap",
        add_widget: bool = False
    ) -> dict:
        """Create a complete interactable object archetype.

        Creates: Actor Blueprint with collision, interaction trigger,
        optional UI widget prompt.

        Args:
            name: Base name for the interactable (e.g. 'Chest').
            path: Content path for the assets.
            mesh_path: Optional static mesh path for visual representation.
            interaction_type: 'overlap' or 'input' based interaction.
            add_widget: Add a floating interaction widget.
        """
        try:
            return _send("create_interactable_archetype", {
                "name": name,
                "path": path,
                "mesh_path": mesh_path,
                "interaction_type": interaction_type,
                "add_widget": add_widget
            })
        except Exception as e:
            logger.error(f"Error creating interactable archetype: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_pickup_item_archetype(
        ctx: Context,
        name: str,
        path: str = "/Game/Blueprints",
        mesh_path: str = "",
        rotate_speed: float = 90.0,
        bob_height: float = 20.0,
        add_data_table_row: bool = True,
        data_table_path: str = ""
    ) -> dict:
        """Create a complete pickup item archetype.

        Creates: Actor Blueprint with mesh, collision sphere, overlap event,
        rotation/bobbing animation, optional Data Table row.

        Args:
            name: Base name for the pickup (e.g. 'HealthPotion').
            path: Content path for the assets.
            mesh_path: Optional static mesh path.
            rotate_speed: Rotation speed in degrees/second.
            bob_height: Bobbing animation height.
            add_data_table_row: Whether to add a row to an items data table.
            data_table_path: Path to the items Data Table (if add_data_table_row).
        """
        try:
            return _send("create_pickup_item_archetype", {
                "name": name,
                "path": path,
                "mesh_path": mesh_path,
                "rotate_speed": rotate_speed,
                "bob_height": bob_height,
                "add_data_table_row": add_data_table_row,
                "data_table_path": data_table_path
            })
        except Exception as e:
            logger.error(f"Error creating pickup archetype: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Recipe tools registered successfully")
