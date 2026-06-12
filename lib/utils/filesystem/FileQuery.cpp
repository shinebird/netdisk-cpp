#include "netdisk-cpp/utils/filesystem/FileQuery.hpp"

#include <filesystem>
#include <optional>

#ifdef _WIN32
    #include <windows.h>
#endif

namespace netdisk::utils::filesystem
{
    namespace internal
    {
#ifdef _WIN32
        struct HandleGuard
        {
                HANDLE h;
                ~HandleGuard()
                {
                    if (h != INVALID_HANDLE_VALUE)
                    {
                        CloseHandle(h);
                    }
                }
        };
#endif
    } // namespace internal

    auto getFileSize(const std::filesystem::path& path) -> std::optional<std::uintmax_t>
    {
        if (path.is_absolute() && path == path.root_path())
        {
            return std::filesystem::space(path).capacity;
        }

        if (isRegularFile(path))
        {
#ifdef _WIN32
            // 1. 使用 CreateFileW 尝试获取句柄
            // FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE:
            // 声明我们愿意与其他进程共享该文件，极大提高打开被锁定文件的成功率
            HANDLE h_file =
                CreateFileW(path.c_str(),
                            0, // 请求零访问权限（仅获取元数据，不请求读/写），进一步降低冲突概率
                            FILE_SHARE_READ, nullptr,
                            OPEN_EXISTING, // 仅当文件存在时打开
                            FILE_ATTRIBUTE_NORMAL, nullptr);

            if (h_file == INVALID_HANDLE_VALUE)
            {
                // 即使使用了最大共享模式依然失败，说明文件确实不存在或被内核级驱动锁定
                return std::nullopt;
            }
            // 确保句柄在任何退出路径下都能正确关闭
            internal::HandleGuard guard{h_file};
            LARGE_INTEGER file_size;
            if (GetFileSizeEx(h_file, &file_size) == 0)
            {
                return std::nullopt;
            }
            return static_cast<std::uintmax_t>(file_size.QuadPart);
#else
            return std::filesystem::file_size(path);
#endif
        }
        return 0;
    }

    auto isRegularFile(const std::filesystem::path& path) -> std::optional<bool>
    {
#ifdef _WIN32
        // 1. 使用 CreateFileW 尝试获取句柄
        // FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE:
        // 声明我们愿意与其他进程共享该文件，极大提高打开被锁定文件的成功率
        HANDLE h_file =
            CreateFileW(path.c_str(),
                        0, // 请求零访问权限（仅获取元数据，不请求读/写），进一步降低冲突概率
                        FILE_SHARE_READ, nullptr,
                        OPEN_EXISTING, // 仅当文件存在时打开
                        FILE_ATTRIBUTE_NORMAL, nullptr);

        if (h_file == INVALID_HANDLE_VALUE)
        {
            // 即使使用了最大共享模式依然失败，说明文件确实不存在或被内核级驱动锁定
            return std::nullopt;
        }
        // 确保句柄在任何退出路径下都能正确关闭
        internal::HandleGuard guard{h_file};
        // 2. 判断是否为常规文件（排除目录、设备、管道等）
        DWORD file_type = GetFileType(h_file);
        if (file_type != FILE_TYPE_DISK)
        {
            return false; // 不是磁盘上的常规文件
        }

        BY_HANDLE_FILE_INFORMATION info;
        if (GetFileInformationByHandle(h_file, &info) == 0)
        {
            return std::nullopt;
        }
        const auto attrs = info.dwFileAttributes;
        return ((attrs & FILE_ATTRIBUTE_DIRECTORY) == 0U) &&
               ((attrs & FILE_ATTRIBUTE_REPARSE_POINT) == 0U) &&
               ((attrs & FILE_ATTRIBUTE_DEVICE) == 0U);
#else
        return std::filesystem::is_regular_file(path);
#endif
    }

    auto isDirectory(const std::filesystem::path& path) -> std::optional<bool>
    {
#ifdef _WIN32
        // 1. 使用 CreateFileW 尝试获取句柄
        // FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE:
        // 声明我们愿意与其他进程共享该文件，极大提高打开被锁定文件的成功率
        HANDLE h_file =
            CreateFileW(path.c_str(),
                        0, // 请求零访问权限（仅获取元数据，不请求读/写），进一步降低冲突概率
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, nullptr,
                        OPEN_EXISTING, // 仅当文件存在时打开
                        FILE_ATTRIBUTE_NORMAL | FILE_FLAG_BACKUP_SEMANTICS, nullptr);

        if (h_file == INVALID_HANDLE_VALUE)
        {
            // 即使使用了最大共享模式依然失败，说明文件确实不存在或被内核级驱动锁定
            return std::nullopt;
        }
        // 确保句柄在任何退出路径下都能正确关闭
        internal::HandleGuard guard{h_file};
        // 2. 判断是否为常规文件（排除目录、设备、管道等）
        if (DWORD file_type = GetFileType(h_file); file_type != FILE_TYPE_DISK)
        {
            return false; // 不是磁盘上的常规文件
        }

        BY_HANDLE_FILE_INFORMATION info;
        if (GetFileInformationByHandle(h_file, &info) == 0)
        {
            return std::nullopt;
        }
        if ((info.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0U)
        {
            return true; // 是目录或磁盘根目录，不是常规文件
        }

        return false;

#else
        return std::filesystem::is_regular_file(path);
#endif
    }

    auto lastWriteTime(const std::filesystem::path& path) -> std::optional<std::int64_t>
    {
#ifdef _WIN32
        HANDLE h_file =
            CreateFileW(path.c_str(),
                        0,                                                      // 零访问权限
                        FILE_SHARE_READ | FILE_SHARE_WRITE | FILE_SHARE_DELETE, // 最大共享
                        nullptr, OPEN_EXISTING,
                        FILE_FLAG_BACKUP_SEMANTICS, // ⭐ 关键：允许打开目录句柄
                        nullptr);

        if (h_file == INVALID_HANDLE_VALUE)
        {
            return std::nullopt;
        }

        BY_HANDLE_FILE_INFORMATION info;
        BOOL ok = GetFileInformationByHandle(h_file, &info);
        CloseHandle(h_file);

        if (ok == 0)
        {
            return std::nullopt;
        }

        const auto file_time = info.ftLastWriteTime;
        // Windows FILETIME: 100ns ticks since 1601-01-01
        // Unix Epoch:      1970-01-01
        // 两者之间的差值:  11644473600 秒 = 116444736000000000 个 100ns ticks
        constexpr long long EPOCH_DIFF_TICKS = 116444736000000000LL;

        long long ticks =
            (static_cast<long long>(file_time.dwHighDateTime) << 32) | file_time.dwLowDateTime;

        if (ticks < EPOCH_DIFF_TICKS)
        {
            return 0; // 防止 1970 年前的时间溢出
        }

        return (ticks - EPOCH_DIFF_TICKS) / 10000; // 100ns → ms

#else
        const auto sctp = std::chrono::time_point_cast<std::chrono::milliseconds>(
            std::filesystem::last_write_time(path));
        return sctp.time_since_epoch().count();
#endif
    }
} // namespace netdisk::utils::filesystem
