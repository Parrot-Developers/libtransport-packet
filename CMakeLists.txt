cmake_minimum_required(VERSION 3.15)
project(libtransport-packet VERSION 1.0)

set(LIB_SOURCES
      src/tpkt.c
      src/tpkt_list.c
)

add_library(transport-packet ${LIB_SOURCES})

option(BUILD_SHARED_LIBS "Build using shared libraries" ON)

target_include_directories(transport-packet PUBLIC
                            $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
                            $<INSTALL_INTERFACE:include>
)

target_compile_definitions(transport-packet PRIVATE "TPKT_API_EXPORTS")
target_compile_options(transport-packet PRIVATE "-fvisibility=hidden")
target_compile_options(transport-packet PRIVATE "-std=gnu99")

target_link_libraries(transport-packet futils pomp ulog)