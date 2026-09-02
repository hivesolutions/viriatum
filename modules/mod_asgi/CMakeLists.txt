# detects Python 3 using python3-config for the ASGI module
find_program(PYTHON3_CONFIG python3-config)
if(PYTHON3_CONFIG)
    execute_process(COMMAND ${PYTHON3_CONFIG} --includes OUTPUT_VARIABLE PY3_INCLUDES_RAW OUTPUT_STRIP_TRAILING_WHITESPACE)
    execute_process(COMMAND ${PYTHON3_CONFIG} --ldflags --embed OUTPUT_VARIABLE PY3_LDFLAGS_RAW OUTPUT_STRIP_TRAILING_WHITESPACE)
    string(REGEX MATCHALL "-I[^ ]+" PY3_INCLUDE_FLAGS "${PY3_INCLUDES_RAW}")
    set(PY3_INCLUDE_DIRS "")
    foreach(FLAG ${PY3_INCLUDE_FLAGS})
        string(SUBSTRING ${FLAG} 2 -1 DIR)
        list(APPEND PY3_INCLUDE_DIRS ${DIR})
    endforeach()
    string(REGEX MATCHALL "-L[^ ]+" PY3_LIB_FLAGS "${PY3_LDFLAGS_RAW}")
    set(PY3_LIB_DIRS "")
    foreach(FLAG ${PY3_LIB_FLAGS})
        string(SUBSTRING ${FLAG} 2 -1 DIR)
        list(APPEND PY3_LIB_DIRS ${DIR})
    endforeach()
    string(REGEX MATCHALL "(^| )-l[^ ]+" PY3_LINK_FLAGS "${PY3_LDFLAGS_RAW}")
    set(PY3_LIBRARIES "")
    foreach(FLAG ${PY3_LINK_FLAGS})
        string(STRIP "${FLAG}" FLAG)
        string(SUBSTRING ${FLAG} 2 -1 LIB)
        list(APPEND PY3_LIBRARIES ${LIB})
    endforeach()
    set(PYTHON3_FOUND TRUE)
endif()

if(PYTHON3_FOUND)
    # the serving of the interface is the one the tree already carries
    # for the extension, it is compiled into the module rather than
    # written a second time, it depends on nothing of the embedded
    # server and registers itself against a service like any handler
    file(GLOB MOD_ASGI_SOURCES src/viriatum_mod_asgi/*.c src/viriatum_mod_asgi/module/*.c)
    list(APPEND MOD_ASGI_SOURCES
        ${CMAKE_SOURCE_DIR}/src/viriatum_python/module/handler_asgi.c
        ${CMAKE_SOURCE_DIR}/src/viriatum_python/module/loop.c
        ${CMAKE_SOURCE_DIR}/src/viriatum/http/websocket.c
    )
    add_library(viriatum_mod_asgi MODULE ${MOD_ASGI_SOURCES})
    set_target_properties(viriatum_mod_asgi PROPERTIES PREFIX "${MODULE_PREFIX}" SUFFIX ${MODULE_SUFFIX})
    target_include_directories(viriatum_mod_asgi PRIVATE
        src/viriatum_mod_asgi
        ${CMAKE_SOURCE_DIR}/src/viriatum
        ${CMAKE_SOURCE_DIR}/src/viriatum_commons
        ${CMAKE_SOURCE_DIR}/src/viriatum_python
        ${PY3_INCLUDE_DIRS}
    )
    target_link_directories(viriatum_mod_asgi PRIVATE ${PY3_LIB_DIRS})
    target_link_libraries(viriatum_mod_asgi viriatum_commons ${PY3_LIBRARIES})
    message(STATUS "Viriatum: mod_asgi enabled")

    # test executable: compiles the module sources together with
    # the test files into a standalone binary that can be invoked
    # to run the mod_asgi test suite
    file(GLOB MOD_ASGI_TEST_SOURCES src/viriatum_mod_asgi/test/*.c)
    add_executable(viriatum_mod_asgi_test
        ${MOD_ASGI_SOURCES}
        ${MOD_ASGI_TEST_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/viriatum/http/http_parser.c
        ${CMAKE_SOURCE_DIR}/src/viriatum/http/http_request.c
    )
    set_target_properties(viriatum_mod_asgi_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    )
    target_include_directories(viriatum_mod_asgi_test PRIVATE
        src/viriatum_mod_asgi
        ${CMAKE_SOURCE_DIR}/src/viriatum
        ${CMAKE_SOURCE_DIR}/src/viriatum_commons
        ${CMAKE_SOURCE_DIR}/src/viriatum_python
        ${PY3_INCLUDE_DIRS}
    )
    target_link_directories(viriatum_mod_asgi_test PRIVATE ${PY3_LIB_DIRS})
    # the core is linked into the suite so that the starting and the
    # stopping of the module may be driven against a real service,
    # which is the whole of what the entry point does
    target_link_libraries(viriatum_mod_asgi_test viriatum_static viriatum_commons ${PY3_LIBRARIES})
    target_compile_definitions(viriatum_mod_asgi_test PRIVATE
        VIRIATUM_TEST_SCRIPT_PATH="${CMAKE_SOURCE_DIR}/modules/mod_asgi/resources/test/test_handler.py"
    )
else()
    message(STATUS "Viriatum: mod_asgi disabled (Python 3 not found)")
endif()
