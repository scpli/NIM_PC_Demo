#pragma once
namespace nim_comp
{
	class MsgShortCutAddForm : public WindowEx
	{
	public:
		MsgShortCutAddForm();
		~MsgShortCutAddForm();

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
	private:
		void InitSubCategoryCombo();
		void OnSubmitMsg(); 

		ui::RichEdit* content;
		ui::RichEdit* keyWord;
		ui::Combo* subCategoryCombo;

		Json::Value subCategoryList;

		ui::Label* lblErr;
	public:
		//typedef std::function<void(std::string)> SelectedMsgItemCallback;
		static const LPCTSTR kClassName;

	};
}
