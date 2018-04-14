set(EV_SRCS
    ev.c
    )

#define some libev macro to proper use libev
add_definitions(-DEV_STANDALONE=1 -DEV_USE_FLOOR -DEV_USE_NANOSLEEP -DEV_MULTIPLICITY=1)

include_directories(".")

add_library(ev ${EV_SRCS})

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

target_link_libraries(ev ${DEP_LIB})

install(TARGETS ev DESTINATION lib)

set(HEADERS
    ev++.h
    )

install(FILES ${HEADERS} DESTINATION include/ev)
