#pragma once
#include "msg_shortcut_add.h"

//typedef std::function<void(std::wstring)> OnSelectEmotion;

namespace nim_comp
{ 
	class MsgShortCutForm : public WindowEx
	{
	public:
		typedef std::function<void(std::string)> SelectedMsgItemCallback;
		static const LPCTSTR kClassName;

	public:
		MsgShortCutForm(const SelectedMsgItemCallback& completedCallback);
		~MsgShortCutForm();

		//�����麯��
		virtual std::wstring GetSkinFolder() override;
		virtual std::wstring GetSkinFile() override;
		virtual std::wstring GetWindowClassName() const override;
		virtual std::wstring GetWindowId() const override;
		virtual UINT GetClassStyle() const override;
		/**
		* ���ڳ�ʼ������
		* @return void	�޷���ֵ
		*/
		virtual void InitWindow() override; 

		/**
		* �������пؼ�������Ϣ
		* @param[in] msg ��Ϣ�������Ϣ
		* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
		*/
		virtual bool OnClicked(ui::EventArgs* msg);
		/**
		* �������пؼ���������Ϣ
		* @param[in] msg ��Ϣ�������Ϣ
		* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
		*/
		virtual bool Notify(ui::EventArgs* msg);

		/**
		* ���ô���id
		* @param[in] window_id ����id
		* @return void
		*/
		void SetWindowId(std::wstring window_id);
	private:
		static const LPCTSTR msgTypePublic;
		static const LPCTSTR msgTypePrivate;
		static const LPCTSTR msgTypeMand;
		static const LPCTSTR bkColorSelected;
		static const LPCTSTR bkColorNormal;

		void LoadMsgPublic(std::wstring name);
		void LoadMsgPrivate(std::wstring name);
		void LoadMsgMand(std::wstring name);
		void LoadMsgItem(std::wstring name, Json::Value msgList, bool isShowDeleteBtn, std::wstring subCategoryId = L"0");

		void LoadSubCategory(std::wstring subCategoryId);

		void InitMsgList();

		//void reloadMsgItem(std::wstring subCategoryId);

		/**
		* ������Ϣ��ť�ĵ�����Ϣ
		* @param[in] param ��Ϣ�������Ϣ
		* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
		*/
		bool OnMsgItemClicked(ui::EventArgs* param, const std::string msgContent);

		void OnMsgItemDelete(MsgBoxRet ret);

		void OnMsgAddSubCategory();
		void onMsgAdd();
	private:
		std::wstring delShortcutId_;
		std::wstring filterPublicSubCategoryId_ = L"0";
		std::wstring filterPrivateSubCategoryId_ = L"0";
		std::wstring filterMandSubCategoryId_ = L"0";


		ui::OptionBox* optBoxPublic;
		ui::OptionBox* optBoxPrivate;
		ui::OptionBox* optBoxMand;


		ui::TileBox* tileBoxMsgPublicList;
		ui::TileBox* tileBoxMsgPrivateList;
		ui::TileBox* tileBoxMsgMandList;

		ui::HBox* boxMsgPrivate;
		ui::HBox* boxAddSubcategory;
		ui::HBox* boxSubcategoryShow;

		ui::Button* btnAddMsgSubCategory;

		ui::RichEdit* richEditCategoryName;

		std::string selectedPublicSubCategoryId;
		std::string selectedPrivateSubCategoryId;
		std::string selectedMandSubCategoryId;

		bool  is_closing_;

		//OnSelectEmotion sel_cb_;
		SelectedMsgItemCallback	completedCallback_;  
		
		Json::Value msgPublic_;
		Json::Value msgPrivate_;
		Json::Value msgMand_;

		Json::Value resSubcategoryList;

		std::wstring window_id_;
	};
}