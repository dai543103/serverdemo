#include "battleroom.h"
#include "common/logger.h"
#include "battleserver.h"
#include "forwarddecl.h"

battleroom::battleroom(int id,bool isPrivate)
{
	m_bPrivate = isPrivate;
	roomId_ = ((std::time(nullptr) & 0x00000fff) << 12) + (id & 0x0000ffff);
	m_status = emRoomState::EM_INIT;
	m_randno = rand() % 100000;
	updateTimer_ = std::make_shared<asio::system_timer>(*g_battleserver->GetIOService());
	//m_quitUserIds.clear();
	SPDLOG_INFO("create room:{}", roomId_);
}

battleroom::~battleroom()
{
    SPDLOG_INFO("free battleroom {}", roomId_);
}

//群发消息
void battleroom::sendRoomMsg(netpacketptr& pack)
{
	if (m_status == emRoomState::EM_INIT)
	{
		SPDLOG_WARN("no one in this room{}", roomId_);
		return;
	}

	for (auto userid : m_players)
	{
		auto user = g_battlelobby->getOnlineUser(userid);
		if (user)
		{
			user->channel->send(pack);
		}
	}
}

//通知队友
void battleroom::sendNotify(int32_t id, netpacketptr& pack)
{
	if (m_status == emRoomState::EM_INIT)
	{
		SPDLOG_WARN("no one in this room{}", roomId_);
		return;
	}

	for (auto userid : m_players)
	{
		auto user = g_battlelobby->getOnlineUser(userid);
		if (user && userid != id)
		{
			user->channel->send(pack);
		}
	}
}

void battleroom::startBattle()
{
	updateTimer_->cancel();
	m_status = emRoomState::EM_BATTLE;
	soapproto::StartBattleNotify rsp;
	rsp.set_time(6); //暂时先这样
	auto pack = netpacket::create(soapproto::msgId::cmdStartBattleNotify, rsp);
	sendRoomMsg(pack);
}

bool battleroom::join(std::shared_ptr<battleuser> user)
{
	for (auto memberid : m_players)
	{
		auto member = g_battlelobby->getOnlineUser(memberid);
		if (member && user->getuid() == member->getuid())
		{
			//断线重连
			if (m_status == emRoomState::EM_BATTLE)
			{
				user->room_ = shared_from_this();
				soapproto::JoinRoomRsp rsp;
				rsp.set_result(0);
				rsp.set_room_id(roomId_);
				for (auto teammateid : m_players)
				{
					auto teammate = g_battlelobby->getOnlineUser(teammateid);
					if (teammate)
					{
						teammate->buildRoomPlayerInfo(rsp.add_other_players());
					}
				}

				rsp.set_random_seed(m_randno);
				rsp.set_start_time(m_create_time + START_BATTLE_TIME * 1000);
				if (m_bPrivate)
				{
					rsp.set_owner_id(*m_players.begin());
				}
				SEND_MESSAGE(user->channel, soapproto::msgId::cmdJoinRoomRsp, rsp);

				soapproto::JoinRoomEvent eventRsp;
				user->buildRoomPlayerInfo(eventRsp.mutable_info());
				auto pack = netpacket::create(soapproto::msgId::cmdJoinRoomEvent, eventRsp);
				sendNotify(user->getuid(), pack);

				return true;
			}
			else
			{
				SPDLOG_ERROR("tht player is in battle already!");
				return false;
			}
		}
	}

	if (m_status == emRoomState::EM_BATTLE)
	{
		SPDLOG_ERROR("tht room is in battle.");
		return false;
	}

	if( m_status == emRoomState::EM_INIT)
	{
		m_create_time = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
		if (!m_bPrivate)
		{
			updateTimer_->expires_from_now(std::chrono::seconds(START_BATTLE_TIME));
			updateTimer_->async_wait([this](asio::error_code ec) {
				if (!ec)
				{
					if (isWait())
					{
						startBattle();
					}
				}
			});
		}
	}

	m_status = emRoomState::EM_WAIT;
	user->room_ = shared_from_this();
	m_players.emplace_back(user->getuid());

	soapproto::JoinRoomRsp rsp;
	rsp.set_result(0);
	rsp.set_room_id(roomId_);
	for (auto teammateid : m_players)
	{
		auto teammate = g_battlelobby->getOnlineUser(teammateid);
		if (teammate)
		{
			teammate->buildRoomPlayerInfo(rsp.add_other_players());
		}
	}

	rsp.set_random_seed(m_randno);
	rsp.set_start_time(m_create_time + START_BATTLE_TIME*1000);
	if (m_bPrivate)
	{
		rsp.set_owner_id(*m_players.begin());
	}
	SEND_MESSAGE(user->channel, soapproto::msgId::cmdJoinRoomRsp, rsp);

	soapproto::JoinRoomEvent eventRsp;
	user->buildRoomPlayerInfo(eventRsp.mutable_info());
	auto pack=netpacket::create(soapproto::msgId::cmdJoinRoomEvent, eventRsp);
	sendNotify(user->getuid(),pack);

	

	if (!m_bPrivate &&  m_players.size() >= MAX_PLAYER_NUM)
	{
		startBattle();
	}
	
	return true;
}

int battleroom::getOwner()
{
	if (m_bPrivate)
	{
		auto it = m_players.begin();
		if (*it)
		{
			return *it;
		}
	}

	return 0;
}

void battleroom::quit(std::shared_ptr<battleuser> user)
{
	soapproto::QuitRoomRsp rsp;
	if (m_status == emRoomState::EM_INIT || user==nullptr)
	{
		SPDLOG_ERROR("this room is empty.{}", roomId_);
		return;
	}

	auto it = std::find_if(std::begin(m_players), std::end(m_players), [user](int& tempid) 
	{
		return tempid == user->getuid();
	});

	if (it != m_players.end())
	{
		bool isOwner = it == m_players.begin();
		user->room_.reset();
		m_players.erase(it);
// 		if (isInBattle())
// 		{
// 			m_quitUserIds.push_back(user->getuid());
// 		}
		
		rsp.set_result(0);
		user->channel->send(netpacket::create(soapproto::msgId::cmdQuitRoomRsp, rsp));

		if (m_players.size() == 0)
		{
			m_status = emRoomState::EM_INIT;
			//m_quitUserIds.clear();
			m_randno = rand() % 100000;
			g_battlelobby->eraseRoom(roomId_);
		}
		else
		{
			if (isWait() && m_bPrivate && isOwner) //等待阶段房主退出解散房间
			{
				auto pack = netpacket::create(soapproto::msgId::cmdQuitRoomEvent, rsp);
				sendRoomMsg(pack);
				for (auto memberid : m_players)
				{
					auto member = g_battlelobby->getOnlineUser(memberid);
					if (member)
					{
						member->room_.reset();
					}
				}
				m_status = emRoomState::EM_INIT;
				//m_quitUserIds.clear();
				m_randno = rand() % 100000;
				m_bPrivate = false;
				m_players.clear();
				g_battlelobby->eraseRoom(roomId_);
			}
			else
			{
				soapproto::QuitRoomEvent rsp;
				rsp.set_user_id(user->getuid());
				auto pack = netpacket::create(soapproto::msgId::cmdQuitRoomEvent, rsp);
				sendRoomMsg(pack);
			}
		}
	}
	else
	{
		rsp.set_result(soapproto::error_code::player_not_in_room);
		user->channel->send(netpacket::create(soapproto::msgId::cmdQuitRoomRsp, rsp));
	}
}



