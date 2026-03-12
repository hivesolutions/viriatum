file(GLOB MOD_GIF_SOURCES src/viriatum_mod_gif/*.c src/viriatum_mod_gif/module/*.c)
add_library(viriatum_mod_gif MODULE ${MOD_GIF_SOURCES})
set_target_properties(viriatum_mod_gif PROPERTIES PREFIX "lib" SUFFIX ${MODULE_SUFFIX})
target_include_directories(viriatum_mod_gif PRIVATE
    src/viriatum_mod_gif
    ${CMAKE_SOURCE_DIR}/src/viriatum
    ${CMAKE_SOURCE_DIR}/src/viriatum_commons
)
target_link_libraries(viriatum_mod_gif viriatum_commons)
