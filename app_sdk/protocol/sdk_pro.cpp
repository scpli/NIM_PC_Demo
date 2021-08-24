#include "stdafx.h"
#include "sdk_pro.h"
#include "app_sdk_interface.h"
namespace app_sdk
{
	//static const std::string cmd_register_account = "/api/createDemoUser";//注册账号

	//app server
	// static const std::string appServerHost = "https://aishiq.cn"; // 正式
	static const std::string appServerHost = "http://47.106.136.196"; // test
	
	static const std::string cmd_register_account = "/api/user";//注册账号;搜索账号
	static const std::string cmd_account_login = "/api/login"; // 登录返回云信 accid , token

	static const std::string cmd_get_chatroomlist = "/api/chatroom/homeList";//获取聊天室列表
	static const std::string cmd_get_chatroomaddress = "/api/chatroom/requestAddress";//获取聊天室连接地址

	static const std::string cmd_get_msgshortcut = "/api/shortcut"; // 快捷语
	static const std::string cmd_del_msgshortcut = "/api/deleteShortcut"; 
	static const std::string cmd_category = "/api/category"; // 快捷语二级分类

	void SDK_PRO::ResponseBase::Parse(const std::string& response) {
		reply_content_ = response;
		pro_reply_code_ = nim::kNIMResError;
		if (GetResponseCode() == nim::kNIMResSuccess)
		{
			pro_reply_code_ = nim::kNIMResSuccess;
			OnParse(response);
		}
			
	}
	std::string SDK_PRO::ResponseBase::GetReplyContent() const 
	{ 
		return reply_content_; 
	}
	int32_t SDK_PRO::ResponseBase::GetProtocolReplyCode() const
	{
		return pro_reply_code_;
	}
	void SDK_PRO::ResponseBase::SetProtocolReplyCode(int code)
	{
		pro_reply_code_ = code;
	}
	void SDK_PRO::ResponseBase::OnParse(const std::string& response) 
	{

	};
	std::string SDK_PRO::RequestBase::GetAPIURL() const
	{
		if (app_url_.empty())
			app_url_ = OnGetAPIURL();
		return app_url_;
	}
	std::string SDK_PRO::RequestBase::OnGetAPIURL() const
	{
		return OnGetHost().append(OnGetAPI());
	}
	std::string  SDK_PRO::RequestBase::OnGetHost() const
	{
		return AppSDKInterface::GetInstance()->GetAppHost();
	}
	bool SDK_PRO::RequestBase::UsePostMethod() const
	{ 
		return IsUsePostMethod();
	}
	bool SDK_PRO::RequestBase::IsUsePostMethod() const
	{
		return true;
	}
	std::string SDK_PRO::RequestBase::OnGetAPI() const
	{
		return ""; 
	};
	void SDK_PRO::RequestBase::GetRequestContent(std::string& content) const 
	{ 
		OnGetRequestContent(content); 
	}
	void SDK_PRO::RequestBase::GetRequestHead(std::map<std::string, std::string>& heads)
	{
		heads["User-Agent"] = "nim_pc";
		heads["appkey"] = AppSDKInterface::GetAppKey();
		heads["charset"] = "utf-8";
		heads["Content-Type"] = "application/json";
		OnGetRequestHead(heads);
	}
	void SDK_PRO::RequestBase::OnGetRequestContent(std::string& content) const
	{
	};	
	void SDK_PRO::RequestBase::OnGetRequestHead(std::map<std::string, std::string>& heads) const
	{
	};
	// appServer 2021-04 =======================
	SDK_PRO::GetMsgShortcutRequest::GetMsgShortcutRequest()
	{
	}
	SDK_PRO::GetMsgShortcutRequest::GetMsgShortcutRequest(std::string accid) :
		accid_(accid)
	{
	}
	std::string SDK_PRO::GetMsgShortcutRequest::OnGetHost() const
	{ 
		std::string host = appServerHost;
		std::string new_host = AppSDKInterface::GetInstance()->GetConfigValue("kAppServerAddress");
		if (!new_host.empty())
		{
			host = new_host;
		}
		return host;
	}
	std::string SDK_PRO::GetMsgShortcutRequest::OnGetAPI() const
	{
		return cmd_get_msgshortcut;
	};
	bool SDK_PRO::GetMsgShortcutRequest::UsePostMethod() const
	{
		return false;
	};
	void SDK_PRO::GetMsgShortcutRequest::OnGetRequestHead(std::map<std::string, std::string>& heads) const
	{
		heads["Content-Type"] = "application/x-www-form-urlencoded";
	};
	void SDK_PRO::GetMsgShortcutRequest::OnGetRequestContent(std::string& content) const
	{
		content.append("accid=").append(accid_);
	}
	void SDK_PRO::GetMsgShortcutResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader; 

		if (reader.parse(response, json_reply) && json_reply.isObject())
		{
			
			int res = json_reply["res"].asInt();

			if (res != nim::kNIMResSuccess)
			{
				if (json_reply.isMember("errormsg"))
				{
					err_msg_ = json_reply["errormsg"].asString();
				}
				SetProtocolReplyCode(res);

				QLOG_ERR(L"Invoke  GetMsgShortcutResponser. Json rescode: {0}.") << res;
				return;
			}

			//返回accid 数组
			if (!json_reply["data"].isArray())
			{
				QLOG_ERR(L"Invoke get GetMsgShortcutResponser error. Reason: Not an array.");
				return;
			}

			msgShortcutList_ = json_reply["data"];
		}
	}
	
	SDK_PRO::DelMsgShortcutRequest::DelMsgShortcutRequest(std::string accid, std::wstring shortCutId) : accid_(accid), shortCutId_(shortCutId)
	{
	}
	void SDK_PRO::DelMsgShortcutRequest::OnGetRequestContent(std::string& content) const
	{
		content.append("accid=").append(accid_)
			.append("&shortcutId=").append(nbase::UTF16ToUTF8(shortCutId_));
	}
	std::string SDK_PRO::DelMsgShortcutRequest::OnGetAPI() const
	{
		return cmd_del_msgshortcut;
	};
	bool SDK_PRO::DelMsgShortcutRequest::UsePostMethod() const
	{
		return true;
	};
	void SDK_PRO::DelMsgShortcutRequest::OnGetRequestHead(std::map<std::string, std::string>& heads) const
	{
		heads["Content-Type"] = "application/x-www-form-urlencoded";
	};
	std::string SDK_PRO::DelMsgShortcutRequest::OnGetHost() const
	{
		std::string host = appServerHost;
		std::string new_host = AppSDKInterface::GetInstance()->GetConfigValue("kAppServerAddress");
		if (!new_host.empty())
		{
			host = new_host;
		}
		return host;
	}
	void SDK_PRO::DelMsgShortcutResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;

		if (reader.parse(response, json_reply) && json_reply.isObject())
		{

			int res = json_reply["res"].asInt();

			if (res != nim::kNIMResSuccess)
			{
				if (json_reply.isMember("errormsg"))
				{
					err_msg_ = json_reply["errormsg"].asString();
				}
				SetProtocolReplyCode(res);

				QLOG_ERR(L"Invoke  GetMsgShortcutResponser. Json rescode: {0}.") << res;
				return;
			}
		}
	}
	//add  msg
	SDK_PRO::AddMsgShortcutRequest::AddMsgShortcutRequest()
	{

	}
	SDK_PRO::AddMsgShortcutRequest::AddMsgShortcutRequest(std::string accid, std::string category, std::string type,
		std::string keyWorks, std::string content, std::string subCategory) :
		accid_(accid), category_(category), type_(type), keyWorks_(keyWorks), content_(content), subCategory_(subCategory)
	{

	}
	bool SDK_PRO::AddMsgShortcutRequest::UsePostMethod() const
	{
		return true;
	}; 
	void SDK_PRO::AddMsgShortcutRequest::OnGetRequestContent(std::string& content) const
	{
		content.append("accid=").append(accid_)
			.append("&category=").append(category_)
			.append("&type=").append(type_)
			.append("&keyWorks=").append(keyWorks_)
		    .append("&content=").append(content_)
		    .append("&subCategoryId=").append(subCategory_);
	}
	void SDK_PRO::AddMsgShortcutResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;

		if (reader.parse(response, json_reply) && json_reply.isObject())
		{

			int res = json_reply["res"].asInt();

			if (res != nim::kNIMResSuccess)
			{
				if (json_reply.isMember("errormsg"))
				{
					err_msg_ = json_reply["errormsg"].asString();
				}
				SetProtocolReplyCode(res);

				QLOG_ERR(L"Invoke  GetMsgShortcutResponser. Json rescode: {0}.") << res;
				return;
			}

			//返回accid 数组
			if (!json_reply["data"].isArray())
			{
				QLOG_ERR(L"Invoke get GetMsgShortcutResponser error. Reason: Not an array.");
				return;
			}

			msgShortcutList_ = json_reply["data"];
		}
	} 
	SDK_PRO::SubCategoryRequest::SubCategoryRequest(std::string accid, std::string categoryId) :
		accid_(accid), categoryId_(categoryId)
	{
	}
	std::string SDK_PRO::SubCategoryRequest::OnGetHost() const
	{
		std::string host = appServerHost;
		std::string new_host = AppSDKInterface::GetInstance()->GetConfigValue("kAppServerAddress");
		if (!new_host.empty())
		{
			host = new_host;
		}
		return host;
	}
	std::string SDK_PRO::SubCategoryRequest::OnGetAPI() const
	{
		return cmd_category;
	};
	void SDK_PRO::SubCategoryRequest::OnGetRequestContent(std::string& content) const
	{
		content.append("accid=").append(accid_).append("&categoryId=").append(categoryId_);
	}
	bool SDK_PRO::SubCategoryRequest::UsePostMethod() const
	{
		return false;
	};
	void SDK_PRO::SubCategoryRequest::OnGetRequestHead(std::map<std::string, std::string>& heads) const
	{
		heads["Content-Type"] = "application/x-www-form-urlencoded";
	};
	void SDK_PRO::SubCategoryResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;

		if (reader.parse(response, json_reply) && json_reply.isObject())
		{

			int res = json_reply["res"].asInt();

			if (res != nim::kNIMResSuccess)
			{
				if (json_reply.isMember("errormsg"))
				{
					err_msg_ = json_reply["errormsg"].asString();
				}
				SetProtocolReplyCode(res);

				QLOG_ERR(L"Invoke  SubCategoryResponse. Json rescode: {0}.") << res;
				return;
			}

			//返回accid 数组
			if (!json_reply["data"].isArray())
			{
				QLOG_ERR(L"Invoke get SubCategoryResponse error. Reason: Not an array.");
				return;
			}

			resList_ = json_reply["data"];
		}
	}
	SDK_PRO::AddSubCategoryRequest::AddSubCategoryRequest(std::string accid, std::string category, std::string name):
		accid_(accid), category_(category), name_(name)
	{
	}
	std::string SDK_PRO::AddSubCategoryRequest::OnGetAPI() const
	{
		return cmd_category;
	};
	void SDK_PRO::AddSubCategoryRequest::OnGetRequestContent(std::string& content) const
	{
		content.append("accid=").append(accid_)
			.append("&category=").append(category_)
			.append("&name=").append(name_);
	}
	void SDK_PRO::AddSubCategoryResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;

		if (reader.parse(response, json_reply) && json_reply.isObject())
		{

			int res = json_reply["res"].asInt();

			if (res != nim::kNIMResSuccess)
			{
				if (json_reply.isMember("errormsg"))
				{
					err_msg_ = json_reply["errormsg"].asString();
				}
				SetProtocolReplyCode(res);

				QLOG_ERR(L"Invoke  AddSubCategoryResponse. Json rescode: {0}.") << res;
				return;
			}

			//返回accid 数组
			if (!json_reply["data"].isArray())
			{
				QLOG_ERR(L"Invoke get AddSubCategoryResponse error. Reason: Not an array.");
				return;
			}

			resList_ = json_reply["data"];
		}
	}
	// end GetMsgShortcut ======================

	// 搜索账号 appServer 2020-09
	SDK_PRO::GetAccidByUsernameRequest::GetAccidByUsernameRequest(std::string username) :
		username_(username)
	{
	}
	std::string SDK_PRO::GetAccidByUsernameRequest::OnGetHost() const
	{
		//std::string host = __super::OnGetHost(); //demo
		std::string host = appServerHost;
		std::string new_host = AppSDKInterface::GetInstance()->GetConfigValue("kAppServerAddress");
		if (!new_host.empty())
		{
			host = new_host;
		}
		return host;
	}
	std::string SDK_PRO::GetAccidByUsernameRequest::OnGetAPI() const
	{
		return cmd_register_account;
	};
	bool SDK_PRO::GetAccidByUsernameRequest::UsePostMethod() const
	{
		return false;
	};
	void SDK_PRO::GetAccidByUsernameRequest::OnGetRequestHead(std::map<std::string, std::string>& heads) const
	{
		heads["Content-Type"] = "application/x-www-form-urlencoded";
	}
	void SDK_PRO::GetAccidByUsernameRequest::OnGetRequestContent(std::string& content) const
	{
		content.append("username=").append(username_);
	}
	void SDK_PRO::GetAccidByUsernameResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;
		QLOG_ERR(L"------------------ AccidByUsernameResponse: {0}.") << response;

		if (reader.parse(response, json_reply) && json_reply.isObject())
		{
			int res = json_reply["res"].asInt();
			if (res != nim::kNIMResSuccess)
			{
				if (json_reply.isMember("errormsg"))
				{
					err_msg_ = json_reply["errormsg"].asString();
				}
				SetProtocolReplyCode(res);

				QLOG_ERR(L"Invoke  login account error. Json rescode: {0}.") << res;
				return;
			}
			//返回accid 数组
			if (!json_reply["data"].isArray())
			{
				QLOG_ERR(L"Invoke get username error. Reason: Not an array.");
				return;
			}

			for (auto it : json_reply["data"])
			{
				//accid_.emplace_back(it.asString());
				accid_ = it.asString();
				//accid_ = json_reply["data"].asString();
			}

			
		}
	}

	//账号登录 appServe 2020-09
	SDK_PRO::LoginAccountRequest::LoginAccountRequest(std::string username, std::string password) :
		username_(username), password_(password)
	{

	}
	std::string SDK_PRO::LoginAccountRequest::OnGetHost() const
	{
		//std::string host = __super::OnGetHost(); //demo
		std::string host = appServerHost;
		std::string new_host = AppSDKInterface::GetInstance()->GetConfigValue("kAppServerAddress");
		if (!new_host.empty())
		{
			host = new_host;
		}
		return host;
	}
	std::string SDK_PRO::LoginAccountRequest::OnGetAPI() const
	{
		return cmd_account_login;
	};
	void SDK_PRO::LoginAccountRequest::OnGetRequestHead(std::map<std::string, std::string>& heads) const
	{
		heads["Content-Type"] = "application/x-www-form-urlencoded";
	}
	void SDK_PRO::LoginAccountRequest::OnGetRequestContent(std::string& content) const
	{
		content.append("username=").append(username_)
			.append("&password=").append(password_);
	}
	void SDK_PRO::LoginAccountResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;
		QLOG_ERR(L"------------------ LoginAccountResponse: {0}.") << response;

		if (reader.parse(response, json_reply) && json_reply.isObject())
		{
			int res = json_reply["res"].asInt();
			if (res != nim::kNIMResSuccess)
			{
				if (json_reply.isMember("errormsg"))
				{
					err_msg_ = json_reply["errormsg"].asString();
				}
				SetProtocolReplyCode(res);

				QLOG_ERR(L"Invoke  login account error. Json rescode: {0}.") << res;
				return;
			}

			accid_ = json_reply["data"]["accid"].asString();
			token_ = json_reply["data"]["token"].asString();
		}
	}

	//注册账号请求/应答
	SDK_PRO::RegisterAccountRequest::RegisterAccountRequest(std::string username, std::string password, std::string nickname) :
		username_(username), password_(password), nickname_(nickname)
	{

	}
	std::string SDK_PRO::RegisterAccountRequest::OnGetHost() const
	{
		//std::string host = __super::OnGetHost(); demo
		std::string host = appServerHost;
		std::string new_host = AppSDKInterface::GetInstance()->GetConfigValue("kAppServerAddress");
		if (!new_host.empty())
		{
			host = new_host;
		}
		return host;
	}
	std::string SDK_PRO::RegisterAccountRequest::OnGetAPI() const
	{
		return cmd_register_account;
	};
	void SDK_PRO::RegisterAccountRequest::OnGetRequestHead(std::map<std::string, std::string>& heads) const
	{
		heads["Content-Type"] = "application/x-www-form-urlencoded";
	}
	void SDK_PRO::RegisterAccountRequest::OnGetRequestContent(std::string& content) const
	{
		content.append("username=").append(username_)
			.append("&password=").append(password_)
			.append("&nickname=").append(nickname_);
	}
	void SDK_PRO::RegisterAccountResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;
		if (reader.parse(response, json_reply) && json_reply.isObject())
		{
			int res = json_reply["res"].asInt();
			if (res != nim::kNIMResSuccess)
			{
				if(json_reply.isMember("errormsg"))
				{
					err_msg_ = json_reply["errormsg"].asString();
				}
				SetProtocolReplyCode(res);
				
				QLOG_ERR(L"Invoke  register accounterror. Json rescode: {0}.") << res;
				return;
			}
		}
	}
	//获取聊天室列表请求/应答
	std::string SDK_PRO::GetChatroomListRequest::OnGetAPI() const
	{
		return   cmd_get_chatroomlist;
	};	
	std::string SDK_PRO::GetChatroomListRequest::OnGetAPIURL() const
	{
		std::string api_addr = __super::OnGetAPIURL();
		std::string new_api_addr = AppSDKInterface::GetInstance()->GetConfigValue("kNIMChatRoomAddress");
		if (!new_api_addr.empty())
			api_addr = new_api_addr;
		return api_addr;
	}
	bool SDK_PRO::GetChatroomListRequest::IsUsePostMethod() const
	{
		return false;
	}
	void SDK_PRO::GetChatroomListResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;
		if (reader.parse(response, json_reply) && json_reply.isObject())
		{
			int res = json_reply["res"].asInt();
			if (res != nim::kNIMResSuccess)
			{
				this->SetProtocolReplyCode(res);
				QLOG_ERR(L"Invoke get room list error. Json rescode: {0}.") << res;
				return;
			}
			if (!json_reply["msg"]["list"].isArray())
			{
				QLOG_ERR(L"Invoke get room list error. Reason: Not an array.");
				return;
			}
			for (auto it : json_reply["msg"]["list"])
			{
				nim_chatroom::ChatRoomInfo info;
				info.id_ = it["roomid"].asInt64();
				info.name_ = it["name"].asString();
				info.creator_id_ = it["creator"].asString();
				nbase::StringToInt(it["onlineusercount"].asString(), &info.online_count_);
				info.valid_flag_ = it["status"].asInt();
				info.ext_ = it["ext"].asString();
				info.announcement_ = it["announcement"].asString();
				info.broadcast_url_ = it["broadcasturl"].asString();
				chatroom_list_.emplace_back(info);
			}			
		}
	};
	//获取聊天室连接地址请求/应答
	SDK_PRO::GetChatroomAddressRequest::GetChatroomAddressRequest() :
		type_(2), uid_(""), room_id_(0)
	{

	}
	std::string SDK_PRO::GetChatroomAddressRequest::OnGetAPIURL() const
	{
		std::string api_addr = __super::OnGetAPIURL();
		std::string new_api_addr = AppSDKInterface::GetInstance()->GetConfigValue("kNIMChatRoomRequest");
		if (!new_api_addr.empty())
			api_addr = new_api_addr;
		return api_addr;
	}
	std::string SDK_PRO::GetChatroomAddressRequest::OnGetAPI() const
	{
		return cmd_get_chatroomaddress;
	}
	void SDK_PRO::GetChatroomAddressRequest::OnGetRequestContent(std::string& content) const
	{
		Json::Value data;
		Json::FastWriter fs;
		data["roomid"] = room_id_;
		data["uid"] = uid_;
		data["type"] = type_;
		content = fs.write(data);
	}
	void SDK_PRO::GetChatroomAddressResponse::OnParse(const std::string& response)
	{
		Json::Value json_reply;
		Json::Reader reader;
		if (reader.parse(response, json_reply) && json_reply.isObject())
		{
			int res = json_reply["res"].asInt();
			if (res != nim::kNIMResSuccess)
			{
				this->SetProtocolReplyCode(res);
				QLOG_ERR(L"Invoke get room address error. Json rescode: {0}.") << res;
				return;
			}
			if (!json_reply["msg"]["addr"].isArray())
			{
				QLOG_ERR(L"Invoke get room address error. Reason: Not an array.");
				return;
			}
			for (auto it : json_reply["msg"]["addr"])
			{
				address_.emplace_back(it.asString());
			}
		}
	}
}