#include "stdafx.h"
#include "stdafx.h"
#include "msg_shortcut_add.h"
#include "resource.h" 
#include "export/nim_ui_all.h"

using namespace ui;
 
namespace nim_comp
{
	const LPCTSTR MsgShortCutAddForm::kClassName = L"MsgShortCutAddForm";

	//MsgShortCutAddForm::MsgShortCutAddForm(Json::Value resSubcategoryList): subCategoryList(resSubcategoryList)
	MsgShortCutAddForm::MsgShortCutAddForm()
	{ 
	}

	MsgShortCutAddForm::~MsgShortCutAddForm()
	{
	}

	std::wstring MsgShortCutAddForm::GetSkinFolder()
	{
		return L"session";
	}

	std::wstring MsgShortCutAddForm::GetSkinFile()
	{
		return L"msg_shortcut_add.xml";
	}

	std::wstring MsgShortCutAddForm::GetWindowClassName() const
	{
		return kClassName;
	}

	std::wstring MsgShortCutAddForm::GetWindowId() const
	{
		return kClassName;
	}

	UINT MsgShortCutAddForm::GetClassStyle() const
	{
		return (UI_CLASSSTYLE_FRAME | CS_DBLCLKS);
	}

	void MsgShortCutAddForm::InitWindow()
	{
		QLOG_PRO(L"InitWindow MsgShortCutAddForm ===========");   
		if (nim_ui::UserConfig::GetInstance()->GetDefaultIcon() > 0)
		{
			SetIcon(nim_ui::UserConfig::GetInstance()->GetDefaultIcon());
		}

		m_pRoot->AttachBubbledEvent(ui::kEventClick, nbase::Bind(&MsgShortCutAddForm::OnClicked, this, std::placeholders::_1));

		content = (ui::RichEdit*)FindControl(L"content");
		content->SetFocus();
		content->Clear();

		keyWord = (ui::RichEdit*)FindControl(L"keyword");
		keyWord->Clear();

		subCategoryCombo = (ui::Combo*)FindControl(L"subCategory");

		lblErr = (ui::Label*)FindControl(L"error_tip");
		lblErr->SetVisible(false);

		this->InitSubCategoryCombo();
	
	}
	void MsgShortCutAddForm::InitSubCategoryCombo()
	{
		subCategoryCombo->RemoveAll();

		

		 

		auto taskCategory = ToWeakCallback([this](int res, const Json::Value& resList, const std::string& err_msg)
		{
			QLOG_PRO(L"InvokeSubCategoryReq callback");
			if (res == 200 && resList.isArray())
			{
				  
				auto create_elem = [](std::string name, std::string subCategoryId)
				{
					ui::ListContainerElement* elem = new ui::ListContainerElement;
					elem->SetClass(L"listitem");
					elem->SetFixedHeight(30);
					elem->SetTextPadding(ui::UiRect(10, 1, 1, 1));
					elem->SetText(nbase::UTF8ToUTF16(name));
					elem->SetFont(2);
					elem->SetDataID(nbase::UTF8ToUTF16(subCategoryId));
					elem->SetStateTextColor(ui::kControlStateNormal, L"profile_account");
					return elem;
				};

				for (auto it : resList)
				{
					std::string name = it["name"].asString();
					std::string subCategoryId = it["categoryId"].asString();
					/*std::string seq = it["serialNumber"].asString();*/

					subCategoryCombo->Add(create_elem(name, subCategoryId));
				}
			}
		});

		app_sdk::AppSDKInterface::GetInstance()->InvokeSubCategoryReq(LoginManager::GetInstance()->GetAccount(), "private", taskCategory);

	}
	bool MsgShortCutAddForm::OnClicked(ui::EventArgs* msg)
	{
		std::wstring name = msg->pSender->GetName();
		QLOG_PRO(L"MsgShortCutAddForm::OnClicked btn name ={0} msg->Type ={1}") << name << msg->Type;

		if (L"btnSubmit" == name)
		{
			OnSubmitMsg();
		}
		else if (L"btnCancel" == name) {
			this->Close();
		}
		return true;
	}
	void MsgShortCutAddForm::OnSubmitMsg()
	{
		QLOG_PRO(L"OnSubmitMsg");

		

		std::string category = "private";
		std::string type = "txt";
		std::string keyWorks = nbase::StringTrim(this->keyWord->GetUTF8Text());
		std::string content =  nbase::StringTrim(this->content->GetUTF8Text());

		if (content.empty())
		{
			lblErr->SetText(L"STRID_SESSION_SHORTCUT_MSG_CONTENT");
			lblErr->SetVisible(true);
			return;
		}

		if (keyWorks.empty())
		{
			lblErr->SetText(L"STRID_SESSION_SHORTCUT_MSG_KEYWORD");
			lblErr->SetVisible(true);
			return;
		}


		int idx = subCategoryCombo->GetCurSel();
		if (subCategoryCombo->GetCurSel() == -1)
		{
			lblErr->SetText(L"STRID_SESSION_SHORTCUT_MSG_SUBTYPE");
			lblErr->SetVisible(true);
			return;
		}

		ui::ListContainerElement* subCategory = (ui::ListContainerElement*)subCategoryCombo->GetItemAt(idx);

		QLOG_PRO(L"subCategory index ={0}") << subCategory->GetUTF8DataID();

		//2021
		auto task = ToWeakCallback([this](int res, const Json::Value& msg_shortcut_list, const std::string& err_msg)
		{
			QLOG_PRO(L"OnSubmitMsg pos ===========================");

			if (res == 200)
			{
				this->content->Clear();
				this->keyWord->Clear(); 
				//todo 更新列表
			}
		});
		app_sdk::AppSDKInterface::GetInstance()->InvokeAddMsgShotCut(LoginManager::GetInstance()->GetAccount(), 
																	category, 
																	type,
																	keyWorks,
																	content, 
																	subCategory->GetUTF8DataID(),
																	task);
	}
	bool MsgShortCutAddForm::Notify(ui::EventArgs* msg)
	{
		QLOG_PRO(L"Notify name ={0}") << msg->Type;

		return true;
	}
}