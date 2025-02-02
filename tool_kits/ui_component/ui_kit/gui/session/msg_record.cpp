#include "stdafx.h"
#include "msg_record.h"
#include "module/audio/audio_manager.h"
#include "module/multi_retweet/multi_retweet_manager.h"
using namespace ui;

namespace nim_comp
{
const int kMsgLogNumberShow = 20;

void MsgRecordForm::ShowMsg(const nim::IMMessage &msg, bool first, bool show_time)
{
	const std::string &bubble_id = msg.client_msg_id_;
	if(bubble_id.empty())
	{
		QLOG_WAR(L"msg id empty");
		return;
	}

	IdBubblePair::iterator it = id_bubble_pair_.find(bubble_id);
	if(it != id_bubble_pair_.end())
	{
		QLOG_WAR(L"repeat msg: {0}") <<bubble_id;
		return;
	}

	MsgBubbleItem* item = NULL;

	if (msg.type_ == nim::kNIMMessageTypeText 
		|| IsNetCallMsg(msg.type_, msg.attach_))
	{
		Json::Value values;
		Json::Reader reader;
		if (reader.parse(msg.attach_, values)
			&& values.isObject()
			&& values.isMember("comment")
			&& values["comment"].asString() == "is_recall_notification")
		{
			MsgBubbleNotice* cell = new MsgBubbleNotice;
			GlobalManager::FillBoxWithCache(cell, L"session/cell_notice.xml");
			if (first)
				msg_list_->AddAt(cell, 0);
			else
				msg_list_->Add(cell);
			cell->InitControl();
			cell->InitInfo(msg, session_id_, true);
			return;
		}
		else
		{
			item = new MsgBubbleText;
		}
	}
	else if (msg.type_ == nim::kNIMMessageTypeImage)
		item = new MsgBubbleImage;
	else if (msg.type_ == nim::kNIMMessageTypeAudio)
		item = new MsgBubbleAudio;
	else if (msg.type_ == nim::kNIMMessageTypeFile)
		item = new MsgBubbleFile;
	else if (msg.type_ == nim::kNIMMessageTypeLocation)
		item = new MsgBubbleLocation;
	else if (msg.type_ == nim::kNIMMessageTypeVideo)
		item = new MsgBubbleVideo;
	else if (msg.type_ == nim::kNIMMessageTypeNotification || msg.type_ == nim::kNIMMessageTypeTips)
	{
		id_bubble_pair_[bubble_id] = NULL;

		MsgBubbleNotice* cell = new MsgBubbleNotice;
		GlobalManager::FillBoxWithCache(cell, L"session/cell_notice.xml");
		if(first)
			msg_list_->AddAt(cell, 0);
		else
			msg_list_->Add(cell);
		cell->InitControl();
		cell->InitInfo(msg, session_id_);
		return;
	}
	else if (msg.type_ == nim::kNIMMessageTypeCustom)
	{
		Json::Value json;
		if (StringToJson(msg.attach_, json) && json.isObject())
		{
			int sub_type = json["type"].asInt();
			if (sub_type == CustomMsgType_Jsb) //finger
			{
				item = new MsgBubbleFinger;
			}
			else if (sub_type == CustomMsgType_SnapChat)
			{
				item = new MsgBubbleSnapChat;
			}
			else if (sub_type == CustomMsgType_Sticker)
			{
				item = new MsgBubbleSticker;
			}
			else if (sub_type == CustomMsgType_MultiRetweet) 
			{
				item = new MsgBubbleMultiRetweet;
			}
			else if (sub_type == CustomMsgType_Rts)
			{
				if (json["data"].isObject())
				{
					int flag = json["data"]["flag"].asInt();
					if (flag == 0)
					{
						item = new MsgBubbleText;
					}
					else if (flag == 1)
					{
						id_bubble_pair_[bubble_id] = NULL;

						MsgBubbleNotice* cell = new MsgBubbleNotice;
						GlobalManager::FillBoxWithCache(cell, L"session/cell_notice.xml");
						if (first)
							msg_list_->AddAt(cell, 0);
						else
							msg_list_->Add(cell);
						cell->InitControl();
						cell->InitInfo(msg, session_id_);
						return;
					}
				}
			}
		}
	}
	else if(msg.type_ == nim::kNIMMessageTypeG2NetCall)
	{
		item = new MsgBubbleText;
	}

	if (item == nullptr)
	{
		QLOG_WAR(L"unknown msg: cid={0} msg_type={1}") << bubble_id << msg.type_;
		item = new MsgBubbleUnknown;
	}

	bool bubble_right = IsBubbleRight(msg);
	if(bubble_right)
		GlobalManager::FillBoxWithCache( item, L"session/bubble_right.xml" );
	else
		GlobalManager::FillBoxWithCache( item, L"session/bubble_left.xml" );

	if(first)
		msg_list_->AddAt(item, 0);
	else
		msg_list_->Add(item);

	id_bubble_pair_[bubble_id] = item;

	std::string sid = nbase::UTF16ToUTF8(GetWindowClassName());

	item->InitControl(bubble_right);
	item->InitInfo(msg);
	item->SetSessionId(sid);
	item->SetSessionType(session_type_);
	item->SetActionMenu(false);
	item->SetShowTime(show_time);
	std::string show_name("");
	item->SetShowName(GetUserShowName(msg, show_name), show_name);

	if (msg.type_ == nim::kNIMMessageTypeAudio)
		item->SetPlayed(true);
	auto it_closure_befor_item_add = closure_befor_item_add_.find(bubble_id);
	if (it_closure_befor_item_add != closure_befor_item_add_.end())
	{
		QLOG_WAR(L"it_closure_befor_item_add != closure_befor_item_add_.end() task count {0}") << it_closure_befor_item_add->second.size();
		for (auto & it : it_closure_befor_item_add->second)
			it();
		closure_befor_item_add_.erase(it_closure_befor_item_add);
	}
	//if( item->NeedDownloadResource() )
	//{
	//	nim::NOS::FetchMedia(msg, nbase::Bind(&MsgRecordForm::OnDownloadCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4), nim::NOS::ProgressCallback());
	//}
}
bool MsgRecordForm::GetUserShowName(const nim::IMMessage &msg, std::string& show_name)
{
	if (IsBubbleRight(msg) || msg.session_type_ == nim::kNIMSessionTypeP2P)
		return false;
	else
	{
		auto get_name_task = [&](void)->std::string{
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
void MsgRecordForm::RefreshRecord(std::string id, nim::NIMSessionType type)
{
	if(id == session_id_)
		return;
	session_id_ = id;
	session_type_ = type;

	//初始化
	first_show_msg_ = true;
	last_msg_time_ = 0;
	farst_msg_time_ = 0;
	last_server_id_ = 0;

	front_has_more_ = true;
	back_has_more_ = true;
	is_loading_ = false;

	std::wstring name;
	if (type == nim::kNIMSessionTypeP2P)
		name = UserService::GetInstance()->GetUserName(id);
	else
		name = TeamService::GetInstance()->GetTeamName(id);
	label_title_->SetText(name);
	SetTaskbarTitle(name);

	msg_list_->RemoveAll();
	closure_befor_item_add_.clear();
	id_bubble_pair_.clear();
	
	AudioManager::GetInstance()->StopPlayAudio(nbase::UTF16ToUTF8(GetWindowId()));

	if (type == nim::kNIMSessionTypeTeam)
	{
		nim::Team::QueryTeamMembersCallback cb = ToWeakCallback([this](const std::string& tid, int member_count, const std::list<nim::TeamMemberProperty>& props) 
		{
			team_member_info_list_.clear();
			for (const auto &info : props)
				team_member_info_list_[info.GetAccountID()] = info;
			ShowMore(false, false);
		});
		nim::Team::QueryTeamMembersAsync(session_id_, cb);
	}
	else
		ShowMore(false, false);
}

void MsgRecordForm::ShowMore(bool more, bool reverse)
{
	is_loading_ = true;
	is_reverse_ = reverse;
	QLOG_APP(L"query online msg begin: id={0} type={1} last_time={2} last_server_id={3}") <<session_id_ <<session_type_ <<farst_msg_time_ <<last_server_id_;
	
	if (is_local_msg_)
	{
		std::string json_extension = "";
		if (reverse)
		{
			Json::Value json_extension_value;
			json_extension_value["direction"] = 1;
			Json::FastWriter fw;
			json_extension = fw.write(json_extension_value);
		}		
		

		//该接口搜索本地记录不支持反向，改用在线搜索
		nim::MsgLog::QueryMsgCallback cb = nbase::Bind(&MsgRecordForm::QueryMsgOnlineCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);
		nim::MsgLog::QueryMsgAsync(session_id_, session_type_, kMsgLogNumberShow, farst_msg_time_, cb, json_extension);
	}
	else
	{
		nim::MsgLog::QueryMsgOnlineAsyncParam param;
		if (reverse)
		{
			param.from_time_ = farst_msg_time_;
			param.end_time_ = farst_msg_time_ * 2;
		}
		else
		{
			param.from_time_ = 0;
			param.end_time_ = farst_msg_time_;
		}
		
		param.id_ = session_id_;
		param.to_type_ = session_type_;
		param.limit_count_ = kMsgLogNumberShow;		
		param.end_msg_id_ = last_server_id_;
		param.reverse_ = reverse;
		param.need_save_to_local_ = true;
		param.auto_download_attachment_ = true;
		nim::MsgLog::QueryMsgCallback cb = nbase::Bind(&MsgRecordForm::QueryMsgOnlineCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

		nim::MsgLog::QueryMsgOnlineAsync(param, cb);
	}
	
}

void MsgRecordForm::ShowMsgs(const std::vector<nim::IMMessage> &msg)
{
	int pos = msg_list_->GetScrollRange().cy - msg_list_->GetScrollPos().cy;	
	bool show_time = false;
	//msg倒序排列
	size_t len = msg.size();
	for(size_t i = 0; i < len; i++)
	{
		if(len == 1 || i == len-1)
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
		ShowMsg(msg[i], !is_reverse_, show_time);
	}
	//修正最近时间
	if(first_show_msg_)
	{
		ShowMore(false, true);
		first_show_msg_ = false;

		msg_list_->EndDown(true, false);

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
		msg_list_->SetPos( msg_list_->GetPos() );

		ui::CSize sz = msg_list_->GetScrollPos();
		sz.cy = msg_list_->GetScrollRange().cy - pos;
		msg_list_->SetScrollPos(sz);
	}
	//修正最远时间
	if(len > 0)
	{
		farst_msg_time_ = msg[len-1].timetag_;
		last_server_id_ = msg[len-1].readonly_server_id_;
	}
}

void MsgRecordForm::QueryMsgOnlineCb(nim::NIMResCode code, const std::string& id, nim::NIMSessionType type, const nim::QueryMsglogResult& result)
{
	QLOG_APP(L"query online msg end: code={0} id={1} type={2} count={3} source={4}") <<code <<id <<type <<result.msglogs_.size()<< result.source_;

	if (id != session_id_)
		return;
	is_loading_ = false;
	LoadingTip(false);

	if (code == nim::kNIMResSuccess)
	{
		std::vector<nim::IMMessage> vec;
		for (auto& msg : result.msglogs_)
		{
			vec.push_back(msg);
		}
		ShowMsgs(vec);

		if (vec.size() < kMsgLogNumberShow)
		{
			if (is_list_top_)
			{
				front_has_more_ = false;
			}
			else
			{
				back_has_more_ = false;
			}			
		}
		else
		{
			if (is_list_top_)
			{
				front_has_more_ = true;
			}
			else
			{
				back_has_more_ = true;
			}
		}
	}
}

void MsgRecordForm::OnDownloadCallback(nim::NIMResCode code, const std::string& file_path, const std::string& sid, const std::string& cid)
{
	if (sid != session_id_)
		return;
	static auto OnDownloadCallback_task = [](MsgBubbleItem* item, bool success, const std::string& file_path) {
		item->OnDownloadCallback(success, file_path);
	};	
	IdBubblePair::iterator it = id_bubble_pair_.find(cid);
	if(it != id_bubble_pair_.end())
	{
		MsgBubbleItem* item = it->second;
		it->second->OnDownloadCallback(code == nim::kNIMResSuccess, file_path);		
	}
	else
	{
		QLOG_WAR(L"MsgRecordForm::OnDownloadCallback cid {0}") << cid;
		closure_befor_item_add_[cid].emplace_back(ToWeakCallback(
			[this, cid, code, file_path]() {
				IdBubblePair::iterator it = id_bubble_pair_.find(cid);
				if (it != id_bubble_pair_.end())
					OnDownloadCallback_task(it->second,code == nim::kNIMResSuccess, file_path);
			})
		);
	}
}

void MsgRecordForm::OnPlayAudioCallback( const std::string &cid, int code )
{
	IdBubblePair::iterator it = id_bubble_pair_.find(cid);
	if(it != id_bubble_pair_.end())
	{
		MsgBubbleAudio* item = dynamic_cast<MsgBubbleAudio*>(it->second);
		if(item)
		{
			item->OnPlayCallback(code);
		}
	}
	if (code != nim::kNIMResSuccess)
	{
		std::wstring tip = nbase::StringPrintf(MutiLanSupport::GetInstance()->GetStringViaID(L"STRID_MSG_RECORD_PLAY_AUDIO_ERROR").c_str(), code);
		ShowMsgBox(m_hWnd, MsgboxCallback(), tip, false);
	}
}

void MsgRecordForm::OnStopAudioCallback( const std::string &cid, int code )
{
	IdBubblePair::iterator it = id_bubble_pair_.find(cid);
	if(it != id_bubble_pair_.end())
	{
		MsgBubbleAudio* item = dynamic_cast<MsgBubbleAudio*>(it->second);
		if(item)
		{
			item->OnStopCallback(code);
		}
	}
}

void MsgRecordForm::RefreshRecord(std::string id, nim::NIMSessionType type, long long farst_msg_time, bool is_local_msg)
{
	if (id == session_id_)
		return;
	session_id_ = id;
	session_type_ = type;

	//初始化
	first_show_msg_ = true;
	last_msg_time_ = 0;
	farst_msg_time_ = farst_msg_time;
	last_server_id_ = 0;

	front_has_more_ = true;
	back_has_more_ = true;
	is_loading_ = false;
	is_local_msg_ = is_local_msg;

	std::wstring name;
	if (type == nim::kNIMSessionTypeP2P)
		name = UserService::GetInstance()->GetUserName(id);
	else
		name = TeamService::GetInstance()->GetTeamName(id);
	label_title_->SetText(name);
	SetTaskbarTitle(name);

	msg_list_->RemoveAll();
	closure_befor_item_add_.clear();
	id_bubble_pair_.clear();

	AudioManager::GetInstance()->StopPlayAudio(nbase::UTF16ToUTF8(GetWindowId()));

	if (type == nim::kNIMSessionTypeTeam)
	{
		nim::Team::QueryTeamMembersCallback cb = ToWeakCallback([this](const std::string& tid, int member_count, const std::list<nim::TeamMemberProperty>& props)
		{
			team_member_info_list_.clear();
			for (const auto &info : props)
				team_member_info_list_[info.GetAccountID()] = info;
			ShowMore(false, false);
		});
		nim::Team::QueryTeamMembersAsync(session_id_, cb);
	}
	else
	{
		ShowMore(false, false);			
	}
		
}
}