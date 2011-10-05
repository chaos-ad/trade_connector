#include "transaq_client.h"
#include "transaq_wrapper.h"

#include <boost/bind.hpp>
#include <boost/array.hpp>

namespace
{
    void throw_error(const boost::system::error_code& err)
    {
        if (err)
        {
            boost::system::system_error e(err);
            boost::throw_exception(e);
        }
    }
}

namespace transaq {


client::client(const std::string& host, const std::string& port, std::string const& path)
    : service()
    , send_size(0)
    , recv_size(0)
    , socket(service)
{
    boost::asio::ip::tcp::resolver::query query(host, port);
    boost::asio::ip::tcp::resolver resolver(service);
    boost::asio::ip::tcp::endpoint endpoint;
    socket.connect(*resolver.resolve(query));

    wrapper::start(boost::bind(&client::handle_data, this, _1), path);
    start_read();
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
    socket.async_receive
    (
        boost::asio::buffer(&recv_size, sizeof(recv_size)),
        boost::bind(&client::handle_read_size, this, _1)
    );
}

void client::handle_read_size(boost::system::error_code err)
{
    throw_error(err);
    recv_size = ntohl(recv_size);
    recv_buffer.resize(recv_size, 0);
    socket.async_receive
    (
        boost::asio::buffer(recv_buffer),
        boost::bind(&client::handle_read_data, this, _1)
    );
}

void client::handle_read_data(boost::system::error_code err)
{
    throw_error(err);
	std::string command(recv_buffer.data(), recv_buffer.size());
    if (command == "PING")
    {
        write("PONG");
    }
    else
    {
        write( transaq::wrapper::send_command(command) );
    }
    start_read();
}

bool client::handle_data(std::string const& data)
{
    service.post(boost::bind(&client::write, this, data));
    return true;
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
        socket.async_send
        (
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
