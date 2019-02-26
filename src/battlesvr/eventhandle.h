#pragma once
#include <functional>
#include "forwarddecl.h"
#include "netpacket.h"

class EventHandle
{
public:
	EventHandle();
	virtual ~EventHandle();

	void registAllHandle();
	void setMsg(const battlespheres::TcpChannelPtr & channel,int iMsgID, const char * dataptr, int len);
	int dispach();
private:
	void handleLoginReq();
	void handleJoinReq();
	void handleQuitRoomReq();
	void handleQueryRankReq();
	void handleBattleResultReq();
	void handleMailFetchListReq();
	void handleFetchMailReq();
	void handlePickMailReq();
	void handleStartBattle();
private:
	battlespheres::TcpChannelPtr m_channel;
	int m_iMsgID;
	const char * m_dataptr;
	int m_msgsize;
	std::map<int, std::function<void()>> m_handles;
};

#define g_eventhandle CSingleton<EventHandle>::Instance()



