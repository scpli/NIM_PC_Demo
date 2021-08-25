#pragma once
namespace nim_comp
{
	class MsgShortCutAddForm : public WindowEx
	{
	public:
		MsgShortCutAddForm();
		~MsgShortCutAddForm();

		//覆盖虚函数
		virtual std::wstring GetSkinFolder() override;
		virtual std::wstring GetSkinFile() override;
		virtual std::wstring GetWindowClassName() const override;
		virtual std::wstring GetWindowId() const override;
		virtual UINT GetClassStyle() const override;
		/**
		* 窗口初始化函数
		* @return void	无返回值
		*/
		virtual void InitWindow() override;

		/**
		* 处理所有控件单击消息
		* @param[in] msg 消息的相关信息
		* @return bool true 继续传递控件消息，false 停止传递控件消息
		*/
		virtual bool OnClicked(ui::EventArgs* msg);
		/**
		* 处理所有控件的所有消息
		* @param[in] msg 消息的相关信息
		* @return bool true 继续传递控件消息，false 停止传递控件消息
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
