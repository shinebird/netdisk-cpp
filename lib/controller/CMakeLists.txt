add_subdirectory(http)

add_library(NetdiskController INTERFACE)
add_library(Netdisk::Controller ALIAS NetdiskController)
target_link_libraries(NetdiskController INTERFACE Netdisk::Controller::HTTP)