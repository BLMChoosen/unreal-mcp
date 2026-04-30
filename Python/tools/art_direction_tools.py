"""
Lighting, weather mood, and post-process art direction tools for Unreal MCP.
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


def _preset_from_prompt(prompt: str) -> str:
    lowered = prompt.lower()
    if any(word in lowered for word in ("terror", "horror", "noite", "night", "dark")):
        return "night_horror"
    if any(word in lowered for word in ("sunset", "dusk", "por do sol", "crepusculo")):
        return "sunset_drama"
    if any(word in lowered for word in ("bright", "day", "dia", "clean")):
        return "bright_day"
    return "cinematic"


def register_art_direction_tools(mcp: FastMCP):
    """Register art direction tools with the MCP server."""

    @mcp.tool()
    def apply_lighting_preset(
        ctx: Context,
        preset: str = "night_horror",
        prompt: str = "",
    ) -> dict:
        """Apply a lighting/post-process preset to the current level."""
        try:
            resolved = _preset_from_prompt(prompt) if prompt else preset
            return _send("apply_lighting_preset", {"preset": resolved, "prompt": prompt})
        except Exception as e:
            logger.error(f"Error applying lighting preset: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def apply_art_direction_prompt(ctx: Context, prompt: str) -> dict:
        """Turn a natural-language art direction prompt into level lighting changes."""
        try:
            return _send(
                "apply_lighting_preset",
                {"preset": _preset_from_prompt(prompt), "prompt": prompt},
            )
        except Exception as e:
            logger.error(f"Error applying art direction prompt: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Art direction tools registered successfully")
