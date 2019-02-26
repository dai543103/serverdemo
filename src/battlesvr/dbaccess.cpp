#include "dbaccess.h"
#include "common/logger.h"
#include "battleuser.h"
#include <chrono>

DBAccess::DBAccess(asio::io_service& loop)
    : logicloop_(loop)
{
}

bool DBAccess::connect(const char * const hostname, const char * const username, const char * const password, const char * const database, const uint16_t port)
{
    try
    {
        auto link = std::make_shared<mysqlcpp>(hostname, username, password, database, port);
        link->exec("SET NAMES UTF8");
        link_ = link;
		DEBUG("connected mysql server success");
    }
    catch (const mysqlexception& e)
    {
        SPDLOG_ERROR("connect to mysql ({}:{}) error ({})", hostname, port, e.what());
        return false;
    }

    keeplink_ = std::make_shared<asio::system_timer>(workloop_);
    std::shared_ptr<std::function<void()>> keeptimer = std::make_shared<std::function<void()>>();
    *keeptimer = [this, keeptimer]()
    {
        keeplink_->expires_from_now(std::chrono::minutes(10));
        keeplink_->async_wait([this, keeptimer](asio::error_code ec)
        {
            if (!ec)
            {
                try
                {
                    typedef std::tuple<std::shared_ptr<std::string>> row;
                    std::vector<row> result;
                    link_->query(&result, "SELECT NOW()");
                    if (!result.empty())
                    {
                        DEBUG("ping mysql server time now {}", *std::get<0>(result.at(0)));
                    }
                }
                catch (const std::exception& e)
                {
                    SPDLOG_ERROR("mysql error ({})", e.what());
                }

                (*keeptimer)();
            }  
        });
    };
    (*keeptimer)();
    thread_ = std::make_shared<std::thread>([this]() { workloop_.run(); });

    return true;
}

void DBAccess::stop()
{
    if (keeplink_)
        keeplink_.reset();

    if (thread_ && thread_->joinable())
        thread_->join();

    if (link_)
        link_.reset();
}

void DBAccess::load_userinfo(uint32_t uid, std::function< void(int, std::shared_ptr<tbl_userinfo>) > callback )
{
    workloop_.post([this, uid, callback]() {
        try {
            std::vector<tbl_userinfo> rows;
            auto stmt = link_->prepare("select * from tbl_userdata where uid = ?");
            stmt->execute(uid);
            stmt->fetch(&rows);

            int code = 0;
            std::shared_ptr<tbl_userinfo> userdata;
            if (rows.empty())
                code = 1;
            else
                userdata = std::make_shared<tbl_userinfo>(rows.at(0));
            logicloop_.dispatch([callback, code, userdata]() {callback(code, userdata); });
        }
        catch (const std::exception& e)
        {
            SPDLOG_ERROR("load userinfo({}) error {}", uid, e.what());
            logicloop_.dispatch([callback]() {callback(-1, nullptr); });
        }
    });
}

void DBAccess::load_iteminfo(uint32_t uid, std::function< void(std::shared_ptr<std::vector<tbl_items>>) > callback)
{
	workloop_.post([this, uid, callback]() {
		try {
			std::vector<tbl_items> rows;
			auto stmt = link_->prepare("select * from tbl_items where uid = ?");
			stmt->execute(uid);
			stmt->fetch(&rows);
		
			if (!rows.empty())    
			{
				std::shared_ptr<std::vector<tbl_items>> data;
				data = std::make_shared<std::vector<tbl_items>>(rows);
				logicloop_.dispatch([callback, data]() {callback(data); });
			}
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("load weapon by uid({}) error {}", uid, e.what());
			logicloop_.dispatch([callback]() {callback(nullptr); });
		}
	});
}

void DBAccess::insert_update_item(uint32_t uid, ItemInfo& weapon)
{
	workloop_.post([this, uid, weapon]() {
		try {
			auto stmt = link_->prepare("insert into tbl_items(uid, itemid) values (?,?) ON DUPLICATE KEY UPDATE state = ?");
			stmt->execute(uid, weapon.item_id, weapon.state);
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("save userinfo({}) error {}", uid, e.what());
		}
	});
}


void DBAccess::insert_update_userinfo(std::shared_ptr<battleuser> user)
{
	workloop_.post([this, user]() {
		try {
			auto stmt = link_->prepare("insert into tbl_userdata(uid, diamonds, golds, maxscore,lastscore) values(?,?,?,?,?) \
				ON DUPLICATE KEY UPDATE diamonds=?, golds=?, maxscore=?, lastscore=?");
			stmt->execute(
				user->GetData()->uid,
				user->GetData()->diamonds,
				user->GetData()->golds,
				user->GetData()->max_socore,
				user->GetData()->last_score,
				user->GetData()->diamonds,
				user->GetData()->golds,
				user->GetData()->max_socore,
				user->GetData()->last_score);
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("save userinfo({}) error {}", user->GetData()->uid, e.what());
		}
	});
}

void DBAccess::start_transaction()
{
	workloop_.post([this]() {
		link_->exec("begin");
	});
}

void DBAccess::commit_transaction()
{
	workloop_.post([this]() {
		link_->exec("commit");
	});
}

void DBAccess::rollback_transaction()
{
	workloop_.post([this]() {
		link_->exec("rollback");
	});
}


void DBAccess::load_usermail(uint32_t uid, std::function<void(std::shared_ptr<std::vector<tbl_usermail>>)> callback)
{
	workloop_.post([this, uid, callback]() {
		try {
			std::vector<tbl_usermail> rows;
			auto stmt = link_->prepare("select * from tbl_mail where uid = ?");
			stmt->execute(uid);
			stmt->fetch(&rows);

			if (!rows.empty())
			{
				std::shared_ptr<std::vector<tbl_usermail>> data;
				data = std::make_shared<std::vector<tbl_usermail>>(rows);
				logicloop_.dispatch([callback, data]() {callback(data); });
			}
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("load mail by uid({}) error {}", uid, e.what());
			logicloop_.dispatch([callback]() {callback(nullptr); });
		}
	});
}

void DBAccess::update_usermail(uint32_t user_id,stMail& mail)
{
	workloop_.post([this, user_id, mail]() {
		try {
			auto stmt = link_->prepare("update tbl_mail set readflag=?,extraflag=?,deleteflag=?,expiretime=? where uid = ? and mailid = ?");
			stmt->execute(mail.isReaded, mail.isExtra, mail.isdeleted, mail.expiretime, user_id, mail.id);
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("update usermail({}) error {}", user_id, e.what());
		}
	});
}

void DBAccess::load_mailContent(uint32_t mailId, std::function<void(int,std::shared_ptr<tbl_mailcontent>)> callback)
{
	workloop_.post([this, mailId, callback]() {
		try {
			std::vector<tbl_mailcontent> rows;
			auto stmt = link_->prepare("select * from tbl_mail_content where mailid = ?");
			stmt->execute(mailId);
			stmt->fetch(&rows);

			int code = 0;
			std::shared_ptr<tbl_mailcontent> data;
			if (!rows.empty())
			{
				code = 1;
				data = std::make_shared<tbl_mailcontent>(rows.at(0));
			}

			logicloop_.dispatch([callback, code, data]() {callback(code, data); });
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("load mail by id({}) error {}", mailId, e.what());
			logicloop_.dispatch([callback]() {callback(-1, nullptr); });
		}
	});
}

void DBAccess::load_fight(uint32_t uid, std::function<void(std::shared_ptr<tbl_fight>)> callback)
{
	workloop_.post([this, uid, callback]() {
		try {
			std::vector<tbl_fight> rows;
			auto stmt = link_->prepare("select * from tbl_fight where uid = ?");
			stmt->execute(uid);
			stmt->fetch(&rows);

			std::shared_ptr<tbl_fight> data;
			if (!rows.empty())
			{
				data = std::make_shared<tbl_fight>(rows.at(0));
				logicloop_.dispatch([callback, data]() {callback(data); });
			}
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("load fight by id({}) error {}", uid, e.what());
			logicloop_.dispatch([callback]() {callback(nullptr); });
		}
	});
}

void DBAccess::update_fight(std::shared_ptr<battleuser> user)
{
	workloop_.post([this, user]() {
		try {
			auto stmt = link_->prepare("update tbl_fight set totalfight = ?,todayfight = ?,gifts = ? where uid = ?");
			stmt->execute(user->GetData()->total_fight_times,user->GetData()->today_fight_times,user->GetData()->szgifts, user->GetData()->uid);
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("save userinfo({}) error {}", user->GetData()->uid, e.what());
		}
	});
}

void DBAccess::update_fight(uint32_t uid)
{
	workloop_.post([this, uid]() {
		try {
			auto stmt = link_->prepare("update tbl_fight set firstfight = now() where uid = ?");
			stmt->execute(uid);
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("save userinfo({}) error {}", uid, e.what());
		}
	});
}

void DBAccess::add_battle_log(uint32_t uid, uint32_t type, int32_t score)
{
	workloop_.post([this, uid,type,score]() {
		try {
			auto stmt = link_->prepare("insert into tbl_battle_log(uid,type,score) value(?,?,?)");
			stmt->execute(uid,type,score);
		}
		catch (const std::exception& e)
		{
			SPDLOG_ERROR("save userinfo({}) error {}", uid, e.what());
		}
	});
}
