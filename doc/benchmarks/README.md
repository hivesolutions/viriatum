# Viriatum Benchmark Reports (BMK)

This directory contains periodic benchmark reports for the Viriatum web server, produced by driving `scripts/benchmark.sh` against the reference servers on a single machine and comparing the result against `scripts/benchmark/baseline.json`.

## Purpose

Benchmark reports serve as:

- **Performance Snapshot**: A point-in-time record of how the server performs against the references it is measured beside, so that a later reader can see what was fast (and what was not) at each report
- **Trend Reference**: A record to compare successive reports against, surfacing slow drifts in throughput, processor cost and memory before they become the shape of the server
- **Regression Guard**: A structured sweep for the losses a change may have introduced, read through the ratio against a reference rather than through an absolute figure that any busy machine moves
- **Audit Trail**: A record of when the serving was last measured, on what machine, under what load and with what findings, so that a performance claim always has a run behind it

## Method

Every report is produced the same way. The harness starts the server and each reference on the same machine, drives them through the same workloads and records one report per pair. The figure that is tracked is the **ratio of the server against a reference measured in the same run**, because absolute numbers from one machine are never comparable to another and are moved by whatever else the machine happens to be doing.

A run is only compared against the baseline when the machine and the shape of the load agree with it, which the harness checks on `machine`, `mode`, `references`, `connections`, `threads` and `workers`. A report taken under a different shape says so and compares nothing.

## Severity

| Level    | Meaning                                                                        |
| -------- | ------------------------------------------------------------------------------ |
| Critical | A ratio lost more than half, or the server stopped serving a workload at all   |
| High     | A ratio lost more than a quarter, or a cost figure more than doubled           |
| Medium   | A ratio lost more than a tenth, which is the threshold a run is read against   |
| Low      | A movement inside the noise of the machine, recorded so that a trend may form  |
| Info     | An observation about the harness or the machine rather than about the serving  |

## Document Index

| ID      | Title                                              | Date       | Status |
| ------- | -------------------------------------------------- | ---------- | ------ |
| BMK-001 | [Serving Benchmark](001-benchmark_2026_08_31.md)   | 2026-08-31 | Final  |
