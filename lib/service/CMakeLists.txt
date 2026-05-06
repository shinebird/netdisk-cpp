add_subdirectory(http)

add_library(NetdiskService INTERFACE)
add_library(Netdisk::Service ALIAS NetdiskService)
target_link_libraries(NetdiskService INTERFACE Netdisk::Service::HTTP)