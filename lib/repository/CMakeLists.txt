add_subdirectory(generic)
add_subdirectory(sqlite)

add_library(NetdiskRepository INTERFACE)
add_library(Netdisk::Repository ALIAS NetdiskRepository)
target_link_libraries(NetdiskRepository INTERFACE Netdisk::Repoistory::Generic)
target_link_libraries(NetdiskRepository INTERFACE Netdisk::Repoistory::SQLite)