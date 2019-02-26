#include "mail.h"
#include "battleserver.h"
#include "battleuser.h"
#include "common/logger.h"

void Mail::clear()
{
	m_unreadedMailNum = 0;
	m_userMails.clear();
}

void Mail::load_Mails()
{
	clear();

	g_battledb->load_usermail(m_userID, [this](std::shared_ptr<std::vector<DBAccess::tbl_usermail>> data) {	
		for (auto& tbl_mail : *data)
		{
			stMail mail;
			mail.id = std::get<1>(tbl_mail);
			mail.isReaded = std::get<2>(tbl_mail);
			mail.isExtra = std::get<3>(tbl_mail);
			mail.isdeleted = std::get<4>(tbl_mail);
			mail.expiretime = std::get<5>(tbl_mail);
			auto mailId = mail.id;
			if (!mail.isdeleted)
			{
				//判断是否到期
				auto now = std::time(nullptr);
				if (mail.expiretime != 0 && now > mail.expiretime)
				{
					mail.isdeleted = 1;
					g_battledb->update_usermail(m_userID, mail);
				}
				else
				{
					m_userMails.insert({ mailId,mail });
					if (!mail.isReaded)
					{
						++m_unreadedMailNum;
					}

					g_battledb->load_mailContent(mailId, [this,mailId,mail](int code, std::shared_ptr<DBAccess::tbl_mailcontent> contentPtr) {
						if (code == 0)
						{
							SPDLOG_ERROR("can't find this mail {}", mailId);
						}
						else if(code == 1)
						{
							stMailContent content;
							content.id = std::get<0>(*contentPtr);
							content.title = *std::get<1>(*contentPtr);
							content.content1 = *std::get<2>(*contentPtr);
							content.content2 = *std::get<3>(*contentPtr);
							content.content3 = *std::get<4>(*contentPtr);
							content.validtime = std::get<5>(*contentPtr);
							if (mail.isExtra)
							{
								stItem item;
								item.id = std::get<6>(*contentPtr);
								item.num = std::get<7>(*contentPtr);
								if (item.id && item.num)
								{
									content.items.push_back(item);
								}
								item.id = std::get<8>(*contentPtr);
								item.num = std::get<9>(*contentPtr);
								if (item.id && item.num)
								{
									content.items.push_back(item);
								}
								item.id = std::get<10>(*contentPtr);
								item.num = std::get<11>(*contentPtr);
								if (item.id && item.num)
								{
									content.items.push_back(item);
								}
								item.id = std::get<12>(*contentPtr);
								item.num = std::get<13>(*contentPtr);
								if (item.id && item.num)
								{
									content.items.push_back(item);
								}
								item.id = std::get<14>(*contentPtr);
								item.num = std::get<15>(*contentPtr);
								if (item.id && item.num)
								{
									content.items.push_back(item);
								}
							}
							m_userMailContent.insert({ content.id, content });
						}
					});
				}
			}
		}
		
		soapproto::MailNumberNotify notify;
		notify.set_newmailnumber(m_unreadedMailNum);
		notify.set_totalmailnumber(m_userMails.size());
		auto user = g_battlelobby->getOnlineUser(m_userID);
		if (user)
		{
			user->channel->send(netpacket::create(soapproto::cmdMailNumberNotify, notify));
		}

		SPDLOG_INFO("新邮件数量：{},邮件总数：{}", m_unreadedMailNum, m_userMails.size());
	});
}

void Mail::FetchList()
{
	soapproto::MailFetchListRsp rsp;
	for (auto usermail : m_userMails)
	{
		auto it = m_userMailContent.find(usermail.second.id);
		if (it == m_userMailContent.end())
		{
			SPDLOG_ERROR("cant't find this mail{}", usermail.second.id);
			rsp.set_result(soapproto::mail_none_exist);
		}
		else
		{
			auto memo = rsp.add_memos();
			memo->set_mailid(usermail.second.id);
			memo->set_status(usermail.second.isReaded);
			memo->set_extrastatus(usermail.second.isExtra);
			memo->set_title(it->second.title);
		}
	}

	rsp.set_result(0);
	auto user=g_battlelobby->getOnlineUser(m_userID);
	if (user)
	{
		user->channel->send(netpacket::create(soapproto::cmdMailFetchListRsp, rsp));
	}
}  

void Mail::FetchMail(uint32_t mailid)
{
	soapproto::FetchMailRsp rsp;

	auto contentIt = m_userMailContent.find(mailid);
	if (contentIt == m_userMailContent.end())
	{
		rsp.set_result(soapproto::mail_none_exist);
		SPDLOG_ERROR("cant't find this mail{}", mailid);
	}
	else
	{
		auto it = m_userMails.find(mailid);
		if (it == m_userMails.end())
		{
			rsp.set_result(soapproto::mail_none_exist);
			SPDLOG_ERROR("cant't find this in user mail{}", mailid);
		}
		else
		{
			rsp.set_result(0);
			auto rsp_content = rsp.mutable_content();
			rsp_content->set_mailid(contentIt->second.id);
			rsp_content->set_content1(contentIt->second.content1);
			rsp_content->set_content2(contentIt->second.content2);
			rsp_content->set_content3(contentIt->second.content3);
			for (auto item:contentIt->second.items)
			{
				auto rsp_content_item=rsp_content->add_items();
				rsp_content_item->set_id(item.id);
				rsp_content_item->set_num(item.num);
				DEBUG("Fetch mail rsp:{},{},{},{}", rsp.result(), rsp.content().content1(),rsp.content().items(0).id(), rsp.content().items(0).num());
			}

			if (it->second.isReaded == 0)
			{
				it->second.isReaded = 1;
				it->second.expiretime = std::time(nullptr) + contentIt->second.validtime;
				g_battledb->update_usermail(m_userID, it->second);
			}
		}
	}

	auto user = g_battlelobby->getOnlineUser(m_userID);
	if (user)
	{
		user->channel->send(netpacket::create(soapproto::cmdFetchMailRsp, rsp));
	}
}

void Mail::PickMail(uint32_t mailid)
{
	
	auto user = g_battlelobby->getOnlineUser(m_userID);
	auto it = m_userMails.find(mailid);
	if (it == m_userMails.end())
	{
		soapproto::PickMailRsp rsp;
		rsp.set_result(soapproto::mail_none_exist);
		user->channel->send(netpacket::create(soapproto::cmdPickMailRsp, rsp));
		SPDLOG_ERROR("cant't find this in user mail{}", mailid);
	}
	else if (it->second.isExtra == 2)
	{
		soapproto::PickMailRsp rsp;
		rsp.set_result(soapproto::mail_alread_picked);
		user->channel->send(netpacket::create(soapproto::cmdPickMailRsp, rsp));
	}
	else if(it->second.isExtra == 0)
	{
		soapproto::PickMailRsp rsp;
		rsp.set_result(soapproto::mail_none_reward);
		user->channel->send(netpacket::create(soapproto::cmdPickMailRsp, rsp));
	}
	else
	{
		auto contentIt = m_userMailContent.find(mailid);
		if (contentIt == m_userMailContent.end())
		{
			soapproto::PickMailRsp rsp;
			rsp.set_result(soapproto::mail_none_exist);
			user->channel->send(netpacket::create(soapproto::cmdPickMailRsp, rsp));
			SPDLOG_ERROR("cant't find this in content mail{}", mailid);
		}
		else
		{
			g_battledb->load_userinfo(m_userID, [this,user,it,contentIt](int result, std::shared_ptr<DBAccess::tbl_userinfo> userdata)
			{
				soapproto::PickMailRsp rsp;
				int error_code = soapproto::error_code::success;
				if (result != 0)
				{
					error_code = soapproto::error_code::server_is_busy;
				}
				else
				{
					user->updateBaseInfo(userdata);
					//发放奖励
					for (auto item : contentIt->second.items)
					{
						if (item.id == GOLD_ID)
						{
							user->GetData()->golds += item.num;
						}
						else if (item.id == DIAMAOND_ID)
						{
							user->GetData()->diamonds += item.num;
						}
						else
						{
							auto it= std::find_if(user->GetData()->items.begin(), user->GetData()->items.end(), [&item](ItemInfo& info_) {
								return item.id == info_.item_id; 
							});

							if (it == user->GetData()->items.end())
							{
								ItemInfo newItem;
								newItem.item_id = item.id;
								newItem.state = 0;
								user->GetData()->items.push_back(newItem);
								g_battledb->insert_update_item(m_userID, newItem);
							}
						}
					}

					rsp.set_usergolds(user->GetData()->golds);
					rsp.set_userdiamonds(user->GetData()->diamonds);
					for (auto& info_ : user->GetData()->items)
					{     
						auto shop_item = rsp.add_useritems();
						shop_item->set_itemid(info_.item_id);
					}

					it->second.isExtra = 2;
					it->second.expiretime = std::time(nullptr) + contentIt->second.validtime;
					g_battledb->update_usermail(m_userID, it->second);
					user->store2DB();
				}

				user->channel->send(netpacket::create(soapproto::cmdPickMailRsp, rsp));
			});
		}
	}
}