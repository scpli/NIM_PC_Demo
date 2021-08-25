#include "stdafx.h"
#include "resource.h"
#include "msg_shortcut.h" 
#include "export/nim_ui_all.h"

using namespace ui;

namespace nim_comp
{
	const LPCTSTR MsgShortCutForm::kClassName = L"MsgShortCutForm"; 
	const LPCTSTR MsgShortCutForm::msgTypePublic = L"msg_public";
	const LPCTSTR MsgShortCutForm::msgTypePrivate = L"msg_private";
	const LPCTSTR MsgShortCutForm::msgTypeMand = L"msg_mand";

	const LPCTSTR MsgShortCutForm::bkColorSelected = L"bk_opt_selected";
	const LPCTSTR MsgShortCutForm::bkColorNormal = L"bk_opt_normal";

	MsgShortCutForm::MsgShortCutForm(const SelectedMsgItemCallback& completedCallback): completedCallback_(completedCallback)
	{
		is_closing_ = false;
	}

	MsgShortCutForm::~MsgShortCutForm()
	{
	}

	std::wstring MsgShortCutForm::GetSkinFolder()
	{
		return L"session";
	}

	std::wstring MsgShortCutForm::GetSkinFile()
	{
		return L"msg_shortcut.xml";
	}

	std::wstring MsgShortCutForm::GetWindowClassName() const
	{
		return kClassName;
	}

	std::wstring MsgShortCutForm::GetWindowId() const
	{
		return window_id_;
	}

	void MsgShortCutForm::SetWindowId(std::wstring window_id)
	{
		window_id_ = window_id;
	}

	UINT MsgShortCutForm::GetClassStyle() const
	{
		return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
	}

	void MsgShortCutForm::InitWindow()
	{
		if (nim_ui::UserConfig::GetInstance()->GetDefaultIcon() > 0)
		{
			SetIcon(nim_ui::UserConfig::GetInstance()->GetDefaultIcon());
		}

		// m_pRoot->AttachBubbledEvent(ui::kEventAll, nbase::Bind(&MsgShortCutForm::Notify, this, std::placeholders::_1));
		m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&MsgShortCutForm::OnClicked, this, std::placeholders::_1));

		optBoxPublic = (OptionBox*)FindControl(msgTypePublic);
		optBoxPublic->SetBkColor(bkColorSelected);
		optBoxPublic->Selected(true, true);

		optBoxPrivate = (OptionBox*)FindControl(msgTypePrivate);
		optBoxMand = (OptionBox*)FindControl(msgTypeMand);
		 

		boxSubcategoryShow = (HBox*)FindControl(L"box_subcategory_show");
		boxSubcategoryShow->SetVisible(false);

		boxMsgPrivate = (HBox*)FindControl(L"box_modify_private");
		boxMsgPrivate->SetVisible(false);

		boxAddSubcategory = (HBox*)FindControl(L"box_subcategory_add");
		boxAddSubcategory->SetVisible(false);


		richEditCategoryName = (RichEdit*)FindControl(L"categoryName");
		richEditCategoryName->Clear();

		btnAddMsgSubCategory = (Button*)FindControl(L"btnAddMsgSubCategory");

		this->InitMsgList();
	}
	bool MsgShortCutForm::Notify(ui::EventArgs* msg)
	{
		QLOG_PRO(L"Notify name ={0}") << msg->Type;

		return true;
	}
	bool MsgShortCutForm::OnClicked(ui::EventArgs* msg)
	{
		std::wstring name = msg->pSender->GetName();

		

		QLOG_PRO(L"btn name ={0} msg->Type ={1}") << name << msg->Type;

		if (!optBoxPublic->IsSelected() && optBoxPublic->GetName() == name)
		{
			LoadMsgPublic(name);
		}
		else if (!optBoxPrivate->IsSelected() && optBoxPrivate->GetName() == name) {
			LoadMsgPrivate(name);
		}
		else if (!optBoxMand->IsSelected() && optBoxMand->GetName() == name) {
			LoadMsgMand(name);
		} 
		else if (name == L"btnAddMsgPrivate")
		{
			onMsgAdd();
		}
		else if (name == btnAddMsgSubCategory->GetName())
		{
			this->boxAddSubcategory->SetVisible(!this->boxAddSubcategory->IsVisible());
		}
		else if (name == L"deleteMsg")
		{
			delShortcutId_ = msg->pSender->GetDataID();

			MsgboxCallback cb = nbase::Bind(&MsgShortCutForm::OnMsgItemDelete, this, std::placeholders::_1);
			ShowMsgBox(this->GetHWND(), cb, L"STRID_SESSION_SHORTCUT_MSG_DELETE", true, L"STRING_TIPS", true, L"STRING_OK", true, L"STRING_NO", true);
		}
		else if (name == L"btnRefresh")
		{
			this->InitMsgList();
		}
		else if (name == L"subCategory")
		{
			// filter sub category
			filterPrivateSubCategoryId_ = msg->pSender->GetDataID();

			QLOG_PRO(L"filterSubCategoryId_ {0}") << filterPrivateSubCategoryId_;
			LoadMsgItem(L"msg_private", msgPrivate_, true, filterPrivateSubCategoryId_);

			this->LoadSubCategory(filterPrivateSubCategoryId_);
		}
		else if (name == L"btn_save_subcategory") 
		{
			auto task = ToWeakCallback([this](int res, const Json::Value& resList, const std::string& err_msg)
			{
				QLOG_PRO(L"InvokeAddSubCategory callback");
				if (res == 200)
				{
					this->boxAddSubcategory->SetVisible(false);
					this->InitMsgList();					
				}
			});

			std::string  name = richEditCategoryName->GetUTF8Text();

			app_sdk::AppSDKInterface::GetInstance()->InvokeAddSubCategory(LoginManager::GetInstance()->GetAccount(), "private", name,  task);
		}
		else if (name == L"btn_cancel_subcategory") {
			this->boxAddSubcategory->SetVisible(false);
		}
		return true;
	}

	void MsgShortCutForm::OnMsgItemDelete(MsgBoxRet ret)
	{
		if (ret == MB_YES)
		{ 

			auto task = ToWeakCallback([this](int res, const std::string& err_msg)
			{
				if (res == 200)
				{
					this->InitMsgList();
				}
			});
			app_sdk::AppSDKInterface::GetInstance()->InvokeDelMsgShotCutByAccid(LoginManager::GetInstance()->GetAccount(), delShortcutId_, task);
		}		
	}
	void MsgShortCutForm::LoadSubCategory(std::wstring subCategoryId)
	{
		this->boxSubcategoryShow->RemoveAll();

		auto create_elem = [](std::wstring subCategoryId, string name, string id)
		{
			ui::Button* elem = new ui::Button;
			//elem->SetClass(L"listitem"); 
			elem->SetFixedHeight(25);
			elem->SetMargin(ui::UiRect(0, 0, 10, 0));
			elem->SetUTF8Text(name);
			elem->SetName(L"subCategory");
			elem->SetUTF8DataID(id);
			elem->SetFont(2);
			elem->SetStateTextColor(ui::kControlStateNormal, L"profile_account");

			if (subCategoryId == nbase::UTF8ToUTF16(id))
			{
				//elem->SetState(ui::ControlStateType::kControlStatePushed);
				elem->SetStateTextColor(ui::ControlStateType::kControlStateNormal, L"link_blue2"); 
			}

			return elem;
		};
		 

		int i = 0;

		for (auto it : this->resSubcategoryList)
		{
			std::string name = it["name"].asString();
			std::string id = it["categoryId"].asString();

			this->boxSubcategoryShow->Add(create_elem(subCategoryId, name, id));

			i++;
		}

		if (i >= 4)
		{
			this->btnAddMsgSubCategory->SetVisible(false);
		}
		else
		{
			this->btnAddMsgSubCategory->SetVisible(true);
		}

	}
	void MsgShortCutForm::InitMsgList()
	{ 
		
		//this->boxSubcategoryShow->RemoveAll(); 

		auto taskCategory = ToWeakCallback([this](int res, const Json::Value& resList, const std::string& err_msg)
		{
			QLOG_PRO(L"InvokeSubCategoryReq callback");
			if (res == 200 && resList.isArray())
			{
				this->resSubcategoryList = resList;
				this->LoadSubCategory(L"0");
				/*
				auto create_elem = [](string name, string id)
				{
					ui::Button* elem = new ui::Button;
					//elem->SetClass(L"listitem"); 
					elem->SetFixedHeight(25);
					elem->SetMargin(ui::UiRect(0, 0, 10, 0));
					elem->SetUTF8Text(name);
					elem->SetName(L"subCategory");
					elem->SetUTF8DataID(id);
					elem->SetFont(2);
					elem->SetStateTextColor(ui::kControlStateNormal, L"profile_account");

					 
					elem->SetStateColor(ui::ControlStateType::kControlStatePushed, L"dark2color");
					
					return elem;
				};

				this->resSubcategoryList = resList;

				int i = 0;

				for (auto it : resList)
				{
					std::string name = it["name"].asString();
					std::string id = it["categoryId"].asString(); 

					this->boxSubcategoryShow->Add(create_elem(name,id));

					i++;
				}

				if (i >= 4)
				{
					this->btnAddMsgSubCategory->SetVisible(false);
				}
				else
				{
					this->btnAddMsgSubCategory->SetVisible(true);
				}
				*/
			}
		});

		app_sdk::AppSDKInterface::GetInstance()->InvokeSubCategoryReq(LoginManager::GetInstance()->GetAccount(), "private", taskCategory);


		//2021
		auto task = ToWeakCallback([this](int res, const Json::Value& msg_shortcut_list, const std::string& err_msg)
		{
			if (res == 200)
			{
				if (msgPublic_.isArray())
					msgPublic_.clear();
				
				if (msgPrivate_.isArray())
					msgPrivate_.clear();

				if (msgMand_.isArray())
					msgMand_.clear();



				for (auto it : msg_shortcut_list)
				{
					std::string category = it["category"].asString();

					// 公共话术
					if (category == "public")
					{
						msgPublic_ = it["shortcutList"];
					}
					else if (category == "private")
					{
						msgPrivate_ = it["shortcutList"];
					}
					else if (category == "mand")
					{
						msgMand_ = it["shortcutList"];
					}
				}

				LoadMsgItem(L"msg_public", msgPublic_, false);
				LoadMsgItem(L"msg_private", msgPrivate_, true);
				LoadMsgItem(L"msg_mand", msgMand_, false);
			}
		});
		app_sdk::AppSDKInterface::GetInstance()->InvokeGetMsgShotCutByAccid(LoginManager::GetInstance()->GetAccount(), task);
	}
	void MsgShortCutForm::OnMsgAddSubCategory()
	{
		QLOG_PRO(L"OnMsgAddSubCategory ====================");
	}
	void MsgShortCutForm::onMsgAdd()
	{
		//MsgShortCutAddFrom 

		MsgShortCutAddForm* f = WindowsManager::SingletonShow<MsgShortCutAddForm>(MsgShortCutAddForm::kClassName);
		f->ShowWindow();
	}
	void MsgShortCutForm::LoadMsgPublic(std::wstring name)
	{
		QLOG_PRO(L"LoadMsgPublic ====================");
		optBoxPublic->Selected(true, true);
		optBoxPublic->SetBkColor(bkColorSelected); 

		optBoxPrivate->Selected(false, false);
		optBoxPrivate->SetBkColor(bkColorNormal);

		optBoxMand->Selected(false, false);
		optBoxMand->SetBkColor(bkColorNormal);

		this->boxMsgPrivate->SetVisible(false);
		this->boxAddSubcategory->SetVisible(false);
		this->boxSubcategoryShow->SetVisible(false); 
	}
	void MsgShortCutForm::LoadMsgPrivate(std::wstring name)
	{
		QLOG_PRO(L"LoadMsgPrice +++++++++++++++++++++");
		optBoxPublic->Selected(false, false);
		optBoxPublic->SetBkColor(bkColorNormal); 
		optBoxPublic->SetBkColor(L"");

		optBoxPrivate->Selected(true, true);
		optBoxPrivate->SetBkColor(bkColorSelected); 

		optBoxMand->Selected(false, false);
		optBoxMand->SetBkColor(bkColorNormal);

		this->boxMsgPrivate->SetVisible(true);
		this->boxAddSubcategory->SetVisible(false);
		this->boxSubcategoryShow->SetVisible(true); 
	}
	void MsgShortCutForm::LoadMsgMand(std::wstring name)
	{
		QLOG_PRO(L"LoadMsgMand ------------------------");
		optBoxPublic->Selected(false, false);
		optBoxPublic->SetBkColor(bkColorNormal);

		optBoxPrivate->Selected(false, false);
		optBoxPrivate->SetBkColor(bkColorNormal);

		optBoxMand->Selected(true, true);
		optBoxMand->SetBkColor(bkColorSelected);

		this->boxMsgPrivate->SetVisible(false);
		this->boxAddSubcategory->SetVisible(false);
		this->boxSubcategoryShow->SetVisible(false);
	}
	void MsgShortCutForm::LoadMsgItem(std::wstring name, Json::Value msgList, bool isShowDeleteBtn, std::wstring subCategoryId)
	{  
		std::wstring box_id = nbase::StringPrintf(L"%s_list", name.c_str());
		ui::TileBox* listBox = (TileBox*)FindControl(box_id);
		listBox->RemoveAll(); 

		if (!msgList.isArray())
			return;


		static std::wstring dir = QPath::GetAppPath();
		static std::wstring iconTxtPath = dir + L"resources\\themes\\default\\plugins\\msg_shortcut\\text.png";
		static std::wstring iconImgPath = dir + L"resources\\themes\\default\\plugins\\msg_shortcut\\img.png";


		auto task = ToWeakCallback([this](ui::TileBox* listBox, bool isShowDeleteBtn, const Json::Value& it) {

			ButtonBox* box = new ButtonBox;
			GlobalManager::FillBoxWithCache(box, L"plugins/msg_shortcut/msg_item.xml");
			
			listBox->Add(box);

			std::string content = it["content"].asString();
			std::string keyword = it["keyWorks"].asString();
			std::string seq = it["serialNumber"].asString();
			std::string shortcutId = it["shortcutId"].asString();

			box->SetKeyboardEnabled(false);
			box->AttachClick(nbase::Bind(&MsgShortCutForm::OnMsgItemClicked, this, std::placeholders::_1, content));


			Control* c = box->FindSubControl(L"icon");
			c->SetBkImage(iconTxtPath);

			Label* lblSeq = (Label*)box->FindSubControl(L"seq");
			lblSeq->SetUTF8Text(seq);

			Label* lblKeyword = (Label*)box->FindSubControl(L"keyword");
			lblKeyword->SetUTF8Text(keyword);


			Label* lblContent = (Label*)box->FindSubControl(L"content");
			lblContent->SetUTF8Text(content);


			Button* btnDel = (Button*)box->FindSubControl(L"deleteMsg");
			btnDel->SetDataID(nbase::UTF8ToUTF16(shortcutId));
			btnDel->SetVisible(isShowDeleteBtn);
		});

		for (auto it : msgList)
		{
			if (subCategoryId != L"0" && it.isMember("subCategoryId"))
			{
				std::wstring subId = nbase::UTF8ToUTF16(it["subCategoryId"].asString());
				if (subCategoryId == subId)
				{
					task(listBox, isShowDeleteBtn, it);
				}
			}
			else {
				task(listBox, isShowDeleteBtn, it);
			}
		}
	}
	bool MsgShortCutForm::OnMsgItemClicked(ui::EventArgs* param, const std::string msgContent)
	{
		if (is_closing_)
			return false;  
		completedCallback_(msgContent);
		return false;
	}
}