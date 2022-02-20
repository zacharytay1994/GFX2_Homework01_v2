/*
* @author:	Zachary Tay
* @date:	20/02/21
* @brief:	vulkan midterm
*/

#include "wndHelper.h"
#include <iostream>

namespace wndHelper
{
	/*!
	 * @brief 
	 *		Handles window procedures
	 * 
	 * @param hWnd 
	 *		Handle to the window
	 * 
	 * @param uMsg 
	 * @param wParam 
	 * @param lParam 
	 * @return 
	*/
	bool g_close_ { false };
	LRESULT CALLBACK WindowProc ( HWND hWnd , UINT uMsg , WPARAM wParam , LPARAM lParam ) noexcept
	{
		switch ( uMsg )
		{
		case WM_CLOSE:
			g_close_ = true;
			break;
		}

		// pass unhandled messages to DefWindowProc
		return DefWindowProc ( hWnd , uMsg , wParam , lParam );
	}

	/*!
	 * @brief 
	 *		Creates a window class
	 * 
	 * @param hInstance
	 *		Instance to the window
	 *
	 * @param wndProc
	 *		Windows procedure to handle input
	 *
	 * @return
	 *		If creation successful
	*/
	bool CreateWindowClass ( HINSTANCE hInstance , WNDPROC wndProc ) noexcept
	{
		// check if already registered the class
		{
			WNDCLASSA C {};
			if ( GetClassInfoA ( hInstance , "VULKANCLASS" , &C ) )
			{
				std::cerr << "### wndHelper::CreateWindowClass failed! Window class already exists." << std::endl;
				return false;
			}
		}

		// define window class
		WNDCLASSEX wnd_class;
		wnd_class.cbSize		= sizeof ( WNDCLASSEX );
		wnd_class.style			= CS_HREDRAW | CS_VREDRAW;
		wnd_class.lpfnWndProc	= wndProc;
		wnd_class.cbClsExtra	= 0;
		wnd_class.cbWndExtra	= 0;
		wnd_class.hInstance		= hInstance;
		wnd_class.hIcon			= LoadIcon ( NULL , IDI_APPLICATION );
		wnd_class.hCursor		= LoadCursor ( NULL , IDC_ARROW );
		wnd_class.hbrBackground = static_cast< HBRUSH >( GetStockObject ( WHITE_BRUSH ) );
		wnd_class.lpszMenuName	= nullptr;
		wnd_class.lpszClassName = TEXT ( "VULKANCLASS" );
		wnd_class.hIconSm		= LoadIcon ( NULL , IDI_WINLOGO );

		// register the window class and check if its successful
		if ( !RegisterClassEx ( &wnd_class ) )
		{
			std::cerr << "### wndHelper::CreateWindowClass failed! Window class not registered." << std::endl;
			return false;
		}

		return true;
	}

	/*!
	 * @brief 
	 *		Creates the actual window
	 * 
	 * @param hInstance
	 *		Handle to the window instance
	 * 
	 * @param hWnd
	 *		Handle to the window
	 * 
	 * @param isFullscreen
	 *		Whether the window is fullscreen or not
	 *
	 * @param width 
	 *		Width of the window
	 * 
	 * @param height 
	 *		Height of the window
	 * 
	 * @return
	 *		If window creation successful
	*/
	bool CreateSystemWindow ( HINSTANCE hInstance , HWND& hWnd , bool isFullscreen , int width , int height )
	{
		// get resolution 
		const int screen_width	= GetSystemMetrics ( SM_CXSCREEN );
		const int screen_height = GetSystemMetrics ( SM_CYSCREEN );
		g_width = width;
		g_height = height;

		if ( isFullscreen )
		{
			DEVMODE dm_screen_settings {};
			memset ( &dm_screen_settings , 0 , sizeof ( dm_screen_settings ) );

			dm_screen_settings.dmSize		= sizeof ( dm_screen_settings );
			dm_screen_settings.dmPelsWidth	= screen_width;
			dm_screen_settings.dmPelsHeight = screen_height;
			dm_screen_settings.dmBitsPerPel = 32;
			dm_screen_settings.dmFields		= DM_BITSPERPEL | DM_PELSWIDTH | DM_PELSHEIGHT;

			// if current resolution not fullscreen, make it fullscreen
			if ( ( width != screen_width ) && ( height != screen_height ) )
			{
				if ( ChangeDisplaySettings ( &dm_screen_settings , CDS_FULLSCREEN ) != DISP_CHANGE_SUCCESSFUL )
				{
					std::cerr << "### wndHelper::CreateSystemWindow failed! Display settings not changed successfully." << std::endl;
					return false;
				}
			}
		}

		// compute window flags
		const DWORD dw_ex_stye	= isFullscreen 
			? WS_EX_APPWINDOW
			: WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
		const DWORD dw_style	= isFullscreen 
			? WS_POPUP | WS_CLIPSIBLINGS | WS_CLIPCHILDREN	
			: WS_OVERLAPPEDWINDOW | WS_CLIPSIBLINGS | WS_CLIPCHILDREN;

		// window rectangle
		RECT window_rect;
		if ( isFullscreen )
		{
			window_rect.left	= static_cast< long >( 0 );
			window_rect.right	= static_cast< long >( screen_width );
			window_rect.top		= static_cast< long >( 0 );
			window_rect.bottom	= static_cast< long >( screen_height );
		}
		else
		{
			window_rect.left	= static_cast< long >( screen_width ) / 2 - width / 2;
			window_rect.right	= static_cast< long >( width );
			window_rect.top		= static_cast< long >( screen_height ) / 2 - height / 2;
			window_rect.bottom	= static_cast< long >( height );
		}

		AdjustWindowRectEx ( &window_rect , dw_style , FALSE , dw_ex_stye );

		// create window
		hWnd = CreateWindowEx
		(
			  0
			, TEXT ( "VULKANCLASS" )
			, TEXT ( "VULKANWINDOW" )
			, dw_style | WS_CLIPSIBLINGS | WS_CLIPCHILDREN
			, window_rect.left
			, window_rect.top
			, window_rect.right
			, window_rect.bottom
			, nullptr
			, nullptr
			, hInstance
			, nullptr
		);

		// check if window successfully created
		if ( !hWnd )
		{
			std::cerr << "### wndHelper::CreateSystemWindow failed! Failed to create system window." << std::endl;
			return false;
		}

		ShowWindow ( hWnd , SW_SHOW );
		SetForegroundWindow ( hWnd );
		SetFocus ( hWnd );

		return true;
	}

	bool Window::Initialize ( Parameters const& params ) noexcept
	{
		const auto hInstance = GetModuleHandle ( nullptr );

		if ( !CreateWindowClass ( hInstance , WindowProc ) )
		{
			std::cerr << "### wndHelper::Window::Initialize failed! Failed to create window class." << std::endl;
			return false;
		}

		if ( !CreateSystemWindow ( hInstance , window_handle_ , params.is_fullscreen_ , params.width_ , params.height_ ) )
		{
			std::cerr << "### wndHelper::Window::Initialize failed! Failed to create system window." << std::endl;
			return false;
		}

		if ( window_handle_ )
		{
			SetWindowLongPtr ( window_handle_ , GWLP_USERDATA , reinterpret_cast< LONG_PTR >( this ) );
		}

		width_ = params.width_;
		height_ = params.height_;

		is_resized_ = false;

		return true;
	}

	HWND Window::GetHandle () const noexcept
	{
		return window_handle_;
	}

	void Window::Update ()
	{

		while ( !WindowShouldClose () )
		{
			PollEvents ();
			if ( WindowShouldClose () )
			{
				break;
			}

			// process vulkan draw logic
		}
	}

	bool Window::WindowShouldClose ()
	{
		return g_close_ || msg_.message == WM_QUIT;
	}

	void Window::PollEvents ()
	{
		if ( PeekMessage ( &msg_ , NULL , 0 , 0 , PM_REMOVE ) )
		{
			TranslateMessage ( &msg_ );
			DispatchMessage ( &msg_ );
		}
	}
}