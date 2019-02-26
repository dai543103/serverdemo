#pragma once

#include <asio.hpp>
#include <memory>
#include <functional>
#include <vector>
#include <array>
#include "any.hpp"
#include <queue>
#include "buffer.hpp"
#include <chrono>
#include "asio/system_timer.hpp"

namespace battlespheres
{
    class TcpChannel;
    using TcpChannelPtr = std::shared_ptr<TcpChannel>;

    struct memory_block
    {
        memory_block(size_t length) :
            buf_(length)
        {

        }

    protected:
        friend class TcpChannel;
        std::vector<char> buf_;
    };

    typedef std::shared_ptr<memory_block> memory_block_t;

    enum TcpChannelEvent
    {
        kClosed = 1,
        kConnected,
        kMessage,
    };

    using EventCallback = std::function<void(TcpChannelEvent, const TcpChannelPtr&)>;

    class TcpChannel : public std::enable_shared_from_this<TcpChannel>
    {
    public:
        enum Status {
            kClosed = 0,
            kConnected = 1,
            kDisconnecting = 2
        };

        TcpChannel(asio::ip::tcp::socket&& socket,asio::io_service& loop);

        ~TcpChannel();

        void start();

		void check();

        void close(bool wait_send_complete = false);

        void send(memory_block_t block);
    public:
        void set_event_callback(EventCallback fn) {
            event_fn_ = fn;
        }
        void set_context(Any&& c) {
            context_[0] = c;
        }
        const Any& context() const {
            return context_[0];
        }

        void set_context(int index, Any&& c) {
            assert(index < kContextCount && index >= 0);
            context_[index] = c;
        }
        const Any& context(int index) const {
            assert(index < kContextCount && index >= 0);
            return context_[index];
        }
        Status status() const {
            return status_;
        }
        asio::ip::tcp::socket& socket() {
            return socket_;
        }
        buffer& inputbuffer() {
            return input_buffer_;
        }
        const asio::ip::tcp::endpoint& remote_endpoint() const {
            return remote_endpoint_;
        }
        const asio::ip::tcp::endpoint& local_endpoint() const {
            return local_endpoint_;
        }
    private:
        void readInLoop();
        bool sendInLoop();
    private:
        asio::ip::tcp::socket socket_;
        buffer input_buffer_;

		std::shared_ptr<asio::system_timer> updateTimer_;
		std::chrono::time_point<std::chrono::system_clock> m_aliveTime;

        enum { kcachebuf, ksendbuf, kbufnum };
        std::array<std::vector<memory_block_t>, kbufnum> output_buffer_;

        enum { kContextCount = 16 };
        Any context_[kContextCount];
        Status status_;
        bool close_after_complete_;

        EventCallback event_fn_;

        asio::ip::tcp::endpoint remote_endpoint_;
        asio::ip::tcp::endpoint local_endpoint_;
    };

}
