## New features

* Support for a bittorrent variant of the viriatum server
* Ass support for a directory on ini files (configuration.d) so that all files there are read in sequence (useful for modules configuration)
* Support for OpenSSL with non blocking sockets (http://stackoverflow.com/questions/5397788/confused-about-openssl-non-blocking-i-o https://groups.google.com/forum/?fromgroups#!topic/mailing.openssl.users/nJRF_JVnPkc)
* Support for async file reading (http://linux.die.net/man/3/aio_read http://msdn.microsoft.com/en-us/library/windows/desktop/aa365747(v=vs.85).aspx http://lxr.evanmiller.org/http/source/os/unix/ngx_aio_read.c http://www.ibm.com/developerworks/linux/library/l-async/)
* Support for Python embedding (mod_python)
* ~~Support for WSGI (mod_wsgi)~~
* Support for Google V8 (mod_v8)
* Support for HTTP ranges (http://www.web-polygraph.org/docs/userman/ranges.html)
* Support for configuration using parsing of json (https://github.com/json-c/json-c) or any other in (http://www.json.org)
* Support for the buffering to support the request object based on the nginx implementation (http://www.slideshare.net/joshzhu/nginx-internals)
* Apply the famous http parser of node.js http://github.com/ry/http-parser or https://github.com/joyent/node/blob/master/tools/wrk/src/http_parser.c
* Support for huffman compression according to the implementation in the mariachi engine (http://code.google.com/p/mariachi/source/browse/src/hive_mariachi/algorithms/compression/huffman.h)
* Implement the memory pool structure based on the nginx implementation

## Refactor / Bug Fixing

* Compile all the regular expression into a single one like nginx (http://nginx.sourcearchive.com/documentation/1.1.4-2/ngx__regex_8c_source.html)
* Implement and refactor win32 conditions (_thread_win32.c) use this link http://thbecker.net/free_software_utilities/fair_monitor_for_win32/start_page.html

## Testing

* [pathoc](http://pathod.net) (a perverse HTTP client) is a nice tool for testing [simple examples](http://corte.si/posts/code/pathod/pythonservers/index.html)
* [SSL Labs SSL test](https://www.ssllabs.com/ssltest/) is a service that provides extensive
SSL testing for a certain domain (universally acclaimed)
* [securityheaders.io](https://securityheaders.io/) provides a web service that verifies a
series of HTTP headers trying to find if they ensure a properly secured web page

## Design decisions

* Create a structure similar to the asynchronous model defined in the colony service implementations
* Separate the concept of service from the handler of the select (select_service does not make sense)

## Performance refactor

* Remove the strlen references and use String_t structures to reduce size calculations (in constant access situations), note that a profile of the static workload taken in August 2026 did not find the length calculations anywhere near the top, so this wants a profile behind it before it is acted upon

## Worker processes

* Spawn for working processes occurs for complete CPU usage (nginx reference - http://wiki.nginx.org/CoreModule#worker_processes)
* Control of the working processed described here (http://nginx.org/en/docs/control.html)
* Some general information about nginx architecture (includes worker process) http://www.slideshare.net/joshzhu/nginx-internals
* Forking a process inherits the hability to handle connections, so thats how nginx works this out (http://stackoverflow.com/questions/670891/is-there-a-way-for-multiple-processes-to-share-a-listening-socket)
* Verify the upstream support (http://wiki.nginx.org/NginxHttpUpstreamModule#upstream)

## Target performance

The numbers that used to sit here were recorded years ago against machines that no longer exist, with one of them marked as unconfirmed, so none of them could be reproduced or compared against. They have been replaced by a harness that measures the serving continuously.

* Run it with `./scripts/benchmark.sh`, the methodology and the configuration of every server are written down in [scripts/benchmark/README.md](../scripts/benchmark/README.md)
* The accepted numbers live in `scripts/benchmark/baseline.json` and every run is compared against them
* The figure to track is the ratio against a reference measured in the same run on the same machine, never an absolute number on its own
