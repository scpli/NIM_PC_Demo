﻿#pragma once
#include "module/session/session_util.h"
#include "module/subscribe_event/subscribe_event_manager.h"
#include "gui/session/control/bubbles/bubble_item.h"


namespace nim_comp
{
/** @class SearchMsgItem
  * @brief 搜索消息记录列表项的UI类
  * @copyright (c) 2015, NetEase Inc. All rights reserved
  * @author towik
  * @date 2021/3/7
  */
class SearchMsgItem : public ui::ListContainerElement
{
public:
	SearchMsgItem();
	virtual ~SearchMsgItem();

	virtual void InitCtrl();

	/**
	* 初始化该搜索结果列表项的显示
	* @param[in] msg 消息内容和信息
	* @return void	无返回值
	*/
	void InitMsg(const std::shared_ptr< nim::SessionData> &msg);

	/**
	 * 初始化一个云端会话列表项
	 * @param[in] msg 云端会话数据信息
	 * @return void 无返回值
	 */
	void SetOnlineSessionType(bool is_online_session);

	/**
	* 用昵称、用户名等信息匹配搜索关键字
	* @param[in] search_key 关键字
	* @return bool true 匹配成功，false 匹配失败
	*/
	bool Match(const UTF8String& search_key) { return false; }

	/**
	* 是否为群组项
	* @return bool true 是，false 否
	*/
	bool GetIsTeam() { return (msg_ == nullptr ? false : msg_->type_ == nim::kNIMSessionTypeTeam); }

	/**
	* 初始化头像和用户名/群名
	* @return void	无返回值
	*/
	void InitUserProfile();

	/**
	* 设置好友在线状态
	* @param[in] EventDataEx 事件数据
	* @return void	无返回值
	*/
	void SetOnlineState(const EventDataEx& data);

	/**
	* 隐藏在线状态
	* @return void	无返回值
	*/
	void HideOnlineState();

	/**
	* 更新会话列表项的显示内容
	* @param[in] id 会话id
	* @return void	无返回值
	*/
	void UpdateMsgContent(const std::string& id = "");

	/**
	* 清理会话列表想的显示内容
	* @return void	无返回值
	*/
	void ClearMsg();

	/**
	* 获取消息时间戳
	* @return long long 时间戳
	*/
	long long GetMsgTime();

	/**
	* 获取会话消息数据
	* @return nim::SessionData 消息内容
	*/
	std::shared_ptr<nim::SessionData>& GetSessionData() { return msg_; }
	
	/**
	* 获取本会话项未读数
	* @return int 未读数
	*/
	int GetUnread();

	/**
	* 设置本会话项未读数
	* @param[in] unread 未读数
	* @return void 无返回值
	*/
	void SetUnread(int unread);

	/**
	* 增加一条本会话项未读数
	* @return void 无返回值
	*/
	void AddUnread();

	/**
	* 重置本会话项未读数
	* @return bool 是否被重置
	*/
	bool ResetUnread();

	/**
	* 删除最近会话项的回调函数
	* @param[in] code 错误码
	* @param[in] result 会话数据
	* @param[in] total_unread_counts 总未读消息数
	* @return void	无返回值
	*/
	static void DeleteRecentSessionCb(nim::NIMResCode code, const nim::SessionData &result, int total_unread_counts);

	/**
	* 批量设置会话项未读消息为已读的回调函数
	* @param[in] res_code 错误码
	* @param[in] uid 会话id
	* @param[in] to_type 会话类型
	* @return void	无返回值
	*/
	static void BatchStatusDeleteCb(nim::NIMResCode res_code, const std::string& uid, nim::NIMSessionType to_type);

	void SetMute(bool mute);

	void AttachListBox(ui::ListBox* list, const std::vector<nim::IMMessage> &msg, bool show);
private:
	/**
	* 显示或隐藏@我消息提示
	* @param[in] show 显示或隐藏
	* @return void 无返回值
	*/
	void ShowAtmeTip(bool show);

	/**
	* 更新控件界面上显示的未读数
	* @return void 无返回值
	*/
	void UpdateUnread();

	/**
	* 弹出菜单
	* @param[in] point 显示菜单的坐标
	* @return void	无返回值
	*/
	void PopupSearchMsgItemMenu(POINT point);

	/**
	* 处理菜单项的单击消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool DelSearchMsgItemMenuItemClick(ui::EventArgs* param);

	/**
	* 处理控件双击消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnDbClicked(ui::EventArgs* arg);

	/**
	* 处理弹出菜单消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnSearchMsgItemMenu(ui::EventArgs* arg);

	/**
	* 处理头像按钮的单击消息
	* @param[in] msg 消息的相关信息
	* @return bool true 继续传递控件消息，false 停止传递控件消息
	*/
	bool OnHeadImageClicked(bool is_robot, ui::EventArgs* arg);

	void ShowMsgs(const std::vector<nim::IMMessage> &msg);

	void ShowMsg(const nim::IMMessage &msg, bool first, bool show_time);

	bool GetUserShowName(const nim::IMMessage &msg, std::string& show_name);

	bool Notify(ui::EventArgs* param);

private:
	ui::ButtonBox*	head_image_;
	ui::Label*		label_name_;
	ui::Label*		label_msg_;
	ui::Label*		label_online_state_;
	ui::Label*		label_atme_;
	ui::Label*		label_time_;
	ui::Box*		box_unread_;
	ui::Label*		label_unread_;
	ui::ListBox*    message_list_box_;

	bool			is_online_session_;
	std::shared_ptr<nim::SessionData> msg_;
	std::set<std::string> relate_ids;

	bool			first_show_msg_;
	long long		last_msg_time_;  //最近的消息时间
	long long		farst_msg_time_; //最远的消息时间
	long long		last_server_id_;

	bool			is_loading_;

	std::vector<nim::IMMessage> msg_vec_;
	typedef std::map<std::string, MsgBubbleItem*> IdBubblePair;
	IdBubblePair	id_bubble_pair_;
	std::map<std::string, nim::TeamMemberProperty> team_member_info_list_; //（会话类型是群聊时有效）保存用户和群相关的信息
};

}
