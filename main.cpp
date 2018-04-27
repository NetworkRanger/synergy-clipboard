#include <windows.h>
#include "resource.h"

#define WM_TRAY (WM_USER + 100)  
#define WM_TASKBAR_CREATED RegisterWindowMessage(TEXT("TaskbarCreated"))  

#define APP_NAME    TEXT("托盘程序")  
#define APP_TIP     TEXT("Win32 API 实现系统托盘程序")

NOTIFYICONDATA nid;     //托盘属性   
HMENU hMenu;            //托盘菜单  

//实例化托盘  
void InitTray(HINSTANCE hInstance, HWND hWnd)
{
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_TRAY;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = WM_TRAY;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY));
	lstrcpy(nid.szTip, APP_NAME);
	hMenu = CreatePopupMenu();//生成托盘菜单  
	AppendMenu(hMenu, MF_STRING, ID_EXIT, TEXT("退出"));
	Shell_NotifyIcon(NIM_ADD, &nid);
}

//窗口过程函数
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	//定义窗口类结构体变量
	WNDCLASS wc;

	//窗口类名称
	static TCHAR *szAppName = TEXT("IPHPJS");

	//窗口句柄
	HWND hwnd = NULL;

	//消息结构
	MSG msg;

	/**
	下面的代码填充窗口类信息，如图标，鼠标样式，背景，过程函数等
	*/
	wc.style = CS_HREDRAW | CS_VREDRAW; //窗口样式
	wc.lpfnWndProc = WndProc; //过程函数
	wc.cbClsExtra = 0;    //扩展字段
	wc.cbWndExtra = 0;        //扩展字段
	wc.hInstance = hInstance; //当前实例句柄
	wc.hIcon = ::LoadIcon(hInstance, IDI_APPLICATION); //设置程序图标
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);        //设置鼠标

	//用白色画刷填充背景
	wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);

	//菜单
	wc.lpszMenuName = NULL;

	//类名
	wc.lpszClassName = szAppName;


	//像操作系统注册窗口类
	if (!::RegisterClass(&wc)) {
		::MessageBox(NULL, TEXT("程序只能在windowsNT下运行"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	//创建窗口
	hwnd = ::CreateWindow(szAppName,        //要注册的窗口类名
		TEXT("Synergy-clipboard"), //窗口标题
		WS_OVERLAPPEDWINDOW,    //窗口样式
		CW_USEDEFAULT,      //窗口距离屏幕左上角的横坐标
		CW_USEDEFAULT,      //窗口距离屏幕左上角的纵坐标
		400,            //窗口宽度
		300,            //窗口高度
		NULL,           //父窗口句柄
		NULL,           //菜单句柄
		hInstance,      //当前实例句柄
		NULL);  //指向一个值的指针，该值传递给窗口 WM_CREATE消息。一般为NULL

	//显示窗口
	//::ShowWindow(hwnd, iCmdShow);

	//更新窗口
	::UpdateWindow(hwnd);
	InitTray(hInstance, hwnd);

	//消息循环，一直停在这里，退出消息循环就表示程序结束了。
	while (::GetMessage(&msg, NULL, 0, 0)) {
		//翻译消息
		::TranslateMessage(&msg);

		//分发消息
		::DispatchMessage(&msg);
	}

	return msg.wParam;
}


/**
消息处理函数
*/
LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static HWND hwndNextViewer;
	switch (message) {
	case WM_DRAWCLIPBOARD:{
		if (OpenClipboard(NULL)){
			if (IsClipboardFormatAvailable(CF_TEXT)){
				HANDLE hClipBoard = GetClipboardData(CF_TEXT);
				char *szBuf = (char *)GlobalLock(hClipBoard);
				GlobalUnlock(hClipBoard);
				EmptyClipboard(); // 清空
				HGLOBAL clipBuffer;
				int nLen = strlen(szBuf) + 1;
				clipBuffer = GlobalAlloc(GMEM_DDESHARE, nLen);
				char *buffer = (char *)GlobalLock(clipBuffer);
				memcpy_s(buffer, nLen, szBuf, nLen);
				GlobalUnlock(clipBuffer);
				SetClipboardData(CF_TEXT, clipBuffer);
			}
			CloseClipboard();
		}
		break;
	}
	case WM_TRAY:
		switch (lParam)
		{
		case WM_RBUTTONDOWN:
			//获取鼠标坐标  
			POINT pt; GetCursorPos(&pt);
			//解决在菜单外单击左键菜单不消失的问题  
			SetForegroundWindow(hwnd);
			//显示并获取选中的菜单
			int cmd = TrackPopupMenu(hMenu, TPM_RETURNCMD, pt.x, pt.y, NULL, hwnd, NULL);
			if (cmd == ID_EXIT)
				PostMessage(hwnd, WM_DESTROY, NULL, NULL);
			break;
		}
		return 0;
	case WM_CREATE:{
		hwndNextViewer = ::SetClipboardViewer(hwnd);
		break;
	}
	case WM_DESTROY:
		//窗口销毁时删除托盘  
		Shell_NotifyIcon(NIM_DELETE, &nid);
		//发送结束请求，里面的参数为退出码
		::PostQuitMessage(0);
		break;
	}

	//调用默认的过程函数
	return ::DefWindowProc(hwnd, message, wParam, lParam);
}