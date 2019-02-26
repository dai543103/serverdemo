#include "battleserver.h"
#include "forwarddecl.h"
#include "battleroom.h"
#include "battleuser.h"
#include "common/logger.h"
#include "common/tcpserver.h"
#include <memory>
#include "httplib/server.hpp"


battleserver::battleserver()
    : server_(loop_)
{
	dbaccess_ = std::make_shared<DBAccess>(loop_);
	redisdb_ = std::make_shared<RedisClient>(loop_);
	lobby_ = std::make_shared<lobby>();
}
battleserver::~battleserver()
{
}

void battleserver::start(nlohmann::json& j)
{
    //启动http服务
	try
	{
		auto thread_ = std::make_shared<std::thread>([this]() {
			http::server::server s("0.0.0.0", "8888");

			SPDLOG_INFO("start http server 8888");
			s.run();
		});
		thread_->detach();
	}
	catch (std::exception& e)
	{
		SPDLOG_ERROR("http exception,{}", e.what());
	}

    //mysql连接
	if (!dbaccess_->connect(j["mysql"]["ip"].get<std::string>().c_str(), j["mysql"]["host"].get<std::string>().c_str(), j["mysql"]["password"].get<std::string>().c_str(), j["mysql"]["name"].get<std::string>().c_str(), j["mysql"]["port"]))
	{
        std::cout<<"123";
		//return;
	}

    //redis连接
	redisdb_->connect(j["redis"]["ip"].get<std::string>().c_str());

    //network
    server_.listenAt(j["ip"], j["port"], std::bind(&battleserver::handleEvent, this, std::placeholders::_1, std::placeholders::_2));
	updateTimer_ = std::make_shared<asio::system_timer>(loop_);
	lobby_->Tick(updateTimer_);
    loop_.run();
}

void battleserver::handleEvent(battlespheres::TcpChannelEvent ev, const battlespheres::TcpChannelPtr & channel)
{
    if (ev == battlespheres::TcpChannelEvent::kConnected)
    {
		SPDLOG_INFO("client ({}) connected", channel->socket().remote_endpoint());
    }
    else if (ev == battlespheres::TcpChannelEvent::kClosed)
    {
		SPDLOG_INFO("client ({}) closed", channel->remote_endpoint());
        auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(channel->context());
        if (user)
        {
			user->room_.reset();
            channel->set_context(battlespheres::Any(nullptr));
			lobby_->removeUser(user->getuid());
        }
    }
    else if (ev == battlespheres::TcpChannelEvent::kMessage)
    {
        auto& buf = channel->inputbuffer();
        char* dataptr = (char*)buf.readptr();
        int length = buf.activesize();
        while (length > 4)//len cmd data
        {
            int len = *reinterpret_cast<int*>(dataptr);
            if ((length - 4) >= len)
            {
                int cmd = *reinterpret_cast<int*>(dataptr + 4);
				g_eventhandle->setMsg(channel, cmd, dataptr + 8, len - 4);
				g_eventhandle->dispach();

                dataptr += 4 + len;
                length -= len + 4;
            }
            else
                break;
        }

        buf.readcomplete((uint8_t*)dataptr - buf.readptr());
        buf.normalize();
    }
}















