# Benchmark

The harness drives Viriatum and the reference servers through the same workloads, on the same machine, one right after the other, and reports them side by side. Run it with `./scripts/benchmark.sh`.

Absolute numbers from a hosted runner are not trustworthy and are not what this measures. **The figure that matters is the ratio of Viriatum against a reference measured in the same run on the same machine.** That ratio is the only comparison that survives a noisy environment, and it is the number to track over time.

## Running it

```bash
./scripts/benchmark.sh
ONLY=static-small-alive ./scripts/benchmark.sh
DURATION=10 REPEATS=5 ./scripts/benchmark.sh
```

Every setting has a default and every one of them is overridable: `BUILD`, `OUTPUT`, `DURATION`, `CONNECTIONS`, `THREADS`, `RATE`, `REPEATS`, `WORKERS`, `PORT`, `SETTLE`, `WAITING` and `ONLY`. The reports land under `benchmark/`, one file per pair of a workload and a server under `benchmark/runs`, with the table in `benchmark/summary.md` and the assembled result in `benchmark/results.json`.

`wrk` is required. Everything else is optional and is skipped, with a line saying so, when it is not on the machine.

## How a measurement is taken

A run before the measured ones is thrown away, so that a cold page cache and a server whose structures have not yet grown never land in a reported figure. The measured runs are repeated `REPEATS` times and the median is kept together with the quartiles, so that a single noisy sample cannot move a result and the spread says whether the median may be trusted at all.

The metrics that would disturb a timed run are taken outside of one: the tail below saturation, the handling of connections and the calls into the kernel each get a run of their own. The counted run is never the timed run.

A measurement that lost a noticeable share of its connections is recorded as invalid rather than reported as a figure, because a machine that has run out of something answers far faster than a server does and would otherwise read as a win.

The load is driven over `CONNECTIONS` connections from `THREADS` threads, 256 and 8 by default. Both matter and both are recorded beside every figure. Too few connections and a server is never asked the question that separates one waiting mechanism from another, which only begins to tell them apart in the hundreds; too many threads and the generator takes the cores away from the subject it is measuring, which was seen to cost around 4% at sixteen of them on a machine of eighteen cores. A run that changes either of these is not comparable against a baseline recorded under the other, and the report says so rather than pretending otherwise.

## Where the servers run

The subject and its references always run **the same way**. Either both come out of a pinned image or both come out of the binaries the machine carries. A subject reached over one stack and a reference reached over another measures the stacks and never the servers, which is exactly what would happen on a machine whose container daemon lives inside a virtual machine of its own.

On Linux the references come from the pinned images with the network of the host, and the subject runs natively beside them, since the two share a kernel and a network already. Elsewhere, if the images are present the subject is put into a container too, and if they are not then everything runs natively. The mode that produced a run is recorded in `benchmark/environment.txt`.

The images are never pulled by a run. Pulling belongs to the workflow, so that a run never sits on a slow registry before measuring anything.

## The configuration of each server

The values below are the ones that actually decide the comparison. Everything else is left at its default.

| Choice | Viriatum | nginx | Caddy | HAProxy | gunicorn | uvicorn |
| --- | --- | --- | --- | --- | --- | --- |
| Worker count | `WORKERS` | `worker_processes` | default | `nbthread` | `--workers` | `--workers` |
| Access logging | `access_log off` | `access_log off` | `output discard` | `log /dev/null` | `/dev/null` | `--no-access-log` |
| Kernel file sending | none | `sendfile off` | default | n/a | n/a | n/a |
| Compression | none | `gzip off` | not enabled | n/a | n/a | n/a |
| Keep-alive idle | default | `65s` | `idle 65s` | `65s` | `--keep-alive 65` | default |
| Requests per connection | unlimited | `1000000` | default | default | default | default |
| Upstream pooling | backends held | `keepalive 128` | default | `maxconn 512` | n/a | n/a |

A few of these deserve their reasoning written down.

**`sendfile off` on nginx is deliberate and it costs nginx.** Viriatum has no path that hands a file to the kernel, so leaving nginx's on would compare the two of them on the largest fixture and on nothing else. With it on, nginx would be considerably faster on the large file. The comparison here is of the request path, not of the copy.

**`keepalive_requests` is raised well past its default.** nginx closes a connection after a thousand requests by default and Viriatum does not close one at all, and that difference alone shows up as a gap that has nothing to do with the serving.

**Viriatum's request log is turned off through `access_log`, the way every reference turns its own off.** The server writes a line per request and leaving it on while the references have theirs off was measured to cost the subject about 6% on the small static workload. The output of the process is still sent to the sink on top of that, so that anything else it writes never reaches a disk during a measurement.

**The Python references are given the shape they are fastest in, not the one that mirrors the subject.** Viriatum's embedded server holds every connection on the loop of a single process. gunicorn's default worker answers one request at a time and loses every connection past the worker count; measured across its shapes it is fastest at `gthread` with the worker count of the subject and 32 threads, and that is what it is given. A reference driven in a shape it was never meant for reports the shape and never the reference.

**The templates have to be under the root the server is given.** The server looks for the listing and the error templates at `<root>/templates/`, and answers an empty body when they are not there. The fixtures did not carry them at first, so both generated workloads were measuring a response of nothing at all against pages of several kilobytes from the references, which is not a comparison of anything. They are copied into the fixtures now.

**`use_template` only turns the engine off for the error page.** It is read in exactly one place, the writing of an error, so the listing renders through the template whichever way the setting is left. Driving the listing with it on and off therefore measures the same thing twice and the two rows come out equal; the error rows are the ones that isolate the cost of the engine, and they do so clearly, the page going from 1265 bytes to 75. The setting describes itself in the shipped configuration as covering the listing as well, which it does not.

**The upstream of the proxy workload must be faster than every proxy in front of it.** It answers out of the cheapest handler there is. While it was serving files instead, all four proxies reported the same figure, because what was being measured was the upstream saturating and not any of them.

## The workloads

Static serving of a small file that fits in a single packet, of the illustration that ships with the project, and of a large file generated at run time, each with the connection kept and again with it closed. The cheapest response the server can produce, which is the ceiling for everything else. The directory listing and the error page, each with the template engine on and off, so that the cost of the engine is isolated. The proxy in front of a fixed upstream. Both Python interfaces, plus a streaming variant that sends its body in several parts.

Only the references that can actually serve a role are put on it. HAProxy serves no file of its own and Pingora is a framework rather than a packaged server, so both are only ever measured in front of an upstream.

## The baseline

`scripts/benchmark/baseline.json` holds the result of an accepted run and every run is compared against it. A workload that moves beyond the spread its baseline was recorded with is marked; one that stays inside it is the same figure and the difference is the noise of the machine.

The baseline is refreshed **only** through the `refresh` input of the workflow, never automatically, so that it cannot quietly ratchet down one run at a time. A baseline recorded on one class of runner does not describe another, which is why the workflow pins its runner and the environment of every run is recorded beside its numbers.

## Known limits

`wrk2` is what produces the latency percentiles that are corrected for coordinated omission. It is unmaintained and carries a copy of LuaJIT that never learnt any 64-bit ARM target, so **it does not build on ARM at all**, neither on macOS nor on Linux. On those machines the run still reports throughput and says in the report that no corrected percentile was taken. It builds and runs on the x86-64 runner the workflow is pinned to.

Calls into the kernel per request are counted with `strace`, so they are only reported on Linux and only when the subject runs natively.

The workloads that close every connection walk through the ephemeral ports of the machine and leave them all behind. A run waits for them to come back between one workload and the next, which is what `SETTLE` and `WAITING` control. On a machine with a narrow port range this is the slowest part of a run; widening the range and allowing the reuse of a waiting port, as the workflow does, makes it far shorter.
