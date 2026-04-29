"""
Sequencer Tools for Unreal MCP.

Create level sequences, bind actors, add transform tracks and keyframes.
"""

import logging
from typing import Dict, List, Any
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def register_sequencer_tools(mcp: FastMCP):
    """Register Sequencer tools with the MCP server."""

    @mcp.tool()
    def create_level_sequence(
        ctx: Context,
        sequence_name: str,
        path: str = "/Game/Sequences",
        duration: float = 5.0
    ) -> Dict[str, Any]:
        """Create a new ULevelSequence asset with a given playback duration."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_level_sequence", {
                "sequence_name": sequence_name,
                "path": path,
                "duration": duration
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating level sequence: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_actor_to_sequence(
        ctx: Context,
        sequence_name: str,
        actor_name: str
    ) -> Dict[str, Any]:
        """Bind an actor to a sequence as a Possessable. Returns binding_id (GUID)."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_actor_to_sequence", {
                "sequence_name": sequence_name,
                "actor_name": actor_name
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding actor to sequence: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_transform_track(
        ctx: Context,
        sequence_name: str,
        binding_id: str
    ) -> Dict[str, Any]:
        """Add a UMovieScene3DTransformTrack to a binding."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_transform_track", {
                "sequence_name": sequence_name,
                "binding_id": binding_id
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding transform track: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_transform_keyframe(
        ctx: Context,
        sequence_name: str,
        binding_id: str,
        time: float,
        location: List[float] = [0.0, 0.0, 0.0],
        rotation: List[float] = [0.0, 0.0, 0.0],
        scale: List[float] = [1.0, 1.0, 1.0]
    ) -> Dict[str, Any]:
        """Add a transform keyframe at the given time (seconds).

        Args:
            location: [X, Y, Z]
            rotation: [Pitch, Yaw, Roll]
            scale: [X, Y, Z]
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_transform_keyframe", {
                "sequence_name": sequence_name,
                "binding_id": binding_id,
                "time": time,
                "location": location,
                "rotation": rotation,
                "scale": scale
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding keyframe: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_sequences_in_level(ctx: Context) -> Dict[str, Any]:
        """List all ULevelSequence assets known to the asset registry."""
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("get_sequences_in_level", {})
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error listing sequences: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Sequencer tools registered successfully")
