#include "stdafx.h"
#include "search_msg_item.h"

#include "module/session/session_manager.h"
#include "module/session/force_push_manager.h"
#include "module/runtime_data/runtime_data_manager.h"
#include "callback/session/session_callback.h"
#include "gui/profile_form/profile_form.h"
#include "gui/session/session_box.h"
#include "gui/session/session_form.h"
#include "gui/session/msg_record.h"
#include "gui/main/control/session_item_helper.h"

#include "shared/ui/ui_menu.h"
#include "shared/pin_yin_helper.h"
#include "shared/ui/toast/toast.h"

#include "module/online_state_event/online_state_event_util.h"
#include "module/session/transfer_file_manager.h"

using namespace ui;

namespace nim_comp
{
	SearchMsgItem::SearchMsgItem()
	: is_online_session_(false)
{
	if (RunTimeDataManager::GetInstance()->GetUIStyle() == UIStyle::join)
	{
		DetachEvent(ui::EventType::kEventSelect);
		AttachSelect(nbase::Bind(&SearchMsgItem::OnDbClicked, this, std::placeholders::_1));
	}
	else
	{
		DetachEvent(ui::EventType::kEventMouseDoubleClick);
		AttachDoubleClick(nbase::Bind(&SearchMsgItem::OnDbClicked, this, std::placeholders::_1));
	}
	SelectWhenInternalMenu(false);
}

SearchMsgItem::~SearchMsgItem()
{

}

void SearchMsgItem::InitCtrl()
{
	
	label_name_ = (Label*) this->FindSubControl(L"label_name");
	label_msg_ = (Label*) this->FindSubControl(L"label_msg");
	label_online_state_ = (Label*) this->FindSubControl(L"online_state");
	label_atme_ = (Label*) this->FindSubControl(L"at_me_label");
	label_time_ = (Label*) this->FindSubControl(L"label_time");
	box_unread_ = (Box*) this->FindSubControl(L"box_unread");
	label_unread_ = (Label*) this->FindSubControl(L"label_unread");
	head_image_ = (ButtonBox*)this->FindSubControl(L"head_image");
}

void SearchMsgItem::InitMsg(const std::shared_ptr< nim::SessionData> &msg)
{
	msg_ = msg;
	if (msg_->type_ == nim::kNIMSessionTypeTeam
		|| msg_->type_ == nim::kNIMSessionTypeP2P)
	{
		head_image_->DetachEvent(ui::EventType::kEventClick);
		head_image_->AttachClick(nbase::Bind(&SearchMsgItem::OnHeadImageClicked, this, msg_->is_robot_session_, std::placeholders::_1));
		DetachEvent(ui::EventType::kEventMouseMenu);
		this->AttachMenu(nbase::Bind(&SearchMsgItem::OnSearchMsgItemMenu, this, std::placeholders::_1));
	}

	SetUTF8Name(msg_->id_);
	SetUTF8DataID(msg_->id_);

	InitUserProfile(); //设置用户名和头像
	UpdateMsgContent(); //更新消息内容
	UpdateUnread(); //刷新未读条数	
	ShowAtmeTip(true);

	//更新时间
	if (msg_->msg_timetag_ > 0 && msg_->msg_status_ != nim::kNIMMsgLogStatusDeleted)
	{
		std::wstring str = GetMessageTime(msg_->msg_timetag_, true);
		label_time_->SetText(str);
		label_time_->SetVisible(true);
	}
	else
		label_time_->SetVisible(false);

	if (msg_->type_ == nim::kNIMSessionTypeTeam) // 需要先获得群里最近一条消息中所有人的昵称，再UpdateMsg
	{
		//head_image_->SetMouseEnabled(false); //群头像不响应点击

		relate_ids.clear();
		relate_ids.insert(msg_->msg_sender_accid_);
		Json::Reader reader;
		Json::Value attach;
		if (reader.parse(msg_->msg_attach_, attach))
		{
			if (attach.isObject() && attach.isMember(nim::kNIMNotificationKeyData))
			{
				Json::Value data = attach[nim::kNIMNotificationKeyData];
				if (data.isObject() && data.isMember(nim::kNIMNotificationKeyDataId))
					relate_ids.insert(data[nim::kNIMNotificationKeyDataId].asString());
				if (data.isObject() && data.isMember(nim::kNIMNotificationKeyUserNameCards) && data[nim::kNIMNotificationKeyUserNameCards].isArray())
				{
					Json::Value name_cards_json = data[nim::kNIMNotificationKeyUserNameCards];
					for (uint32_t i = 0; i < name_cards_json.size(); i++)
						relate_ids.insert(name_cards_json[i][nim::kNIMNameCardKeyAccid].asString());
				}
			}
		}

		if (!relate_ids.empty())
		{
			std::list<std::string> uids(relate_ids.cbegin(), relate_ids.cend());
			std::list<nim::UserNameCard> uinfos;
			UserService::GetInstance()->GetUserInfos(uids, uinfos);
		}
	}
	if (msg_->type_ == nim::kNIMSessionTypeP2P)
	{
		if (SubscribeEventManager::GetInstance()->IsEnabled())
		{
			EventDataEx data;
			SubscribeEventManager::GetInstance()->GetEventData(nim::kNIMEventTypeOnlineState, msg->id_, data);
			if (data.online_client_.online_client_type_.size() == 0 && msg->id_ == LoginManager::GetInstance()->GetAccount())
				data.online_client_.online_client_type_.insert(nim::kNIMClientTypePCWindows);
			SetOnlineState(data);
		}
		SetMute(nim_comp::MuteBlackService::GetInstance()->IsInMuteList(msg_->id_));
	}
	else
	{
		HideOnlineState();
		SetMute(nim_comp::SessionManager::GetInstance()->IsTeamMsgMuteShown(msg_->id_, -1));
	}
}

void SearchMsgItem::SetOnlineSessionType(bool is_online_session)
{
	is_online_session_ = is_online_session;
}

void SearchMsgItem::SetMute(bool mute)
{
	FindSubControl(L"not_disturb")->SetVisible(mute);
}

void SearchMsgItem::InitUserProfile()
{
	if (msg_->type_ == nim::kNIMSessionTypeP2P)
	{
		if (LoginManager::GetInstance()->IsEqual(msg_->id_))
		{
			label_name_->SetText(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_MAINWINDOW_MY_MOBILEPHONE"));
		}
		else
		{
			label_name_->SetText(UserService::GetInstance()->GetUserName(msg_->id_));
		}
		head_image_->SetBkImage(PhotoService::GetInstance()->GetUserPhoto(msg_->id_));
	}
	else
	{
		std::wstring tname = TeamService::GetInstance()->GetTeamName(msg_->id_);
		label_name_->SetText(tname);
		head_image_->SetBkImage(PhotoService::GetInstance()->GetTeamPhoto(msg_->id_, false));
	}
}

void SearchMsgItem::SetOnlineState(const EventDataEx& data)
{
	if (label_online_state_ == nullptr)
		return;
	label_online_state_->SetText(OnlineStateEventUtil::GetOnlineState(msg_->id_, data, true));
	if (!label_online_state_->IsVisible())
		label_online_state_->SetVisible(true);
}
void SearchMsgItem::HideOnlineState()
{
	if (label_online_state_ == nullptr)
		return;
	label_online_state_->SetVisible(false);
}
void SearchMsgItem::UpdateMsgContent(const std::string& id /*= ""*/)
{
	if (!id.empty() && relate_ids.find(id) == relate_ids.cend())
		return;

	std::wstring show_text;
	if (msg_->msg_status_ != nim::kNIMMsgLogStatusDeleted)
	{
		SessionItemHelper::GetMsgContent(*msg_, show_text);

		bool need_prefix = true;
		if (msg_->msg_type_ == nim::kNIMMessageTypeText)
		{
			Json::Value values;
			Json::Reader reader;
			if (reader.parse(msg_->msg_attach_, values)
				&& values.isObject()
				&& values.isMember("comment")
				&& values["comment"].asString() == "is_recall_notification")
			{
				if (values.isMember("notify_from"))
				{
					need_prefix = false;
					std::string from_id = values["notify_from"].asString();
					std::string from_nick = values["from_nick"].asString();
					std::string operator_id = values["operator_id"].asString();
					if (operator_id.empty())
						operator_id = from_id;
					show_text = GetRecallNotifyTextEx(msg_->id_, msg_->type_, from_id, operator_id,from_nick);
				}
			}
		}

		if (need_prefix && msg_->type_ == nim::kNIMSessionTypeTeam)
		{
			if (msg_->msg_type_ == nim::kNIMMessageTypeNotification && !IsNetCallMsg((nim::NIMMessageType)msg_->msg_type_, msg_->msg_attach_))
				; // do nothing
			else
			{
				switch (msg_->msg_type_)
				{
				case nim::kNIMMessageTypeText:
				case nim::kNIMMessageTypeImage:
				case nim::kNIMMessageTypeAudio:
				case nim::kNIMMessageTypeVideo:
				case nim::kNIMMessageTypeLocation:
				case nim::kNIMMessageTypeNotification:
				case nim::kNIMMessageTypeFile:
				case nim::kNIMMessageTypeTips:
				case nim::kNIMMessageTypeCustom:
				case nim::kNIMMessageTypeUnknown:
				{
					std::wstring nick_name = UserService::GetInstance()->GetUserName(msg_->msg_sender_accid_);
					if (!nick_name.empty())
					{
						show_text = nick_name + L": " + show_text;
					}
				}
					break;
				default:
					break;
				}						
			}
		}

		if (msg_->msg_status_ == nim::kNIMMsgLogStatusSendFailed)
		{
			show_text = MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_ITEM_MSG_TYPE_FAILED") + show_text;
		}
	}
	label_msg_->SetText(show_text);
}

void SearchMsgItem::ClearMsg()
{
	label_msg_->SetText(L"");
	label_time_->SetVisible(false);
	ResetUnread();
}

long long SearchMsgItem::GetMsgTime()
{
	return msg_->msg_timetag_;
}

int SearchMsgItem::GetUnread()
{
	return msg_->unread_count_;
}

void SearchMsgItem::SetUnread(int unread)
{
	if (is_online_session_)
		msg_->unread_count_ = unread;
	UpdateUnread();
}

void SearchMsgItem::AddUnread()
{
	if (is_online_session_)
		msg_->unread_count_ += 1;
	UpdateUnread();
	ShowAtmeTip(true);
}

bool SearchMsgItem::ResetUnread()
{
	if (is_online_session_ && msg_->unread_count_ != 0)
		msg_->unread_count_ = 0;
	UpdateUnread();
	ShowAtmeTip(false);
	//InvokeResetUnread(msg_->id_, msg_->type_);
	Invalidate();
	return true;
}

void SearchMsgItem::DeleteRecentSessionCb(nim::NIMResCode code, const nim::SessionData &result, int total_unread_counts)
{
	QLOG_APP(L"delete recent session, code={0} command={1} total_un_cn={2}") << code << result.command_ << total_unread_counts;
}

void SearchMsgItem::BatchStatusDeleteCb(nim::NIMResCode res_code, const std::string& uid, nim::NIMSessionType to_type)
{
	QLOG_APP(L"batch delete msg, id={0} type={1} code={2}") << uid << to_type << res_code;
	//if (res_code == nim::kNIMResSuccess)
	//	nim::Session::SetUnreadCountZeroAsync(to_type, uid, nim::Session::SetUnreadCountZeroCallback());
}

void SearchMsgItem::ShowAtmeTip(bool show)
{
	if (show)
	{
		label_atme_->SetVisible((ForcePushManager::GetInstance()->IsContainAtMeMsg(msg_->id_)));
	}
	else
	{
		label_atme_->SetVisible(false);
	}
}

void SearchMsgItem::UpdateUnread()
{
	if (msg_->unread_count_ > 0)
	{
		if (msg_->unread_count_ < 100) {
			label_unread_->SetText(nbase::StringPrintf(L"%d", msg_->unread_count_));
		}
		else {
			label_unread_->SetText(L"99+");
		}
		box_unread_->SetVisible(true);
	}
	else
	{
		box_unread_->SetVisible(false);
		label_unread_->SetText(L"0");
	}

	if (msg_->unread_count_ == 0)
		// 重置对应会话中的@me消息为已读
		ForcePushManager::GetInstance()->ResetAtMeMsg(msg_->id_);

	// 通知会话窗口中的会话合并项
	SessionBox *session_box = SessionManager::GetInstance()->FindSessionBox(msg_->id_);
	if (session_box)
		session_box->GetSessionForm()->InvokeSetSessionUnread(msg_->id_, msg_->unread_count_);
}

void SearchMsgItem::PopupSearchMsgItemMenu(POINT point)
{
	if (is_online_session_)//因为涉及到位置切换，所以取消右键
		return;
	CMenuWnd* pMenu = new CMenuWnd(NULL);
	STRINGorID xml(L"session_item_menu.xml");
	pMenu->Init(xml, _T("xml"), point);

	CMenuElementUI* del_session_item = (CMenuElementUI*)pMenu->FindControl(L"del_session_item");
	del_session_item->AttachSelect(nbase::Bind(&SearchMsgItem::DelSearchMsgItemMenuItemClick, this, std::placeholders::_1));
	if (is_online_session_)
	{
		auto del_session_label = dynamic_cast<Label*>(del_session_item->FindSubControl(L"del_session_item_text"));
		if (del_session_label)
			del_session_label->SetTextId(L"STRID_SESSION_ITEM_DELETE_ONLINE_SESSION");
	}

	CMenuElementUI* del_session_msg = (CMenuElementUI*)pMenu->FindControl(L"del_session_msg");
	del_session_msg->AttachSelect(nbase::Bind(&SearchMsgItem::DelSearchMsgItemMenuItemClick, this, std::placeholders::_1));

	ui::ListContainerElement* del_session_msg_online = (ui::ListContainerElement*)pMenu->FindControl(L"del_session_msg_online");
	if (del_session_msg_online != nullptr)
		if (msg_->type_ == nim::kNIMSessionTypeP2P)
		{
			del_session_msg_online->SetVisible(true);
			del_session_msg_online->AttachSelect(nbase::Bind(&SearchMsgItem::DelSearchMsgItemMenuItemClick, this, std::placeholders::_1));
		}
		else
		{
			del_session_msg_online->SetVisible(false);
		}
		
	pMenu->Show();
}

bool SearchMsgItem::DelSearchMsgItemMenuItemClick(ui::EventArgs* param)
{
	std::wstring name = param->pSender->GetName();
	if (name == L"del_session_item")
	{
		bool has_transfer_task = TransferFileManager::GetInstance()->HasTransferTask(msg_->id_);

		auto closure = [this, has_transfer_task](MsgBoxRet ret) {
			if (ret == MB_YES)
			{
				if (is_online_session_)
				{
					nim::SessionOnLineServiceHelper::DeleteSessionParam delete_param;
					delete_param.AddSession(msg_->type_, msg_->id_);
					nim::SessionOnLineService::DeleteSession(delete_param, [](nim::NIMResCode res_code) {
						QLOG_APP(L"delete online session, code={0}") << res_code;
					});
				}
				else
				{
					nim::Session::DeleteRecentSession(msg_->type_, msg_->id_, nbase::Bind(&SearchMsgItem::DeleteRecentSessionCb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
				}
				SubscribeEventManager::GetInstance()->UnSubscribeSessionEvent(*msg_);
				m_pWindow->SendNotify(this, ui::kEventNotify, 0, 0);

				if (has_transfer_task)
				{
					TransferFileManager::GetInstance()->RemoveAllTaskBySessionBoxId(msg_->id_);
				}
			}
		};

		if (has_transfer_task)
		{
			ShowMsgBox(nullptr, closure, L"STRID_SESSION_HAS_TRANSFER_FILE_TASK", true, L"STRING_MULTIVIDEOCHATFORM_TITLE_PROMPT", true, L"STRING_OK", true, L"STRING_NO", true);
		}
		else
		{
			closure(MB_YES);
		}
	}
	else if (name == L"del_session_msg")
	{
		nim::Session::SetUnreadCountZeroAsync(msg_->type_, msg_->id_, ToWeakCallback([this](nim::NIMResCode res_code, const nim::SessionData&, int){
			if (res_code == nim::kNIMResSuccess)			
			nim::MsgLog::BatchStatusDeleteAsyncEx(msg_->id_, msg_->type_, (atoi(GetConfigValue("kNIMMsglogRevert").c_str()) != 0),
				nbase::Bind(&SearchMsgItem::BatchStatusDeleteCb, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3));
		}));		
	}
	else if (name == L"del_session_msg_online")
	{
		auto box = dynamic_cast<ui::ListContainerElement*>(param->pSender);
		bool delete_roaming = dynamic_cast<ui::CheckBox*>(box->FindSubControl(L"chkbox_delete_roaming"))->IsSelected();
		auto task = [this, delete_roaming]() {
			nim::MsgLog::DeleteHistoryOnlineAsync(msg_->id_, delete_roaming, "", \
				ToWeakCallback([](nim::NIMResCode res_code, const std::string& session_id) {
					ShowMsgBox(nullptr, nullptr, \
						(res_code == nim::NIMResCode::kNIMResSuccess ? 
						L"STRID_SESSION_ITEM_DELETE_ONLINE_MSGLOG_SUCCESS":
						L"STRID_SESSION_ITEM_DELETE_ONLINE_MSGLOG_FAILED" ),
						true, L"STRING_MULTIVIDEOCHATFORM_TITLE_PROMPT", true, L"STRING_OK", true);
			}));
		};
		Post2UI(ToWeakCallback(task));
		box->GetWindow()->Close();
	}
	return true;
}

bool SearchMsgItem::OnDbClicked(ui::EventArgs* arg)
{
	if (msg_->type_ == nim::kNIMSessionTypeTeam
		|| msg_->type_ == nim::kNIMSessionTypeP2P)
	{
		//SessionManager::GetInstance()->OpenSessionBox(msg_->id_, msg_->type_);
		//MsgRecordForm* f = WindowsManager::SingletonShow<MsgRecordForm>(MsgRecordForm::kClassName);
		//f->RefreshRecord(msg_->id_, msg_->type_);
		ShowMsgs(msg_vec_);
	}
	else
	{
		shared::Toast::ShowToast(
			ui::MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_SESSION_ITEM_NON_TEAM_P2P_NOT_SUPPORTED_TIP"), 
			1000);
	}
	return true;
}

bool SearchMsgItem::OnSearchMsgItemMenu(ui::EventArgs* arg)
{
	POINT point;
	::GetCursorPos(&point);
	PopupSearchMsgItemMenu(point);
	return true;
}

bool SearchMsgItem::OnHeadImageClicked(bool is_robot, ui::EventArgs * arg)
{
	if (msg_->type_ == nim::kNIMSessionTypeTeam)
	{
		auto team_info = nim::Team::QueryTeamInfoBlock(msg_->id_);
		TeamInfoForm::ShowTeamInfoForm(false, team_info.GetType(), msg_->id_, team_info);
	}
	else if (msg_->type_ == nim::kNIMSessionTypeP2P)
	{
		ProfileForm::ShowProfileForm(msg_->id_, is_robot);
	}
	return true;
}

void SearchMsgItem::AttachListBox(ui::ListBox* list, const std::vector<nim::IMMessage> &msg, bool show)
{
	first_show_msg_ = true;
	last_msg_time_ = 0;
	farst_msg_time_ = 0;
	last_server_id_ = 0;
	id_bubble_pair_.clear();

	if (list != NULL)
	{
		message_list_box_ = list;
		msg_vec_ = msg;
		if (msg_->type_ == nim::kNIMSessionTypeTeam)
		{
			nim::Team::QueryTeamMembersCallback cb = ToWeakCallback([this](const std::string& tid, int member_count, const std::list<nim::TeamMemberProperty>& props)
			{
				team_member_info_list_.clear();
				for (const auto &info : props)
					team_member_info_list_[info.GetAccountID()] = info;
			});
			nim::Team::QueryTeamMembersAsync(msg_->id_, cb);
		}
		if (show)
		{
			ShowMsgs(msg_vec_);
		}
	}
}

void SearchMsgItem::ShowMsgs(const std::vector<nim::IMMessage> &msg)
{
	if (NULL == message_list_box_) {
		QLOG_APP(L"[SearchMsgItem::ShowMsgs] message_list_box_ is null");
		return;
	}
	message_list_box_->RemoveAll();
	id_bubble_pair_.clear();

	int pos = message_list_box_->GetScrollRange().cy - message_list_box_->GetScrollPos().cy;
	bool show_time = false;
	//msg倒序排列
	size_t len = msg.size();
	for (size_t i = 0; i < len; i++)
	{
		if (len == 1 || i == len - 1)
		{
			show_time = true;
		}
		else
		{
			long long older_time = 0;
			for (size_t j = i + 1; j < len; j++)
			{
				if (!IsNoticeMsg(msg[j]) && !IsRTSMsg(msg[j].type_, msg[j].attach_))
				{
					older_time = msg[j].timetag_;
					break;
				}
			}
			show_time = CheckIfShowTime(older_time, msg[i].timetag_);
		}
		ShowMsg(msg[i], true, show_time);
	}
	//修正最近时间
	if (first_show_msg_)
	{
		first_show_msg_ = false;
		message_list_box_->EndDown(true, false);
		if (len > 0 && last_msg_time_ == 0)
		{
			for (const auto &i : msg)
			{
				if (!IsNoticeMsg(i) && !IsRTSMsg(i.type_, i.attach_))
				{
					last_msg_time_ = i.timetag_;
					break;
				}
			}
		}
	}
	else
	{
		message_list_box_->SetPos(message_list_box_->GetPos());

		ui::CSize sz = message_list_box_->GetScrollPos();
		sz.cy = message_list_box_->GetScrollRange().cy - pos;
		message_list_box_->SetScrollPos(sz);
	}
	//修正最远时间
	if (len > 0)
	{
		farst_msg_time_ = msg[len - 1].timetag_;
		last_server_id_ = msg[len - 1].readonly_server_id_;
	}
}

void SearchMsgItem::ShowMsg(const nim::IMMessage &msg, bool first, bool show_time)
{
	const std::string &bubble_id = msg.client_msg_id_;
	if (bubble_id.empty())
	{
		QLOG_WAR(L"msg id empty");
		return;
	}

	IdBubblePair::iterator it = id_bubble_pair_.find(bubble_id);
	if (it != id_bubble_pair_.end())
	{
		QLOG_WAR(L"repeat msg: {0}") << bubble_id;
		return;
	}

	MsgBubbleItem* item = new MsgBubbleText;


	bool bubble_right = IsBubbleRight(msg);
	
	GlobalManager::FillBoxWithCache(item, L"session/bubble_left.xml");

	if (first)
		message_list_box_->AddAt(item, 0);
	else
		message_list_box_->Add(item);

	id_bubble_pair_[bubble_id] = item;

	std::string sid = nbase::UTF16ToUTF8(L"MsgRecordForm");

	item->InitControl(bubble_right);
	item->InitInfo(msg);
	item->SetSessionId(sid);
	item->SetSessionType(msg_->type_);
	item->SetActionMenu(false);
	item->SetShowTime(show_time);
	std::string show_name("");
	item->SetShowName(GetUserShowName(msg, show_name), show_name);
	item->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&SearchMsgItem::Notify, this, std::placeholders::_1));
	item->SetContextShow(true);
}

bool SearchMsgItem::GetUserShowName(const nim::IMMessage &msg, std::string& show_name)
{
	if (IsBubbleRight(msg) || msg.session_type_ == nim::kNIMSessionTypeP2P)
		return false;
	else
	{
		auto get_name_task = [&](void)->std::string {
			auto iter = team_member_info_list_.find(msg.sender_accid_);
			if (iter != team_member_info_list_.cend() && !iter->second.GetNick().empty())//显示群名片
				return iter->second.GetNick();
			else//显示备注名或昵称				
				return nbase::UTF16ToUTF8(UserService::GetInstance()->GetUserName(msg.sender_accid_));
		};
		if (msg.type_ != nim::kNIMMessageTypeRobot)
		{
			show_name = std::move(get_name_task());
		}
		else
		{
			show_name = "";
		}
		return true;
	}
}

bool SearchMsgItem::Notify(ui::EventArgs* param)
{
	if (param->Type == ui::kEventNotify)
	{
		if (param->wParam == BET_SHOW_CONTEXT)
		{
			MsgBubbleItem* item = dynamic_cast<MsgBubbleItem*>(param->pSender);
			assert(item);
			nim::IMMessage md = item->GetMsg();

			MsgRecordForm* f = WindowsManager::SingletonShow<MsgRecordForm>(MsgRecordForm::kClassName);
			f->RefreshRecord(md.local_talk_id_, md.session_type_, md.timetag_+1, false);
			return false;
		}
	}
	return true;
}
}