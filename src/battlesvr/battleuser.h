#pragma once

#include "forwarddecl.h"
#include "dbaccess.h"
#include "netpacket.h"
#include "mail.h"

class battleroom;

enum userstatus
{
	wait_load = 0,      //等待加载数据
	load_complete = 1,  //加载数据完成
};

class battleuser : public std::enable_shared_from_this<battleuser>
{
public:
    battleuser(uint32_t uid, const battlespheres::TcpChannelPtr & channel);
    ~battleuser();

public:
    userstatus status;
    battlespheres::TcpChannelPtr channel;
	std::weak_ptr<battleroom> room_;

public:
	void loadUser(std::shared_ptr<DBAccess::tbl_userinfo> userdata);
	void loadItem();
	void updateBaseInfo(std::shared_ptr<DBAccess::tbl_userinfo> userdata);
	void handleBattleResultReq(soapproto::BattleResultReq& req);
	void store2DB();
	void buildUserInfo(soapproto::UserInfo* info) const;
	void buildRoomPlayerInfo(soapproto::RoomPlayerInfo* info) const;
	void handleStartBattle();

	inline int32_t getuid() { return m_userData.uid; }
	inline TUserData* GetData() { return &m_userData; }
	inline std::shared_ptr<Mail> GetMail() { return m_mail; }
private: 
	asio::io_service loop_;
	TUserData m_userData;

	std::shared_ptr<asio::system_timer> m_missionTimer;
	std::shared_ptr<Mail> m_mail;
};
