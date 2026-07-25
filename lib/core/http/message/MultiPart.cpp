#include <boost/beast/http/buffer_body.hpp>

#include "netdisk-cpp/core/http/message/MultiPart.hpp"
#include "netdisk-cpp/utils/Common.hpp"
#include "netdisk-cpp/utils/url/HTTPParamEncodings.hpp"

#include <cstddef>
#include <format>
#include <limits>
#include <optional>
#include <string_view>

namespace netdisk::core::http::message
{

    template <typename ContentType, typename ReturnType, typename... Args>
    auto makePartImpl(std::string_view boundary, std::string_view field_name,
                      std::string_view filename, std::string_view mime_type, ContentType content,
                      Args&&... args) -> ReturnType
    {
        ReturnType mp_part(std::forward<Args>(args)...);
        mp_part.part_header_ =
            std::format("--{}\r\nContent-Disposition: form-data; name=\"{}\"; "
                        "{}\r\nContent-Type: {}\r\n\r\n",
                        boundary, field_name,
                        utils::url::encodeContentDispositionFileName(filename), mime_type);
        mp_part.content_ = std::move(content);
        return mp_part;
    }
    template <typename ContentType, typename ReturnType, typename... Args>
    auto makePartImpl(std::string_view boundary, std::string_view field_name,
                      std::string_view mime_type, ContentType content, Args&&... args) -> ReturnType
    {
        ReturnType mp_part(std::forward<Args>(args)...);
        mp_part.part_header_ = std::format("--{}\r\nContent-Disposition: form-data; "
                                           "name=\"{}\"\r\nContent-Type: {}\r\n\r\n",
                                           boundary, field_name, mime_type);
        mp_part.content_ = std::move(content);
        return mp_part;
    }

    template <MultipartPart Part>
    auto addFieldsImpl(Part& part, const std::span<FieldView>& fields) -> void
    {
        std::string buf;
        buf.reserve(fields.size() * 40);
        auto out = std::back_inserter(buf);
        for (const auto& [name, value] : fields)
        {
            out = std::format_to(out, "{}: {}\r\n", name, value);
        }
        const auto original_header = std::string_view(part.part_header_);
        part.part_header_ = original_header.subview(0, original_header.size() - 2) + buf;
    }
    // namespace

    auto MultipartPartStatic::size() const noexcept -> std::size_t
    {
        return part_header_.size() + content_.size();
    }

    auto MultipartPartStatic::contentSize() const noexcept -> std::size_t
    {
        return content_.size();
    }

    auto MultipartPartStatic::partHeader() const noexcept -> std::string_view
    {
        return part_header_;
    }

    auto MultipartPartStatic::content() const noexcept -> std::string_view { return content_; }

    auto MultipartPartStatic::addFields(const std::span<FieldView>& fields) -> void
    {
        addFieldsImpl(*this, fields);
    }

    auto MultipartPartStatic::makePart(std::string_view boundary, std::string_view field_name,
                                       std::string_view filename, std::string_view mime_type,
                                       std::string_view content) -> MultipartPartStatic
    {
        return makePartImpl<std::string_view, MultipartPartStatic>(boundary, field_name, filename,
                                                                   mime_type, content);
    }

    auto MultipartPartStatic::makePart(std::string_view boundary, std::string_view field_name,
                                       std::string_view mime_type, std::string_view content)
        -> MultipartPartStatic
    {
        return makePartImpl<std::string_view, MultipartPartStatic>(boundary, field_name, mime_type,
                                                                   content);
    }

    auto MultipartPartDynamic::size() const noexcept -> std::size_t
    {
        if (content_size_ == std::numeric_limits<std::size_t>::max())
        {
            return content_size_;
        }
        return content_size_ + part_header_.size();
    }

    auto MultipartPartDynamic::contentSize() const noexcept -> std::size_t { return content_size_; }

    auto MultipartPartDynamic::partHeader() const noexcept -> std::string_view
    {
        return part_header_;
    }

    auto MultipartPartDynamic::content() -> boost::cobalt::generator<std::string_view>&
    {
        return content_;
    }

    auto MultipartPartDynamic::addFields(const std::span<FieldView>& fields) -> void
    {
        addFieldsImpl(*this, fields);
    }

    MultipartPartDynamic::MultipartPartDynamic(std::size_t content_size) noexcept
        : content_size_(content_size)
    {
    }

    auto MultipartPartDynamic::makePart(std::string_view boundary, std::string_view field_name,
                                        std::string_view filename, std::string_view mime_type,
                                        boost::cobalt::generator<std::string_view> content,
                                        std::size_t size) -> MultipartPartDynamic
    {
        return makePartImpl<boost::cobalt::generator<std::string_view>, MultipartPartDynamic>(
            boundary, field_name, filename, mime_type, std::move(content), size);
    }

    auto MultipartPartDynamic::makePart(std::string_view boundary, std::string_view field_name,
                                        std::string_view mime_type,
                                        boost::cobalt::generator<std::string_view> content,
                                        std::size_t size) -> MultipartPartDynamic
    {
        return makePartImpl<boost::cobalt::generator<std::string_view>, MultipartPartDynamic>(
            boundary, field_name, mime_type, std::move(content), size);
    }

    MultipartContext::MultipartContext(std::string boundary, std::vector<MultipartPart> parts)
        : boundary_(std::move(boundary)),
          closing_boundary_(std::format("\r\n--{}--\r\n", boundary_)), parts_(std::move(parts))
    {
    }

    auto MultipartContext::totalSize() noexcept -> std::optional<std::size_t>
    {
        if (use_trunk_)
        {
            return std::nullopt;
        }
        if (total_size_ != 0)
        {
            return total_size_;
        }
        std::size_t total_size = 0;
        const auto visitors = utils::Overloads{
            [this, &total_size](const auto& value) -> void
            {
                if (value.size() == std::numeric_limits<std::size_t>::max())
                {
                    use_trunk_ = true;
                }
                else
                {
                    total_size += value.size();
                }
            },
        };
        for (const auto& part : parts_)
        {
            std::visit(visitors, part);
        }
        total_size += closing_boundary_.size();
        total_size_ = total_size;
        return use_trunk_ ? std::nullopt : std::optional<std::size_t>{total_size};
    }

    auto MultipartContext::fillBuffer(boost::beast::http::buffer_body::value_type& body)
        -> boost::cobalt::task<bool>
    {
        switch (current_phase_)
        {
        // ---- 发送当前 part 的 header ----
        case Phase::part_header:

            if (part_index_ >= parts_.size())
            {
                // 所有 part 发完 → 进入 closing boundary
                current_phase_ = Phase::closing;
                co_return co_await fillBuffer(body);
            }
            std::visit(
                [&body](auto& part)
                {
                    body.data = (void*)part.partHeader().data();
                    body.size = part.partHeader().size();
                    body.more = true;
                },
                parts_[part_index_]);
            body.more = true;
            content_offset_ = 0;
            current_phase_ = Phase::part_content;
            co_return true;

        // ---- 分块发送当前 part 的 content ----
        case Phase::part_content:
        {
            auto& part = parts_[part_index_];
            if (const auto* ptr = std::get_if<MultipartPartStatic>(std::addressof(part));
                ptr != nullptr)
            {
                std::size_t remaining = ptr->contentSize() - content_offset_;

                if (remaining == 0)
                {
                    // 当前 part content 发完 → 移到下一个 part
                    ++part_index_;
                    current_phase_ = Phase::part_header;
                    co_return co_await fillBuffer(body); // 尾递归填充下一段
                }

                std::size_t chunk = std::min(remaining, 64 * 1024ULL);
                body.data = (void*)(ptr->content().data() + content_offset_); // ← 零拷贝
                body.size = chunk;
                body.more = true;
                content_offset_ += chunk;
                co_return true;
            }
            if (auto* ptr = std::get_if<MultipartPartDynamic>(std::addressof(part)); ptr != nullptr)
            {
                if (!ptr->content())
                {
                    // 当前 part content 发完 → 移到下一个 part
                    ++part_index_;
                    current_phase_ = Phase::part_header;
                    co_return co_await fillBuffer(body); // 尾递归填充下一段
                }
                auto data = co_await ptr->content();
                body.data = (void*)data.data();
                body.size = data.size();
                body.more = true;
                co_return true;
            }
            co_return true;
        }

        // ---- 发送 closing boundary ----
        case Phase::closing:
            body.data = closing_boundary_.data();
            body.size = closing_boundary_.size();
            body.more = false; // ← 整个 response 结束
            current_phase_ = Phase::done;
            co_return true;

        case Phase::done:
            body.data = nullptr;
            body.size = 0;
            body.more = false;
            co_return false;
        }
        co_return false;
    }

    auto MultipartContext::getBoundary() const noexcept -> std::string_view { return boundary_; }
} // namespace netdisk::core::http::message
