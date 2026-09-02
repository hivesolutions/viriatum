# detects Python 3 using python3-config for the Python module
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
    # the serving of the more recent interface rests on the event
    # loop of the interpreter, which arrived with the third version
    # of it, so an older one carries the older interface alone
    execute_process(
        COMMAND ${PYTHON3_CONFIG} --embed
        OUTPUT_QUIET ERROR_QUIET RESULT_VARIABLE PY3_EMBED_RESULT
    )
    find_program(PYTHON3_BINARY NAMES python3 Python)
    if(PYTHON3_BINARY)
        execute_process(
            COMMAND ${PYTHON3_BINARY} -c "import sys; print(1 if sys.version_info >= (3, 8) else 0)"
            OUTPUT_VARIABLE PY3_ASGI_RAW OUTPUT_STRIP_TRAILING_WHITESPACE
        )
    endif()
    if(PY3_ASGI_RAW STREQUAL "1")
        set(PYTHON3_ASGI TRUE)
    endif()

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
    # the serving of the more recent interface is the one the tree
    # already carries for the extension, it is compiled into the module
    # rather than written a second time, it depends on nothing of the
    # embedded server and registers itself against a service like any
    file(GLOB MOD_PYTHON_SOURCES src/viriatum_mod_python/*.c src/viriatum_mod_python/module/*.c)
    if(PYTHON3_ASGI)
        list(APPEND MOD_PYTHON_SOURCES
            ${CMAKE_SOURCE_DIR}/src/viriatum_python/module/handler_asgi.c
            ${CMAKE_SOURCE_DIR}/src/viriatum_python/module/loop.c
            ${CMAKE_SOURCE_DIR}/src/viriatum/http/websocket.c
        )
        add_compile_definitions(VIRIATUM_ASGI)
        message(STATUS "Viriatum: mod_python serves both of the interfaces")
    else()
        message(STATUS "Viriatum: mod_python serves the older interface alone")
    endif()
    add_library(viriatum_mod_python MODULE ${MOD_PYTHON_SOURCES})
    set_target_properties(viriatum_mod_python PROPERTIES PREFIX "${MODULE_PREFIX}" SUFFIX ${MODULE_SUFFIX})
    target_include_directories(viriatum_mod_python PRIVATE
        src/viriatum_mod_python
        ${CMAKE_SOURCE_DIR}/src/viriatum
        ${CMAKE_SOURCE_DIR}/src/viriatum_commons
        ${CMAKE_SOURCE_DIR}/src/viriatum_python
        ${PY3_INCLUDE_DIRS}
    )
    target_link_directories(viriatum_mod_python PRIVATE ${PY3_LIB_DIRS})
    target_link_libraries(viriatum_mod_python viriatum_commons ${PY3_LIBRARIES})
    message(STATUS "Viriatum: mod_python enabled")

    # test executable: compiles the module sources together with
    # the test files into a standalone binary that can be invoked
    # to run the mod_python test suite
    file(GLOB MOD_PYTHON_TEST_SOURCES src/viriatum_mod_python/test/*.c)
    add_executable(viriatum_mod_python_test
        ${MOD_PYTHON_SOURCES}
        ${MOD_PYTHON_TEST_SOURCES}
        ${CMAKE_SOURCE_DIR}/src/viriatum/http/http_parser.c
        ${CMAKE_SOURCE_DIR}/src/viriatum/http/http_request.c
    )
    set_target_properties(viriatum_mod_python_test PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_RELEASE ${CMAKE_BINARY_DIR}/bin
        RUNTIME_OUTPUT_DIRECTORY_DEBUG ${CMAKE_BINARY_DIR}/bin
    )
    target_include_directories(viriatum_mod_python_test PRIVATE
        src/viriatum_mod_python
        ${CMAKE_SOURCE_DIR}/src/viriatum
        ${CMAKE_SOURCE_DIR}/src/viriatum_commons
        ${CMAKE_SOURCE_DIR}/src/viriatum_python
        ${PY3_INCLUDE_DIRS}
    )
    target_link_directories(viriatum_mod_python_test PRIVATE ${PY3_LIB_DIRS})
    target_link_libraries(viriatum_mod_python_test viriatum_static viriatum_commons ${PY3_LIBRARIES})
    target_compile_definitions(viriatum_mod_python_test PRIVATE
        VIRIATUM_TEST_SCRIPT_PATH="${CMAKE_SOURCE_DIR}/modules/mod_python/resources/test/test_handler.py"
        VIRIATUM_TEST_SCRIPT_ASGI_PATH="${CMAKE_SOURCE_DIR}/modules/mod_python/resources/test/test_handler_asgi.py"
    )
else()
    message(STATUS "Viriatum: mod_python disabled (Python 3 not found)")
endif()
