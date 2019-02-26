#include "lobby.h"
#include "common/base64.h"
#include "common/md5.h"
#include "common/json.hpp"
#include "battleuser.h"
#include "battleroom.h"
#include "battleserver.h"
#include "common/logger.h"
#include <chrono>

lobby::lobby()
{
	//wait_room.reset();
	m_LastStartTime = std::chrono::system_clock::now();
	m_isClearDayRank = false;
}

int lobby::checkToken(const std::string& token, TokenInfo& tokenInfo)
{
 	std::string decodetoken; 
	decodetoken.resize(token.size() * 2);
 	int decodelen = 0;
 
 	base64_decode(token, (unsigned char*)decodetoken.data(), decodelen);
 	if (decodelen <= 0)
 	{
 		SPDLOG_WARN("token is null!");
 		return -1;
 	}
 
 	decodetoken.resize(decodelen);
	try
	{
		nlohmann::json decodejson = nlohmann::json::parse(decodetoken);
		tokenInfo.uid = decodejson["u"];
		tokenInfo.timestamp = decodejson["t"];
		tokenInfo.token = decodejson["m"];
	}
	catch (const std::exception& e)
	{
		SPDLOG_ERROR("login token ({}) json parse exception ({})", token, e.what());
		return -2;
	}

	return 0;
}

void lobby::handleLoginReq(const battlespheres::TcpChannelPtr & channel, soapproto::LoginReq & loginReq)
{
	TokenInfo tokenInfo;
	int errorCode = checkToken(loginReq.token(), tokenInfo);
	if (errorCode != 0)
	{
		soapproto::LoginRsp loginRsp;
		loginRsp.set_result(soapproto::error_code::token_verify_error);
		loginRsp.set_timestamp(std::time(nullptr));
		loginRsp.set_millisecond(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
		channel->send(netpacket::create(soapproto::msgId::cmdLoginRsp, loginRsp));

		return;
	}

	if (errorCode == 0)
	{
		mogo::CMd5 md5(fmt::format("{}{}{}", tokenInfo.uid, tokenInfo.timestamp, "123456"));
		std::string md5str = md5.toString();
		if (md5str != tokenInfo.token)
		{
			soapproto::LoginRsp loginRsp;
			loginRsp.set_result(soapproto::error_code::token_verify_error);
			loginRsp.set_timestamp(std::time(nullptr));
			loginRsp.set_millisecond(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
			channel->send(netpacket::create(soapproto::msgId::cmdLoginRsp, loginRsp));
			SPDLOG_WARN("token not match {} != {}", md5str, tokenInfo.token);
		}
		else
		{
			auto now = std::time(nullptr);
			if (now - tokenInfo.timestamp > CLIENT_KEEP_TOKEN_TIME)
			{
				soapproto::LoginRsp loginRsp;
				loginRsp.set_result(soapproto::error_code::token_verify_expired);
				loginRsp.set_timestamp(std::time(nullptr)); 
				loginRsp.set_millisecond(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
				channel->send(netpacket::create(soapproto::msgId::cmdLoginRsp, loginRsp));
				SPDLOG_WARN("login time expired {} - {} > 10", now, tokenInfo.timestamp);
				return;
			}

			auto user = getOnlineUser(tokenInfo.uid);
			if (user)
			{
				if (user->channel)
				{
					user->room_.reset();
					user->channel->set_context(battlespheres::Any(nullptr));
					user->channel->close(true);
					user->channel = nullptr;
				}

				user->channel = channel;
				channel->set_context(battlespheres::Any(user));

				soapproto::LoginRsp loginRsp;
				loginRsp.set_result(0);
				loginRsp.set_timestamp(std::time(nullptr));
				loginRsp.set_millisecond(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
				channel->send(netpacket::create(soapproto::msgId::cmdLoginRsp, loginRsp));
				return;
			}
			
			g_battledb->load_userinfo(tokenInfo.uid, [this, channel, tokenInfo](int result, std::shared_ptr<DBAccess::tbl_userinfo> userdata)
			{
				if (channel->status() != battlespheres::TcpChannel::kConnected)
					return;

				int error_code = soapproto::error_code::success;
				if (result != 0)
				{
					error_code = soapproto::error_code::server_is_busy;
				}
				else
				{
					auto user = std::make_shared<battleuser>(std::get<0>(*userdata), channel);
					user->loadUser(userdata);
					users_.emplace(user->getuid(), user);
				}

				soapproto::LoginRsp loginRsp;
				loginRsp.set_result(error_code);
				loginRsp.set_timestamp(std::time(nullptr));
				loginRsp.set_millisecond(std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count());
				channel->send(netpacket::create(soapproto::msgId::cmdLoginRsp, loginRsp));
			});
		}
	}
}

void lobby::rechargeNotify(uint32_t error_code, std::string error_reason, int32_t uid)
{
	auto user = getOnlineUser(uid);
	if (user)
	{
		soapproto::RechargeNotify notify;
		notify.set_error_code(error_code);
		notify.set_uid(uid);
		notify.set_error_reason(error_reason);
		user->channel->send(netpacket::create(soapproto::cmdRechargeNotify, notify));
		SPDLOG_INFO("send to clent http msg,{}", uid);
	}
	else
	{
		SPDLOG_ERROR("玩家尚未登陆");
	}
}

void lobby::rechargeNotify(uint32_t error_code,int32_t uid,uint32_t gold,uint32_t diamond, uint32_t rmbcount,std::string szitems,std::string szgift)
{
	auto user = getOnlineUser(uid);
	if (user)
	{
		user->GetData()->golds = gold;
		user->GetData()->diamonds = diamond;

		soapproto::RechargeNotify notify;
		notify.set_error_code(error_code);
		notify.set_uid(uid);
		notify.set_golds(gold);
		notify.set_diamonds(diamond);
		notify.set_items(szitems);
		notify.set_gifts(szgift);
		notify.set_rmbcount(rmbcount);
		user->channel->send(netpacket::create(soapproto::cmdRechargeNotify, notify));
		SPDLOG_INFO("send to clent http msg,{}",uid);
	}
	else
	{
		SPDLOG_ERROR("玩家尚未登陆");
	}
}

void lobby::updateRedis()
{
	g_redisdb->zrevrange(day_bath_rank_key, 0, RANK_NUM - 1, [this](int error_code, std::shared_ptr<std::vector<TRankInfo>> rankptr, std::shared_ptr<std::map<std::string, std::string>> nameptr) {
		if (error_code != 0)
		{
			SPDLOG_ERROR("redis revrange is empty");
		}
		else
		{
			if (nameptr->size() != rankptr->size())
			{
				SPDLOG_ERROR("redis异常");
			}

			SPDLOG_INFO("查询前100排名：{}", day_bath_rank_key);
			m_dayBathRank.clear();
			auto ranks = *rankptr;
			for (auto& info : *rankptr)
			{
				auto mapIt = nameptr->find(std::to_string(info.id));
				if (mapIt != nameptr->end())
				{
					info.name = mapIt->second;
				}

				m_dayBathRank.push_back(std::move(info));
				DEBUG("名次：{},id:{},score:{},name{}", info.index, info.id, info.score,info.name);
			}
		}
	});

	g_redisdb->zrevrange(bath_rank_key, 0, RANK_NUM - 1, [this](int error_code, std::shared_ptr<std::vector<TRankInfo>> rankptr, std::shared_ptr<std::map<std::string, std::string>> nameptr) {
		if (error_code != 0)
		{
			SPDLOG_ERROR("redis revrange is empty");
		}
		else
		{
			if (nameptr->size() != rankptr->size())
			{
				SPDLOG_ERROR("redis异常");
			}
			SPDLOG_INFO("查询前100排名：{}", bath_rank_key);
			m_BathRank.clear();
			auto ranks = *rankptr;
			for (auto& info : *rankptr)
			{
				auto mapIt = nameptr->find(std::to_string(info.id));
				if (mapIt != nameptr->end())
				{
					info.name = mapIt->second;
				}

				m_BathRank.push_back(std::move(info));
				DEBUG("名次：{},id:{},score:{},name{}", info.index, info.id, info.score, info.name);
			}
		}
	});

	g_redisdb->zrevrange(day_health_rank_key, 0, RANK_NUM - 1, [this](int error_code, std::shared_ptr<std::vector<TRankInfo>> rankptr, std::shared_ptr<std::map<std::string, std::string>> nameptr) {
		if (error_code != 0)
		{
			SPDLOG_ERROR("redis revrange is empty");
		}
		else
		{
			if (nameptr->size() != rankptr->size())
			{
				SPDLOG_ERROR("redis异常");
			}

			SPDLOG_INFO("查询前100排名：{}", day_health_rank_key);
			m_dayHealthRank.clear();
			auto ranks = *rankptr;
			for (auto& info : *rankptr)
			{
				auto mapIt = nameptr->find(std::to_string(info.id));
				if (mapIt != nameptr->end())
				{
					info.name = mapIt->second;
				}

				m_dayHealthRank.push_back(std::move(info));
				DEBUG("名次：{},id:{},score:{},name{}", info.index, info.id, info.score, info.name);
			}
		}
	});

	g_redisdb->zrevrange(health_rank_key, 0, RANK_NUM - 1, [this](int error_code, std::shared_ptr<std::vector<TRankInfo>> rankptr, std::shared_ptr<std::map<std::string, std::string>> nameptr) {
		if (error_code != 0)
		{
			SPDLOG_ERROR("redis revrange is empty");
		}
		else
		{
			if (nameptr->size() != rankptr->size())
			{
				SPDLOG_ERROR("redis异常");
			}

			SPDLOG_INFO("查询前100排名：{}", health_rank_key);
			auto ranks = *rankptr;
			m_HealthRank.clear();
			for (auto& info : *rankptr)
			{
				auto mapIt = nameptr->find(std::to_string(info.id));
				if (mapIt != nameptr->end())
				{
					info.name = mapIt->second;
				}

				m_HealthRank.push_back(std::move(info));
				DEBUG("名次：{},id:{},score:{},name{}", info.index, info.id, info.score, info.name);
			}
		}
	});
}

void lobby::reloadItem(int32_t uid)
{
	auto user = getOnlineUser(uid);
	if (user)
	{
		user->loadItem();
	}
}

void lobby::rename(int32_t uid, std::string szname)
{
	bool ischanged = false;
	auto user = getOnlineUser(uid);
	if (user)
	{
		user->GetData()->name = szname;
	}

	auto it=std::find_if(m_dayBathRank.begin(), m_dayBathRank.end(), [uid, szname](TRankInfo &info) { return info.id == uid; });
	if (it != m_dayBathRank.end())
	{
		it->name = szname;
		
		if (!ischanged)
		{
			g_redisdb->hset(name_rank_key, uid, szname);
			ischanged = true;
		}
		
	}    

	it = std::find_if(m_BathRank.begin(), m_BathRank.end(), [uid, szname](TRankInfo &info) { return info.id == uid; });
	if (it != m_BathRank.end())
	{
		it->name = szname;
		if (!ischanged)
		{
			g_redisdb->hset(name_rank_key, uid, szname);
			ischanged = true;
		}
	}

	it = std::find_if(m_dayHealthRank.begin(), m_dayHealthRank.end(), [uid, szname](TRankInfo &info) { return info.id == uid; });
	if (it != m_dayHealthRank.end())
	{
		it->name = szname;
		if (!ischanged)
		{
			g_redisdb->hset(name_rank_key, uid, szname);
			ischanged = true;
		}
	}

	it = std::find_if(m_HealthRank.begin(), m_HealthRank.end(), [uid, szname](TRankInfo &info) { return info.id == uid; });
	if (it != m_HealthRank.end())
	{
		it->name = szname;
		if (!ischanged)
		{
			g_redisdb->hset(name_rank_key, uid, szname);
			ischanged = true;
		}
	}

}

void lobby::handleQueryRankReq(const battlespheres::TcpChannelPtr& channel, soapproto::QueryRankReq& Req)
{
	auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(channel->context());
	if (!user)
	{
		SPDLOG_ERROR("user({}) login request, duplicate login");
		return;
	}

	std::string szRankType;
	std::vector<TRankInfo>* info_;
	if (Req.ranktype() == 1)
	{
		if (Req.range() == 1)
		{
			info_ = &m_dayBathRank;
			szRankType = day_bath_rank_key;
		}
		else
		{
			info_ = &m_BathRank;
			szRankType = bath_rank_key;
		}
	}
	else
	{
		if (Req.range() == 1)
		{
			info_ = &m_dayHealthRank;
			szRankType = day_health_rank_key;
		}
		else
		{
			info_ = &m_HealthRank;
			szRankType = health_rank_key;
		}
	}

	SPDLOG_INFO("send 100排名：{}", day_bath_rank_key);
	soapproto::QueryRankRsp rsp;

	for (auto& info : *info_)
	{
		if (info.id == user->getuid())
		{
			rsp.set_rank(info.index);
		}

		auto temp = rsp.add_infos();
		temp->set_rank(info.index);
		temp->set_id(info.id);
		temp->set_nickname(info.name);
		temp->set_score(info.score);
		DEBUG("名次：{},id:{},score:{}.", info.index, info.id, info.score);
	}

	channel->send(netpacket::create(soapproto::msgId::cmdQueryRankRsp, rsp));
}

void lobby::handleJoinReq(const battlespheres::TcpChannelPtr & channel, soapproto::JoinRoomReq & joinReq)
{
	auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(channel->context());
	if (user == nullptr)
	{
		SPDLOG_ERROR("user info is null");
		soapproto::JoinRoomRsp rsp;
		rsp.set_result(soapproto::error_code::player_info_invalid);
		channel->send(netpacket::create(soapproto::msgId::cmdJoinRoomRsp, rsp));
		return;
	}

	if (user->room_.lock())
	{
		SPDLOG_ERROR("join room failed，player already have room（{}）", user->room_.lock()->getRoomId());
		soapproto::JoinRoomRsp rsp;
		rsp.set_result(soapproto::error_code::player_already_in_room);
		SEND_MESSAGE(channel, soapproto::msgId::cmdJoinRoomRsp, rsp);
		return;
	}

	if (joinReq.type() == 1) //随机匹配
	{
		for (auto& room : running_rooms_)
		{
			if (!room.second->isPrivate() && !room.second->isInBattle())
			{
				room.second->join(user);
				return;
			}
		}

		auto new_room = std::make_shared<battleroom>(user->getuid());
		new_room->join(user);
		running_rooms_.insert({ new_room->getRoomId(), new_room });
	}
	else if (joinReq.type() == 2) //创建房间
	{
		auto new_room = std::make_shared<battleroom>(user->getuid(),true);
		new_room->join(user);
		running_rooms_.insert({ new_room->getRoomId(), new_room });
	}
	else if (joinReq.type() == 3) //加入房间
	{
		auto it = running_rooms_.find(joinReq.room_id());
		if (it == running_rooms_.end())
		{
			SPDLOG_ERROR("join room failed， room {} not exist", joinReq.room_id());
			soapproto::JoinRoomRsp rsp;
			rsp.set_result(soapproto::error_code::player_not_in_room);
			SEND_MESSAGE(channel, soapproto::msgId::cmdJoinRoomRsp, rsp);
		}
		else
		{
			it->second->join(user);
		}
	}

	
// 
// 	if (wait_room == nullptr)
// 	{
// 		if (idle_rooms_.empty())
// 		{
// 			wait_room = std::make_shared<battleroom>();
// 			wait_room->init();
// 		}
// 		else
// 		{
// 			wait_room = idle_rooms_.top();
// 			idle_rooms_.pop();
// 		}
// 	}
// 
// 	wait_room->join(user);
// 
// 	if (wait_room->isInBattle())
// 	{
// 		running_rooms_.insert({ wait_room->getRoomId(), wait_room });
// 		wait_room.reset();
// 	}
}

void lobby::eraseRoom(int32_t roomid)
{
	auto it = running_rooms_.find(roomid);
	if (it != running_rooms_.end())
	{
		if (it->second->isEmpty())
		{
			running_rooms_.erase(it);
		}
	}
}


void lobby::handleQuitRoomReq(const battlespheres::TcpChannelPtr & channel, soapproto::QuitRoomReq & quitReq)
{
	auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(channel->context());
	if (user == nullptr)
	{
		SPDLOG_ERROR("player info is null");
		return;
	}

	if (user->room_.lock())
	{
		auto it = running_rooms_.find(user->room_.lock()->getRoomId());
		if (it != running_rooms_.end())
		{
			it->second->quit(user);
		}
	}
	

// 	if (user->room_.lock())
// 	{
// 		if (user->room_.lock()->isWait())
// 		{
// 			user->room_.lock()->quit(user);
// 		}
// 		else
// 		{
// 			auto it = running_rooms_.find(user->room_.lock()->getRoomId());
// 			if (it != running_rooms_.end())
// 			{
// 				it->second->quit(user);
// 				if (it->second->isEmpty())
// 				{
// 					idle_rooms_.emplace(it->second);
// 					running_rooms_.erase(it);
// 				}
// 			}
// 		}
// 	}
}

std::shared_ptr<battleuser> lobby::getOnlineUser(uint32_t uid)
{
	auto it = users_.find(uid);
	if (it != users_.end())
	{
		return it->second;
	}

	return nullptr;
}

void lobby::Tick(std::shared_ptr<asio::system_timer>& update_timer)
{
	SPDLOG_INFO("[start tick]");
	updateRedis();
	auto update_func = std::make_shared<std::function<void()>>();

	*update_func = [this, update_func, update_timer]()
	{
		update_timer->expires_from_now(std::chrono::minutes(10));
		update_timer->async_wait([this, update_func](asio::error_code ec)
		{
			if (!ec)
			{
// 				for (auto it = running_rooms_.begin();it !=running_rooms_.end();)
// 				{
// 					if (it->second->isEmpty())
// 					{
// 						it = running_rooms_.erase(it);
// 					}
// 					else
// 					{
// 						++it;
// 					}
// 				}

				auto interval_minutes = std::chrono::duration_cast<std::chrono::minutes>(std::chrono::system_clock::now() - m_LastStartTime).count();
				if (interval_minutes < 8 * 60) //17点开服的话到第二天2点期间刷新
				{
					updateRedis();
					SPDLOG_INFO("[Rank] update rank!");
				}
				else if (interval_minutes < 14 * 60) //第二天6点清空每日
				{
					if (!m_isClearDayRank)
					{
						g_redisdb->zremrangebyrank(day_bath_rank_key, 0, -1, [](int count) {
							SPDLOG_INFO("remove day_bath_rank_key ,number:{}", count);
						});

						g_redisdb->zremrangebyrank(day_health_rank_key, 0, -1, [](int count) {
							SPDLOG_INFO("remove day_health_rank_key ,number:{}", count);
						});

						m_isClearDayRank = true;
						SPDLOG_INFO("[Rank ] clear day rank!");
					}
				}
				else if (interval_minutes < 24 * 60) //6点到17点刷新
				{
					updateRedis();
					SPDLOG_INFO("[Rank] update rank!");
				}
				else  //第二天重置
				{
					m_LastStartTime = std::chrono::system_clock::now();
					m_isClearDayRank = false;
					SPDLOG_INFO("[Rank] reset time!");
				}

				(*update_func)();
			}
		});
	};
	
	(*update_func)();
}