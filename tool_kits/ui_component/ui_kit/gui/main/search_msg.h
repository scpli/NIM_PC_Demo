#pragma once
#include "gui/main/control/event_item.h"
#include "module/service/photo_service.h"

namespace nim_comp
{
	class SearchMsg : public WindowEx
	{
	public:
		SearchMsg();
		~SearchMsg();

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

		virtual LRESULT HandleMessage(UINT uMsg, WPARAM wParam, LPARAM lParam) override;
		 

		/**
		* �������пؼ�������Ϣ
		* @param[in] msg ��Ϣ�������Ϣ
		* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
		*/
		virtual bool OnClicked(ui::EventArgs* msg);

		void setSessionList(std::vector< std::shared_ptr<nim::SessionData>> session_list_sort_data);

	private:
		/**
		* �������пؼ���������Ϣ
		* @param[in] msg ��Ϣ�������Ϣ
		* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
		*/
		bool Notify(ui::EventArgs* msg);

		/**
		* ��ʼ��������
		* @return void
		*/
		void InitSearchBar();
		/**
		* �������������ݸı����Ϣ
		* @param[in] msg ��Ϣ�������Ϣ
		* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
		*/
		bool SearchEditChange(ui::EventArgs* param);

		/**
		* ����������������ť�ĵ�����Ϣ
		* @param[in] msg ��Ϣ�������Ϣ
		* @return bool true �������ݿؼ���Ϣ��false ֹͣ���ݿؼ���Ϣ
		*/
		bool OnClearInputBtnClicked(ui::EventArgs* param);

		bool OnReturnEventsClick(ui::EventArgs* param);
		void OnWndSizeMax(bool max);

		void OnQueryMsgCallback(nim::NIMResCode code, const std::string& query_id, nim::NIMSessionType query_type, const nim::QueryMsglogResult& result);

		void FillSearchResultList(nim::IMMessage msg);
		void FillSearchResultList(nim::SessionData session_data, const std::vector<nim::IMMessage> &msg, bool show);
		void setMessageListData(const std::string& msg_id, std::vector<nim::IMMessage> vec);


	public:
		static const LPCTSTR kClassName;
	private:
		ui::Button*		btn_recycle_;
		ui::ListBox*	event_list_;

		ui::Button* btn_max_restore_;

		ui::RichEdit*	search_edit_;
		ui::Button*		btn_clear_input_;
		ui::ListBox*	search_result_list_;
		ui::ButtonBox* search_bar_;
		ui::ListBox*	msg_list_;

		std::vector< std::shared_ptr<nim::SessionData>> session_list_sort_data_;
		std::map < std::string, std::vector<nim::IMMessage>> message_list_sort_data_;

	};

}