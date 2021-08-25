#include "stdafx.h"
#include "resource.h"
#include "search_msg.h"
#include "control/custom_msg.h"
#include "export/nim_ui_all.h"
#include "control/search_msg_item.h"

using namespace ui;

namespace nim_comp
{
	const LPCTSTR SearchMsg::kClassName = L"Search";

	const int kSysmsgCount = 20;

	SearchMsg::SearchMsg()
	{
	}

	SearchMsg::~SearchMsg()
	{

	}

	std::wstring SearchMsg::GetSkinFolder()
	{
		return L"main";
	}

	std::wstring SearchMsg::GetSkinFile()
	{
		return L"search_msg.xml";
	}

	std::wstring SearchMsg::GetWindowClassName() const
	{
		return kClassName;
	}

	std::wstring SearchMsg::GetWindowId() const
	{
		return kClassName;
	}

	UINT SearchMsg::GetClassStyle() const
	{
		return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
	}

	void SearchMsg::InitWindow()
	{
		if (nim_ui::UserConfig::GetInstance()->GetDefaultIcon() > 0)
		{
			SetIcon(nim_ui::UserConfig::GetInstance()->GetDefaultIcon());
		}

		btn_max_restore_ = static_cast<ui::Button*>(FindControl(L"btn_max_restore"));

		m_pRoot->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&SearchMsg::Notify, this, std::placeholders::_1));
		m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&SearchMsg::OnClicked, this, std::placeholders::_1));

		InitSearchBar();
	}

	void SearchMsg::InitSearchBar()
	{
		search_bar_ = dynamic_cast<ui::ButtonBox*>(FindControl(L"search_bar"));

		search_edit_ = static_cast<ui::RichEdit*>(FindControl(_T("search_edit")));

		search_edit_->AttachTextChange(nbase::Bind(&SearchMsg::SearchEditChange, this, std::placeholders::_1));
		search_edit_->SetLimitText(30);

		search_edit_->AttachBubbledEvent(ui::kEventKeyDown, nbase::Bind(&SearchMsg::Notify, this, std::placeholders::_1));
		
		btn_clear_input_ = static_cast<ui::Button*>(FindControl(L"clear_input"));
		btn_clear_input_->AttachClick(nbase::Bind(&SearchMsg::OnClearInputBtnClicked, this, std::placeholders::_1));
		btn_clear_input_->SetNoFocus();

		search_result_list_ = static_cast<ui::ListBox*>(FindControl(_T("search_result_list")));
		search_result_list_->AttachBubbledEvent(ui::kEventReturn, nbase::Bind(&SearchMsg::OnReturnEventsClick, this, std::placeholders::_1));
		
		search_result_list_->AttachBubbledEvent(ui::kEventMouseDoubleClick, [this](ui::EventArgs* param) {
			search_result_list_->SetVisible(false);
			return true;
		});

		msg_list_ = static_cast<ui::ListBox*>(FindControl(_T("msg_list")));
		msg_list_->AttachBubbledEvent(ui::kEventReturn, nbase::Bind(&SearchMsg::OnReturnEventsClick, this, std::placeholders::_1));
	}

	bool SearchMsg::SearchEditChange(ui::EventArgs* param)
	{
		QLOG_APP(L"Search msg callback 搜索聊天记录 ");

		std::string search_key = nbase::UTF16ToUTF8(search_edit_->GetText());
		bool has_serch_key = !search_key.empty();

		//RunTimeDataManager::GetInstance()->SetSearchingFriendState(has_serch_key);

		btn_clear_input_->SetVisible(has_serch_key);
		search_result_list_->RemoveAll();
		message_list_sort_data_.clear();
		msg_list_->SetVisible(has_serch_key);

		//FindControl(L"no_search_result_tip")->SetVisible(has_serch_key);

		if (has_serch_key)
		{
			// 2020-10-07

			auto cb = ToWeakCallback([this](nim::NIMResCode code, const std::string& query_id, nim::NIMSessionType query_type, const nim::QueryMsglogResult& result) {

				QLOG_APP(L"搜索聊天记录 结果 msg callback {0}, {1}, {2}") << code << query_id << result.msglogs_.size();

				std::vector<nim::IMMessage> vec;
				for each (auto msg in result.msglogs_)
				{
					if (msg.local_talk_id_ != query_id)
					{
						continue;
					}
					size_t len = vec.size();
					if (len > 0)
					{		
						size_t i = 0;
						for (i = 0; i < len; i++)
						{
							if (vec[i].timetag_ == msg.timetag_)
							{
								break;
							}
							if (vec[i].timetag_ < msg.timetag_)
							{
								vec.insert(vec.begin() + i, msg);
								break;
							}
						}
						if (i == len)
						{
							vec.push_back(msg);
						}
					}
					else
					{
						vec.push_back(msg);
					}		
				}
				if (vec.size() > 0)
				{
					setMessageListData(query_id, vec);
				}				
			});

			
			//nim::MsgLog::QueryMsgAsync("62170059", nim::kNIMSessionTypeP2P, 80, 1614689845, cb, "");
			
			/*nim::MsgLog::QueryMsgByOptionsAsyncParam param;
			param.ids_ = ids;
			param.query_range_ = nim::NIMMsgLogQueryRange::kNIMMsgLogQueryRangeAllP2P;
			param.limit_count_ = 50;
			param.reverse_ = false;
			param.from_time_ = 0;
			param.end_time_ = 1614526767;
			param.msg_type_ = nim::NIMMessageType::kNIMMessageTypeText;
			param.search_content_ = search_key;
			nim::MsgLog::QueryMsgByOptionsAsyncEx(param, cb);*/

			//nim::MsgLog::QueryMsgByOptionsAsyncEx(param, nbase::Bind(&SearchMsg::OnQueryMsgCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));
			std::list<std::string> ids;
			for (auto it : session_list_sort_data_)
			{	
				ids.push_back(it->id_);
				nim::MsgLog::QueryMsgByOptionsAsync(nim::NIMMsgLogQueryRange::kNIMMsgLogQueryRangeAllP2P,
					ids,
					100,
					0,
					1000 * nbase::Time::Now().ToTimeT(),
					"",
					false,
					nim::NIMMessageType::kNIMMessageTypeText,
					search_key,
					cb);
				ids.clear();
			}
			

			//nim::MsgLog::QueryMsgByKeywordOnlineParam param;
			//param.id_ = "62170059";
			//param.from_time_ = 0;
			//param.end_time_ = 1614689845;
			//param.limit_count_ = 20;
			//param.reverse_ = false;
			//param.to_type_ = nim::kNIMSessionTypeP2P;
			//param.keyword_ = search_key;
			//nim::MsgLog::QueryMsgByKeywordOnlineAsync(param, nbase::Bind(&SearchMsg::OnQueryMsgCallback, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4));


			/*FindControl(L"no_search_result_tip")->SetVisible(search_result_list_->GetCount() == 0);
			if (search_result_list_->GetCount() > 0)
				search_result_list_->SelectItem(0);*/
		}
		return true;
	}

	void SearchMsg::OnQueryMsgCallback(nim::NIMResCode code, const std::string& query_id, nim::NIMSessionType query_type, const nim::QueryMsglogResult& result)
	{
		QLOG_APP(L"OnQueryMsgCallback 搜索聊天记录 结果 ======= {0}") << code;

		std::vector<nim::IMMessage> vec;
		for each (auto msg in result.msglogs_)
		{
			FillSearchResultList(msg);
		}	
	}

	bool SearchMsg::OnClearInputBtnClicked(ui::EventArgs* param)
	{
		search_edit_->SetText(L"");
		return true;
	}
	bool SearchMsg::OnReturnEventsClick(ui::EventArgs* param)
	{
		if (param->Type == ui::kEventReturn)
		{
			nim_comp::FriendItem* item = dynamic_cast<nim_comp::FriendItem*>(param->pSender);
			assert(item);
			if (item)
			{
				search_edit_->SetText(L"");
				nim_comp::SessionManager::GetInstance()->OpenSessionBox(item->GetUTF8DataID(), item->GetFriendItemType() == kFriendItemTypeTeam ? nim::kNIMSessionTypeTeam : nim::kNIMSessionTypeP2P);
			}
		}

		return true;
	}

	bool SearchMsg::Notify(ui::EventArgs* msg)
	{
		return true;
	}

	void SearchMsg::OnWndSizeMax(bool max)
	{
		/*if (btn_max_restore_)
			btn_max_restore_->SetClass(max ? L"btn_wnd_restore" : L"btn_wnd_max");*/
	}

	LRESULT SearchMsg::HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		if (uMsg == WM_SIZE)
		{
			if (wParam == SIZE_RESTORED)
				OnWndSizeMax(false);
			else if (wParam == SIZE_MAXIMIZED)
				OnWndSizeMax(true);
		}
		auto ret = __super::HandleMessage(uMsg, wParam, lParam);
		/*if (uMsg == WM_CREATE)
		{
			shadow_wnd_ = new ShadowWnd();
			shadow_wnd_->Create(GetHWND(), shadow_wnd_->GetWindowClassName().c_str(), WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX, 0);
			SetWindowLong(shadow_wnd_->GetHWND(), GWL_EXSTYLE, GetWindowLong(shadow_wnd_->GetHWND(), GWL_EXSTYLE) | WS_EX_TRANSPARENT);
			shadow_wnd_->ShowWindow(true, false);
		}
		if (uMsg == WM_MOVE || uMsg == WM_SIZE)
		{
			if (shadow_wnd_ != nullptr)
			{
				if (shadow_wnd_->GetHWND() != NULL && ::IsWindow(shadow_wnd_->GetHWND()))
				{
					int shadow_corner = 14;
					auto pos = this->GetPos(true);
					shadow_corner = DpiManager::GetInstance()->ScaleInt(shadow_corner);
					pos.Inflate(shadow_corner, shadow_corner);
					shadow_wnd_->SetPos(pos, false, SWP_NOZORDER | SWP_NOACTIVATE, NULL, false);
				}
			}
		}*/
		return ret;
	}

	bool SearchMsg::OnClicked(ui::EventArgs* msg)
	{
		std::wstring name = msg->pSender->GetName();
		if (name == L"btn_wnd_close")
		{
			SendMessage(WM_SYSCOMMAND, SC_MINIMIZE);
			::ShowWindow(m_hWnd, SW_HIDE);
		}
		else if (name == L"btn_max_restore")
		{
			DWORD style = GetWindowLong(m_hWnd, GWL_STYLE);
			if (style & WS_MAXIMIZE)
				::ShowWindow(m_hWnd, SW_RESTORE);
			else
				::ShowWindow(m_hWnd, SW_MAXIMIZE);
		}
		return true;
	}

	void SearchMsg::FillSearchResultList(nim::IMMessage msg)
	{
		nim::SessionData session_data;
		session_data.id_ = msg.local_talk_id_;
		session_data.msg_attach_ = msg.attach_;
		session_data.msg_content_ = msg.content_;
		session_data.msg_id_ = msg.client_msg_id_;
		session_data.msg_type_ = msg.type_;
		session_data.msg_status_ = msg.status_;
		session_data.msg_sub_status_ = msg.sub_status_;
		session_data.msg_sender_accid_ = msg.sender_accid_;
		session_data.msg_timetag_ = msg.timetag_;
		session_data.last_updated_msg_ = true;

		SearchMsgItem* item = new SearchMsgItem;
		std::wstring session_item_xml = L"main/session_item.xml";
		if (ui::GlobalManager::GetLanguageSetting().m_enumType == ui::LanguageType::Simplified_Chinese)
			session_item_xml = L"main/session_item.xml";
		if (ui::GlobalManager::GetLanguageSetting().m_enumType == ui::LanguageType::American_English)
			session_item_xml = L"main/session_item_en.xml";
		GlobalManager::FillBoxWithCache(item, session_item_xml);
		item->InitCtrl();
		item->DetachEvent(ui::EventType::kEventAll);
		item->InitMsg(std::make_shared<nim::SessionData>(session_data));

		search_result_list_->Add(item);
	}

	void SearchMsg::FillSearchResultList(nim::SessionData session_data, const std::vector<nim::IMMessage> &msg, bool show)
	{	
		SearchMsgItem* item = new SearchMsgItem;
		std::wstring session_item_xml = L"main/session_item.xml";
		if (ui::GlobalManager::GetLanguageSetting().m_enumType == ui::LanguageType::Simplified_Chinese)
			session_item_xml = L"main/session_item.xml";
		if (ui::GlobalManager::GetLanguageSetting().m_enumType == ui::LanguageType::American_English)
			session_item_xml = L"main/session_item_en.xml";
		GlobalManager::FillBoxWithCache(item, session_item_xml);
		item->InitCtrl();
		item->DetachEvent(ui::EventType::kEventAll);
		item->InitMsg(std::make_shared<nim::SessionData>(session_data));
		item->AttachListBox(msg_list_, msg, show);

		search_result_list_->Add(item);
	}

	void SearchMsg::setSessionList(std::vector< std::shared_ptr<nim::SessionData>> session_list_sort_data)
	{
		session_list_sort_data_ = session_list_sort_data;
	}

	void SearchMsg::setMessageListData(const std::string& msg_id, std::vector<nim::IMMessage> vec)
	{
		auto it = message_list_sort_data_.find(msg_id);
		if (it != message_list_sort_data_.end())
		{
			size_t msgLen = it->second.size();
			size_t len = vec.size();
			size_t i = 0;
			for (i = 0; i < len; i++)
			{
				size_t j = 0;
				for (j = 0; j < msgLen; j++)
				{
					if (it->second[j].timetag_ == vec[i].timetag_)
					{
						break;
					}
					if (it->second[j].timetag_ < vec[i].timetag_)
					{
						it->second.insert(it->second.begin() + j, vec[i]);
						break;
					}
				}
				if (j == msgLen)
				{
					it->second.push_back(vec[i]);
				}
			}			
		}
		else
		{
			message_list_sort_data_[msg_id] = vec;
		}

		search_result_list_->RemoveAll();
		bool show = true;
		for (auto it : message_list_sort_data_)
		{
			for (auto session_item : session_list_sort_data_)
			{
				if (session_item->id_ == it.first)
				{
					FillSearchResultList(*session_item, it.second, show);
					if (show) {
						show = false;
					}
					break;
				}
			}
		}
		if (search_result_list_->GetCount() > 0)
		{
			search_result_list_->SetVisible(true);
		}
	}

	/*void SearchMsg::ReloadEvents()
	{ 
	}*/


}