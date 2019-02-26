#pragma once
#include "forwarddecl.h"
#include "dbaccess.h"
#include "soap.pb.h"
#include "eventhandle.h"
#include <stack>
#include <unordered_map>

class battleroom;
class battleuser;

struct TokenInfo
{
	uint32_t uid;
	uint32_t timestamp;
	std::string token;
};


class lobby
{
public:
	lobby();

	void Tick(std::shared_ptr<asio::system_timer>& update_timer);
	inline void removeUser(const uint32_t& uid) { users_.erase(uid); }
	std::shared_ptr<battleuser> getOnlineUser(uint32_t uid);
	
public:
	void handleLoginReq(const battlespheres::TcpChannelPtr& channel,soapproto::LoginReq& loginReq);
	void handleJoinReq(const battlespheres::TcpChannelPtr& channel,soapproto::JoinRoomReq& joinReq);
	void handleQuitRoomReq(const battlespheres::TcpChannelPtr& channel,soapproto::QuitRoomReq& quitReq);
	void handleQueryRankReq(const battlespheres::TcpChannelPtr& channel,soapproto::QueryRankReq& Req);

	void rechargeNotify(uint32_t error_code, int32_t uid, uint32_t gold, uint32_t diamond, uint32_t rmbcount, std::string szitems, std::string szgift);
	void rechargeNotify(uint32_t error_code, std::string error_reason, int32_t uid);
	void rename(int32_t uid, std::string szname);
	void reloadItem(int32_t uid);
	void eraseRoom(int32_t roomid);

private:
	int checkToken(const std::string& token, TokenInfo& tokenInfo);
	void updateRedis();

private:
	std::unordered_map<uint32_t, std::shared_ptr<battleuser>> users_;
// 	std::shared_ptr<battleroom> wait_room; //当前排队的房间
 	std::map <uint32_t, std::shared_ptr<battleroom>> running_rooms_;
// 	std::stack<std::shared_ptr<battleroom>> idle_rooms_; //空闲房间

	std::vector<TRankInfo> m_dayBathRank;
	std::vector<TRankInfo> m_BathRank;
	std::vector<TRankInfo> m_dayHealthRank;
	std::vector<TRankInfo> m_HealthRank;

	std::chrono::system_clock::time_point m_LastStartTime;
	bool m_isClearDayRank;
};     