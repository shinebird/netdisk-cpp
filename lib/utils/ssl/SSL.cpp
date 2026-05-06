#include <boost/asio.hpp>

#include <netdisk-cpp/utils/ssl/SSL.hpp>
#include <netdisk-cpp/utils/ssl/SSLKey.hpp>

namespace netdisk::utils::ssl
{
    void configureSSLContext(boost::asio::ssl::context& ctx)
    {
        namespace ssl = boost::asio::ssl;
        ctx.set_options(ssl::context::no_sslv2 | ssl::context::default_workarounds |
                        ssl::context::single_dh_use);
        ctx.set_password_callback(
            [](std::size_t, boost::asio::ssl::context_base::password_purpose) -> const char*
            { return "test"; });
        ctx.use_certificate_chain(boost::asio::buffer(data::ssl::cert_pem));
        ctx.use_private_key(boost::asio::buffer(data::ssl::key_pem), ssl::context::pem);

        ctx.use_tmp_dh(boost::asio::buffer(data::ssl::dh));
    }
} // namespace netdisk::utils::ssl
