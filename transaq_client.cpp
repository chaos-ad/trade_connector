#include "logger.h"
#include "transaq_client.h"
#include "transaq_wrapper.h"

#include <iostream>
#include <boost/bind.hpp>
#include <boost/array.hpp>
#include <boost/date_time/posix_time/ptime.hpp>
#include <boost/date_time/posix_time/posix_time_io.hpp>


boost::posix_time::ptime get_time()
{
    return boost::posix_time::second_clock::local_time();
}

namespace
{
    void throw_error(const boost::system::error_code& err)
    {
        if (err)
        {
			std::clog << get_time() << ": Error: " << err.message() << std::endl;
            boost::system::system_error e(err);
            boost::throw_exception(e);
        }
    }
}

namespace transaq {


client::client
(
    std::string const& host,
    std::string const& port,
    std::string const& libpath,
    std::string const& keyfile,
    std::string const& certfile,
    std::string const& logfile
)
    : send_size(0)
    , recv_size(0)
    , context(boost::asio::ssl::context::sslv3)
    , sigset(service, SIGINT, SIGTERM)
{
    if (!logfile.empty())
    {
        logger::install(logfile, true);
    }

    boost::asio::ssl::verify_mode mode =
        boost::asio::ssl::verify_peer &
        boost::asio::ssl::verify_fail_if_no_peer_cert;

    context.set_verify_mode(mode);
    context.use_private_key_file(keyfile, boost::asio::ssl::context::pem);
    context.use_certificate_file(certfile, boost::asio::ssl::context::pem);

    std::clog << get_time() << ": Using private key: " << keyfile  << std::endl;
    std::clog << get_time() << ": Using certificate: " << certfile << std::endl;
    std::clog << get_time() << ": Connecting to host " << host << ":" << port << "..." << std::endl;

    socket.reset(new ssl_socket_t(service, context));

    boost::asio::ip::tcp::resolver::query query(host, port);
    boost::asio::ip::tcp::resolver resolver(service);
    boost::asio::connect(socket->lowest_layer(), resolver.resolve(query));
    socket->handshake(boost::asio::ssl::stream_base::client);

    std::clog << get_time() << ": Connected" << std::endl;

    wrapper::start(boost::bind(&client::handle_data, this, _1), libpath);
    sigset.async_wait(boost::bind(&client::handle_signal, this, _1, _2));
    start_read();
}

client::~client()
{
    wrapper::stop();
    logger::uninstall();
}

void client::start()
{
    service.run();
}

void client::stop()
{
    service.stop();
}

void client::start_read()
{
    boost::asio::async_read
    (
        *socket,
        boost::asio::buffer(&recv_size, sizeof(recv_size)),
        boost::bind(&client::handle_read_size, this, _1)
    );
}

void client::handle_read_size(boost::system::error_code err)
{
    throw_error(err);
    recv_size = ntohl(recv_size);
    recv_buffer.resize(recv_size, 0);
    boost::asio::async_read
    (
        *socket,
        boost::asio::buffer(recv_buffer),
        boost::bind(&client::handle_read_data, this, _1)
    );
}

void client::handle_read_data(boost::system::error_code err)
{
    throw_error(err);
	std::string command(recv_buffer.data(), recv_buffer.size());
    std::clog << get_time() << ": Sending command:" << std::endl << command << std::endl << std::endl;
	write( transaq::wrapper::send_command(command) );
	start_read();
}

// called from transaq thread:
bool client::handle_data(std::string const& data)
{
    service.post(boost::bind(&client::write, this, data));
    return true;
}

void client::handle_signal(boost::system::error_code const& err, int signal_number)
{
    if(!err)
    {
        stop();
    }
}

void client::write(std::string const& data)
{
    send_buffer.push_back(data);
    start_write();
}

void client::start_write()
{
    if (!send_size && !send_buffer.empty())
    {
        send_size = htonl(send_buffer.front().size());
        boost::array<boost::asio::const_buffer, 2> buffers;
        buffers[0] = boost::asio::buffer(&send_size, sizeof(send_size));
        buffers[1] = boost::asio::buffer(send_buffer.front());
        boost::asio::async_write
        (
            *socket,
            buffers,
            boost::bind(&client::handle_write, this, _1)
        );
    }
}

void client::handle_write(boost::system::error_code err)
{
    throw_error(err);
    send_buffer.pop_front();
    send_size = 0;
    start_write();
}

}
