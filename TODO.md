## New features

* Support for a bittorrent variant of the viriatum server
* Ass support for a directory on ini files (configuration.d) so that all files there are read in sequence (usefull for modules configuration)
* Support for OpenSSL with non blocking sockets (http://stackoverflow.com/questions/5397788/confused-about-openssl-non-blocking-i-o https://groups.google.com/forum/?fromgroups#!topic/mailing.openssl.users/nJRF_JVnPkc)
* Support for the SPDY protocol (http://dev.chromium.org/spdy)
* Support for async file reading (http://linux.die.net/man/3/aio_read http://msdn.microsoft.com/en-us/library/windows/desktop/aa365747(v=vs.85).aspx http://lxr.evanmiller.org/http/source/os/unix/ngx_aio_read.c http://www.ibm.com/developerworks/linux/library/l-async/) 
* Support for Python embedding (mod_python)
* Support for WSGI (mod_wsgi)
* Support for Google V8 (mod_v8) 
* Support for HTTP ranges (http://www.web-polygraph.org/docs/userman/ranges.html)
* Support for configuration using parsing of json (https://github.com/json-c/json-c) or any other in (http://www.json.org)
* Support for the buffering to support the request object based on the nginx implementation (http://www.slideshare.net/joshzhu/nginx-internals)
* Apply the famous http parser of node.js http://github.com/ry/http-parser
* Support for huffman compression according to the implementation in the mariachi engine (http://code.google.com/p/mariachi/source/browse/src/hive_mariachi/algorithms/compression/huffman.h)
* Implement the memory pool structure based on the nginx implementation

## Refactor / Bug Fixing

* Compile all the regular expression into a single one like nginx (http://nginx.sourcearchive.com/documentation/1.1.4-2/ngx__regex_8c_source.html)
* Implement and refactor win32 conditions (_thread_win32.c) use this link http://thbecker.net/free_software_utilities/fair_monitor_for_win32/start_page.html

## Design decisions

* Create a structure similiar to the asynchronous model defined in the colony service implementations
* Separate the concept of service from the handler of the select (select_service does not make sensess)

## Performance refactor

* Remove the strlen references and use String_t structures to reduce size calculations (in constant access situations)

## Worker processes

* Spawn for working processes occurs for complete cpu usage (nginx reference - http://wiki.nginx.org/CoreModule#worker_processes)
* Control of the working processed described here (http://nginx.org/en/docs/control.html)
* Some general information about nginx architecture (includes worker process) http://www.slideshare.net/joshzhu/nginx-internals
* Forking a process inherits the hability to handle connections, so thats how nginx works this out (http://stackoverflow.com/questions/670891/is-there-a-way-for-multiple-processes-to-share-a-listening-socket)
* Verify the upstream support (http://wiki.nginx.org/NginxHttpUpstreamModule#upstream)

## Target performance

* `ab -n 10000 -c 100 -k http://servidor2.hive:9090/error` should be able to run at 45K-60K requests per second (using file handler without template for error)
* `ab -n 10000 -c 100 -k  http://srio.hive:9090/eclipse` should be able to run at 15K-24K requests per second raw power (not confirmed)
* `weighttp -n 10000 -t 5 -c 8 http://127.0.0.1:9090/resources/images/illustration/main-illustration.png` should be able to handle between 320MB and 500MB per second or 13K-20K request (under node2.startomni.com)