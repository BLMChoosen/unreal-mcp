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

    # ===== Phase 3: Extended BT/EQS Integration =====

    @mcp.tool()
    def create_bt_task_blueprint(
        ctx: Context,
        name: str,
        path: str = "/Game/AI"
    ) -> dict:
        """Create a BTTask Blueprint (subclass of BTTask_BlueprintBase).

        Args:
            name: Name for the task Blueprint (e.g. 'BTTask_AttackPlayer').
            path: Content path for the asset.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_bt_task_blueprint", {
                "name": name, "path": path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating BT task blueprint: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_bt_service_blueprint(
        ctx: Context,
        name: str,
        path: str = "/Game/AI"
    ) -> dict:
        """Create a BTService Blueprint (subclass of BTService_BlueprintBase).

        Args:
            name: Name for the service Blueprint (e.g. 'BTService_UpdateTarget').
            path: Content path for the asset.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_bt_service_blueprint", {
                "name": name, "path": path
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating BT service blueprint: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def add_bt_run_eqs_query(
        ctx: Context,
        behavior_tree_path: str,
        eqs_query_path: str,
        blackboard_key: str = "MoveToLocation",
        parent_node_id: str = ""
    ) -> dict:
        """Add a 'Run EQS Query' task node to a Behavior Tree.

        Args:
            behavior_tree_path: Content path of the Behavior Tree.
            eqs_query_path: Content path of the EQS query to run.
            blackboard_key: Blackboard key to store results in.
            parent_node_id: Optional parent composite node ID.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("add_bt_run_eqs_query", {
                "behavior_tree": behavior_tree_path,
                "eqs_query": eqs_query_path,
                "blackboard_key": blackboard_key,
                "parent_node_id": parent_node_id
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error adding Run EQS Query: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def set_bt_node_blackboard_key(
        ctx: Context,
        behavior_tree_path: str,
        node_id: str,
        key_name: str,
        property_name: str = "BlackboardKey"
    ) -> dict:
        """Set a Blackboard key on a BT task or decorator node.

        Args:
            behavior_tree_path: Content path of the Behavior Tree.
            node_id: ID of the task or decorator node.
            key_name: Name of the Blackboard key to assign.
            property_name: Property name holding the BB key (default: 'BlackboardKey').
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("set_bt_node_blackboard_key", {
                "behavior_tree": behavior_tree_path,
                "node_id": node_id,
                "key_name": key_name,
                "property_name": property_name
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error setting BT node BB key: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def create_full_ai_setup(
        ctx: Context,
        name: str,
        path: str = "/Game/AI",
        pawn_parent_class: str = "Character",
        add_eqs: bool = True,
        add_perception: bool = True
    ) -> dict:
        """Create a complete AI setup: Blackboard + Behavior Tree + EQS + AIController + Pawn.

        All assets are auto-wired together.

        Args:
            name: Base name for all assets (e.g. 'EnemyGuard').
            path: Content path for the assets.
            pawn_parent_class: Parent class for the Pawn Blueprint.
            add_eqs: Create an EQS query asset.
            add_perception: Add AIPerception to the AIController.
        """
        from unreal_mcp_server import get_unreal_connection
        try:
            unreal = get_unreal_connection()
            if not unreal:
                return {"success": False, "message": "Failed to connect to Unreal Engine"}
            response = unreal.send_command("create_full_ai_setup", {
                "name": name,
                "path": path,
                "pawn_parent_class": pawn_parent_class,
                "add_eqs": add_eqs,
                "add_perception": add_perception
            })
            return response or {"success": False, "message": "No response"}
        except Exception as e:
            logger.error(f"Error creating full AI setup: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Behavior tree tools registered successfully")
