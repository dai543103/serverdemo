#pragma once

#include "forwarddecl.h"
#include "soap.pb.h"
#include "common/SingletonTemplate.hpp"
#include "netpacket.h"
#include "common/tcpserver.h"
#include "lobby.h"
#include "redisClient.h"
#include "dbaccess.h"
#include "common/json.hpp"


class battleserver
{
public:
    battleserver();
	~battleserver();

    void start(nlohmann::json& j);
    void handleEvent(battlespheres::TcpChannelEvent ev, const battlespheres::TcpChannelPtr& channel);
	inline std::shared_ptr<DBAccess> GetDbAccess() { return dbaccess_; }
	inline std::shared_ptr<lobby> GetLobby() { return lobby_; }
	inline std::shared_ptr<RedisClient> GetRedisDB() { return redisdb_; }
	inline asio::io_service* GetIOService() { return &loop_; }
    
private:
    asio::io_service loop_;
	std::shared_ptr<asio::system_timer> updateTimer_;
    battlespheres::TcpServer server_;
	std::shared_ptr<DBAccess> dbaccess_;
	std::shared_ptr<RedisClient> redisdb_;
	std::shared_ptr<lobby> lobby_;
	std::shared_ptr<std::thread> thread_;
};

#define g_battleserver  CSingleton<battleserver>::Instance()
#define g_battledb   g_battleserver->GetDbAccess()
#define g_battlelobby g_battleserver->GetLobby()
#define g_redisdb g_battleserver->GetRedisDB()