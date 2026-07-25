#pragma once

#include <boost/beast/http/buffer_body.hpp>
#include <boost/cobalt/generator.hpp>
#include <boost/cobalt/task.hpp>

#include "netdisk-cpp/core/http/message/Field.hpp"
#include "netdisk-cpp/utils/concept/Common.hpp"

#include <limits>
#include <optional>
#include <span>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

namespace netdisk::core::http::message
{
    namespace internal
    {
        struct PartHeaderProbe
        {
                template <class T> static auto type(const T& t) -> decltype(t.part_header_);
        };
    } // namespace internal

    template <class T>
    concept MultipartPart = requires(T value) {
        utils::reflect::StringLike<
            std::remove_cvref_t<decltype(internal::PartHeaderProbe::type(value))>>;
    };

    class MultipartPartStatic
    {
            friend struct internal::PartHeaderProbe;
            template <typename ContentType, typename ReturnType, typename... Args>
            friend auto makePartImpl(std::string_view boundary, std::string_view field_name,
                                     std::string_view filename, std::string_view mime_type,
                                     ContentType content, Args&&... args) -> ReturnType;

            template <typename ContentType, typename ReturnType, typename... Args>
            friend auto makePartImpl(std::string_view boundary, std::string_view field_name,
                                     std::string_view mime_type, ContentType content,
                                     Args&&... args) -> ReturnType;

            template <MultipartPart Part>
            friend auto addFieldsImpl(Part& part, const std::span<FieldView>& fields) -> void;

        public:
            [[nodiscard]] auto size() const noexcept -> std::size_t;
            [[nodiscard]] auto contentSize() const noexcept -> std::size_t;
            [[nodiscard]] auto partHeader() const noexcept -> std::string_view;
            [[nodiscard]] auto content() const noexcept -> std::string_view;
            template <FieldLike... Fields> auto addFields(const Fields&... fields) -> void
            {
                std::array<const FieldView, sizeof...(fields)> fields_array = {
                    fields...,
                };
                addFields(fields_array);
            }
            auto addFields(const std::span<FieldView>& fields) -> void;

            static auto makePart(std::string_view boundary, std::string_view field_name,
                                 std::string_view filename, std::string_view mime_type,
                                 std::string_view content) -> MultipartPartStatic;
            static auto makePart(std::string_view boundary, std::string_view field_name,
                                 std::string_view mime_type, std::string_view content)
                -> MultipartPartStatic;

        private:
            std::string part_header_; // "--boundary\r\n...headers...\r\n\r\n"
            // 外部内存，零拷贝
            // footer 不需要单独存储：除最后一个 part 外，下一个 part 的 header
            // 天然充当分隔符；最后一个 part 之后由 context 追加 closing boundary
            std::string_view content_;
    };

    class MultipartPartDynamic
    {
            friend struct internal::PartHeaderProbe;
            template <typename ContentType, typename ReturnType, typename... Args>
            friend auto makePartImpl(std::string_view boundary, std::string_view field_name,
                                     std::string_view filename, std::string_view mime_type,
                                     ContentType content, Args&&... args) -> ReturnType;

            template <typename ContentType, typename ReturnType, typename... Args>
            friend auto makePartImpl(std::string_view boundary, std::string_view field_name,
                                     std::string_view mime_type, ContentType content,
                                     Args&&... args) -> ReturnType;

            template <MultipartPart Part>
            friend auto addFieldsImpl(Part& part, const std::span<FieldView>& fields) -> void;

        public:
            explicit MultipartPartDynamic(std::size_t content_size) noexcept;
            [[nodiscard]] auto size() const noexcept -> std::size_t;
            [[nodiscard]] auto contentSize() const noexcept -> std::size_t;
            [[nodiscard]] auto partHeader() const noexcept -> std::string_view;
            auto content() -> boost::cobalt::generator<std::string_view>&;
            template <FieldLike... Fields> auto addFields(const Fields&... fields) -> void
            {
                std::array<const FieldView, sizeof...(fields)> fields_array = {
                    fields...,
                };
                addFields(fields_array);
            }
            auto addFields(const std::span<FieldView>& fields) -> void;
            static auto makePart(std::string_view boundary, std::string_view field_name,
                                 std::string_view filename, std::string_view mime_type,
                                 boost::cobalt::generator<std::string_view> content,
                                 std::size_t size = std::numeric_limits<std::size_t>::max())
                -> MultipartPartDynamic;
            static auto makePart(std::string_view boundary, std::string_view field_name,
                                 std::string_view mime_type,
                                 boost::cobalt::generator<std::string_view> content,
                                 std::size_t size = std::numeric_limits<std::size_t>::max())
                -> MultipartPartDynamic;

        private:
            std::string part_header_; // "--boundary\r\n...headers...\r\n\r\n"
            std::size_t content_size_;
            boost::cobalt::generator<std::string_view> content_ =
                boost::cobalt::noop<std::string_view>();
    };

    class MultipartContext
    {
        public:
            using MultipartPart = std::variant<MultipartPartStatic, MultipartPartDynamic>;
            explicit MultipartContext(std::string boundary, std::vector<MultipartPart> parts);
            [[nodiscard]] auto totalSize() noexcept -> std::optional<std::size_t>;
            auto fillBuffer(boost::beast::http::buffer_body::value_type& body)
                -> boost::cobalt::task<bool>;
            [[nodiscard]] auto getBoundary() const noexcept -> std::string_view;

        private:
            std::string boundary_;
            std::string closing_boundary_; // "\r\n--boundary--\r\n"
            std::vector<MultipartPart> parts_;

            enum class Phase : char
            {
                part_header,
                part_content,
                closing,
                done,
            };
            Phase current_phase_ = Phase::part_header;
            std::size_t part_index_ = 0;     // 当前正在发送的 part
            std::size_t content_offset_ = 0; // 当前 part 已发送偏移
            std::size_t total_size_ = 0;
            bool use_trunk_ = false;
    };
} // namespace netdisk::core::http::message
