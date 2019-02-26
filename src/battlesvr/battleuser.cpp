#include "battleuser.h"
#include "common/logger.h"
#include "battleserver.h"
#include "battleroom.h"
#include "common/base.h"
#include "common/json.hpp"
#include <stdio.h>

battleuser::battleuser(uint32_t uid, const battlespheres::TcpChannelPtr & channelptr)
	:channel(channelptr)
	, status(wait_load)
{
	m_userData.uid = uid;
	m_mail = std::make_shared<Mail>(uid);
    logger->info("create battle user({})", m_userData.uid);
}

battleuser::~battleuser()
{
    logger->info("delete battle user({})", m_userData.uid);
}

void battleuser::updateBaseInfo(std::shared_ptr<DBAccess::tbl_userinfo> userdata)
{
	if (std::get<1>(*userdata))
	{
		m_userData.name = *std::get<1>(*userdata);
	}
	
	m_userData.diamonds = std::get<2>(*userdata);
	m_userData.golds = std::get<3>(*userdata);
	m_userData.max_socore = std::get<4>(*userdata);
	m_userData.last_score = std::get<5>(*userdata);	
}

void battleuser::loadUser(std::shared_ptr<DBAccess::tbl_userinfo> userdata)
{
	channel->set_context(battlespheres::Any(shared_from_this()));
	updateBaseInfo(userdata);

	g_redisdb->zscore(day_bath_rank_key, std::to_string(m_userData.uid), [&](int score) {
		m_userData.max_bath_day_score = score;
		DEBUG("loadUser,uid:{},max_bath_day_score:{}", m_userData.uid,score);
	});
	g_redisdb->zscore(bath_rank_key, std::to_string(m_userData.uid), [&](int score) {
		m_userData.max_bath_score = score;
		DEBUG("loadUser,uid:{},max_bath_score:{}", m_userData.uid, score);
	});
	g_redisdb->zscore(day_health_rank_key, std::to_string(m_userData.uid), [&](int score) {
		m_userData.max_health_day_score = score;
		DEBUG("loadUser,uid:{},max_health_day_score:{}", m_userData.uid, score);
	});
	g_redisdb->zscore(health_rank_key, std::to_string(m_userData.uid), [&](int score) {
		m_userData.max_health_score = score;
		DEBUG("loadUser,uid:{},max_health_score:{}", m_userData.uid, score);
	});

	g_redisdb->hset(name_rank_key, m_userData.uid, m_userData.name);

	m_userData.gifts.clear();
	g_battledb->load_fight(m_userData.uid, [this](std::shared_ptr<DBAccess::tbl_fight> fight) {
		if (fight)
		{
			m_userData.firstlogin = *std::get<1>(*fight);
			m_userData.firstfight = *std::get<2>(*fight);
			m_userData.daylogin = *std::get<3>(*fight);
			m_userData.total_fight_times = std::get<4>(*fight);
			m_userData.today_fight_times = std::get<5>(*fight);
			m_userData.szgifts = *std::get<6>(*fight);

			SplitString2Int(m_userData.szgifts, m_userData.gifts, "|");
			m_userData.first_login_time = StringToDatetime(m_userData.firstlogin);
			m_userData.day_login_time = StringToDatetime(m_userData.daylogin);
			m_userData.first_fight_time = StringToDatetime(m_userData.firstfight);
		}
	});

	loadItem();
	m_mail->load_Mails();

	DEBUG("loadUser success,uid:{}", m_userData.uid);
	status = load_complete;
}


void battleuser::handleStartBattle()
{
	if (room_.lock())
	{
		if (!room_.lock()->isWait())
		{
			SPDLOG_ERROR("room state is not for waitting!");
		}

		if (room_.lock()->getOwner() != getuid())
		{
			SPDLOG_ERROR(" not owner or room state is wrong");
		}

		room_.lock()->startBattle();
	}
}

void battleuser::loadItem()
{
	m_userData.items.clear();
	g_battledb->load_iteminfo(m_userData.uid, [this](std::shared_ptr<std::vector<DBAccess::tbl_items>> pItems) {
		for (auto& t : *pItems)
		{
			ItemInfo info_;
			info_.item_id = std::get<1>(t);
			info_.state = std::get<2>(t);
			m_userData.items.push_back(std::move(info_));
		}
	});
}

void battleuser::buildRoomPlayerInfo(soapproto::RoomPlayerInfo* info) const
{
	info->set_user_id(m_userData.uid);
	info->set_nickname(m_userData.name);
	for(auto it =std::begin(m_userData.items);it!=std::end(m_userData.items);it++)
	{
		if (it->state == 1)
		{
			info->add_equips(it->item_id);
		}
	}
}

void battleuser::buildUserInfo(soapproto::UserInfo * info) const
{
    info->set_user_id(m_userData.uid);
    info->set_nickname(m_userData.name);
    info->set_diamonds(m_userData.diamonds);
    info->set_golds(m_userData.golds);
    info->set_max_socore(m_userData.max_socore);
	info->set_last_score(m_userData.last_score);
}

void battleuser::handleBattleResultReq(soapproto::BattleResultReq& req)
{
	m_userData.today_fight_times++;
	m_userData.total_fight_times++;
	if (m_userData.total_fight_times == 1)
	{
		g_battledb->update_fight(m_userData.uid);
		m_userData.first_fight_time = std::time(nullptr);
	}

	//拼成字符串，，SB
	std::string szgift = std::to_string(m_userData.gifts[0]) + "|" + std::to_string(m_userData.gifts[1]) + "|" + std::to_string(m_userData.gifts[2]) + "|";

	//首发礼包
	szgift += (std::to_string(m_userData.gifts[SHOUFA_GIFT_ID]));
	if (m_userData.gifts[SHOUFA_GIFT_ID] == 1)
	{
		szgift += "-1517155200";
	}
	szgift += "|";

	//极速礼包
	szgift += (std::to_string(m_userData.gifts[JISU_GIFT_ID]) + "|");

	auto now = std::time(nullptr);
	auto endtime = m_userData.first_fight_time + 2 * 60 * 60;

	//优惠礼包
	if (m_userData.gifts[YOUHUI_GIFT_ID] != 2)
	{
		if (m_userData.total_fight_times > 0)
		{
			if (now > endtime)
			{
				m_userData.gifts[YOUHUI_GIFT_ID] = 2;
				m_userData.gifts[TODAY_GIFT_ID] = 2;
			}
			else
			{
				m_userData.gifts[YOUHUI_GIFT_ID] = 1;
			}
		}
		else
		{
			m_userData.gifts[YOUHUI_GIFT_ID] = 0;
		}
	}
	szgift += std::to_string(m_userData.gifts[YOUHUI_GIFT_ID]);
	if (m_userData.gifts[YOUHUI_GIFT_ID] == 1)
	{
		szgift += ("-" + std::to_string(endtime));
	}
	szgift += "|";

	//今日特惠
	if (m_userData.gifts[YOUHUI_GIFT_ID] == 2)
	{
		if (m_userData.gifts[TODAY_GIFT_ID] != 2)
		{
			endtime = m_userData.day_login_time + 2 * 60 * 60;
			if (now > endtime)
			{
				m_userData.gifts[TODAY_GIFT_ID] = 2;
			}
			else
			{
				m_userData.gifts[TODAY_GIFT_ID] = 1;
			}
		}
// 		else //hehe
// 		{
// 			endtime = m_userData.first_fight_time + 24 * 60 * 60;
// 			if (now > endtime)
// 			{
// 				endtime = m_userData.day_login_time + 2 * 60 * 60;
// 				if (now > endtime)
// 				{
// 					m_userData.gifts[TODAY_GIFT_ID] = 0;
// 				}
// 				else
// 				{
// 					m_userData.gifts[TODAY_GIFT_ID] = 1;
// 				}
// 			}
// 		}
	}
	
	szgift += std::to_string(m_userData.gifts[TODAY_GIFT_ID]);
	if (m_userData.gifts[TODAY_GIFT_ID] == 1)
	{
		szgift += ("-" + std::to_string(endtime));
	}

	//保存礼包数据
	PieceInt2String(m_userData.gifts, m_userData.szgifts, "|");
	g_battledb->update_fight(shared_from_this());
	SPDLOG_INFO("player{}:gift info:{}", m_userData.uid, m_userData.szgifts);

	//更新排行榜
	auto score = req.score();
	if (req.type() == 1)
	{
		if (score > m_userData.max_bath_score)
		{
			g_redisdb->zadd(bath_rank_key, score, std::to_string(m_userData.uid), m_userData.name);
			m_userData.max_bath_score = score;
		}

		if (score > m_userData.max_bath_day_score)
		{
			g_redisdb->zadd(day_bath_rank_key, score, std::to_string(m_userData.uid), m_userData.name);
			m_userData.max_bath_day_score = score;
		}
	}
	else
	{
		if (score > m_userData.max_health_score)
		{
			g_redisdb->zadd(health_rank_key, score, std::to_string(m_userData.uid), m_userData.name);
			m_userData.max_health_score = score;
		}

		if (score > m_userData.max_health_day_score)
		{
			g_redisdb->zadd(day_health_rank_key, score, std::to_string(m_userData.uid), m_userData.name);
			m_userData.max_health_day_score = score;
		}
	}

	//保存日志
	g_battledb->add_battle_log(m_userData.uid, req.type(), score);

	soapproto::BattleResultRsp rsp;
	rsp.set_result(0);
	rsp.set_gifts(szgift);
	channel->send(netpacket::create(soapproto::cmdBattleResultRsp, rsp));
}


void battleuser::store2DB()
{
	g_battledb->insert_update_userinfo(shared_from_this());
}





