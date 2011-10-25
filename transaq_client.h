#pragma once

#include <deque>
#include <boost/asio.hpp>
#include <boost/asio/ssl.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/asio/signal_set.hpp>

namespace transaq
{
    class client
    {
    public :
        client
        (
            std::string const& host,
            std::string const& port,
            std::string const& libpath,
            std::string const& keyfile,
            std::string const& certfile,
            std::string const& logfile
        );
        ~client();

        void start();
        void stop();

    private :
        void start_read();
        void handle_read_size(boost::system::error_code err);
        void handle_read_data(boost::system::error_code err);
        void handle_signal(boost::system::error_code const& err, int signal_number);

        bool handle_data(std::string const& data); // called from different thread

        void write(std::string const& data);
        void start_write();
        void handle_write(boost::system::error_code err);

    private :
        typedef boost::asio::ip::tcp::socket        socket_t;
        typedef boost::asio::ssl::stream<socket_t>  ssl_socket_t;

    private :
        uint32_t                        recv_size;
        std::vector<char>               recv_buffer;

        uint32_t                        send_size;
        std::deque<std::string>         send_buffer;

        boost::asio::io_service         service;
        boost::asio::ssl::context       context;
        boost::shared_ptr<ssl_socket_t> socket;
        boost::asio::signal_set         sigset;
    };
}