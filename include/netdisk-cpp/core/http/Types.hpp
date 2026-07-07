#pragma once

#include "netdisk-cpp/core/http/Request.hpp"
#include "netdisk-cpp/utils/url/Matches.hpp"

#include <boost/asio/io_context.hpp>
#include <boost/asio/ssl/context.hpp>
#include <boost/beast.hpp>
#include <boost/cobalt.hpp>

namespace netdisk::core::http
{
    class Connection;
    class Config;

    using ExecutorType = boost::cobalt::use_op_t::executor_with_default<boost::cobalt::executor>;
    using SocketType = boost::asio::ip::tcp::socket::rebind_executor<ExecutorType>::other;
    using SSLSocketType = boost::asio::ssl::stream<SocketType>;
    using AcceptorType = boost::asio::ip::tcp::acceptor::rebind_executor<ExecutorType>::other;
    using WebsocketType = boost::beast::websocket::stream<SSLSocketType>;
    using RequestHandler = std::function<boost::cobalt::task<Request>(
        boost::beast::http::request_parser<boost::beast::http::empty_body>&, SSLSocketType&,
        boost::beast::flat_buffer&, const boost::urls::matches&, Config&, std::uint64_t,
        std::any&)>;
    using ResponseHandler = std::function<boost::cobalt::task<void>(
        Connection&, const boost::urls::matches&, Config&, std::uint64_t, std::any&)>;

    using AuthorizationHandler = std::function<bool(RequestView&, Config&)>;
} // namespace netdisk::core::http
