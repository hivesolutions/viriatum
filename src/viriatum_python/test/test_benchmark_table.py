#!/usr/bin/python
# -*- coding: utf-8 -*-

import io
import os
import json
import shutil
import sys
import tempfile
import unittest

sys.path.insert(
    0,
    os.path.join(
        os.path.dirname(os.path.abspath(__file__)), "..", "..", "..", "scripts"
    ),
)

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


class BenchmarkTableTest(unittest.TestCase):
    """
    Test suite for the builder of the benchmark table, exercising the
    reading of the reports, the ratios against the references and the
    comparison of a run against the recorded baseline.
    """

    def setUp(self):
        self.output = tempfile.mkdtemp()
        os.makedirs(os.path.join(self.output, "runs"))

    def tearDown(self):
        shutil.rmtree(self.output, ignore_errors=True)

    def _write(self, item, name=None):
        # writes a report the way the harness does, one file per pair
        # of a workload and a server, named after the two of them
        name = name or "%s.%s.json" % (item["workload"], item["server"])
        path = os.path.join(self.output, "runs", name)
        with io.open(path, "wb") as file:
            file.write(json.dumps(item).encode("utf-8"))
        return path

    def test_load(self):
        path = self._write(SUBJECT)
        self.assertEqual(benchmark_table.load(path)["server"], "viriatum")
        self.assertEqual(benchmark_table.load(os.path.join(self.output, "gone")), None)

    def test_load_invalid(self):
        # a report that was truncated by a run that never finished is
        # skipped rather than taking the complete report down with it
        path = os.path.join(self.output, "runs", "broken.json")
        with io.open(path, "wb") as file:
            file.write(b"{ this is not json")
        self.assertEqual(benchmark_table.load(path), None)

    def test_environment(self):
        with io.open(os.path.join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Linux x86_64\nflags: -O3 -DNDEBUG\nbroken line\n")
        values = benchmark_table.environment(self.output)
        self.assertEqual(values["machine"], "Linux x86_64")
        self.assertEqual(values["flags"], "-O3 -DNDEBUG")
        self.assertEqual(len(values), 2)

    def test_environment_missing(self):
        self.assertEqual(benchmark_table.environment(self.output), {})

    def test_stored(self):
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

    def test_comparable(self):
        # a run of the same machine and the same shape of load may be
        # compared against the baseline, one of another machine is
        # describing that machine and never this one
        recorded = dict(machine="Linux", mode="native", connections="64")
        self.assertEqual(benchmark_table.comparable(dict(recorded), recorded), True)
        self.assertEqual(
            benchmark_table.comparable(dict(recorded, machine="Darwin"), recorded), False
        )
        self.assertEqual(
            benchmark_table.comparable(dict(recorded, connections="128"), recorded), False
        )

    def test_comparable_unrecorded(self):
        # a baseline that never recorded where it was taken is compared
        # against, there is nothing to say it should not be
        self.assertEqual(benchmark_table.comparable(dict(machine="Linux"), {}), True)

    def test_results(self):
        self._write(SUBJECT)
        self._write(REFERENCE)
        rows = benchmark_table.results(self.output)
        self.assertEqual(len(rows), 2)

    def test_results_missing(self):
        self.assertEqual(benchmark_table.results(os.path.join(self.output, "gone")), [])

    def test_results_skips_broken(self):
        # the run of a pair that never completed leaves a report that
        # cannot be read, the table is still built out of the rest
        self._write(SUBJECT)
        with io.open(os.path.join(self.output, "runs", "broken.json"), "wb") as file:
            file.write(b"{")
        with io.open(os.path.join(self.output, "runs", "notes.txt"), "wb") as file:
            file.write(b"ignored")
        rows = benchmark_table.results(self.output)
        self.assertEqual(len(rows), 1)

    def test_index(self):
        known = benchmark_table.index([SUBJECT, REFERENCE])
        self.assertEqual(known[("static-small-alive", "nginx")]["rps"], 2000.0)

    def test_workloads(self):
        # the order the harness drove the workloads in is the order
        # they are listed in, and a workload shows up exactly once
        other = dict(SUBJECT, workload="proxy-alive")
        names = benchmark_table.workloads([SUBJECT, REFERENCE, other])
        self.assertEqual(names, ["static-small-alive", "proxy-alive"])

    def test_servers(self):
        # the subject leads and the references follow in the recorded
        # order, which is the one the table is meant to read in
        rows = [REFERENCE, SUBJECT, dict(SUBJECT, server="caddy")]
        names = benchmark_table.servers(rows, "static-small-alive")
        self.assertEqual(names, ["viriatum", "nginx", "caddy"])

    def test_servers_unknown(self):
        # a reference that the recorded order knows nothing about is
        # kept last rather than being dropped from the table
        rows = [SUBJECT, dict(SUBJECT, server="lighttpd")]
        names = benchmark_table.servers(rows, "static-small-alive")
        self.assertEqual(names, ["viriatum", "lighttpd"])

    def test_ratio(self):
        self.assertEqual(benchmark_table.ratio(SUBJECT, REFERENCE), 0.5)

    def test_ratio_invalid(self):
        # a measurement that was discarded is never one half of a
        # ratio, a server that served nothing at all would otherwise
        # read as one the subject is infinitely faster than
        invalid = dict(REFERENCE, valid=False)
        self.assertEqual(benchmark_table.ratio(SUBJECT, invalid), None)
        self.assertEqual(
            benchmark_table.ratio(dict(SUBJECT, valid=False), REFERENCE), None
        )

    def test_ratio_missing(self):
        # a reference that never ran carries no figure and so no ratio
        # is reported against it, which is not the same as a ratio of
        # zero and must never be printed as one
        self.assertEqual(benchmark_table.ratio(SUBJECT, None), None)
        self.assertEqual(benchmark_table.ratio(None, REFERENCE), None)
        self.assertEqual(benchmark_table.ratio(SUBJECT, dict(REFERENCE, rps=0.0)), None)

    def test_moved(self):
        # a figure inside the spread the baseline was recorded with is
        # the very same figure, only one outside of it has moved
        self.assertEqual(benchmark_table.moved(dict(SUBJECT, rps=1000.0), SUBJECT), None)
        self.assertEqual(
            benchmark_table.moved(dict(SUBJECT, rps=900.0), SUBJECT), "slower"
        )
        self.assertEqual(
            benchmark_table.moved(dict(SUBJECT, rps=1100.0), SUBJECT), "faster"
        )

    def test_moved_edges(self):
        # the bounds themselves are inside the spread, a figure that
        # lands exactly on one of them has not moved anywhere
        self.assertEqual(benchmark_table.moved(dict(SUBJECT, rps=950.0), SUBJECT), None)
        self.assertEqual(benchmark_table.moved(dict(SUBJECT, rps=1050.0), SUBJECT), None)

    def test_moved_invalid(self):
        # a measurement that lost its connections is never compared
        # against anything, the figure of it is not a figure at all
        current = dict(SUBJECT, rps=10.0, valid=False)
        self.assertEqual(benchmark_table.moved(current, SUBJECT), None)

    def test_moved_missing(self):
        self.assertEqual(benchmark_table.moved(SUBJECT, None), None)
        self.assertEqual(benchmark_table.moved(None, SUBJECT), None)
        self.assertEqual(
            benchmark_table.moved(SUBJECT, dict(SUBJECT, rps_low=0.0, rps_high=0.0)),
            None,
        )

    def test_delta(self):
        self.assertAlmostEqual(
            benchmark_table.delta(dict(SUBJECT, rps=1100.0), SUBJECT), 10.0
        )
        self.assertAlmostEqual(
            benchmark_table.delta(dict(SUBJECT, rps=900.0), SUBJECT), -10.0
        )

    def test_delta_missing(self):
        self.assertEqual(benchmark_table.delta(SUBJECT, None), None)
        self.assertEqual(benchmark_table.delta(SUBJECT, dict(SUBJECT, rps=0.0)), None)

    def test_marker(self):
        self.assertEqual(benchmark_table.marker(SUBJECT, None), "")
        self.assertEqual(benchmark_table.marker(SUBJECT, "slower"), "🔻")
        self.assertEqual(benchmark_table.marker(SUBJECT, "faster"), "🔺")

    def test_marker_invalid(self):
        # the mark of a measurement that is not a figure wins over the
        # one of a move, a run that lost its connections looks faster
        # than it is and must never be reported as an improvement
        item = dict(SUBJECT, valid=False)
        self.assertEqual(benchmark_table.marker(item, "faster"), "⚠️")

    def test_format_rps(self):
        self.assertEqual(benchmark_table.format_rps(1234.6), "1235")
        self.assertEqual(benchmark_table.format_rps(0), "-")
        self.assertEqual(benchmark_table.format_rps(None), "-")

    def test_format_latency(self):
        # the tail is recorded in microseconds and read in the
        # milliseconds a person is able to reason about
        self.assertEqual(benchmark_table.format_latency(1500), "1.50")
        self.assertEqual(benchmark_table.format_latency(0), "-")

    def test_table(self):
        lines = benchmark_table.table([SUBJECT, REFERENCE], None)
        self.assertEqual(len(lines), 4)
        self.assertTrue("**viriatum**" in lines[2])
        self.assertTrue("nginx 0.50x" in lines[2])
        self.assertTrue("1.50" in lines[2])
        self.assertTrue("| nginx |" in lines[3])

    def test_table_baseline(self):
        # the change against the baseline shows up on every row and
        # the mark only on the ones that left the recorded spread
        current = dict(SUBJECT, rps=800.0)
        lines = benchmark_table.table([current], [SUBJECT])
        self.assertTrue("-20.0%" in lines[2])
        self.assertTrue("🔻" in lines[2])

    def test_table_no_reference(self):
        # a workload the references were all skipped on still reports
        # the subject, without a ratio it has nothing to be against
        lines = benchmark_table.table([SUBJECT], None)
        self.assertEqual(len(lines), 3)
        self.assertTrue("<sub>" not in lines[2])

    def test_main(self):
        self._write(SUBJECT)
        self._write(REFERENCE)
        self.assertEqual(benchmark_table.main_with(self.output, None), 0)

        # the assembled result is written beside the table so that a
        # later run is able to compare against it without walking
        # every one of the reports again
        with io.open(os.path.join(self.output, "results.json"), "rb") as file:
            rows = json.loads(file.read().decode("utf-8"))
        self.assertEqual(len(rows), 2)

    def test_main_environment(self):
        # the machine and the build a figure came out of are part of
        # the figure, a run on another of either is never comparable
        # and the report says which one produced it
        self._write(SUBJECT)
        with io.open(os.path.join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Linux x86_64\ncompiler: gcc 14")

        stream = io.StringIO()
        stdout = sys.stdout
        try:
            sys.stdout = stream
            self.assertEqual(benchmark_table.main_with(self.output, None), 0)
        finally:
            sys.stdout = stdout

        self.assertTrue("machine: Linux x86_64" in stream.getvalue())
        self.assertTrue("<summary>Environment</summary>" in stream.getvalue())

    def test_main_baseline(self):
        # the baseline is read off the path it was handed and the
        # change against it lands on every row of the table
        self._write(dict(SUBJECT, rps=500.0))
        path = os.path.join(self.output, "baseline.json")
        with io.open(path, "wb") as file:
            file.write(json.dumps([SUBJECT]).encode("utf-8"))

        stream = io.StringIO()
        stdout = sys.stdout
        try:
            sys.stdout = stream
            self.assertEqual(benchmark_table.main_with(self.output, path), 0)
        finally:
            sys.stdout = stdout

        self.assertTrue("-50.0%" in stream.getvalue())

    def test_main_baseline_elsewhere(self):
        # a baseline recorded on another machine is never compared
        # against, marking every row as moved would only teach whoever
        # reads the table to stop reading the marks at all
        self._write(dict(SUBJECT, rps=500.0))
        with io.open(os.path.join(self.output, "environment.txt"), "wb") as file:
            file.write(b"machine: Darwin arm64\nmode: native")

        path = os.path.join(self.output, "baseline.json")
        with io.open(path, "wb") as file:
            file.write(
                json.dumps(
                    dict(
                        results=[SUBJECT],
                        environment=dict(machine="Linux x86_64", mode="native"),
                    )
                ).encode("utf-8")
            )

        stream = io.StringIO()
        stdout = sys.stdout
        try:
            sys.stdout = stream
            self.assertEqual(benchmark_table.main_with(self.output, path), 0)
        finally:
            sys.stdout = stdout

        self.assertTrue("-50.0%" not in stream.getvalue())
        self.assertTrue("so nothing was compared against it" in stream.getvalue())

    def test_main_empty(self):
        self.assertEqual(benchmark_table.main_with(self.output, None), 1)

    def test_main_arguments(self):
        # the entry point reads the output and the baseline off the
        # arguments, the baseline being the optional one of the two
        self._write(SUBJECT)
        argv = sys.argv
        try:
            sys.argv = ["benchmark_table.py", self.output]
            self.assertEqual(benchmark_table.main(), 0)
        finally:
            sys.argv = argv


if __name__ == "__main__":
    unittest.main()
