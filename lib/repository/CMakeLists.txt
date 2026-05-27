add_subdirectory(generic)

if(NETDISK_REPOSITORY_DATABASE STREQUAL "SQLITE")
    add_subdirectory(sqlite)
endif()

add_library(NetdiskRepository INTERFACE)
add_library(Netdisk::Repository ALIAS NetdiskRepository)
target_link_libraries(NetdiskRepository INTERFACE Netdisk::Repoistory::Generic)
target_link_libraries(NetdiskRepository INTERFACE Netdisk::Repoistory::SQLite)