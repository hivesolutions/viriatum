file(GLOB MOD_DIAG_SOURCES src/viriatum_mod_diag/*.c src/viriatum_mod_diag/module/*.c)
add_library(viriatum_mod_diag MODULE ${MOD_DIAG_SOURCES})
set_target_properties(viriatum_mod_diag PROPERTIES PREFIX "${MODULE_PREFIX}" SUFFIX ${MODULE_SUFFIX})
target_include_directories(viriatum_mod_diag PRIVATE
    src/viriatum_mod_diag
    ${CMAKE_SOURCE_DIR}/src/viriatum
    ${CMAKE_SOURCE_DIR}/src/viriatum_commons
)
target_link_libraries(viriatum_mod_diag viriatum_commons)
