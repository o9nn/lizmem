include_directories(${CMAKE_CURRENT_SOURCE_DIR})

collect_sources(MEMORY)

add_library(mfsmemory ${MEMORY_SOURCES})

create_unittest(mfsmemory ${MEMORY_TESTS})
link_unittest(mfsmemory mfsmemory mfscommon)
