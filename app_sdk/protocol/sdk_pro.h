#pragma once
#include "base/http/http_protocol_interface_define.h"
namespace app_sdk
{	
	/** @class SDK_PRO
	* @brief app_sdk 协议定义
	* @copyright (c) 2018, NetEase Inc. All rights reserved
	* @date 2018/4/26
	*/
	class SDK_PRO
	{
	private:	
		/** @class ResponseBase
		* @brief app_sdk 应答基类
		* @copyright (c) 2018, NetEase Inc. All rights reserved
		* @date 2018/4/26
		*/
		class ResponseBase : public IHttpResponse
		{
		public:			
			ResponseBase() = default;
			virtual ~ResponseBase() = default;
			//解析应答数据
			virtual void Parse(const std::string& response) override;
			//获取原始应答数据
			std::string GetReplyContent() const;
			//获取协议定义的业务返回码
			virtual int32_t GetProtocolReplyCode() const;
		protected:
			virtual void OnParse(const std::string& response);
			void SetProtocolReplyCode(int code);
		private:
			std::string reply_content_;//应答原始数据
			int32_t pro_reply_code_;//业务返回码
		};
		/** @class RequestBase
		* @brief app_sdk 请求基类
		* @copyright (c) 2018, NetEase Inc. All rights reserved
		* @date 2018/4/26
		*/
		class RequestBase : public IHttpRequest
		{
		public:
			RequestBase() = default;
			virtual ~RequestBase() = default;
			virtual std::string GetAPIURL() const override;
			virtual bool UsePostMethod() const override;
			virtual void GetRequestContent(std::string& content) const override;
			virtual void GetRequestHead(std::map<std::string, std::string>& heads) override;
		protected:
			virtual bool IsUsePostMethod() const;
			virtual void OnGetRequestContent(std::string& content) const;
			//获取请求的连接地址，可以直接返回，或者由OnGetHost与OnGetApi拼起来，可以参考示例程序
			virtual std::string OnGetAPIURL() const;
			virtual std::string OnGetHost() const;
			virtual std::string OnGetAPI() const;
			virtual void OnGetRequestHead(std::map<std::string, std::string>& heads) const;
		private:
			mutable std::string app_url_;
		};
		// get accid by username 2020-09
		class GetAccidByUsernameRequest : public RequestBase
		{
		public:
			GetAccidByUsernameRequest(std::string username);
		protected:
			virtual std::string OnGetHost() const override;
			virtual std::string OnGetAPI() const override;
			virtual bool UsePostMethod() const override;
			virtual void OnGetRequestHead(std::map<std::string, std::string>& heads) const override;
			virtual void OnGetRequestContent(std::string& content) const override;
		public:
			std::string username_;
		};
		class GetAccidByUsernameResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:
			std::string err_msg_;
			std::string accid_;
			//std::list<std::string> accid_;
		};
		// 获取快捷语 2021-04
		class GetMsgShortcutRequest : public RequestBase
		{
		public:
			GetMsgShortcutRequest();
			GetMsgShortcutRequest(std::string accid);
			std::string accid_;
		protected:
			virtual std::string OnGetHost() const override;
			virtual std::string OnGetAPI() const override;
			virtual bool UsePostMethod() const override;
			virtual void OnGetRequestHead(std::map<std::string, std::string>& heads) const override;
			virtual void OnGetRequestContent(std::string& content) const override;
		};
		class GetMsgShortcutResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:
			//std::list<Json::Value> msg_shortcut_list_;
			Json::Value msgShortcutList_;
			std::string err_msg_;
		};
		class DelMsgShortcutRequest : public RequestBase
		{
		public:
			DelMsgShortcutRequest(std::string accid, std::wstring shortCutId);
			std::wstring shortCutId_;
			std::string accid_;
		protected: 
			virtual std::string OnGetHost() const override;
			virtual std::string OnGetAPI() const override;
			virtual bool UsePostMethod() const override;
			virtual void OnGetRequestHead(std::map<std::string, std::string>& heads) const override;
			virtual void OnGetRequestContent(std::string& content) const override;
		};
		class DelMsgShortcutResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:  
			std::string err_msg_;
		};
		class AddMsgShortcutRequest : public GetMsgShortcutRequest
		{
		public:
			AddMsgShortcutRequest();
			AddMsgShortcutRequest(std::string accid, std::string category, std::string type, 
								  std::string keyWorks, std::string content, std::string subCategory);
			std::string accid_;
			std::string category_;
			std::string type_;
			std::string keyWorks_;
			std::string content_;
			std::string subCategory_;
		protected: 
			virtual bool UsePostMethod() const override; 
			virtual void OnGetRequestContent(std::string& content) const override;
		};
		class AddMsgShortcutResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:
			//std::list<Json::Value> msg_shortcut_list_;
			Json::Value msgShortcutList_;
			std::string err_msg_;
		};
		class SubCategoryRequest : public RequestBase
		{
		public: 
			SubCategoryRequest(std::string accid, std::string categoryId);
			std::string accid_;
			std::string categoryId_;
		protected: 
			virtual std::string OnGetHost() const override;
			virtual std::string OnGetAPI() const override;
			virtual bool UsePostMethod() const override;
			virtual void OnGetRequestHead(std::map<std::string, std::string>& heads) const override;
			virtual void OnGetRequestContent(std::string& content) const override;
		};
		class SubCategoryResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public: 
			Json::Value resList_;
			std::string err_msg_;
		};
		class AddSubCategoryRequest : public AddMsgShortcutRequest
		{
		public:
			AddSubCategoryRequest(std::string accid, std::string category, std::string name);
			std::string accid_;
			std::string category_;
			std::string name_;
		protected:
			virtual std::string OnGetAPI() const override;
			virtual void OnGetRequestContent(std::string& content) const override;
		};
		class AddSubCategoryResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:
			Json::Value resList_;
			std::string err_msg_;
		};
		// 账号登录 appServe 2020-09 
		class LoginAccountRequest : public RequestBase
		{
		public:
			LoginAccountRequest(std::string username, std::string password);
		protected:
			virtual std::string OnGetHost() const override;
			virtual std::string OnGetAPI() const override;
			virtual void OnGetRequestHead(std::map<std::string, std::string>& heads) const override;
			virtual void OnGetRequestContent(std::string& content) const override;
		public:
			std::string username_;
			std::string password_;
		};
		class LoginAccountResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:
			std::string err_msg_;
			std::string accid_;
			std::string token_;
		};

		//注册账号请求/应答
		class RegisterAccountRequest : public RequestBase
		{		
		public:
			RegisterAccountRequest(std::string username, std::string password, std::string nickname);
		protected:
			virtual std::string OnGetHost() const override;
			virtual std::string OnGetAPI() const override;
			virtual void OnGetRequestHead(std::map<std::string, std::string>& heads) const override;
			virtual void OnGetRequestContent(std::string& content) const override;
		public:
			std::string username_;
			std::string password_;
			std::string nickname_;
		};
		class RegisterAccountResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:
			std::string err_msg_;
		};
		//获取聊天室列表请求/应答
		class GetChatroomListRequest : public RequestBase
		{
		protected:
			virtual std::string OnGetAPIURL() const;
			virtual std::string OnGetAPI() const override;		
			virtual bool IsUsePostMethod() const override;
		};
		class GetChatroomListResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:
			std::vector<nim_chatroom::ChatRoomInfo> chatroom_list_;
		};
		//获取聊天室连接地址请求/应答
		class GetChatroomAddressRequest : public RequestBase
		{
		public:
			GetChatroomAddressRequest();
		protected:
			virtual std::string OnGetAPIURL() const;
			virtual std::string OnGetAPI() const override;
			virtual void OnGetRequestContent(std::string& content) const override;
		public:
			__int64 room_id_;
			std::string uid_;
			int type_;
		};
		class GetChatroomAddressResponse : public ResponseBase
		{
		protected:
			virtual void OnParse(const std::string& response) override;
		public:
			std::list<std::string> address_;	/**< 聊天室地址，地址通过应用服务器接口获取 */
		};
	public:
		/****************************对外暴露定义*****************************/

		// 账号登录 appServe 2020-09
		using LoginAccountReq = TSharedHttpRequest< LoginAccountRequest>;
		using LoginAccountRsp = TSharedHttpResponse<LoginAccountResponse>;
		//搜索账号请求/应答
		using GetAccidByUsernameReq = TSharedHttpRequest<GetAccidByUsernameRequest>;
		using GetAccidByUsernameRsp = TSharedHttpResponse<GetAccidByUsernameResponse>;
		//快捷语 2021
		using GetMsgShortcutRequestReq = TSharedHttpRequest<GetMsgShortcutRequest>;
		using GetMsgShortcutRequestRsp = TSharedHttpResponse<GetMsgShortcutResponse>;

		using AddMsgShortcutRequestReq = TSharedHttpRequest<AddMsgShortcutRequest>;
		using AddMsgShortcutRequestRsp = TSharedHttpResponse<AddMsgShortcutResponse>;

		using DelMsgShortcutRequestReq = TSharedHttpRequest<DelMsgShortcutRequest>;
		using DelMsgShortcutRequestRsp = TSharedHttpResponse<DelMsgShortcutResponse>;

		using GetSubCategoryReq = TSharedHttpRequest<SubCategoryRequest>;
		using GetSubCategoryRsp = TSharedHttpResponse<SubCategoryResponse>;

		using AddSubCategoryRequestReq = TSharedHttpRequest<AddSubCategoryRequest>;
		using AddSubCategoryRequestRsp = TSharedHttpResponse<AddSubCategoryResponse>;
		//注册账号请求/应答
		using RegisterAccountReq = TSharedHttpRequest<RegisterAccountRequest>;
		using RegisterAccountRsp = TSharedHttpResponse<RegisterAccountResponse>;
		//获取聊天室列表请求/应答
		using GetChatroomListReq = TSharedHttpRequest<GetChatroomListRequest>;
		using GetChatroomListRsp = TSharedHttpResponse<GetChatroomListResponse>;
		//获取聊天室列表请求/应答
		using GetChatroomAddressReq = TSharedHttpRequest<GetChatroomAddressRequest>;
		using GetChatroomAddressRsp = TSharedHttpResponse<GetChatroomAddressResponse>;
	};	
}
using app_sdk_pro = app_sdk::SDK_PRO;