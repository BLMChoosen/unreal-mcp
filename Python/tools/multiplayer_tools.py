"""
Multiplayer replication helpers for Unreal MCP.
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


def register_multiplayer_tools(mcp: FastMCP):
    """Register multiplayer and replication tools with the MCP server."""

    @mcp.tool()
    def build_replication_plan(
        ctx: Context,
        variables: list,
        rpcs: list = None,
    ) -> dict:
        """Validate a Blueprint replication plan before applying it."""
        try:
            issues = []
            valid_replication = {"Replicated", "RepNotify", "ReplicatedUsing"}
            valid_rpc = {"RunOnServer", "Server", "Client", "RunOnClient", "Multicast", "NetMulticast"}
            for variable in variables:
                if not variable.get("name"):
                    issues.append({"item": variable, "issue": "Variable missing name"})
                if variable.get("replication", "Replicated") not in valid_replication:
                    issues.append({"item": variable, "issue": "Unknown replication mode"})
            for rpc in rpcs or []:
                if not rpc.get("event_name"):
                    issues.append({"item": rpc, "issue": "RPC missing event_name"})
                if rpc.get("rpc_type", "RunOnServer") not in valid_rpc:
                    issues.append({"item": rpc, "issue": "Unknown RPC type"})
            return {
                "success": True,
                "valid": not issues,
                "plan": {"variables": variables, "rpcs": rpcs or []},
                "issues": issues,
            }
        except Exception as e:
            logger.error(f"Error building replication plan: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_blueprint_variable_replication(
        ctx: Context,
        blueprint_name: str,
        variable_name: str,
        replication: str = "Replicated",
        notify_function: str = "",
    ) -> dict:
        """Mark a Blueprint variable as Replicated or RepNotify."""
        try:
            return _send(
                "set_blueprint_variable_replication",
                {
                    "blueprint_name": blueprint_name,
                    "variable_name": variable_name,
                    "replication": replication,
                    "notify_function": notify_function,
                },
            )
        except Exception as e:
            logger.error(f"Error setting variable replication: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_blueprint_rpc_event(
        ctx: Context,
        blueprint_name: str,
        event_name: str,
        rpc_type: str = "RunOnServer",
        reliable: bool = True,
        position: list = None,
    ) -> dict:
        """Create a replicated custom event in a Blueprint."""
        try:
            return _send(
                "create_blueprint_rpc_event",
                {
                    "blueprint_name": blueprint_name,
                    "event_name": event_name,
                    "rpc_type": rpc_type,
                    "reliable": reliable,
                    "position": position or [0.0, 0.0],
                },
            )
        except Exception as e:
            logger.error(f"Error creating Blueprint RPC event: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Multiplayer tools registered successfully")
