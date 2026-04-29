"""
Behavior Tree Tools for Unreal MCP.

Add and connect Behavior Tree nodes (Selector, Sequence, Tasks, Decorators).
"""

import logging
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def register_behavior_tree_tools(mcp: FastMCP):
    """Register Behavior Tree tools with the MCP server."""

    @mcp.tool()
    def add_behavior_tree_node(
        ctx: Context,
        behavior_tree_path: str,
        node_type: str,
        node_name: str = None,
        parent_node_id: str = None,
        task_class: str = None
    ) -> dict:
        """
        Add a node to a Behavior Tree.

        Supported node_type values: Selector, Sequence, Task.

        Examples:
            add_behavior_tree_node("/Game/AI/BT_Enemy.BT_Enemy", "Selector")
            add_behavior_tree_node("/Game/AI/BT_Enemy.BT_Enemy", "Task", task_class="/Script/AIModule.BTTask_MoveTo")
            add_behavior_tree_node("/Game/AI/BT_Enemy.BT_Enemy", "Sequence", parent_node_id="BTComposite_Sequence_0")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "behavior_tree": behavior_tree_path,
                "node_type": node_type
            }
            if node_name:
                params["node_name"] = node_name
            if parent_node_id:
                params["parent_node_id"] = parent_node_id
            if task_class:
                params["task_class"] = task_class

            response = unreal.send_command("add_behavior_tree_node", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding behavior tree node: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def connect_behavior_tree_nodes(
        ctx: Context,
        behavior_tree_path: str,
        parent_node_id: str,
        child_node_id: str,
        index: int = -1
    ) -> dict:
        """
        Connect an existing child node under a parent composite node.

        Examples:
            connect_behavior_tree_nodes("/Game/AI/BT_Enemy.BT_Enemy", "BTComposite_Selector_0", "BTComposite_Sequence_1")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "behavior_tree": behavior_tree_path,
                "parent_node_id": parent_node_id,
                "child_node_id": child_node_id,
                "index": int(index)
            }

            response = unreal.send_command("connect_behavior_tree_nodes", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error connecting behavior tree nodes: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_behavior_tree_decorator(
        ctx: Context,
        behavior_tree_path: str,
        parent_node_id: str,
        child_node_id: str,
        decorator_type: str,
        decorator_name: str = None,
        decorator_params: dict = None
    ) -> dict:
        """
        Add a decorator to a parent->child link in a Behavior Tree.

        Supported decorator_type values: Blackboard, Cooldown, Loop, TimeLimit.

        Examples:
            add_behavior_tree_decorator(
                "/Game/AI/BT_Enemy.BT_Enemy",
                "BTComposite_Selector_0",
                "BTTask_MoveTo_0",
                "Blackboard",
                decorator_params={"key_name": "TargetActor"}
            )
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            if decorator_params is None:
                decorator_params = {}

            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            params = {
                "behavior_tree": behavior_tree_path,
                "parent_node_id": parent_node_id,
                "child_node_id": child_node_id,
                "decorator_type": decorator_type,
                "decorator_params": decorator_params
            }
            if decorator_name:
                params["decorator_name"] = decorator_name

            response = unreal.send_command("add_behavior_tree_decorator", params)
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding behavior tree decorator: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def get_behavior_tree_nodes(
        ctx: Context,
        behavior_tree_path: str
    ) -> dict:
        """
        List nodes in a Behavior Tree with their IDs and types.

        Examples:
            get_behavior_tree_nodes("/Game/AI/BT_Enemy.BT_Enemy")
        """
        from unreal_mcp_server import get_unreal_connection

        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}

            response = unreal.send_command("get_behavior_tree_nodes", {
                "behavior_tree": behavior_tree_path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error listing behavior tree nodes: {e}")
            return {"success": False, "message": str(e)}
