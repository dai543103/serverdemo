#pragma once

#include <memory>
#include <array>
#include <vector>
#include <string>
#include <map>

#define MAX_PLAYER_NUM 3
#define MAX_IDLE_ROOM_NUM 20
#define RANK_NUM 100
#define CLIENT_KEEP_TOKEN_TIME 36000
#define START_BATTLE_TIME 10

#define COMPARE_USERS(user1,user2) ((user1)->getuid() == (user2)->getuid())

#define SEND_MESSAGE(channel,msgid,msg) ((channel)->send(netpacket::create((msgid), (msg))))

#define DIAMAOND_ID (1001)
#define GOLD_ID (1002)

#define SHOUFA_GIFT_ID (3)
#define JISU_GIFT_ID (4)
#define YOUHUI_GIFT_ID (5)
#define TODAY_GIFT_ID (6)

const std::string day_bath_rank_key = "bath_day_rank";//大澡堂
const std::string bath_rank_key = "bath_rank";
const std::string day_health_rank_key = "health_day_rank"; //大保健
const std::string health_rank_key = "health_rank";
const std::string name_rank_key = "names";

enum class emPLayType
{
	Bath, //大澡堂
	Health,	//大保健
};

struct ItemInfo
{
	uint32_t item_id;
	uint32_t state;
};

struct TUserData
{
	uint32_t uid;
	std::string name;
	uint32_t diamonds;
	uint32_t golds;
	uint32_t max_socore;  
	uint32_t last_score; //上次成绩
	std::string szgifts;
	std::vector<ItemInfo> items;
	std::vector<uint32_t> gifts;

	uint32_t max_bath_day_score; //今日大澡堂最好成绩
	uint32_t max_bath_score;
	uint32_t max_health_day_score;
	uint32_t max_health_score;

	uint32_t today_fight_times;
	uint32_t total_fight_times;
	std::string firstlogin;//第一次登陆时间
	std::string firstfight;//第一次战斗时间
	std::string daylogin;
	time_t first_login_time;
	time_t first_fight_time;
	time_t day_login_time;
};


struct TRankInfo
{
	int32_t index; //排名位置
	int32_t id;
	int32_t score;
	std::string name;
};

enum emItemType
{
	EM_GOLD,
	EM_DIAMOND,
	EM_GOOD,
};

struct stMail
{
	uint32_t id; //邮件id
	uint32_t isReaded;
	uint32_t isExtra;
	uint32_t isdeleted;
	int expiretime;//到期时间
};

struct stItem
{
	uint32_t id;
	uint32_t num;
};

struct stMailContent
{
	uint32_t id;
	std::string title;
	std::string content1;
	std::string content2;
	std::string content3;
	int validtime;
	std::vector<stItem> items;
};