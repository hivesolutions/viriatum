#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Builds the markdown table of the benchmark out of the reports that
the harness has written, one per pair of a workload and a server,
the result is meant to be appended to the step summary of the
workflow the same way the one of the coverage is.
"""

import os
import sys
import json

SUBJECT = "viriatum"
""" The name the server under test is recorded under, every other
server of a workload is a reference that it is reported against """

ORDER = ("viriatum", "nginx", "caddy", "openlitespeed", "haproxy", "pingora", "gunicorn", "uvicorn")
""" The order the servers are listed in, the subject first and then
the references, so that a table always reads the same way """


COMPARABLE = ("machine", "mode", "references", "connections", "threads", "workers")
""" The parts of an environment that decide whether two runs may be
compared at all, a figure taken on another machine or under another
shape of load describes that one and never this one """


def load(path):
    if not os.path.exists(path):
        return None
    try:
        with open(path, "rb") as file:
            return json.loads(file.read().decode("utf-8"))
    except ValueError:
        return None


def environment(output):
    # reads the description of the run that the harness wrote beside
    # its reports, one value per line, so that a figure may always be
    # traced back to the machine and the build that produced it
    path = os.path.join(output, "environment.txt")
    if not os.path.exists(path):
        return {}
    values = {}
    with open(path, "rb") as file:
        for line in file.read().decode("utf-8").splitlines():
            if ":" not in line:
                continue
            name, value = line.split(":", 1)
            values[name.strip()] = value.strip()
    return values


def stored(baseline):
    # the recorded runs and the environment they were taken under, a
    # baseline written before the environment was recorded carries the
    # rows at the top level and is still understood
    if isinstance(baseline, dict):
        return baseline.get("results", []), baseline.get("environment", {})
    return baseline or [], {}


def comparable(current, recorded):
    # says whether the run at hand may be compared against the one the
    # baseline holds, the parts that decide it being the machine and
    # the shape of the load rather than every value of either
    if not recorded:
        return True
    for name in COMPARABLE:
        if current.get(name) != recorded.get(name):
            return False
    return True


def differing(current, recorded):
    # the parts of the environment that keep the run at hand from being
    # compared against the one the baseline holds, named so that
    # whoever reads the report knows what has to be refreshed
    if not recorded:
        return []
    return [name for name in COMPARABLE if current.get(name) != recorded.get(name)]


def describe(values, names):
    # writes the named parts of an environment out, so that a mismatch
    # may be read without the baseline having to be opened beside it
    return ", ".join("%s %s" % (name, values.get(name, "?")) for name in names)


def order(output):
    # the order the harness drove the workloads in, which it wrote out
    # before driving any of them, the reports themselves are named
    # files and reading a directory hands them back in the order of
    # the alphabet rather than the order they were measured in
    path = os.path.join(output, "workloads.txt")
    if not os.path.exists(path):
        return []
    names = []
    with open(path, "rb") as file:
        for line in file.read().decode("utf-8").splitlines():
            name = line.split("|")[0].strip()
            if name:
                names.append(name)
    return names


def results(output):
    # gathers one entry per pair of a workload and a server, each of
    # them written by the harness as a report of its own, the ones
    # that could not be parsed are left out rather than failing
    rows = []
    runs = os.path.join(output, "runs")
    if not os.path.isdir(runs):
        return rows
    for name in sorted(os.listdir(runs)):
        if not name.endswith(".json"):
            continue
        item = load(os.path.join(runs, name))
        if item is None:
            continue
        rows.append(item)

    # the rows are handed back in the order the workloads were driven,
    # one the harness recorded, so that the table reads the way the
    # run did rather than the way the alphabet does
    driven = order(output)
    if driven:
        rows.sort(
            key=lambda item: driven.index(item["workload"])
            if item["workload"] in driven
            else len(driven)
        )
    return rows


def index(rows):
    # maps every entry by the pair that identifies it, which is what
    # both the ratio against a reference and the comparison against
    # the baseline are looked up through
    return dict(((item["workload"], item["server"]), item) for item in rows)


def workloads(rows):
    # the workloads in the order the harness drove them, a set would
    # lose that order and the table would stop being readable
    names = []
    for item in rows:
        if item["workload"] not in names:
            names.append(item["workload"])
    return names


def servers(rows, workload):
    # the servers of a workload, the subject first and the references
    # after it, one that the order does not know about is kept last
    # so that a reference added later still shows up in the table
    names = [item["server"] for item in rows if item["workload"] == workload]
    return sorted(
        names, key=lambda name: (ORDER.index(name) if name in ORDER else len(ORDER), name)
    )


def ratio(subject, reference):
    # the figure of the subject against the one of the reference, the
    # only number of the report that survives a noisy machine, as the
    # two of them were measured on it one right after the other, a
    # measurement that was not a figure is never one half of a ratio
    if reference is None or not reference.get("rps"):
        return None
    if subject is None or not subject.get("rps"):
        return None
    if not reference.get("valid", True) or not subject.get("valid", True):
        return None
    return subject["rps"] / reference["rps"]


def moved(current, baseline):
    # says whether a figure has moved beyond the spread the baseline
    # was recorded with, a run inside that spread is the same run and
    # the difference of it is the noise of the machine and nothing else
    if baseline is None or current is None:
        return None
    if not current.get("valid", True):
        return None
    low = baseline.get("rps_low", 0.0)
    high = baseline.get("rps_high", 0.0)
    if not low or not high:
        return None
    if current["rps"] < low:
        return "slower"
    if current["rps"] > high:
        return "faster"
    return None


def delta(current, baseline):
    # the change of a figure against the baseline, as a share of it,
    # so that the workloads may be read against one another
    if baseline is None or current is None or not baseline.get("rps"):
        return None
    return (current["rps"] - baseline["rps"]) * 100.0 / baseline["rps"]


def marker(item, state):
    # the mark that closes a row, a measurement that lost its
    # connections is never a figure at all and says so, one that moved
    # beyond the spread of the baseline is pointed at either way it went
    if not item.get("valid", True):
        return "⚠️"
    if state == "slower":
        return "🔻"
    if state == "faster":
        return "🔺"
    return ""


def format_rps(value):
    return "%.0f" % value if value else "-"


def format_latency(value):
    # the tail is recorded in microseconds and read in milliseconds,
    # a value of zero stands for a run no corrected figure was taken on
    return "%.2f" % (value / 1000.0) if value else "-"


def table(rows, baseline):
    # formats one row per pair of a workload and a server, the subject
    # of a workload carrying the ratio against each of its references
    # and every row carrying the change against the baseline
    known = index(rows)
    recorded = index(baseline) if baseline else {}

    lines = [
        "| Workload | Server | Req/s | p50 | p99 | p99.9 | CPU ms/1k | RSS MB | vs base | |",
        "| --- | --- | ---: | ---: | ---: | ---: | ---: | ---: | ---: | :-: |",
    ]

    for workload in workloads(rows):
        subject = known.get((workload, SUBJECT))
        for server in servers(rows, workload):
            item = known[(workload, server)]
            valid = item.get("valid", True)
            change = delta(item, recorded.get((workload, server)))
            state = moved(item, recorded.get((workload, server)))

            # the subject is reported against the references of its
            # workload, a reference against the subject, so that the
            # ratio reads the same way whichever row it sits on
            if server == SUBJECT:
                against = [
                    "%s %.2fx" % (name, ratio(subject, known.get((workload, name))))
                    for name in servers(rows, workload)
                    if name != SUBJECT and ratio(subject, known.get((workload, name)))
                ]
                name = "**%s**" % server
                if against:
                    name = "%s <br><sub>%s</sub>" % (name, ", ".join(against))
            else:
                name = server

            lines.append(
                "| `%s` | %s | %s | %s | %s | %s | %s | %s | %s | %s |"
                % (
                    workload,
                    name,
                    format_rps(item.get("rps")) if valid else "-",
                    format_latency(item.get("latency_p50_us")),
                    format_latency(item.get("latency_p99_us")),
                    format_latency(item.get("latency_p999_us")),
                    "%.1f" % item["cpu_ms_per_k"] if valid else "-",
                    "%.1f" % (item.get("peak_rss_kb", 0) / 1024.0) if valid else "-",
                    "-" if change is None else "%+.1f%%" % change,
                    marker(item, state),
                )
            )

    return lines


def main_with(output, path):
    rows = results(output)
    if not rows:
        print("## Benchmark\n\nNo benchmark data was produced.")
        return 1

    current = environment(output)
    recorded, taken = stored(load(path)) if path else ([], {})
    carried = bool(recorded)

    # the baseline of another machine describes that machine, marking
    # every row of this one as having moved against it would only ever
    # teach whoever reads the table to stop reading the marks
    matched = comparable(current, taken)
    if not matched:
        recorded = []

    print("## Benchmark\n")
    print("\n".join(table(rows, recorded)))

    if not path:
        print("\nNo baseline was compared against, the change column is empty.")
    elif not recorded:
        names = differing(current, taken)
        if carried and names:
            print(
                "\nThe baseline was recorded with %s and this ran with %s, so nothing "
                "was compared against it, refresh the baseline through the input of "
                "the workflow to compare against this shape."
                % (describe(taken, names), describe(current, names))
            )
        else:
            print("\nThe baseline carries no results, the change column is empty.")

    # the environment a figure came out of is part of the figure, a
    # run on another machine or another build is never comparable and
    # the report says which one produced it rather than implying it
    if current:
        print("\n<details><summary>Environment</summary>\n")
        print("```")
        for name in sorted(current):
            print("%s: %s" % (name, current[name]))
        print("```")
        print("\n</details>")

    # the assembled result is written beside the table together with
    # the environment it was taken under, so that a later run is able
    # to tell whether it may be compared against this one at all
    with open(os.path.join(output, "results.json"), "wb") as file:
        file.write(
            json.dumps(
                dict(environment=current, results=rows), indent=2, sort_keys=True
            ).encode("utf-8")
        )

    # the run reports and never gates, a hosted runner is far too
    # noisy for a figure of performance to fail a build on
    return 0


def main():
    return main_with(sys.argv[1], sys.argv[2] if len(sys.argv) > 2 else None)


if __name__ == "__main__":
    sys.exit(main())
