#include "redisClient.h"
#include "common/logger.h"
#include "common/json.hpp"

RedisClient::RedisClient(asio::io_service& mainloop)
	:logic_loop(mainloop)
{

}

void RedisClient::connect(const std::string& szIP, const uint16_t port /* = 6379 */)
{
	redis_ = std::make_shared<Redis>(logic_loop);
	redis_->connect(szIP, port, [&](bool result, const std::string& v) {
		if (result)
		{
			redis_->link->errorHandler = [&](const std::string& v)
			{
				SPDLOG_ERROR("redis cmd run error:{}", v);
			};
			DEBUG("connect redis success.");
		}
		else
		{
			SPDLOG_ERROR("redis connect error:{}", v);
		}
	});
}

void RedisClient::stop()
{

}

void RedisClient::commit()
{
	if (redis_)
	{
		redis_->link->commit();
	}
}

void RedisClient::zadd(std::string key, uint32_t score, std::string id, std::string name)
{
	redis_->zadd(key, std::to_string(score), id, nullptr);
	redis_->hset(name_rank_key, id, name, nullptr);
	redis_->link->commit();
}

void RedisClient::zremrangebyrank(std::string key, int start, int stop, std::function<void(int)> cb)
{
	redis_->zremrangebyrank(key, start, stop, [cb](const RedisValue& v) {
		cb(v.integer);
	});

	redis_->link->commit();
}

void RedisClient::zrevrange(std::string key, int min, int max, std::function<void(int, std::shared_ptr<std::vector<TRankInfo>>, std::shared_ptr<std::map<std::string, std::string>>) > cb)
{
	redis_->zrevrange(key, std::to_string(min), std::to_string(max), true, [this, cb](const RedisValue& v) {
		if (v.type == v.reply_type::null)
		{
			SPDLOG_INFO("redis recv failed,type=null");
			logic_loop.dispatch([cb]() { cb(-1, nullptr, nullptr); });
		}
		else
		{
			int pos = 0;
			std::vector<TRankInfo> ranks;
			std::vector<std::string> ids;

			auto it = std::begin(v.elements);
			while (it != v.elements.end())
			{
				TRankInfo info;
				info.index = pos + 1;
				info.id = std::stoi(std::string(it->str.data(), it->str.size()));
				ids.push_back(std::string(it->str.data(), it->str.size()));
				it++;
			
				try
				{
					info.score = std::stoi(std::string(it->str.data(), it->str.size()));
				}
				catch (const std::exception& e)
				{
					info.score = 10000;
					SPDLOG_ERROR(" string error: stoi,{}", info.id);
				}
				
				it++;
				pos++;
				ranks.push_back(info);
			}


			redis_->hmget(name_rank_key, ids, [this, cb, ids, ranks](const RedisValue& v)
			{
				std::map<std::string, std::string> id_names;
				if (v.type == RedisValue::reply_type::array)
				{
					int i = 0;
					std::string szempty = "null_name";
					for (auto& a : v.elements)
					{
						if (a.type == RedisValue::reply_type::null)
						{
							id_names[ids[i]] = szempty;
						}
						else
						{
							id_names[ids[i]] = std::string(a.str.data(), a.str.size());
						}

						i++;
					}
				}
				else
				{
					if (!ids.empty())
					{
						id_names.insert(std::make_pair(ids[0], std::string(v.str.data(), v.str.size())));
					}
				}

				auto rankptr = std::make_shared<std::vector<TRankInfo>>(ranks);
				auto nameptr = std::make_shared<std::map<std::string, std::string>>(id_names);
				logic_loop.dispatch([cb, rankptr, nameptr]() { cb(0, rankptr, nameptr); });
			});
		}
	});
	redis_->link->commit();
}

void RedisClient::zscore(std::string key, std::string member, std::function<void(int)> cb)
{
	redis_->zscore(key, member, [this, cb, member](const RedisValue& v) {
		int score = 0;
		if (v.type != v.reply_type::null)
		{
			int score = std::stoi(std::string(v.str.data(), v.str.size()));
		}

		logic_loop.dispatch([cb, score]() { cb(score); });
	});
	redis_->link->commit();
}

void RedisClient::hset(std::string key, uint32_t uid, std::string name)
{
	redis_->hset(key, std::to_string(uid), name, nullptr);
}

void RedisClient::hget(std::string key, uint32_t uid, std::function<void(int error_code, std::string)>cb)
{
	redis_->hget(key, std::to_string(uid), [this, cb](const RedisValue& v) {
		if (v.type != v.reply_type::null)
		{
			auto name = std::string(v.str.data(), v.str.size());
			logic_loop.dispatch([cb, name]() { cb(0, name); });
		}
		else
		{
			logic_loop.dispatch([cb]() { cb(-1, nullptr); });
		}


	});

	redis_->link->commit();
}



