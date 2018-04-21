add_subdirectory(ev)

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)

set(SRC_SRCS
    EventLoop.cc
    Acceptor.cc
    Channel.cc
    InetAddress.cc
    Socket.cc
    SocketsOps.cc
)

set(SRCS ${SRC_SRCS} ${EV_SRCS})

include_directories(".")

add_library(core ${SRCS})

install(TARGETS core DESTINATION lib)

set(HEADERS
    Callbacks.h
    CurrentThread.h
    EventLoop.h
    Mutex.h
    Types.h
)

install(FILES ${HEADERS} DESTINATION include/)
