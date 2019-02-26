#pragma once
#include "asio.hpp"
#include "common/redis.h"
#include "forwarddecl.h"
#include <memory>
#include <thread>

class RedisClient
{
public:
	RedisClient(asio::io_service& mainloop);
	void connect(const std::string& szIP,const uint16_t port = 6379);
	void stop();
	void commit();

	void zadd(std::string key,uint32_t score,std::string member, std::string name);
	void zremrangebyrank(std::string key, int start, int stop, std::function<void(int)> cb);
	void zrevrange(std::string key, int min, int max, std::function<void(int,std::shared_ptr<std::vector<TRankInfo>>,std::shared_ptr<std::map<std::string,std::string>>) >cb);
	void zscore(std::string key, std::string member,std::function<void(int)> cb);
	void hset(std::string key, uint32_t uid, std::string name);
	void hget(std::string key, uint32_t uid, std::function<void(int,std::string) >cb);
private:
	asio::io_service& logic_loop;
	std::shared_ptr<Redis> redis_;
};