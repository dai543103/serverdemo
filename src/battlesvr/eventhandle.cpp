#include "eventhandle.h"
#include "common/base64.h"
#include "common/md5.h"
#include "common/json.hpp"
#include "common/logger.h"
#include "battleuser.h"
#include "battleroom.h"
#include "battleserver.h"


EventHandle::EventHandle()
{
	m_channel = nullptr;
	m_iMsgID = 0;
	m_dataptr = nullptr;
	m_msgsize = 0;
}

EventHandle::~EventHandle()
{

}

int EventHandle::dispach()
{
	if (m_iMsgID > 1000)
	{
		auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(m_channel->context());
		if (user == nullptr)
		{
			SPDLOG_ERROR("load user info req, but user is null");
			return -1;
		}

		if (user->room_.lock())
		{
			DEBUG("[ROOM]dispach message ,cmd({})", m_iMsgID);
			auto pack = std::make_shared<netpacket>(m_iMsgID, m_dataptr, m_msgsize);
			user->room_.lock()->sendNotify(user->getuid(), pack);
		}

		return 0;
	}

	auto it = m_handles.find(m_iMsgID);
	if (it != m_handles.end())
	{
		auto callback = it->second;
		DEBUG("dispach message ,cmd({})", m_iMsgID);
		callback();
	}
	else
	{
		SPDLOG_ERROR("msgid is unknown,cmd({})", m_iMsgID);
	}
	return 0;
}

void EventHandle::registAllHandle()
{
	m_handles.insert(std::make_pair(soapproto::msgId::cmdLoginReq, std::bind(&EventHandle::handleLoginReq, this)));
	m_handles.insert(std::make_pair(soapproto::msgId::cmdJoinRoomReq, std::bind(&EventHandle::handleJoinReq, this)));
	m_handles.insert(std::make_pair(soapproto::msgId::cmdQuitRoomReq, std::bind(&EventHandle::handleQuitRoomReq, this)));
	m_handles.insert(std::make_pair(soapproto::msgId::cmdQueryRankReq, std::bind(&EventHandle::handleQueryRankReq, this)));
	m_handles.insert(std::make_pair(soapproto::msgId::cmdBattleResultReq, std::bind(&EventHandle::handleBattleResultReq, this)));
	m_handles.insert(std::make_pair(soapproto::msgId::cmdMailFetchListReq, std::bind(&EventHandle::handleMailFetchListReq, this)));
	m_handles.insert(std::make_pair(soapproto::msgId::cmdFetchMailReq, std::bind(&EventHandle::handleFetchMailReq, this)));
	m_handles.insert(std::make_pair(soapproto::msgId::cmdPickMailReq, std::bind(&EventHandle::handlePickMailReq, this)));
	m_handles.insert(std::make_pair(soapproto::msgId::cmdStartBattleNotify, std::bind(&EventHandle::handleStartBattle, this)));
}

void EventHandle::setMsg(const battlespheres::TcpChannelPtr & channel, int iMsgID, const char * dataptr, int len)
{
	m_channel = channel;
	m_iMsgID = iMsgID;
	m_dataptr = dataptr;
	m_msgsize = len;
}

void EventHandle::handleLoginReq()
{
	soapproto::LoginReq loginReq;
	if (loginReq.ParseFromArray(m_dataptr, m_msgsize))
	{
		g_battlelobby->handleLoginReq(m_channel, loginReq);
	}
}

void EventHandle::handleJoinReq()
{
	soapproto::JoinRoomReq matchReq;
	if (matchReq.ParseFromArray(m_dataptr, m_msgsize))
	{
		g_battlelobby->handleJoinReq(m_channel, matchReq);
	}
}

void EventHandle::handleStartBattle()
{
	soapproto::StartBattleNotify notify;
	if (notify.ParseFromArray(m_dataptr, m_msgsize))
	{
		auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(m_channel->context());

		if (user == nullptr)
		{
			SPDLOG_ERROR("user info is null");
			soapproto::BattleResultRsp rsp;
			rsp.set_result(soapproto::error_code::player_info_invalid);
			m_channel->send(netpacket::create(soapproto::msgId::cmdBattleResultRsp, rsp));
			return;
		}

		user->handleStartBattle();
	}

}

void EventHandle::handleQuitRoomReq()
{
	soapproto::QuitRoomReq quitReq;
	if (quitReq.ParseFromArray(m_dataptr, m_msgsize))
	{
		g_battlelobby->handleQuitRoomReq(m_channel, quitReq);
	}
}

void EventHandle::handleBattleResultReq()
{
	soapproto::BattleResultReq req;
	if (req.ParseFromArray(m_dataptr, m_msgsize))
	{
		auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(m_channel->context());

		if (user == nullptr)
		{
			SPDLOG_ERROR("user info is null");
			soapproto::BattleResultRsp rsp;
			rsp.set_result(soapproto::error_code::player_info_invalid);
			m_channel->send(netpacket::create(soapproto::msgId::cmdBattleResultRsp, rsp));
			return;
		}

		user->handleBattleResultReq(req);
	}
}

void EventHandle::handleQueryRankReq()
{
	soapproto::QueryRankReq rankReq;
	if (rankReq.ParseFromArray(m_dataptr, m_msgsize))
	{
		g_battlelobby->handleQueryRankReq(m_channel, rankReq);
	}

}

void EventHandle::handleMailFetchListReq()
{
	soapproto::MailFetchListReq req;
	if (req.ParseFromArray(m_dataptr, m_msgsize))
	{
		soapproto::MailFetchListRsp rsp;
		auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(m_channel->context());
		if (user == nullptr)
		{
			SPDLOG_ERROR("user info is null");
			rsp.set_result(soapproto::error_code::player_info_invalid);
			m_channel->send(netpacket::create(soapproto::msgId::cmdMailFetchListRsp, rsp));
			return;
		}

		user->GetMail()->FetchList();
	}
}

void EventHandle::handleFetchMailReq()
{
	soapproto::FetchMailReq req;
	if (req.ParseFromArray(m_dataptr, m_msgsize))
	{
		soapproto::FetchMailRsp rsp;
		auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(m_channel->context());
		if (user == nullptr)
		{
			SPDLOG_ERROR("user info is null");
			rsp.set_result(soapproto::error_code::player_info_invalid);
			m_channel->send(netpacket::create(soapproto::msgId::cmdFetchMailRsp, rsp));
			return;
		}

		user->GetMail()->FetchMail(req.mailid());
	}
}

void EventHandle::handlePickMailReq()
{
	soapproto::PickMailReq req;
	if (req.ParseFromArray(m_dataptr, m_msgsize))
	{
		soapproto::PickMailRsp rsp;
		auto user = battlespheres::any_cast<std::shared_ptr<battleuser>>(m_channel->context());
		if (user == nullptr)
		{
			SPDLOG_ERROR("user info is null");
			rsp.set_result(soapproto::error_code::player_info_invalid);
			m_channel->send(netpacket::create(soapproto::msgId::cmdPickMailRsp, rsp));
			return;
		}

		user->GetMail()->PickMail(req.mailid());
	}
}

