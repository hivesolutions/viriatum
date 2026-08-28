#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Builds the markdown table of the coverage out of the reports that
have been exported by the coverage script, the result is meant to
be appended to the step summary of the workflow.
"""

import os
import sys
import json


def load(path):
    if not os.path.exists(path):
        return None
    try:
        with open(path, "rb") as file:
            return json.loads(file.read().decode("utf-8"))
    except ValueError:
        return None


def rows_native(data, root):
    # gathers one row per source file of the native extension, the
    # names are relative so that the table stays readable
    rows = []
    for item in data["data"][0].get("files", []):
        summary = item["summary"]["lines"]
        name = os.path.relpath(item["filename"], root)
        rows.append((name, summary["count"], summary["count"] - summary["covered"]))
    return sorted(rows)


def rows_python(data, root):
    # gathers one row per module of the pure python surface, the
    # statements are the equivalent of the lines of the native one
    rows = []
    for name, item in sorted(data.get("files", {}).items()):
        summary = item["summary"]
        rows.append(
            (
                os.path.relpath(os.path.join(root, name), root),
                summary["num_statements"],
                summary["missing_lines"],
            )
        )
    return rows


def table(rows, threshold):
    # formats the rows as a markdown table, the files that fall below
    # the threshold are marked so that they stand out
    lines = [
        "| File | Lines | Missed | Cover | |",
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


def main():
    output = sys.argv[1]
    threshold = float(sys.argv[2]) if len(sys.argv) > 2 else 90.0
    root = os.path.abspath(os.path.join(os.path.dirname(__file__), ".."))

    rows = []
    native = load(os.path.join(output, "native.json"))
    if native:
        rows += rows_native(native, root)
    python = load(os.path.join(output, "python.json"))
    if python:
        rows += rows_python(python, root)

    if not rows:
        print("## Coverage\n\nNo coverage data was produced.")
        return 0

    lines, cover = table(rows, threshold)
    print("## Coverage\n")
    print("\n".join(lines))
    print(
        "\nThreshold is %.0f%%, the overall coverage of the measured sources is %.2f%%."
        % (threshold, cover)
    )
    return 0


if __name__ == "__main__":
    sys.exit(main())
