#pragma once
namespace app_sdk
{
	//格式化账号密码回调
	using OnFormatAccountAndPasswordCallback = std::function<void(bool ret,/*是否进行登录*/const std::string& account, const std::string& password)>;
	//注册新账号回调
	using OnRegisterAccountCallback = std::function<void(int code, const std::string& err_msg)>;
	//账号登录回调 2020
	using OnLoginAccountCallback = std::function<void(int code, const std::string& accid, const std::string& token, const std::string& err_msg)>;
	// 搜索accid by username
	using OnSearchAccidCallback = std::function<void(int code, const std::string& accid, const std::string& err_msg)>;
	// 快捷语消息 2021-04
	using OnGetMsgShortCutCallback = std::function<void(int code, const Json::Value& msg_shortcut_list, const std::string& err_msg)>;
	using OnAddMsgShortCutCallback = std::function<void(int code, const Json::Value& msg_shortcut_list, const std::string& err_msg)>;
	using OnDelMsgShortCutCallback = std::function<void(int code, const std::string& err_msg)>;

	using OnGetSubCategoryCallback = std::function<void(int code, const Json::Value& msg_shortcut_list, const std::string& err_msg)>;
	using OnAddSubCategoryCallback = std::function<void(int code, const Json::Value& msg_shortcut_list, const std::string& err_msg)>;
	//获取聊天室列表回调
	using OnGetChatroomListCallback = std::function<void(int code, const std::vector< nim_chatroom::ChatRoomInfo>& chatroom_list)>;
	//获取聊天室地址回调
	using OnGetChatroomAddressCallback = std::function<void(int code, const std::list< std::string>& address_list)>;
}
