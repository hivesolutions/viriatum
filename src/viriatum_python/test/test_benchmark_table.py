#!/usr/bin/python
# -*- coding: utf-8 -*-

"""
Test suite for the builder of the benchmark table, driving it over
reports written the way the harness writes them so that the reading
of the reports, the ratios against the references and the comparison
of a run against the recorded baseline are all exercised.

Each test builds the reports it needs under a directory of its own
and takes it down again afterwards, copying and adjusting the two
module level reports that stand for a subject and for a reference, so
that no test is ever able to see what another one wrote.

Run from the project root with:
    python -m unittest discover -s src/viriatum_python/test
"""

from contextlib import redirect_stdout
from io import StringIO
from json import dumps, loads
from os import makedirs
from os.path import abspath, dirname, join
from shutil import rmtree
from sys import argv, path
from tempfile import mkdtemp
from typing import Any
from unittest import TestCase, main

path.insert(0, join(dirname(abspath(__file__)), "..", "..", "..", "scripts"))

import benchmark_table

SUBJECT = {
    "workload": "static-small-alive",
    "server": "viriatum",
    "version": "0.5.0",
    "valid": True,
    "rps": 1000.0,
    "rps_low": 950.0,
    "rps_high": 1050.0,
    "transfer_bps": 1000.0,
    "errors_socket": 0,
    "errors_status": 0,
    "peak_rss_kb": 2048,
    "cpu_ms_per_k": 40.0,
    "latency_corrected": True,
    "latency_p50_us": 1500,
    "latency_p99_us": 9000,
    "latency_p999_us": 20000,
    "accept_rps": 500.0,
    "requests_per_connection": 100.0,
    "syscalls_per_request": 12.0,
}
""" The report of a single pair as the harness writes it, the tests
below build the cases they need by copying and adjusting it """

REFERENCE = dict(SUBJECT, server="nginx", rps=2000.0, rps_low=1900.0, rps_high=2100.0)
""" The counterpart of the report above for a reference, twice as
fast so that a ratio of one half is the expected one """


class BenchmarkTableTest(TestCase):
    """
    Test suite for the builder of the benchmark table, exercising the
    reading of the reports, the ratios against the references and the
    comparison of a run against the recorded baseline.
    """

    def setUp(self) -> None:
        self.output = mkdtemp()
        makedirs(join(self.output, "runs"))

    def tearDown(self) -> None:
        rmtree(self.output, ignore_errors=True)

    def _write(self, item: dict[str, Any], name: str | None = None) -> str:
        # writes a report the way the harness does, one file per pair
        # of a workload and a server, named after the two of them
        name = name or "%s.%s.json" % (item["workload"], item["server"])
        path = join(self.output, "runs", name)
        with open(path, "wb") as file:
            file.write(dumps(item).encode("utf-8"))
        return path

    def test_load(self) -> None:
        path = self._write(SUBJECT)
        self.assertEqual(benchmark_table.load(path)["server"], "viriatum")
        self.assertEqual(benchmark_table.load(join(self.output, "gone")), None)

    def test_load_invalid(self) -> None:
        # a report that was truncated by a run that never finished is
        # skipped rather than taking the complete report down with it
        path = join(self.output, "runs", "broken.json")
        with open(path, "wb") as file:
            file.write(b"{ this is not json")
        self.assertEqual(benchmark_table.load(path), None)

    def test_environment(self) -> None:
        with open(join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Linux x86_64\nflags: -O3 -DNDEBUG\nbroken line\n")
        values = benchmark_table.environment(self.output)
        self.assertEqual(values["machine"], "Linux x86_64")
        self.assertEqual(values["flags"], "-O3 -DNDEBUG")
        self.assertEqual(len(values), 2)

    def test_environment_missing(self) -> None:
        self.assertEqual(benchmark_table.environment(self.output), {})

    def test_stored(self) -> None:
        # the baseline carries the runs together with the machine they
        # were taken on, one written before that was recorded carries
        # only the runs and is still understood
        rows, taken = benchmark_table.stored(
            dict(results=[SUBJECT], environment=dict(machine="Linux"))
        )
        self.assertEqual(len(rows), 1)
        self.assertEqual(taken["machine"], "Linux")

        rows, taken = benchmark_table.stored([SUBJECT])
        self.assertEqual(len(rows), 1)
        self.assertEqual(taken, {})

        self.assertEqual(benchmark_table.stored(None), ([], {}))

    def test_comparable(self) -> None:
        # a run of the same machine and the same shape of load may be
        # compared against the baseline, one of another machine is
        # describing that machine and never this one
        recorded = dict(machine="Linux", mode="native", connections="64")
        self.assertEqual(benchmark_table.comparable(dict(recorded), recorded), True)
        self.assertEqual(
            benchmark_table.comparable(dict(recorded, machine="Darwin"), recorded),
            False,
        )
        self.assertEqual(
            benchmark_table.comparable(dict(recorded, connections="128"), recorded),
            False,
        )

    def test_comparable_unrecorded(self) -> None:
        # a baseline that never recorded where it was taken is compared
        # against, there is nothing to say it should not be
        self.assertEqual(benchmark_table.comparable(dict(machine="Linux"), {}), True)

    def test_differing(self) -> None:
        # the parts that do not meet are the ones named, and only
        # those, a part that is the same in both is never pointed at
        recorded = dict(machine="Linux", mode="native", connections="64", threads="4")
        self.assertEqual(benchmark_table.differing(dict(recorded), recorded), [])
        self.assertEqual(
            benchmark_table.differing(dict(recorded, connections="256"), recorded),
            ["connections"],
        )
        self.assertEqual(
            benchmark_table.differing(
                dict(recorded, connections="256", threads="8"), recorded
            ),
            ["connections", "threads"],
        )

    def test_differing_unrecorded(self) -> None:
        # a baseline that never recorded where it was taken has nothing
        # to differ on, the run is compared against it as it is
        self.assertEqual(benchmark_table.differing(dict(machine="Linux"), {}), [])

    def test_describe(self) -> None:
        # the named parts are written out in the order they are given
        # and a part the environment never carried is still named
        values = dict(connections="64", threads="4")
        self.assertEqual(
            benchmark_table.describe(values, ["connections", "threads"]),
            "connections 64, threads 4",
        )
        self.assertEqual(benchmark_table.describe(values, ["workers"]), "workers ?")
        self.assertEqual(benchmark_table.describe(values, []), "")

    def test_order(self) -> None:
        with open(join(self.output, "workloads.txt"), "wb") as file:
            file.write(
                b"static-small-alive|static|file|/small.html||On|\nproxy-alive|proxy|\n"
            )
        self.assertEqual(
            benchmark_table.order(self.output), ["static-small-alive", "proxy-alive"]
        )

    def test_order_missing(self) -> None:
        self.assertEqual(benchmark_table.order(self.output), [])

    def test_results(self) -> None:
        self._write(SUBJECT)
        self._write(REFERENCE)
        rows = benchmark_table.results(self.output)
        self.assertEqual(len(rows), 2)

    def test_results_driven_order(self) -> None:
        # the reports are named files and reading a directory hands
        # them back alphabetically, the table is meant to read in the
        # order the workloads were actually driven in
        self._write(dict(SUBJECT, workload="proxy-alive"))
        self._write(SUBJECT)
        with open(join(self.output, "workloads.txt"), "wb") as file:
            file.write(b"static-small-alive|static|\nproxy-alive|proxy|\n")

        rows = benchmark_table.results(self.output)
        self.assertEqual(
            [item["workload"] for item in rows], ["static-small-alive", "proxy-alive"]
        )

    def test_results_missing(self) -> None:
        self.assertEqual(benchmark_table.results(join(self.output, "gone")), [])

    def test_results_skips_broken(self) -> None:
        # the run of a pair that never completed leaves a report that
        # cannot be read, the table is still built out of the rest
        self._write(SUBJECT)
        with open(join(self.output, "runs", "broken.json"), "wb") as file:
            file.write(b"{")
        with open(join(self.output, "runs", "notes.txt"), "wb") as file:
            file.write(b"ignored")
        rows = benchmark_table.results(self.output)
        self.assertEqual(len(rows), 1)

    def test_index(self) -> None:
        known = benchmark_table.index([SUBJECT, REFERENCE])
        self.assertEqual(known[("static-small-alive", "nginx")]["rps"], 2000.0)

    def test_workloads(self) -> None:
        # the order the harness drove the workloads in is the order
        # they are listed in, and a workload shows up exactly once
        other = dict(SUBJECT, workload="proxy-alive")
        names = benchmark_table.workloads([SUBJECT, REFERENCE, other])
        self.assertEqual(names, ["static-small-alive", "proxy-alive"])

    def test_servers(self) -> None:
        # the subject leads and the references follow in the recorded
        # order, which is the one the table is meant to read in
        rows = [REFERENCE, SUBJECT, dict(SUBJECT, server="caddy")]
        names = benchmark_table.servers(rows, "static-small-alive")
        self.assertEqual(names, ["viriatum", "nginx", "caddy"])

    def test_servers_unknown(self) -> None:
        # a reference that the recorded order knows nothing about is
        # kept last rather than being dropped from the table
        rows = [SUBJECT, dict(SUBJECT, server="lighttpd")]
        names = benchmark_table.servers(rows, "static-small-alive")
        self.assertEqual(names, ["viriatum", "lighttpd"])

    def test_ratio(self) -> None:
        self.assertEqual(benchmark_table.ratio(SUBJECT, REFERENCE), 0.5)

    def test_ratio_invalid(self) -> None:
        # a measurement that was discarded is never one half of a
        # ratio, a server that served nothing at all would otherwise
        # read as one the subject is infinitely faster than
        invalid = dict(REFERENCE, valid=False)
        self.assertEqual(benchmark_table.ratio(SUBJECT, invalid), None)
        self.assertEqual(
            benchmark_table.ratio(dict(SUBJECT, valid=False), REFERENCE), None
        )

    def test_ratio_missing(self) -> None:
        # a reference that never ran carries no figure and so no ratio
        # is reported against it, which is not the same as a ratio of
        # zero and must never be printed as one
        self.assertEqual(benchmark_table.ratio(SUBJECT, None), None)
        self.assertEqual(benchmark_table.ratio(None, REFERENCE), None)
        self.assertEqual(benchmark_table.ratio(SUBJECT, dict(REFERENCE, rps=0.0)), None)

    def test_moved(self) -> None:
        # a figure inside the spread the baseline was recorded with is
        # the very same figure, only one outside of it has moved
        self.assertEqual(
            benchmark_table.moved(dict(SUBJECT, rps=1000.0), SUBJECT), None
        )
        self.assertEqual(
            benchmark_table.moved(dict(SUBJECT, rps=900.0), SUBJECT), "slower"
        )
        self.assertEqual(
            benchmark_table.moved(dict(SUBJECT, rps=1100.0), SUBJECT), "faster"
        )

    def test_moved_edges(self) -> None:
        # the bounds themselves are inside the spread, a figure that
        # lands exactly on one of them has not moved anywhere
        self.assertEqual(benchmark_table.moved(dict(SUBJECT, rps=950.0), SUBJECT), None)
        self.assertEqual(
            benchmark_table.moved(dict(SUBJECT, rps=1050.0), SUBJECT), None
        )

    def test_moved_invalid(self) -> None:
        # a measurement that lost its connections is never compared
        # against anything, the figure of it is not a figure at all
        current = dict(SUBJECT, rps=10.0, valid=False)
        self.assertEqual(benchmark_table.moved(current, SUBJECT), None)

    def test_moved_missing(self) -> None:
        self.assertEqual(benchmark_table.moved(SUBJECT, None), None)
        self.assertEqual(benchmark_table.moved(None, SUBJECT), None)
        self.assertEqual(
            benchmark_table.moved(SUBJECT, dict(SUBJECT, rps_low=0.0, rps_high=0.0)),
            None,
        )

    def test_delta(self) -> None:
        self.assertAlmostEqual(
            benchmark_table.delta(dict(SUBJECT, rps=1100.0), SUBJECT), 10.0
        )
        self.assertAlmostEqual(
            benchmark_table.delta(dict(SUBJECT, rps=900.0), SUBJECT), -10.0
        )

    def test_delta_missing(self) -> None:
        self.assertEqual(benchmark_table.delta(SUBJECT, None), None)
        self.assertEqual(benchmark_table.delta(SUBJECT, dict(SUBJECT, rps=0.0)), None)

    def test_marker(self) -> None:
        self.assertEqual(benchmark_table.marker(SUBJECT, None), "")
        self.assertEqual(benchmark_table.marker(SUBJECT, "slower"), "🔻")
        self.assertEqual(benchmark_table.marker(SUBJECT, "faster"), "🔺")

    def test_marker_invalid(self) -> None:
        # the mark of a measurement that is not a figure wins over the
        # one of a move, a run that lost its connections looks faster
        # than it is and must never be reported as an improvement
        item = dict(SUBJECT, valid=False)
        self.assertEqual(benchmark_table.marker(item, "faster"), "⚠️")

    def test_format_rps(self) -> None:
        self.assertEqual(benchmark_table.format_rps(1234.6), "1235")
        self.assertEqual(benchmark_table.format_rps(0), "-")
        self.assertEqual(benchmark_table.format_rps(None), "-")

    def test_format_latency(self) -> None:
        # the tail is recorded in microseconds and read in the
        # milliseconds a person is able to reason about
        self.assertEqual(benchmark_table.format_latency(1500), "1.50")
        self.assertEqual(benchmark_table.format_latency(0), "-")

    def test_format_cpu(self) -> None:
        # the processor time of a container is never asked of it, a
        # zero stands for a run it was not taken on rather than for a
        # server that consumed nothing at all while it served
        self.assertEqual(benchmark_table.format_cpu(40.0), "40.0")
        self.assertEqual(benchmark_table.format_cpu(0), "-")
        self.assertEqual(benchmark_table.format_cpu(None), "-")

    def test_table(self) -> None:
        lines = benchmark_table.table([SUBJECT, REFERENCE], None)
        self.assertEqual(len(lines), 4)
        self.assertTrue("**viriatum**" in lines[2])
        self.assertTrue("nginx 0.50x" in lines[2])
        self.assertTrue("1.50" in lines[2])
        self.assertTrue("| nginx |" in lines[3])

    def test_table_baseline(self) -> None:
        # the change against the baseline shows up on every row and
        # the mark only on the ones that left the recorded spread
        current = dict(SUBJECT, rps=800.0)
        lines = benchmark_table.table([current], [SUBJECT])
        self.assertTrue("-20.0%" in lines[2])
        self.assertTrue("🔻" in lines[2])

    def test_table_discarded(self) -> None:
        # a measurement that is not a figure carries no figures at all,
        # the cost of a request divided by almost no requests is a
        # vast number and printing it would read as a real one
        item = dict(SUBJECT, valid=False, rps=3.0, cpu_ms_per_k=6030000.0)
        lines = benchmark_table.table([item], None)
        self.assertTrue("6030000" not in lines[2])
        self.assertTrue("1.50" not in lines[2])
        self.assertTrue("| - | - |" in lines[2])
        self.assertTrue("⚠️" in lines[2])

    def test_table_discarded_baseline(self) -> None:
        # the change of a discarded measurement against the baseline
        # describes the losing of the connections and never the
        # serving, a run that answered almost nothing is no regression
        # of the server and must never be reported as one
        item = dict(SUBJECT, valid=False, rps=3.0)
        lines = benchmark_table.table([item], [SUBJECT])
        self.assertTrue("%" not in lines[2])
        self.assertTrue("⚠️" in lines[2])

    def test_table_no_reference(self) -> None:
        # a workload the references were all skipped on still reports
        # the subject, without a ratio it has nothing to be against
        lines = benchmark_table.table([SUBJECT], None)
        self.assertEqual(len(lines), 3)
        self.assertTrue("<sub>" not in lines[2])

    def test_main(self) -> None:
        self._write(SUBJECT)
        self._write(REFERENCE)
        self.assertEqual(benchmark_table.main_with(self.output, None), 0)

        # the assembled result is written beside the table so that a
        # later run is able to compare against it without walking
        # every one of the reports again
        with open(join(self.output, "results.json"), "rb") as file:
            rows = loads(file.read().decode("utf-8"))
        self.assertEqual(sorted(rows), ["environment", "results"])
        self.assertEqual(len(rows["results"]), 2)

    def test_main_environment(self) -> None:
        # the machine and the build a figure came out of are part of
        # the figure, a run on another of either is never comparable
        # and the report says which one produced it
        self._write(SUBJECT)
        with open(join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Linux x86_64\ncompiler: gcc 14")

        stream = StringIO()
        with redirect_stdout(stream):
            self.assertEqual(benchmark_table.main_with(self.output, None), 0)

        self.assertTrue("machine: Linux x86_64" in stream.getvalue())
        self.assertTrue("<summary>Environment</summary>" in stream.getvalue())

    def test_main_baseline(self) -> None:
        # the baseline is read off the path it was handed and the
        # change against it lands on every row of the table
        self._write(dict(SUBJECT, rps=500.0))
        path = join(self.output, "baseline.json")
        with open(path, "wb") as file:
            file.write(dumps([SUBJECT]).encode("utf-8"))

        stream = StringIO()
        with redirect_stdout(stream):
            self.assertEqual(benchmark_table.main_with(self.output, path), 0)

        self.assertTrue("-50.0%" in stream.getvalue())

    def test_main_baseline_elsewhere(self) -> None:
        # a baseline recorded on another machine is never compared
        # against, marking every row as moved would only teach whoever
        # reads the table to stop reading the marks at all
        self._write(dict(SUBJECT, rps=500.0))
        with open(join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Darwin arm64\nmode: native")

        path = join(self.output, "baseline.json")
        with open(path, "wb") as file:
            file.write(
                dumps(
                    dict(
                        results=[SUBJECT],
                        environment=dict(machine="Linux x86_64", mode="native"),
                    )
                ).encode("utf-8")
            )

        stream = StringIO()
        with redirect_stdout(stream):
            self.assertEqual(benchmark_table.main_with(self.output, path), 0)

        self.assertTrue("-50.0%" not in stream.getvalue())
        self.assertTrue("so nothing was compared against it" in stream.getvalue())

    def test_main_baseline_shape(self) -> None:
        # a baseline taken under another shape of load is not compared
        # against either, and the report names the parts that do not
        # meet rather than the machine, which is the same one
        self._write(dict(SUBJECT, rps=500.0))
        with open(join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Linux x86_64\nconnections: 256\nthreads: 8")

        path = join(self.output, "baseline.json")
        with open(path, "wb") as file:
            file.write(
                dumps(
                    dict(
                        results=[SUBJECT],
                        environment=dict(
                            machine="Linux x86_64", connections="64", threads="4"
                        ),
                    )
                ).encode("utf-8")
            )

        stream = StringIO()
        with redirect_stdout(stream):
            self.assertEqual(benchmark_table.main_with(self.output, path), 0)

        value = stream.getvalue()
        self.assertTrue("-50.0%" not in value)
        self.assertTrue("connections 64, threads 4" in value)
        self.assertTrue("connections 256, threads 8" in value)
        self.assertTrue("refresh the baseline" in value)

    def test_main_baseline_barren(self) -> None:
        # a baseline taken under the very same shape and carrying no
        # results at all is not a mismatch, so the report says there is
        # nothing to compare against rather than naming a part
        self._write(SUBJECT)
        with open(join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Linux x86_64\nconnections: 64")

        path = join(self.output, "baseline.json")
        with open(path, "wb") as file:
            file.write(
                dumps(
                    dict(
                        results=[],
                        environment=dict(machine="Linux x86_64", connections="64"),
                    )
                ).encode("utf-8")
            )

        stream = StringIO()
        with redirect_stdout(stream):
            self.assertEqual(benchmark_table.main_with(self.output, path), 0)

        value = stream.getvalue()
        self.assertTrue("The baseline carries no results" in value)
        self.assertTrue("refresh the baseline" not in value)

    def test_main_baseline_barren_elsewhere(self) -> None:
        # a baseline that carries no results and was taken under
        # another shape is not a mismatch to be refreshed, there is
        # nothing in it to compare against under any shape at all
        self._write(SUBJECT)
        with open(join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Linux x86_64\nconnections: 256")

        path = join(self.output, "baseline.json")
        with open(path, "wb") as file:
            file.write(
                dumps(
                    dict(
                        results=[],
                        environment=dict(machine="Linux x86_64", connections="64"),
                    )
                ).encode("utf-8")
            )

        stream = StringIO()
        with redirect_stdout(stream):
            self.assertEqual(benchmark_table.main_with(self.output, path), 0)

        value = stream.getvalue()
        self.assertTrue("The baseline carries no results" in value)
        self.assertTrue("refresh the baseline" not in value)

    def test_main_empty(self) -> None:
        self.assertEqual(benchmark_table.main_with(self.output, None), 1)

    def test_main_arguments(self) -> None:
        # the entry point reads the output and the baseline off the
        # arguments, the baseline being the optional one of the two
        self._write(SUBJECT)
        arguments = list(argv)
        try:
            argv[:] = ["benchmark_table.py", self.output]
            self.assertEqual(benchmark_table.main(), 0)
        finally:
            argv[:] = arguments


if __name__ == "__main__":
    main()
