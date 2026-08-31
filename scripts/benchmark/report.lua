-- Hive Viriatum Web Server
-- Copyright (c) 2008-2026 Hive Solutions Lda.
--
-- Reports the numbers of a run of the generator through the hook it
-- offers for that purpose, so that the harness reads them out of a
-- structure instead of scraping them off the line that closes a run.

-- the percentiles that are gathered out of every run, the ones of a
-- generator that corrects for coordinated omission are the only ones
-- that mean anything at the tail, the rest are kept for the record
done = function(summary, latency, requests)
    local path = os.getenv("BENCHMARK_REPORT") or "report.json"
    local file = io.open(path, "w")

    -- the duration is reported in microseconds by the generator, the
    -- rate is derived from it rather than taken off the summary line
    -- so that it carries the precision the structure has
    local seconds = summary.duration / 1000000.0
    local rps = 0.0
    local transfer = 0.0
    if seconds > 0 then
        rps = summary.requests / seconds
        transfer = summary.bytes / seconds
    end

    file:write(string.format(
        '{\n' ..
        '  "requests": %d,\n' ..
        '  "duration_us": %d,\n' ..
        '  "bytes": %d,\n' ..
        '  "rps": %.3f,\n' ..
        '  "transfer_bps": %.3f,\n' ..
        '  "errors": {\n' ..
        '    "connect": %d,\n' ..
        '    "read": %d,\n' ..
        '    "write": %d,\n' ..
        '    "status": %d,\n' ..
        '    "timeout": %d\n' ..
        '  },\n' ..
        '  "latency_us": {\n' ..
        '    "min": %d,\n' ..
        '    "mean": %.3f,\n' ..
        '    "max": %d,\n' ..
        '    "p50": %d,\n' ..
        '    "p75": %d,\n' ..
        '    "p90": %d,\n' ..
        '    "p99": %d,\n' ..
        '    "p999": %d\n' ..
        '  }\n' ..
        '}\n',
        summary.requests, summary.duration, summary.bytes, rps, transfer,
        summary.errors.connect, summary.errors.read, summary.errors.write,
        summary.errors.status, summary.errors.timeout,
        latency.min, latency.mean, latency.max,
        latency:percentile(50.0), latency:percentile(75.0),
        latency:percentile(90.0), latency:percentile(99.0),
        latency:percentile(99.9)
    ))

    file:close()
end
