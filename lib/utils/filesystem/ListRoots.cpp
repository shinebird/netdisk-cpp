// Note: 部分内容由Qwen 3.7-Max生成

#include "netdisk-cpp/utils/filesystem/ListRoots.hpp"

#ifdef _WIN32
    #include <windows.h>
#else
    #include <cstdio>   // setmntent, endmntent
    #include <mntent.h> // getmntent

#endif

namespace netdisk::utils::filesystem
{
    auto listRoots() -> std::vector<std::filesystem::path>
    {
        std::vector<std::filesystem::path> roots;

#ifdef _WIN32
        // GetLogicalDrives 返回位掩码，bit0=A:, bit1=B:, ...
        DWORD drives = GetLogicalDrives();
        for (char c = 'A'; c <= 'Z'; ++c)
        {
            if ((drives & (1U << (c - 'A'))) != 0U)
            {
                roots.emplace_back(std::string(1, c) + ":\\");
            }
        }
#else
        // POSIX: 解析 /proc/mounts 获取所有挂载点
        FILE* fp = setmntent("/proc/mounts", "r");
        if (!fp)
        {
            // macOS/BSD 回退到 /etc/mtab
            fp = setmntent("/etc/mtab", "r");
        }
        if (fp)
        {
            struct mntent* entry;
            while ((entry = getmntent(fp)) != nullptr)
            {
                roots.emplace_back(entry->mnt_dir);
            }
            endmntent(fp);
        }
        else
        {
            // 最终回退：至少返回 /
            roots.emplace_back("/");
        }
#endif

        return roots;
    }
} // namespace netdisk::utils::filesystem
