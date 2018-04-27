#include <windows.h>
#include "resource.h"

#define WM_TRAY (WM_USER + 100)  
#define WM_TASKBAR_CREATED RegisterWindowMessage(TEXT("TaskbarCreated"))  

#define APP_NAME    TEXT("���̳���")  
#define APP_TIP     TEXT("Win32 API ʵ��ϵͳ���̳���")

NOTIFYICONDATA nid;     //��������   
HMENU hMenu;            //���̲˵�  

//ʵ��������  
void InitTray(HINSTANCE hInstance, HWND hWnd)
{
	nid.cbSize = sizeof(NOTIFYICONDATA);
	nid.hWnd = hWnd;
	nid.uID = IDI_TRAY;
	nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP | NIF_INFO;
	nid.uCallbackMessage = WM_TRAY;
	nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_TRAY));
	lstrcpy(nid.szTip, APP_NAME);
	hMenu = CreatePopupMenu();//�������̲˵�  
	AppendMenu(hMenu, MF_STRING, ID_EXIT, TEXT("�˳�"));
	Shell_NotifyIcon(NIM_ADD, &nid);
}

//���ڹ��̺���
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
	//���崰����ṹ�����
	WNDCLASS wc;

	//����������
	static TCHAR *szAppName = TEXT("IPHPJS");

	//���ھ��
	HWND hwnd = NULL;

	//��Ϣ�ṹ
	MSG msg;

	/**
	����Ĵ�����䴰������Ϣ����ͼ�꣬�����ʽ�����������̺�����
	*/
	wc.style = CS_HREDRAW | CS_VREDRAW; //������ʽ
	wc.lpfnWndProc = WndProc; //���̺���
	wc.cbClsExtra = 0;    //��չ�ֶ�
	wc.cbWndExtra = 0;        //��չ�ֶ�
	wc.hInstance = hInstance; //��ǰʵ�����
	wc.hIcon = ::LoadIcon(hInstance, IDI_APPLICATION); //���ó���ͼ��
	wc.hCursor = ::LoadCursor(NULL, IDC_ARROW);        //�������

	//�ð�ɫ��ˢ��䱳��
	wc.hbrBackground = (HBRUSH)::GetStockObject(WHITE_BRUSH);

	//�˵�
	wc.lpszMenuName = NULL;

	//����
	wc.lpszClassName = szAppName;


	//�����ϵͳע�ᴰ����
	if (!::RegisterClass(&wc)) {
		::MessageBox(NULL, TEXT("����ֻ����windowsNT������"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	//��������
	hwnd = ::CreateWindow(szAppName,        //Ҫע��Ĵ�������
		TEXT("Synergy-clipboard"), //���ڱ���
		WS_OVERLAPPEDWINDOW,    //������ʽ
		CW_USEDEFAULT,      //���ھ�����Ļ���Ͻǵĺ�����
		CW_USEDEFAULT,      //���ھ�����Ļ���Ͻǵ�������
		400,            //���ڿ��
		300,            //���ڸ߶�
		NULL,           //�����ھ��
		NULL,           //�˵����
		hInstance,      //��ǰʵ�����
		NULL);  //ָ��һ��ֵ��ָ�룬��ֵ���ݸ����� WM_CREATE��Ϣ��һ��ΪNULL

	//��ʾ����
	//::ShowWindow(hwnd, iCmdShow);

	//���´���
	::UpdateWindow(hwnd);
	InitTray(hInstance, hwnd);

	//��Ϣѭ����һֱͣ������˳���Ϣѭ���ͱ�ʾ��������ˡ�
	while (::GetMessage(&msg, NULL, 0, 0)) {
		//������Ϣ
		::TranslateMessage(&msg);

		//�ַ���Ϣ
		::DispatchMessage(&msg);
	}

	return msg.wParam;
}


/**
��Ϣ������
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
				EmptyClipboard(); // ���
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
			//��ȡ�������  
			POINT pt; GetCursorPos(&pt);
			//����ڲ˵��ⵥ������˵�����ʧ������  
			SetForegroundWindow(hwnd);
			//��ʾ����ȡѡ�еĲ˵�
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
		//��������ʱɾ������  
		Shell_NotifyIcon(NIM_DELETE, &nid);
		//���ͽ�����������Ĳ���Ϊ�˳���
		::PostQuitMessage(0);
		break;
	}

	//����Ĭ�ϵĹ��̺���
	return ::DefWindowProc(hwnd, message, wParam, lParam);
}