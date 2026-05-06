add_subdirectory(http)

add_library(NetdiskCore INTERFACE)
add_library(Netdisk::Core ALIAS NetdiskCore)
target_link_libraries(NetdiskCore INTERFACE Netdisk::Core::HTTP)