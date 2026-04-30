"""
Animation, AnimBP, Montage, and IK retargeting tools for Unreal MCP.
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


def register_animation_tools(mcp: FastMCP):
    """Register animation authoring tools with the MCP server."""

    @mcp.tool()
    def build_anim_state_machine_spec(
        ctx: Context,
        states: list,
        transitions: list = None,
        blend_spaces: list = None,
    ) -> dict:
        """Normalize an AnimBP state machine spec for later graph creation."""
        try:
            state_names = [state.get("name", "") for state in states if state.get("name")]
            issues = []
            for transition in transitions or []:
                if transition.get("from") not in state_names:
                    issues.append({"transition": transition, "issue": "Unknown source state"})
                if transition.get("to") not in state_names:
                    issues.append({"transition": transition, "issue": "Unknown target state"})
            return {
                "success": True,
                "valid": not issues,
                "spec": {
                    "states": states,
                    "transitions": transitions or [],
                    "blend_spaces": blend_spaces or [],
                },
                "issues": issues,
            }
        except Exception as e:
            logger.error(f"Error building AnimBP spec: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_anim_blueprint(
        ctx: Context,
        name: str,
        skeleton_path: str,
        path: str = "/Game/Animation",
    ) -> dict:
        """Create an Animation Blueprint asset for a Skeleton."""
        try:
            return _send(
                "create_anim_blueprint",
                {"name": name, "path": path, "skeleton_path": skeleton_path},
            )
        except Exception as e:
            logger.error(f"Error creating AnimBP: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_blend_space(
        ctx: Context,
        name: str,
        skeleton_path: str,
        path: str = "/Game/Animation",
        axis_spec: dict = None,
    ) -> dict:
        """Create a Blend Space asset and record its axis configuration."""
        try:
            return _send(
                "create_blend_space",
                {
                    "name": name,
                    "path": path,
                    "skeleton_path": skeleton_path,
                    "axis_spec": axis_spec or {},
                },
            )
        except Exception as e:
            logger.error(f"Error creating Blend Space: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_anim_montage_from_sequence(
        ctx: Context,
        name: str,
        sequence_path: str,
        path: str = "/Game/Animation",
    ) -> dict:
        """Create an Anim Montage asset from an animation sequence."""
        try:
            return _send(
                "create_anim_montage_from_sequence",
                {"name": name, "path": path, "sequence_path": sequence_path},
            )
        except Exception as e:
            logger.error(f"Error creating Anim Montage: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_anim_notify(
        ctx: Context,
        animation_path: str,
        notify_name: str,
        time_seconds: float,
        notify_class: str = "",
    ) -> dict:
        """Add or record an animation notify intent at a specific time."""
        try:
            return _send(
                "add_anim_notify",
                {
                    "animation_path": animation_path,
                    "notify_name": notify_name,
                    "time_seconds": time_seconds,
                    "notify_class": notify_class,
                },
            )
        except Exception as e:
            logger.error(f"Error adding Anim Notify: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_ik_rig(
        ctx: Context,
        name: str,
        skeletal_mesh_path: str,
        path: str = "/Game/Animation/IK",
    ) -> dict:
        """Create an IK Rig asset for a skeletal mesh."""
        try:
            return _send(
                "create_ik_rig",
                {"name": name, "path": path, "skeletal_mesh_path": skeletal_mesh_path},
            )
        except Exception as e:
            logger.error(f"Error creating IK Rig: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_ik_retargeter(
        ctx: Context,
        name: str,
        source_ik_rig_path: str,
        target_ik_rig_path: str,
        path: str = "/Game/Animation/IK",
    ) -> dict:
        """Create an IK Retargeter asset between source and target IK Rigs."""
        try:
            return _send(
                "create_ik_retargeter",
                {
                    "name": name,
                    "path": path,
                    "source_ik_rig_path": source_ik_rig_path,
                    "target_ik_rig_path": target_ik_rig_path,
                },
            )
        except Exception as e:
            logger.error(f"Error creating IK Retargeter: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Animation tools registered successfully")
