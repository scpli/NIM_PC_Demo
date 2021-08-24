#include "stdafx.h"
#include "image_view_manager.h"

using namespace nbase;


ImageViewManager::ImageViewManager() 
	: image_view_window_(NULL)
	, message_id_(L"")
{

}

ImageViewManager::~ImageViewManager()
{
	if (image_view_window_)
	{
		image_view_window_ = NULL;
	}
}

bool ImageViewManager::StartViewPic(const std::wstring &path_pic, std::wstring message_id, bool size, bool nosave)
{
	if (!image_view_window_)
	{
		image_view_window_ = new ImageViewForm();
		image_view_window_->Create(NULL, _T(""), WS_OVERLAPPEDWINDOW, 0L);
	}
	message_id_ = message_id;
	image_view_window_->StartViewPic(path_pic, size, nosave);

	if (!IsWindowVisible(image_view_window_->GetHWND()))
	{
		image_view_window_->ShowWindow(true);
		image_view_window_->CenterWindow();
	}
	::BringWindowToTop(image_view_window_->GetHWND());
	::SetForegroundWindow(image_view_window_->GetHWND());
	::SetWindowPos(image_view_window_->GetHWND(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	::SetWindowPos(image_view_window_->GetHWND(), HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
	return true;
}
ImageViewManager::ImageViewSetNoTopMostCB ImageViewManager::StartViewPicEx(const std::wstring &path_pic, std::wstring message_id, bool size, bool nosave , bool topmost )
{
	if (StartViewPic(path_pic, message_id, size, nosave))
	{
		if (topmost)
		{
			::SetWindowPos(image_view_window_->GetHWND(), HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
			auto view_wnd = image_view_window_->GetHWND();
			if (::IsWindow(view_wnd))
			{
				return [view_wnd]() {
					::SetWindowPos(view_wnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE);
				};
			}
		}
	}
	return []() {};
}
bool ImageViewManager::JudgeViewImage(const std::wstring &form_id)
{
	bool bRet = false;
	if (image_view_window_)
	{
		if (form_id.compare(image_view_window_->GetWindowClassName()) == 0)
		{
			bRet = true;
		}
	}
	return bRet;
}

void ImageViewManager::ResetImageViewWndPoint(void)
{
	image_view_window_ = NULL;
}

bool ImageViewManager::SetImageViewList(ImageViewInfoList info_list, std::wstring message_id)
{
	if (message_id == message_id_)
	{
		int list_size = (int)image_view_info_list_.size();
		for (auto new_item = info_list.cbegin(); new_item != info_list.cend(); new_item++)
		{
			bool is_add = false;
			if (0 == list_size)
			{
				image_view_info_list_.push_back(*new_item);
				continue;
			}
			for (int j = list_size - 1; j >= 0; --j)
			{
				ImageViewInfo list_item = image_view_info_list_.at(j);
				if (list_item.id_ == new_item->id_)
				{
					is_add = true;
					break;
				}
			}
			if (is_add)
			{
				continue;
			}
			for (int k = list_size - 1; k >= 0; --k)
			{
				ImageViewInfo list_item = image_view_info_list_.at(k);
				if (list_item.timetag_ < new_item->timetag_)
				{
					image_view_info_list_.insert(image_view_info_list_.cbegin() + (k + 1), *new_item);
					is_add = true;
					break;
				}
			}
			if (!is_add)
			{
				image_view_info_list_.insert(image_view_info_list_.cbegin(), *new_item);
			}
		}
		if (image_view_window_)
		{
			image_view_window_->CenterWindow();
			image_view_window_->CheckNextOrPreImageBtnStatus();
			image_view_window_->SetNextOrPreImageBtnVisible(list_size > 1);
		}
	}
	return true;
}

bool ImageViewManager::ShowNextImageView(bool next)
{
	int list_size = (int)image_view_info_list_.size();
	for (int i=0; i < list_size; ++i)
	{
		ImageViewInfo info = image_view_info_list_.at(i);
		if (info.id_ == message_id_)
		{
			int j = i;
			if (next)
			{
				j++;
			} 
			else
			{
				j--;
			}

			if (j >= 0 && j < list_size)
			{
				info = image_view_info_list_.at(j);

				if(nbase::FilePathIsExist(info.path_, false))
				{
					StartViewPic(info.path_, info.id_, false);
				}
				//else
				//{
					//std::string path, url;
					//nbase::UTF16ToUTF8(info.path_, path);
					//nbase::UTF16ToUTF8(info.url_, url);
					//ViewImage(url, path, info.id_, false);
				//}

				return true;
			}

		}
	}
	return false;
}

bool ImageViewManager::IsExistNextImage(bool next)
{

	for (int i = 0; i < (int)image_view_info_list_.size(); ++i)
	{
		ImageViewInfo info = image_view_info_list_.at(i);
		if (info.id_ == message_id_)
		{
			int j = i;
			if (next)
			{
				j++;
			}				
			else
			{
				j--;
			}
			if (j >= 0 && j < (int)image_view_info_list_.size())
				return true;
		}
	}
	return false;
}

void ImageViewManager::GetImagePage(long& cur, long& sum)
{
	cur = 0;
	sum = image_view_info_list_.size();
	for (long i=0; i<sum; ++i)
	{
		ImageViewInfo info = image_view_info_list_.at(i);
		if (info.id_ == message_id_)
		{
			cur = i+1;
			return;
		}
	}
}

void ImageViewManager::OnImageReady(std::wstring &path_pic, bool size)
{
	if (!image_view_window_)
		return;

	if (image_view_window_->CurrentImagePath() == path_pic)
	{
		StartViewPic(path_pic, L"", size);
		image_view_window_->CenterWindow();
	}
}
