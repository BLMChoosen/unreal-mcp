"""
MetaSound and dialogue audio tools for Unreal MCP.
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


def register_metasound_tools(mcp: FastMCP):
    """Register MetaSound and dialogue tools with the MCP server."""

    @mcp.tool()
    def build_metasound_graph_spec(
        ctx: Context,
        nodes: list,
        connections: list = None,
        inputs: dict = None,
        outputs: dict = None,
    ) -> dict:
        """Normalize a MetaSound graph spec before applying it to an asset."""
        try:
            node_ids = {node.get("id") for node in nodes if node.get("id")}
            issues = []
            for connection in connections or []:
                if connection.get("from_node") not in node_ids:
                    issues.append({"connection": connection, "issue": "Unknown from_node"})
                if connection.get("to_node") not in node_ids:
                    issues.append({"connection": connection, "issue": "Unknown to_node"})
            return {
                "success": True,
                "valid": not issues,
                "spec": {
                    "nodes": nodes,
                    "connections": connections or [],
                    "inputs": inputs or {},
                    "outputs": outputs or {},
                },
                "issues": issues,
            }
        except Exception as e:
            logger.error(f"Error building MetaSound graph spec: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_metasound_source(
        ctx: Context,
        name: str,
        path: str = "/Game/Audio/MetaSounds",
    ) -> dict:
        """Create a MetaSound Source asset when MetaSound plugins are available."""
        try:
            return _send("create_metasound_source", {"name": name, "path": path})
        except Exception as e:
            logger.error(f"Error creating MetaSound Source: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def configure_metasound_graph(
        ctx: Context,
        metasound_path: str,
        nodes: list,
        connections: list = None,
    ) -> dict:
        """Attach a MetaSound graph spec to a MetaSound asset."""
        try:
            return _send(
                "configure_metasound_graph",
                {
                    "metasound_path": metasound_path,
                    "nodes": nodes,
                    "connections": connections or [],
                },
            )
        except Exception as e:
            logger.error(f"Error configuring MetaSound graph: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def import_dialogue_batch(
        ctx: Context,
        dialogue: list,
        destination_path: str = "/Game/Audio/Dialogues",
        import_audio: bool = True,
    ) -> dict:
        """Import dialogue JSON rows and optional audio files as Sound Waves.

        Each dialogue item can include: id, speaker, text, audio_file.
        """
        try:
            return _send(
                "import_dialogue_batch",
                {
                    "dialogue": dialogue,
                    "destination_path": destination_path,
                    "import_audio": import_audio,
                },
            )
        except Exception as e:
            logger.error(f"Error importing dialogue batch: {e}")
            return {"success": False, "message": str(e)}

    logger.info("MetaSound tools registered successfully")
