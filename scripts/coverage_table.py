#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Coverage table builder that turns the reports exported by the
coverage script into the markdown table that closes a run, meant to
be appended to the step summary of the workflow the same way the one
of the benchmark is.

Reads the export of the native tree and the one of the python surface
out of the output directory, collapses the sources into one row per
directory, and marks the ones that fall below the threshold the run
was held to.

Features:
    - Both toolchains understood, the export of llvm carrying the
      files under a data element and the one of gcovr at the top
      level, so that the table is the same one for either of them.
    - The native tree and the python surface reported together, the
      statements of the latter standing for the lines of the former.
    - One row per directory rather than per source, a tree of a few
      hundred sources not being readable a line at a time, the detail
      of each one staying in the artifact of the run.
    - The threshold applied to the native rows alone, the coverage
      script holding the python surface to one of its own.

Run from the project root with:
    python scripts/coverage_table.py <output> [threshold]

Arguments:
    output      The directory the coverage script wrote its reports into
    threshold   The percentage the native tree is held to, 90 by default
"""

from json import loads
from os.path import abspath, dirname, exists, join, relpath
from sys import argv, exit
from typing import Any

Row = tuple[str, int, int]
""" A single row of the table, the name of what is measured together
with the lines it carries and the ones that were never reached """


def load(path: str) -> Any | None:
    if not exists(path):
        return None
    try:
        with open(path, "rb") as file:
            return loads(file.read().decode("utf-8"))
    except ValueError:
        return None


def rows_native(data: dict[str, Any], root: str) -> list[Row]:
    # gathers one row per source file of the native tree, the export
    # of llvm carries the files under a data element while the one of
    # gcovr carries them at the top level, both are understood so that
    # the table is the same one for either of the toolchains
    rows = []
    if "data" in data:
        for item in data["data"][0].get("files", []):
            summary = item["summary"]["lines"]
            name = relpath(item["filename"], root)
            rows.append((name, summary["count"], summary["count"] - summary["covered"]))
    else:
        for item in data.get("files", []):
            name = relpath(join(root, item["filename"]), root)
            rows.append(
                (name, item["line_total"], item["line_total"] - item["line_covered"])
            )
    return sorted(rows)


def rows_python(data: dict[str, Any], root: str) -> list[Row]:
    # gathers one row per module of the pure python surface, the
    # statements are the equivalent of the lines of the native one
    rows = []
    for name, item in sorted(data.get("files", {}).items()):
        summary = item["summary"]
        rows.append(
            (
                relpath(join(root, name), root),
                summary["num_statements"],
                summary["missing_lines"],
            )
        )
    return rows


def group(rows: list[Row]) -> list[Row]:
    # collapses the rows into one per directory, the complete tree
    # carries a few hundred sources and a row for each of them would
    # not be readable, the per file detail stays in the artifact
    groups = {}
    for name, count, absent in rows:
        directory = dirname(name)
        total, missed = groups.get(directory, (0, 0))
        groups[directory] = (total + count, missed + absent)
    return [(name, groups[name][0], groups[name][1]) for name in sorted(groups)]


def coverage(rows: list[Row]) -> float:
    # calculates the overall coverage of the provided rows, an
    # empty set of rows is considered to be fully covered
    total = sum(count for _name, count, _absent in rows)
    missed = sum(absent for _name, _count, absent in rows)
    return 100.0 if total == 0 else (total - missed) * 100.0 / total


def table(rows: list[Row], threshold: float) -> tuple[list[str], float]:
    # formats the rows as a markdown table, the directories that fall
    # below the threshold are marked so that they stand out
    lines = [
        "| Directory | Lines | Missed | Cover | |",
        "| --- | ---: | ---: | ---: | :-: |",
    ]
    total = missed = 0
    for name, count, absent in rows:
        total += count
        missed += absent
        cover = 100.0 if count == 0 else (count - absent) * 100.0 / count
        lines.append(
            "| `%s` | %d | %d | %.2f%% | %s |"
            % (name, count, absent, cover, "✅" if cover >= threshold else "⚠️")
        )
    cover = 100.0 if total == 0 else (total - missed) * 100.0 / total
    lines.append(
        "| **total** | **%d** | **%d** | **%.2f%%** | %s |"
        % (total, missed, cover, "✅" if cover >= threshold else "⚠️")
    )
    return lines, cover


def main() -> int:
    output = argv[1]
    threshold = float(argv[2]) if len(argv) > 2 else 90.0
    root = abspath(join(dirname(__file__), ".."))

    native_rows = []
    native = load(join(output, "native.json"))
    if native:
        native_rows = rows_native(native, root)

    rows = list(native_rows)
    python = load(join(output, "python.json"))
    if python:
        rows += rows_python(python, root)

    if not rows:
        print("## Coverage\n\nNo coverage data was produced.")
        return 1

    lines, cover = table(group(rows), threshold)
    print("## Coverage\n")
    print("\n".join(lines))
    print(
        "\nThreshold is %.0f%%, the overall coverage of the measured sources is %.2f%%."
        % (threshold, cover)
    )

    # the threshold is only applied to the native rows, the coverage
    # script validates the python surface on its own and against a
    # threshold of its own, a well covered package may not make up
    # for a native tree that falls below the required value
    if not native_rows:
        return 1
    return 0 if coverage(native_rows) >= threshold else 1


if __name__ == "__main__":
    exit(main())
