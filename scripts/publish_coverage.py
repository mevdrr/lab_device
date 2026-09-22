"""Publish measured coverage to a separate Git branch without modifying source files."""
import json
import os
from pathlib import Path
import subprocess

BRANCH = "coverage-variant-12"


def git(*args, data=None):
    return subprocess.check_output(["git", *args], input=data).decode("utf-8").strip()


def main():
    if os.environ.get("GITHUB_EVENT_NAME") != "push":
        raise RuntimeError("Coverage publication is allowed only on push")
    if os.environ.get("GITHUB_REPOSITORY") != "mevdrr/lab_device":
        raise RuntimeError("Only the student fork publishes this badge")
    if os.environ.get("GITHUB_REF") not in {"refs/heads/variant-12-recycle", "refs/heads/main"}:
        raise RuntimeError("Unexpected source branch")
    source = os.environ["GITHUB_SHA"]
    summary = json.loads(Path("coverage/summary.json").read_text(encoding="utf-8"))
    note = (
        "# Coverage of variant 12\n\n"
        f"Source commit: {source}\n\n"
        f"Lines: {summary['line_covered']}/{summary['line_total']}\n\n"
        "Generated from GoogleTest execution by GitHub Actions. "
        "HTML report and JUnit results are attached to the workflow run.\n"
    )
    files = {
        "coverage.svg": Path("coverage/coverage.svg").read_bytes(),
        "summary.json": Path("coverage/summary.json").read_bytes(),
        "README.md": note.encode("utf-8"),
    }
    entries = []
    for name, content in sorted(files.items()):
        blob = git("hash-object", "-w", "--stdin", data=content)
        entries.append(f"100644 blob {blob}\t{name}\n")
    tree = git("mktree", data="".join(entries).encode("utf-8"))
    parent = git("ls-remote", "--heads", "origin", f"refs/heads/{BRANCH}")
    args = ["commit-tree", tree]
    if parent:
        git("fetch", "--no-tags", "origin", f"refs/heads/{BRANCH}")
        args.extend(["-p", git("rev-parse", "FETCH_HEAD")])
    commit = git(*args, data=f"Update measured coverage for {source}\n".encode("utf-8"))
    # No force push: concurrent updates must not overwrite another published report.
    subprocess.run(["git", "push", "origin", f"{commit}:refs/heads/{BRANCH}"], check=True)


if __name__ == "__main__":
    main()
