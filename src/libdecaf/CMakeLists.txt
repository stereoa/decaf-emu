project(libdecaf)

include_directories(".")
include_directories("src")

file(GLOB_RECURSE SOURCE_FILES *.cpp)
file(GLOB_RECURSE HEADER_FILES *.h)

add_library(libdecaf STATIC ${SOURCE_FILES} ${HEADER_FILES})
GroupSources(src)

target_link_libraries(libdecaf
    common
    libcpu
    libgfd
    libgpu
    ${ADDRLIB_LIBRARIES}
    ${CEREAL_LIBRARIES}
    ${FIXED_POINT_LIBRARIES}
    ${IMGUI_LIBRARIES}
    ${PUGIXML_LIBRARIES}
    ${ZLIB_LINK})

if(APPLE)
    target_compile_definitions(libdecaf PUBLIC DECAF_NOGL)
else()
    target_link_libraries(libdecaf
        ${GLBINDING_LIBRARIES}
        ${OPENGL_LIBRARIES})
endif()

if(MSVC)
    target_link_libraries(libdecaf ws2_32)
    target_compile_options(libdecaf PUBLIC /wd4251)
endif()
