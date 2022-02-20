/*
* @author:	Zachary Tay
* @date:	20/02/21
* @brief:	vulkan midterm
*/

#pragma once
#include <Windows.h>

namespace wndHelper
{
	static int g_width , g_height;
	struct Window
	{
		struct Parameters
		{
			int width_ { 0 } , height_ { 0 };
			bool is_fullscreen_ { false };
		};

		bool Initialize ( Parameters const& params ) noexcept;

		HWND GetHandle () const noexcept;

		void Update ();

		bool WindowShouldClose ();

		void PollEvents ();

	private:
		int		width_;
		int		height_;
		bool	is_minimized_;
		bool	is_resized_ { false };
		HWND	window_handle_;
		MSG		msg_;

	};
}