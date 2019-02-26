#pragma once

#include "forwarddecl.h"
#include "netpacket.h"
#include "battleuser.h"


class battleroom : public std::enable_shared_from_this<battleroom>
{
public:
    enum class emRoomState : uint8_t
    {
        EM_INIT,
		EM_WAIT,
		EM_BATTLE,
    };

	battleroom() = delete;
    battleroom( int id,bool isPrivate =false);
    ~battleroom();

	//inline void setRand() { m_randno = rand() % 100000; };
    bool join(std::shared_ptr<battleuser> user);
	void startBattle();
    void quit(std::shared_ptr<battleuser> user);

	void sendNotify(int32_t id, netpacketptr& pack);
	void sendRoomMsg(netpacketptr& pack);

	inline  uint32_t getRoomId() const { return roomId_; }
	inline int32_t getPlayerNum() { return m_players.size(); }
	inline bool isEmpty() const { return m_status == emRoomState::EM_INIT; }
	inline bool isInBattle() const { return m_status == emRoomState::EM_BATTLE; }
	inline bool isWait() const { return m_status == emRoomState::EM_WAIT; }	
	inline bool isPrivate() const { return m_bPrivate; }

	int getOwner();

private:
    uint32_t roomId_;      
	emRoomState m_status;       
	std::vector<int> m_players;
	//std::vector<int> m_quitUserIds;
	std::shared_ptr<asio::system_timer> updateTimer_;
	uint32_t m_randno;
	long long m_create_time;
	bool m_bPrivate;
};
