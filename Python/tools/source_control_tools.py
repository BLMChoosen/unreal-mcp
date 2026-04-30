"""
Git, Perforce, and binary asset conflict helpers for Unreal MCP.
"""

import hashlib
import logging
import os
import subprocess
from pathlib import Path
from mcp.server.fastmcp import FastMCP, Context

logger = logging.getLogger("UnrealMCP")


def _repo_root() -> Path:
    return Path(__file__).resolve().parents[2]


def _run(args: list[str], cwd: str | None = None) -> dict:
    try:
        completed = subprocess.run(
            args,
            cwd=cwd or str(_repo_root()),
            capture_output=True,
            text=True,
            timeout=60,
            check=False,
        )
        return {
            "success": completed.returncode == 0,
            "returncode": completed.returncode,
            "stdout": completed.stdout,
            "stderr": completed.stderr,
            "command": args,
        }
    except Exception as e:
        logger.error(f"Source control command failed: {e}")
        return {"success": False, "message": str(e), "command": args}


def _resolve_path(path: str, cwd: str | None = None) -> Path:
    base = Path(cwd) if cwd else _repo_root()
    candidate = Path(path)
    return candidate if candidate.is_absolute() else base / candidate


def register_source_control_tools(mcp: FastMCP):
    """Register source control tools with the MCP server."""

    @mcp.tool()
    def git_status(ctx: Context, cwd: str = "") -> dict:
        """Return porcelain and branch status for the Git repository."""
        return _run(["git", "status", "--short", "--branch"], cwd or str(_repo_root()))

    @mcp.tool()
    def git_diff(
        ctx: Context,
        paths: list = None,
        staged: bool = False,
        cwd: str = "",
    ) -> dict:
        """Return Git diff text for selected paths."""
        args = ["git", "diff"]
        if staged:
            args.append("--staged")
        args.extend(["--"] + (paths or []))
        return _run(args, cwd or str(_repo_root()))

    @mcp.tool()
    def git_commit_changes(
        ctx: Context,
        message: str,
        paths: list = None,
        stage_all: bool = False,
        cwd: str = "",
    ) -> dict:
        """Stage selected files or all changes and create a Git commit."""
        workdir = cwd or str(_repo_root())
        if stage_all:
            add_result = _run(["git", "add", "-A"], workdir)
        elif paths:
            add_result = _run(["git", "add", "--"] + paths, workdir)
        else:
            return {
                "success": False,
                "message": "Provide paths or set stage_all=True before committing.",
            }
        if not add_result.get("success"):
            return add_result
        commit_result = _run(["git", "commit", "-m", message], workdir)
        commit_result["staged"] = add_result
        return commit_result

    @mcp.tool()
    def p4_status(ctx: Context, cwd: str = "") -> dict:
        """Return Perforce opened files for the current workspace."""
        return _run(["p4", "opened"], cwd or str(_repo_root()))

    @mcp.tool()
    def p4_submit(
        ctx: Context,
        description: str,
        cwd: str = "",
    ) -> dict:
        """Submit the current Perforce changelist with a description."""
        return _run(["p4", "submit", "-d", description], cwd or str(_repo_root()))

    @mcp.tool()
    def explain_uasset_conflict(ctx: Context, file_path: str, cwd: str = "") -> dict:
        """Summarize a binary .uasset conflict using Git stages, sizes, and hashes."""
        try:
            workdir = cwd or str(_repo_root())
            resolved = _resolve_path(file_path, workdir)
            info = {
                "file_path": str(resolved),
                "exists": resolved.exists(),
                "size_bytes": resolved.stat().st_size if resolved.exists() else 0,
                "sha256": "",
                "git_stages": "",
            }
            if resolved.exists():
                hasher = hashlib.sha256()
                with resolved.open("rb") as handle:
                    for chunk in iter(lambda: handle.read(1024 * 1024), b""):
                        hasher.update(chunk)
                info["sha256"] = hasher.hexdigest()

            rel = os.path.relpath(resolved, workdir)
            stages = _run(["git", "ls-files", "-u", "--", rel], workdir)
            info["git_stages"] = stages.get("stdout", "")
            info["interpretation"] = (
                "This is a binary Unreal asset. Use the listed Git stages to identify base/ours/theirs, "
                "then open the competing assets in Unreal or restore one side; text merge is not reliable."
            )
            return {"success": True, "data": info}
        except Exception as e:
            logger.error(f"Error explaining uasset conflict: {e}")
            return {"success": False, "message": str(e)}

    logger.info("Source control tools registered successfully")
