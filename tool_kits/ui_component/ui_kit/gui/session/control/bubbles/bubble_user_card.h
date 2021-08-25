#pragma once
#include "bubble_item.h"

namespace nim_comp
{
	class MsgBubbleUserCard : public MsgBubbleItem
	{
		public:
			/**
			* ��ʼ���ؼ��ڲ�ָ��
			* @param[in] bubble_right �Ƿ���ʾ���Ҳ�
			* @return void �޷���ֵ
			*/
			virtual void InitControl(bool bubble_right);

			/**
			* ��ʼ���ؼ����
			* @param[in] msg ��Ϣ��Ϣ�ṹ��
			* @return void �޷���ֵ
			*/
			virtual void InitInfo(const nim::IMMessage &msg);

		private:
			/**
			* ��Ӧ����Ϣ����һ���Ϣ�������˵�
			* @param[in] param �������Ĳ˵���������Ϣ
			* @return bool ����ֵtrue: �������ݿؼ���Ϣ�� false: ֹͣ���ݿؼ���Ϣ
			*/
			bool OnMenu(ui::EventArgs* arg);

		private:
			ui::Box*	msg_sticker_;

	};
}