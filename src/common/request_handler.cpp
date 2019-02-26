//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2016 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "httplib/request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "httplib/mime_types.hpp"
#include "httplib/reply.hpp"
#include "httplib/request.hpp"
#include "json.hpp"
#include "logger.h"
#include "battlesvr/battleserver.h"

namespace http {
	namespace server {

		void request_handler::handle_request(const request& req, reply& rep)
		{
			std::string msg = req.uri;
			msg.erase(0, 1);
			//SPDLOG_INFO("recv http post ,{}", msg);
			try
			{
				nlohmann::json msg_json = nlohmann::json::parse(msg);
				if (msg_json["cmd"] == "recharge")
				{
					int32_t error_code = msg_json["errorcode"];
					int32_t uid = msg_json["uid"];
					if (error_code == 0)
					{
						int32_t golds = msg_json["golds"];
						int32_t diamonds = msg_json["diamonds"];
						int32_t rmbcount = msg_json["rmbcount"];
						std::string items = msg_json["items"];
						std::string gifts = msg_json["gifts"];
						g_battlelobby->rechargeNotify(0, uid, golds, diamonds, rmbcount, items, gifts);
					}
					else
					{
						std::string error_reason = msg_json["errorreason"];
						g_battlelobby->rechargeNotify(error_code, error_reason, uid);
					}
				}
				else if (msg_json["cmd"] == "rename")
				{
					g_battlelobby->rename(msg_json["uid"], msg_json["name"]);
				}
				else if (msg_json["cmd"] == "reloadItem")
				{
					g_battlelobby->reloadItem(msg_json["uid"]);
				}
				else {
					SPDLOG_INFO("[http recv] 无法识别的cmd");
				}


				rep = reply::stock_reply(reply::ok);
			}
			catch (const std::exception&)
			{
				rep = reply::stock_reply(reply::not_found);
			}
			
		}

	} // namespace server
} // namespace http
