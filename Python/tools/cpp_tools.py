"""
C++ class wizard and Live Coding helpers for Unreal MCP.
"""

import logging
import re
from pathlib import Path
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


PARENT_CLASSES = {
    "Actor": ("AActor", "GameFramework/Actor.h"),
    "Pawn": ("APawn", "GameFramework/Pawn.h"),
    "Character": ("ACharacter", "GameFramework/Character.h"),
    "PlayerController": ("APlayerController", "GameFramework/PlayerController.h"),
    "ActorComponent": ("UActorComponent", "Components/ActorComponent.h"),
    "SceneComponent": ("USceneComponent", "Components/SceneComponent.h"),
    "Object": ("UObject", "UObject/Object.h"),
}


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def _project_root(project_root: str = "") -> Path:
    return Path(project_root) if project_root else _repo_root() / "MCPGameProject"


def _send(command: str, params: dict) -> dict:
    from unreal_mcp_server import get_unreal_connection

    unreal = get_unreal_connection()
    if not unreal:
        return {"success": False, "message": "Failed to connect to Unreal Engine"}
    response = unreal.send_command(command, params)
    return response or {"success": False, "message": "No response"}


def _module_name(project_root: Path, module_name: str = "") -> str:
    if module_name:
        return module_name
    source_dir = project_root / "Source"
    if source_dir.exists():
        for child in source_dir.iterdir():
            if child.is_dir() and (child / f"{child.name}.Build.cs").exists():
                return child.name
    return project_root.stem


def _validate_class_name(class_name: str) -> bool:
    return bool(re.match(r"^[A-Z][A-Za-z0-9_]*$", class_name))


def register_cpp_tools(mcp: FastMCP):
    """Register C++ automation tools with the MCP server."""

    @mcp.tool()
    def create_unreal_cpp_class(
        ctx: Context,
        class_name: str,
        parent_class: str = "Actor",
        module_name: str = "",
        project_root: str = "",
        public: bool = True,
    ) -> dict:
        """Create Unreal C++ .h/.cpp boilerplate similar to Add C++ Class."""
        try:
            if not _validate_class_name(class_name):
                return {
                    "success": False,
                    "message": "class_name must start with an uppercase letter and contain only letters, numbers, or underscores.",
                }

            project = _project_root(project_root)
            module = _module_name(project, module_name)
            superclass, include_path = PARENT_CLASSES.get(parent_class, (parent_class, "CoreMinimal.h"))
            api_macro = f"{module.upper()}_API"
            source_root = project / "Source" / module
            header_dir = source_root / ("Public" if public else "Private")
            cpp_dir = source_root / "Private"
            header_dir.mkdir(parents=True, exist_ok=True)
            cpp_dir.mkdir(parents=True, exist_ok=True)

            prefix = "U" if superclass.startswith("U") else "A"
            unreal_class_name = class_name if class_name.startswith(("A", "U")) else f"{prefix}{class_name}"
            generated_base = unreal_class_name[1:] if unreal_class_name[:1] in ("A", "U") else unreal_class_name
            header_path = header_dir / f"{generated_base}.h"
            cpp_path = cpp_dir / f"{generated_base}.cpp"
            if header_path.exists() or cpp_path.exists():
                return {"success": False, "message": f"Class files already exist for {generated_base}"}

            header = f"""#pragma once

#include "CoreMinimal.h"
#include "{include_path}"
#include "{generated_base}.generated.h"

UCLASS()
class {api_macro} {unreal_class_name} : public {superclass}
{{
    GENERATED_BODY()

public:
    {unreal_class_name}();
}};
"""
            cpp = f"""#include "{generated_base}.h"

{unreal_class_name}::{unreal_class_name}()
{{
}}
"""
            header_path.write_text(header, encoding="utf-8")
            cpp_path.write_text(cpp, encoding="utf-8")
            return {
                "success": True,
                "data": {
                    "module": module,
                    "class_name": unreal_class_name,
                    "parent_class": superclass,
                    "header": str(header_path),
                    "source": str(cpp_path),
                },
            }
        except Exception as e:
            logger.error(f"Error creating C++ class: {e}")
            return {"success": False, "message": str(e)}

    @mcp.tool()
    def trigger_live_coding_compile(ctx: Context) -> dict:
        """Trigger Unreal Editor Live Coding compile."""
        try:
            return _send("trigger_live_coding_compile", {})
        except Exception as e:
            logger.error(f"Error triggering Live Coding: {e}")
            return {"success": False, "message": str(e)}

    logger.info("C++ tools registered successfully")
