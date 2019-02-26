#pragma once

#include <thread>
#include <asio.hpp>
#include "forwarddecl.h"
#include <asio/system_timer.hpp>
#include "mysqlcpp/mysqlcpp.h"

class battleuser;

class DBAccess
{
public:
    using tbl_userinfo = std::tuple<
        uint32_t,   //uid
        std::shared_ptr<std::string>, //nickname
        uint32_t,   //diamonds
        uint32_t,   //golds
        uint32_t,   //maxscore
		uint16_t, //lastscore
		uint32_t, //version
		uint32_t>; //rmb

	using tbl_fight = std::tuple<
		uint32_t,
		std::shared_ptr<std::string>,
		std::shared_ptr<std::string>,
		std::shared_ptr<std::string>,
		uint32_t,
		uint32_t,
		std::shared_ptr<std::string>>;

	using tbl_items = std::tuple<
		uint32_t,
		uint32_t,
		uint32_t,
		std::shared_ptr<std::string>>; //buy_tirm

	using tbl_usermail = std::tuple<
		int, //uid
		int, //mailid
		short, //readflag
		short, //附件
		short,//删除flag 
		int>; //到期时间

	using tbl_mailcontent = std::tuple<
		int,//mailid
		std::shared_ptr<std::string>, //title
		std::shared_ptr<std::string>, //content1
		std::shared_ptr<std::string>, //content2
		std::shared_ptr<std::string>, //content3
		int,//有效期限
		int,// 物品1
		int,
		int,// 物品2
		int,
		int,// 物品3
		int,
		int,// 物品4
		int,
		int,// 物品5
		int>;
public:
    DBAccess(asio::io_service& loop);
    bool connect(const char *const hostname,const char *const username,const char *const password,const char *const database,const uint16_t port = 3306);
    void stop();
   
    void load_userinfo(uint32_t uid, std::function<void(int, std::shared_ptr<tbl_userinfo>)> callback);
	void insert_update_userinfo(std::shared_ptr<battleuser> user);
	void load_iteminfo(uint32_t uid, std::function< void(std::shared_ptr<std::vector<tbl_items>>) > callback);
	void insert_update_item(uint32_t uid, ItemInfo& weapon);
	void load_usermail(uint32_t uid, std::function< void(std::shared_ptr<std::vector<tbl_usermail>>) > callback);
	void update_usermail(uint32_t user_id, stMail& mail);
	void load_mailContent(uint32_t mailId, std::function<void(int, std::shared_ptr<tbl_mailcontent>) > callback);
	void load_fight(uint32_t uid, std::function<void(std::shared_ptr<tbl_fight>) > callback);
	void update_fight(std::shared_ptr<battleuser> user);
	void update_fight(uint32_t uid);
	void add_battle_log(uint32_t uid, uint32_t type, int32_t score);

public:
	void start_transaction();
	void commit_transaction();
	void rollback_transaction();

private:
    asio::io_service workloop_;
    asio::io_service& logicloop_;
    std::shared_ptr<mysqlcpp> link_;
    std::shared_ptr<std::thread> thread_;
    std::shared_ptr<asio::system_timer> keeplink_;
};
