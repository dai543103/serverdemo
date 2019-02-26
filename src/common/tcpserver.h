#pragma once

#include <asio.hpp>
#include "tcpchannel.h"

namespace battlespheres 
{

    class TcpServer
    {
    public:
        TcpServer(asio::io_service& loop);

        void listenAt(const std::string& ip,const int32_t port, EventCallback fn);

        void connectTo(const std::string& ip, const int32_t port, EventCallback fn);
    private:
        void handle_accept();
    private:
        asio::io_service& loop_;
        asio::ip::tcp::socket socket_;
        std::shared_ptr<asio::ip::tcp::acceptor> acceptor_;

        std::string listen_addr_;
        EventCallback event_fn_;
    };

}