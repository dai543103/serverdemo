#pragma once
#include "forwarddecl.h"

class Mail
{
public:
	Mail() = delete;
	Mail(uint32_t uid) :m_userID(uid) {}
	void clear();

	void load_Mails();
	void FetchList();
	void FetchMail(uint32_t mailid);
	void PickMail(uint32_t mailid);
private:
	uint32_t m_userID;
	std::map<uint32_t, stMail> m_userMails;
	std::map<uint32_t, stMailContent> m_userMailContent;
	uint32_t m_unreadedMailNum;//Î´¶ÁÓÊ¼şÊıÄ¿

};
