#include "tcpchannel.h"
#include <iostream>

namespace battlespheres
{
    const uint32_t kInputsize = 8192;

    TcpChannel::TcpChannel(asio::ip::tcp::socket && socket,asio::io_service& loop)
        : socket_(std::move(socket))
        , input_buffer_(kInputsize)
        , status_(kClosed)
        , close_after_complete_(false)
    {
		updateTimer_ = std::make_shared<asio::system_timer>(loop);
        remote_endpoint_ = socket_.remote_endpoint();
        local_endpoint_ = socket_.local_endpoint();
        output_buffer_[0].reserve(kInputsize);
        output_buffer_[1].reserve(kInputsize);
    }

    TcpChannel::~TcpChannel()
    {
    }

	void TcpChannel::check()
	{
		auto update_func = std::make_shared<std::function<void()>>();
		*update_func = [this, update_func]()
		{
			updateTimer_->expires_from_now(std::chrono::minutes(10));
			updateTimer_->async_wait([this, update_func](asio::error_code ec)
			{
				if (!ec)
				{
						auto diff = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - m_aliveTime).count();
						if (diff > 30)
						{
							//std::cout << "[socket]  time out,socket close" << std::endl;
							if (event_fn_) {
								event_fn_(TcpChannelEvent::kClosed, shared_from_this());
								asio::error_code ec;
								socket_.shutdown(asio::socket_base::shutdown_both, ec);
							}
						}
						else
						{
							//std::cout << "[socket]  check again" << std::endl;
							(*update_func)();
						}	
				}
			});
		};

		(*update_func)();
	}

    void TcpChannel::start()
    {
        status_ = kConnected;
        if (event_fn_)
            event_fn_(TcpChannelEvent::kConnected, shared_from_this());

		check();
        readInLoop();
    }

    void TcpChannel::close(bool wait_send_complete/* = true*/)
    {
        if (status_ == kConnected)
        {
            status_ = kDisconnecting;

            close_after_complete_ = wait_send_complete;
            if (output_buffer_[kcachebuf].empty() && output_buffer_[ksendbuf].empty())
            {
                asio::error_code ec;
                socket_.shutdown(asio::socket_base::shutdown_both, ec);
            }
        }
    }

    void TcpChannel::send(memory_block_t block)
    {
        if (status_ == kConnected)
        {
            output_buffer_[kcachebuf].push_back(block);
            sendInLoop();
        }
    }

    void TcpChannel::readInLoop()
    {
		m_aliveTime = std::chrono::system_clock::now();
        input_buffer_.normalize();
        input_buffer_.ensurefreespace();

        socket_.async_read_some(asio::buffer(input_buffer_.writeptr(), input_buffer_.remainingspace()),
            [this, self = shared_from_this()](asio::error_code ec, size_t bytes_transferred) {
            if (!ec)
            {
                input_buffer_.writecomplete(bytes_transferred);

                if (event_fn_)
                    event_fn_(TcpChannelEvent::kMessage, self);

                readInLoop();
            }
            else
            {
                if (status_ != kClosed)
                {
                    status_ = kClosed;
                    if (event_fn_)
                        event_fn_(TcpChannelEvent::kClosed, self);
                }
            }
        });
    }

    bool TcpChannel::sendInLoop()
    {
        if (!output_buffer_[kcachebuf].empty() && output_buffer_[ksendbuf].empty())
        {
            std::swap(output_buffer_[kcachebuf], output_buffer_[ksendbuf]);
            assert(output_buffer_[kcachebuf].empty());
            assert(!output_buffer_[ksendbuf].empty());

            std::vector<asio::const_buffer> buffers;
            size_t bytes_to_transferred = 0;
            for (auto & block : output_buffer_[ksendbuf])
            {
                buffers.emplace_back(asio::const_buffer(block->buf_.data(), block->buf_.size()));
                bytes_to_transferred += block->buf_.size();
            }

            asio::async_write(socket_, buffers,
                [this, self = shared_from_this(), bytes_to_transferred](asio::error_code ec, size_t bytes_transferred) {
                if (!ec)
                {
                    assert(bytes_transferred == bytes_to_transferred);
                    output_buffer_[ksendbuf].clear();
                    if (!sendInLoop() && close_after_complete_)
                    {
                        asio::error_code ec;
                        socket_.shutdown(asio::socket_base::shutdown_both, ec);
                    }
                }
                else
                {
                    if (status_ != kClosed)
                    {
                        status_ = kClosed;
                        if (event_fn_)
                            event_fn_(TcpChannelEvent::kClosed, self);
                    }
                }
            });

            return true;
        }
        return false;
    }

}
