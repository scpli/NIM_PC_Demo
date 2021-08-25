#include "stdafx.h"
#include "bubble_image.h"
#include "image_view/src/image_view_manager.h"
#include "util/user.h"
#include "shared/zoom_image.h"

using namespace ui;

namespace nim_comp
{
void MsgBubbleImage::InitControl(bool bubble_right)
{
	__super::InitControl(bubble_right);

	msg_image_ = new ButtonBox;
	if(bubble_right)
		GlobalManager::FillBoxWithCache(msg_image_, L"session/image_right.xml");
	else
		GlobalManager::FillBoxWithCache(msg_image_, L"session/image_left.xml");
	bubble_box_->Add( msg_image_ );

	image_ = msg_image_->FindSubControl(L"image");

	msg_image_->AttachClick(nbase::Bind(&MsgBubbleImage::OnClicked, this, std::placeholders::_1));
	msg_image_->AttachMenu(nbase::Bind(&MsgBubbleImage::OnMenu, this, std::placeholders::_1));
}

void MsgBubbleImage::InitInfo(const nim::IMMessage &msg)
{
	__super::InitInfo(msg);

	SetCanView(false);
	InitResPath(msg_);

	if (nbase::FilePathIsExist(thumb_, false)) //thumb_图片存在
	{
		if (CheckThumbImage())
		{
			image_->SetFixedWidth(thumb_image_width_, false); 
			image_->SetFixedHeight(thumb_image_height_);
			image_->SetBkImage(thumb_);
			if (!my_msg_ || !(msg_.status_ == nim::kNIMMsgLogStatusSendFailed))
				SetLoadStatus(RS_LOAD_OK);
			if (my_msg_ && msg_.status_ == nim::kNIMMsgLogStatusSending)
				SetMsgStatus(nim::kNIMMsgLogStatusSending);
			SetCanView(true);
		}
		else //图片有错误
			SetLoadStatus(RS_LOAD_NO);
	}
	else if (nbase::FilePathIsExist(path_, false)) //缩略图不存在，而原图存在
	{
		DoZoom();
	}
	else //thumb_图片和原图都不存在
	{
		if (msg_.rescode_ == 0) //读取消息历史
		{
			SetLoadStatus(RS_LOADING);
		}
		else //接收
		{
			if (msg_.rescode_ == nim::kNIMResSuccess
				|| msg_.rescode_ == nim::kNIMLocalResExist)
				SetLoadStatus(RS_LOADING);
			else if (msg_.rescode_ == nim::kNIMLocalResParameterError)
				SetLoadStatus(RS_LOAD_NO);
			else
			{
				SetLoadStatus(RS_LOAD_NO);
				QLOG_WAR(L"unknown receive msg code {0}") << msg_.rescode_;
			}
		}
	}
}

bool MsgBubbleImage::OnClicked( ui::EventArgs* arg )
{
	if( nbase::FilePathIsExist(path_, false) )
	{
		ImageViewManager::GetInstance()->StartViewPic(path_, nbase::UTF8ToUTF16(msg_.client_msg_id_), true);
	}
	else //原图不存在
	{
		ImageViewManager::GetInstance()->StartViewPic(path_, nbase::UTF8ToUTF16(msg_.client_msg_id_), true);
		nim::NOS::DownloadMediaCallback cb = [](nim::NIMResCode res_code, const std::string& file_path, const std::string&, const std::string&)
		{
			if (res_code == nim::kNIMResSuccess)
			{
				ImageViewManager* image_view_mgr = ImageViewManager::GetInstance();
				if (image_view_mgr)
					image_view_mgr->OnImageReady(nbase::UTF8ToUTF16(file_path), true);
			}
		};
		nim::NOS::FetchMedia(msg_, cb, nim::NOS::ProgressCallback());
	}
	SetCurImageMsg();
	QueryImageMsg(true);
	QueryImageMsg(false);
	return true;
}

void MsgBubbleImage::SetCanView(bool can)
{
	msg_image_->SetEnabled(can);
}

void MsgBubbleImage::OnDownloadCallback( bool success, const std::string& file_path )
{
	if (image_checked_)
		return;

	ASSERT(!file_path.empty());
	std::wstring wpath = nbase::UTF8ToUTF16(file_path);
	if (wpath == thumb_) //下载的是缩略图
	{
		if (success)
		{
			ListBox* lbx = dynamic_cast<ListBox*>(this->GetParent());
			ASSERT(lbx);
			bool at_end = lbx->IsAtEnd();

			SetLoadStatus(RS_LOAD_OK);
			if (CheckThumbImage())
			{
				image_->SetFixedWidth(thumb_image_width_, false); 
				image_->SetFixedHeight(thumb_image_height_);
				image_->SetBkImage(thumb_);
			}
			SetCanView(true);
			
			if (at_end)
				lbx->EndDown(true, false);
		}
		else
		{
			SetLoadStatus(RS_LOAD_NO);
		}
	}
	else if (wpath == path_) //下载的是原图
	{
		if (success)
		{
			ListBox* lbx = dynamic_cast<ListBox*>(this->GetParent());
			ASSERT(lbx);
			bool at_end = lbx->IsAtEnd();

			SetLoadStatus(RS_LOAD_OK);

			DoZoom();

			if (at_end)
				lbx->EndDown(true, false);
		}
		else
		{
			SetLoadStatus(RS_LOAD_NO);
		}
	}
}

void MsgBubbleImage::DoZoom()
{
	if (image_checked_)
		return;

	if( !nbase::FilePathIsExist(path_, false) )
	{
		InitResPath(msg_);
		if (!nbase::FilePathIsExist(path_, false))
		{
			QLOG_ERR(L"Image not exist: {0}") << path_;
			return;
		}
	}

	const int cx = 270, cy = 180, xf = 78 - 5, yf = 57 - 5;

	std::wstring mime_type = ZoomImage::GetMimeType(path_);
	if( mime_type.empty() )
	{
		QLOG_ERR(L"Image wrong: {0}") <<path_;
		return;
	}
	SetCanView(true);

	SIZE sz = { 0, 0 };
	bool resize = CalculateImageSize(path_, sz, cx, cy);

	image_->SetFixedWidth(sz.cx, false);
	image_->SetFixedHeight(sz.cy);

	if(mime_type == kImageGIF)
	{
		image_->SetBkImage(path_);
	}
	else
	{
		if(resize)
		{
			ZoomImageF(path_, thumb_, sz.cx, sz.cy);
			image_->SetBkImage(thumb_);
		}
		else
		{
			nbase::CopyFile(path_, thumb_);
			image_->SetBkImage(thumb_);
			if(sz.cx < xf && sz.cy < yf)
			{
				image_->SetForeStateImage(kControlStateNormal, L"");
			}
		}
	}
	image_checked_ = true;
}

bool MsgBubbleImage::OnMenu( ui::EventArgs* arg )
{
	bool show_retweet = true;
	if (msg_.type_ == nim::kNIMMessageTypeCustom)
	{
		Json::Value json;
		if (StringToJson(msg_.attach_, json) && json.isObject())
		{
			int sub_type = json["type"].asInt();
			if (sub_type == CustomMsgType_SnapChat)
				show_retweet = false;
		}
	}
	PopupMenu(true, true, show_retweet);
	return false;
}

void MsgBubbleImage::OnMenuCopy()
{  
	bool result = false;
	Gdiplus::Bitmap *gdibmp = Gdiplus::Bitmap::FromFile(nbase::UTF8ToUTF16(msg_.local_res_path_).data());
	if (gdibmp)
	{
		HBITMAP hbitmap;
		gdibmp->GetHBITMAP(0, &hbitmap);
		if (OpenClipboard(NULL))
		{
			EmptyClipboard();
			DIBSECTION ds;
			if (GetObject(hbitmap, sizeof(DIBSECTION), &ds))
			{
				HDC hdc = GetDC(HWND_DESKTOP);
				//create compatible bitmap (get DDB from DIB)
				HBITMAP hbitmap_ddb = CreateDIBitmap(hdc, &ds.dsBmih, CBM_INIT,
					ds.dsBm.bmBits, (BITMAPINFO*)&ds.dsBmih, DIB_RGB_COLORS);
				ReleaseDC(HWND_DESKTOP, hdc);
				SetClipboardData(CF_BITMAP, hbitmap_ddb);
				DeleteObject(hbitmap_ddb);
				result = true;
			}
			CloseClipboard();
		}

		//cleanup:
		DeleteObject(hbitmap);
		delete gdibmp;
	}
}

bool MsgBubbleImage::NeedDownloadResource()
{
	//if( nbase::FilePathIsExist(thumb_, false) )
		return false;
	//else
	//	return true;
}
bool MsgBubbleImage::NeedDownloadResourceEx()
{
	if( nbase::FilePathIsExist(path_, false) )
		return false;
	else
		return true;
}

void MsgBubbleImage::SetCurImageMsg()
{
	ImageViewManager::ImageViewInfoList info_list;
	ImageViewManager::ImageViewInfo image_msg;
	image_msg.id_ = nbase::UTF8ToUTF16(msg_.client_msg_id_);
	image_msg.path_ = nbase::UTF8ToUTF16(msg_.local_res_path_);
	image_msg.url_ = nbase::UTF8ToUTF16(nim::Talk::GetAttachmentPathFromMsg(msg_));
	image_msg.timetag_ = msg_.timetag_;
	info_list.push_back(image_msg);
	ImageViewManager::GetInstance()->SetImageViewList(info_list, nbase::UTF8ToUTF16(msg_.client_msg_id_));
}

void MsgBubbleImage::QueryImageMsg(bool forward)
{
	QLOG_APP(L"query image msg begin: ");

	nim::MsgLog::QueryMsgOnlineAsyncParam param;
	param.id_ = msg_.local_talk_id_;
	param.to_type_ = nim::kNIMSessionTypeP2P;
	param.limit_count_ = 50;		
	param.need_save_to_local_ = true;
	param.auto_download_attachment_ = true;
	param.is_exclusion_type_ = false;
	if (forward)
	{
		param.reverse_ = false;
		param.from_time_ = 0;
		param.end_time_ = msg_.timetag_;
		param.end_msg_id_ = msg_.readonly_server_id_;
	}
	else
	{
		param.reverse_ = true;
		param.from_time_ = msg_.timetag_;
		param.end_time_ = 1000 * nbase::Time::Now().ToTimeT();
	}

	std::vector<nim::NIMMessageType> msg_type_list;
	msg_type_list.push_back(nim::kNIMMessageTypeImage);
	param.msg_type_list_ = msg_type_list;
	nim::MsgLog::QueryMsgCallback cb = nbase::Bind(&MsgBubbleImage::QueryMsgOnlineCb, this, std::placeholders::_1, std::placeholders::_2, std::placeholders::_3, std::placeholders::_4);

	nim::MsgLog::QueryMsgOnlineAsync(param, cb);
}

void MsgBubbleImage::QueryMsgOnlineCb(nim::NIMResCode code, const std::string& id, nim::NIMSessionType type, const nim::QueryMsglogResult& result)
{
	QLOG_APP(L"query online msg end: code={0} id={1} type={2} count={3} source={4}") << code << id << type << result.msglogs_.size() << result.source_;

	if (id != msg_.local_talk_id_)
	{
		QLOG_APP(L"msg not match: local_talk_id={0}") << msg_.local_talk_id_;
		return;
	}		

	if (code == nim::kNIMResSuccess)
	{
		ImageViewManager::ImageViewInfoList info_list;
		for (auto& msg : result.msglogs_)
		{			
			ImageViewManager::ImageViewInfo image_msg;
			image_msg.id_ = nbase::UTF8ToUTF16(msg.client_msg_id_);
			image_msg.path_ = nbase::UTF8ToUTF16(msg.local_res_path_);
			image_msg.url_ = nbase::UTF8ToUTF16(nim::Talk::GetAttachmentPathFromMsg(msg));
			image_msg.timetag_ = msg.timetag_;
			info_list.push_back(image_msg);
		}
		ImageViewManager::GetInstance()->SetImageViewList(info_list, nbase::UTF8ToUTF16(msg_.client_msg_id_));
	}
}
}