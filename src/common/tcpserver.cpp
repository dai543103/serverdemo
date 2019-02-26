#include "tcpserver.h"

namespace battlespheres
{

    TcpServer::TcpServer(asio::io_service & loop)
        : loop_(loop)
        , socket_(loop)
        , acceptor_()
    {
    }

    void TcpServer::listenAt(const std::string& ip,const int32_t port, EventCallback fn)
    {
        event_fn_ = fn;

        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(ip),port);

        acceptor_ = std::make_shared<asio::ip::tcp::acceptor>(loop_, endpoint);
        acceptor_->set_option(asio::socket_base::reuse_address(true));
        handle_accept();
    }

    void TcpServer::connectTo(const std::string& ip, const int32_t port, EventCallback fn)
    {

        asio::ip::tcp::endpoint endpoint(asio::ip::address::from_string(ip),port);

        auto socket = std::make_shared<asio::ip::tcp::socket>(loop_);
        socket->async_connect(endpoint, [this, socket, fn](asio::error_code ec)
        {
            if (!ec)
            {
                auto newcon = std::make_shared<TcpChannel>(std::move(*socket), loop_);
                newcon->set_event_callback(fn);
                newcon->start();
            }
            else
            {
                fn(TcpChannelEvent::kClosed, nullptr);
            }
        });
    }

    void TcpServer::handle_accept()
    {
        acceptor_->async_accept(socket_, [this](asio::error_code ec)
        {
            if (!ec)
            {
                auto newcon = std::make_shared<TcpChannel>(std::move(socket_),loop_);
                newcon->set_event_callback(event_fn_);
                newcon->start();
            }

            handle_accept();
        });
    }

}
