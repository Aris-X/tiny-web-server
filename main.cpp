// main.cpp : 定义应用程序的入口点。
//

#include "framework.h"
#include "WindowsProject1.h"
#include "server.h"
#include <shlobj.h>
#include <commdlg.h>
#include <shellapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")
#pragma comment(lib, "ws2_32.lib")

#define MAX_LOADSTRING 100

void AddLog(HWND hWnd, const wchar_t* msg);

// 全局变量:
HINSTANCE hInst;                                // 当前实例
static HWND hStatusBar;
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPWSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);


	// 初始化全局字符串
	LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
	LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
	MyRegisterClass(hInstance);

	// 执行应用程序初始化:
	if (!InitInstance(hInstance, nCmdShow))
	{
		return FALSE;
	}

	HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WINDOWSPROJECT1));

	MSG msg;

	// 主消息循环:
	while (GetMessage(&msg, nullptr, 0, 0))
	{
		if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	return (int)msg.wParam;
}



//
//  函数: MyRegisterClass()
//
//  目标: 注册窗口类。
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
	WNDCLASSEXW wcex;

	wcex.cbSize = sizeof(WNDCLASSEX);

	wcex.style = CS_HREDRAW | CS_VREDRAW;
	wcex.lpfnWndProc = WndProc;
	wcex.cbClsExtra = 0;
	wcex.cbWndExtra = 0;
	wcex.hInstance = hInstance;
	wcex.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
	wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
	wcex.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wcex.lpszMenuName = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
	wcex.lpszClassName = szWindowClass;
	wcex.hIconSm = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

	return RegisterClassExW(&wcex);
}

//
//   函数: InitInstance(HINSTANCE, int)
//
//   目标: 保存实例句柄并创建主窗口
//
//   注释:
//
//        在此函数中，我们在全局变量中保存实例句柄并
//        创建和显示主程序窗口。
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance; // 将实例句柄存储在全局变量中

	INITCOMMONCONTROLSEX icex = { sizeof(INITCOMMONCONTROLSEX), ICC_BAR_CLASSES };
	InitCommonControlsEx(&icex);

	HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT, 0, 500, 490, nullptr, nullptr, hInstance, nullptr);

	if (!hWnd)
	{
		return FALSE;
	}

	ShowWindow(hWnd, nCmdShow);

	HFONT hFont = CreateFontW(16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE,
		DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS,
		CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, L"Segoe UI");

	// 服务器分组
	HWND hGroupServer = CreateWindowW(L"BUTTON", L"服务器",
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		10, 10, 460, 55, hWnd, (HMENU)-1, hInst, nullptr);
	SendMessage(hGroupServer, WM_SETFONT, (WPARAM)hFont, TRUE);

	HWND hBtnStart = CreateWindowW(L"BUTTON", L"启动WEB服务",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 26, 100, 30, hWnd, (HMENU)IDC_BTN_START_WebServer, hInst, nullptr);
	HWND hBtnStop = CreateWindowW(L"BUTTON", L"停止WEB服务",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		132, 26, 100, 30, hWnd, (HMENU)IDC_BTN_STOP_WebServer, hInst, nullptr);
	EnableWindow(hBtnStop, FALSE);
	SendMessage(hBtnStart, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hBtnStop, WM_SETFONT, (WPARAM)hFont, TRUE);

	HWND hIpLabel = CreateWindowW(L"STATIC", L"IP:",
		WS_CHILD | WS_VISIBLE,
		244, 31, 20, 20, hWnd, (HMENU)-1, hInst, nullptr);
	SendMessage(hIpLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	HWND hAddr = CreateWindowW(L"ComboBox", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_BORDER | WS_TABSTOP,
		264, 26, 85, 200, hWnd, (HMENU)IDC_EDIT_ADDR, hInst, nullptr);
	SendMessage(hAddr, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hAddr, CB_ADDSTRING, 0, (LPARAM)L"0.0.0.0");
	SendMessage(hAddr, CB_ADDSTRING, 0, (LPARAM)L"127.0.0.1");
	SendMessage(hAddr, CB_ADDSTRING, 0, (LPARAM)L"192.168.1.1");
	SendMessage(hAddr, CB_ADDSTRING, 0, (LPARAM)L"10.0.0.1");
	SetDlgItemText(hWnd, IDC_EDIT_ADDR, L"127.0.0.1");

	HWND hPortLabel = CreateWindowW(L"STATIC", L"端口:",
		WS_CHILD | WS_VISIBLE,
		361, 31, 30, 20, hWnd, (HMENU)-1, hInst, nullptr);
	SendMessage(hPortLabel, WM_SETFONT, (WPARAM)hFont, TRUE);
	HWND hPort = CreateWindowW(L"ComboBox", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWN | WS_BORDER | WS_TABSTOP,
		391, 26, 65, 200, hWnd, (HMENU)IDC_EDIT_PORT, hInst, nullptr);
	SendMessage(hPort, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hPort, CB_ADDSTRING, 0, (LPARAM)L"80");
	SendMessage(hPort, CB_ADDSTRING, 0, (LPARAM)L"443");
	SendMessage(hPort, CB_ADDSTRING, 0, (LPARAM)L"3000");
	SendMessage(hPort, CB_ADDSTRING, 0, (LPARAM)L"5000");
	SendMessage(hPort, CB_ADDSTRING, 0, (LPARAM)L"8080");
	SetDlgItemText(hWnd, IDC_EDIT_PORT, L"8080");

	// 模式选择
	HWND hModeLabel = CreateWindowW(L"STATIC", L"模式:",
		WS_CHILD | WS_VISIBLE,
		20, 80, 35, 20, hWnd, (HMENU)-1, hInst, nullptr);
	SendMessage(hModeLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

	HWND hRadioDef = CreateWindowW(L"BUTTON", L"默认页面",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		58, 76, 90, 24, hWnd, (HMENU)IDC_RADIO_DEFAULT, hInst, nullptr);
	SendMessage(hRadioDef, WM_SETFONT, (WPARAM)hFont, TRUE);

	HWND hRadioDir = CreateWindowW(L"BUTTON", L"目录模式",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		152, 76, 90, 24, hWnd, (HMENU)IDC_RADIO_DIR, hInst, nullptr);
	SendMessage(hRadioDir, WM_SETFONT, (WPARAM)hFont, TRUE);

	HWND hRadioFile = CreateWindowW(L"BUTTON", L"文件模式",
		WS_CHILD | WS_VISIBLE | BS_AUTORADIOBUTTON,
		246, 76, 90, 24, hWnd, (HMENU)IDC_RADIO_FILE, hInst, nullptr);
	SendMessage(hRadioFile, WM_SETFONT, (WPARAM)hFont, TRUE);

	CheckRadioButton(hWnd, IDC_RADIO_DEFAULT, IDC_RADIO_FILE, IDC_RADIO_DEFAULT);

	// 共享选择按钮
	HWND hBtnSelect = CreateWindowW(L"BUTTON", L"选择目录",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		20, 108, 85, 28, hWnd, (HMENU)IDC_BTN_SELECT_DIR, hInst, nullptr);
	SendMessage(hBtnSelect, WM_SETFONT, (WPARAM)hFont, TRUE);

	// 路径显示框
	HWND hPathEdit = CreateWindowW(L"EDIT", L"",
		WS_CHILD | WS_VISIBLE | WS_BORDER | ES_READONLY | ES_AUTOHSCROLL,
		110, 108, 280, 28, hWnd, (HMENU)IDC_EDIT_DIR, hInst, nullptr);
	SendMessage(hPathEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

	// 安全模式复选框
	HWND hChkSafe = CreateWindowW(L"BUTTON", L"安全模式",
		WS_CHILD | WS_VISIBLE | BS_AUTOCHECKBOX,
		395, 108, 75, 28, hWnd, (HMENU)IDC_CHK_SAFEMODE, hInst, nullptr);
	SendMessage(hChkSafe, WM_SETFONT, (WPARAM)hFont, TRUE);
	SendMessage(hChkSafe, BM_SETCHECK, BST_CHECKED, 0);

	// 默认模式禁用路径相关控件
	EnableWindow(hBtnSelect, FALSE);
	EnableWindow(hPathEdit, FALSE);
	EnableWindow(hChkSafe, FALSE);

	// URL显示
	HWND hUrlLabel = CreateWindowW(L"STATIC", L"地址:",
		WS_CHILD | WS_VISIBLE,
		20, 148, 35, 20, hWnd, (HMENU)-1, hInst, nullptr);
	SendMessage(hUrlLabel, WM_SETFONT, (WPARAM)hFont, TRUE);

	HWND hUrlCombo = CreateWindowW(L"ComboBox", L"",
		WS_CHILD | WS_VISIBLE | CBS_DROPDOWNLIST | WS_BORDER,
		55, 145, 310, 200, hWnd, (HMENU)IDC_EDIT_URL, hInst, nullptr);
	SendMessage(hUrlCombo, WM_SETFONT, (WPARAM)hFont, TRUE);

	HWND hBtnOpen = CreateWindowW(L"BUTTON", L"打开",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		370, 145, 45, 28, hWnd, (HMENU)IDC_BTN_OPEN_URL, hInst, nullptr);
	SendMessage(hBtnOpen, WM_SETFONT, (WPARAM)hFont, TRUE);

	HWND hBtnCopy = CreateWindowW(L"BUTTON", L"复制",
		WS_CHILD | WS_VISIBLE | BS_PUSHBUTTON,
		420, 145, 45, 28, hWnd, (HMENU)IDC_BTN_COPY_URL, hInst, nullptr);
	SendMessage(hBtnCopy, WM_SETFONT, (WPARAM)hFont, TRUE);

	// 日志分组框
	HWND hGroup = CreateWindowW(L"BUTTON", L"运行日志",
		WS_CHILD | WS_VISIBLE | BS_GROUPBOX,
		10, 182, 460, 250, hWnd, (HMENU)IDC_GROUP_LOG, hInst, nullptr);
	SendMessage(hGroup, WM_SETFONT, (WPARAM)hFont, TRUE);

	// 创建日志框
	HWND hEdit = CreateWindowW(L"EDIT", nullptr,
		WS_CHILD | WS_VISIBLE | ES_MULTILINE | ES_READONLY | WS_VSCROLL | ES_AUTOVSCROLL,
		18, 200, 444, 220, hWnd, (HMENU)IDC_Message_001, hInst, nullptr);
	SetDlgItemText(hWnd, IDC_Message_001, L"");
	SendMessage(hEdit, WM_SETFONT, (WPARAM)hFont, TRUE);

	// 初始禁用URL控件
	EnableWindow(hUrlCombo, FALSE);
	EnableWindow(hBtnOpen, FALSE);
	EnableWindow(hBtnCopy, FALSE);

	HMENU hMenu = GetMenu(hWnd);
	HMENU hHelp = GetSubMenu(hMenu, 1);
	InsertMenuW(hHelp, 0, MF_BYPOSITION | MF_STRING, IDM_TUTORIAL, L"使用教程(&T)...");

	// 状态栏
	hStatusBar = CreateWindowW(STATUSCLASSNAMEW, NULL,
		WS_CHILD | WS_VISIBLE,
		0, 0, 0, 0, hWnd, (HMENU)1016, hInst, nullptr);
	int parts[] = { -1, 180 };
	SendMessage(hStatusBar, SB_SETPARTS, 2, (LPARAM)parts);
	SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)L"就绪");

	UpdateWindow(hWnd);

	return TRUE;
}

//
//  函数: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  目标: 处理主窗口的消息。
//
//  WM_COMMAND  - 处理应用程序菜单
//  WM_PAINT    - 绘制主窗口
//  WM_DESTROY  - 发送退出消息并返回
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_COMMAND:
	{
		int wmId = LOWORD(wParam);
		// 分析菜单选择:
		switch (wmId)
		{
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			break;
		case IDM_TUTORIAL:
			MessageBoxW(hWnd,
				L"使用教程\n\n"
				L"1. 选择模式：\n"
				L"   默认页面 — 使用内置模板\n"
				L"   目录模式 — 选择文件夹作为Web根目录\n"
				L"   文件模式 — 选择网页文件，安全模式只暴露该文件\n\n"
				L"2. 设置IP和端口（地址栏可下拉选择IP）\n\n"
				L"3. 点击「启动WEB服务」开始运行\n\n"
				L"4. 地址栏点击「打开」访问，或「复制」复制链接\n\n"
				L"5. 点击「停止WEB服务」关闭\n\n"
				L"6. 已启动时可随时「重启WEB服务」以新配置重启",
				L"使用教程", MB_OK | MB_ICONINFORMATION);
			break;
		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		case IDC_BTN_START_WebServer: {
			if (serverRunning()) serverStop();

			EnableWindow(GetDlgItem(hWnd, IDC_BTN_STOP_WebServer), TRUE);

			wchar_t addrBuf[64];
			GetDlgItemText(hWnd, IDC_EDIT_ADDR, addrBuf, 64);
			char addr[64];
			WideCharToMultiByte(CP_UTF8, 0, addrBuf, -1, addr, 64, nullptr, nullptr);

			BOOL ok;
			int port = GetDlgItemInt(hWnd, IDC_EDIT_PORT, &ok, FALSE);
			if (!ok || port < 1 || port > 65535) port = 8080;

			wchar_t pathBuf[MAX_PATH] = {0};
			GetDlgItemText(hWnd, IDC_EDIT_DIR, pathBuf, MAX_PATH);
			char path[MAX_PATH];
			WideCharToMultiByte(CP_UTF8, 0, pathBuf, -1, path, MAX_PATH, nullptr, nullptr);

			BOOL safe = SendMessage(GetDlgItem(hWnd, IDC_CHK_SAFEMODE), BM_GETCHECK, 0, 0) == BST_CHECKED;

			if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_DIR), BM_GETCHECK, 0, 0) == BST_CHECKED)
				serverStart(addr, port, path, "", safe);
			else if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_FILE), BM_GETCHECK, 0, 0) == BST_CHECKED)
				serverStart(addr, port, "", path, safe);
			else
				serverStart(addr, port, "", "", true);
			SetDlgItemText(hWnd, IDC_BTN_START_WebServer, L"重启WEB服务");

			HWND hUrlCombo = GetDlgItem(hWnd, IDC_EDIT_URL);
			SendMessage(hUrlCombo, CB_RESETCONTENT, 0, 0);

			if (wcscmp(addrBuf, L"0.0.0.0") == 0) {
				WSADATA wsaData;
				WSAStartup(MAKEWORD(2, 2), &wsaData);
				char hostname[256] = { 0 };
				if (gethostname(hostname, 256) == 0) {
					struct addrinfo hints = { 0 };
					hints.ai_family = AF_INET;
					hints.ai_socktype = SOCK_STREAM;
					struct addrinfo* result = nullptr;
					if (getaddrinfo(hostname, nullptr, &hints, &result) == 0) {
						for (struct addrinfo* p = result; p; p = p->ai_next) {
							struct sockaddr_in* sa = (struct sockaddr_in*)p->ai_addr;
							wchar_t ip[64];
							InetNtopW(AF_INET, &sa->sin_addr, ip, 64);
							if (wcscmp(ip, L"127.0.0.1") != 0) {
								wchar_t url[128];
								wsprintfW(url, L"http://%s:%d", ip, port);
								SendMessage(hUrlCombo, CB_ADDSTRING, 0, (LPARAM)url);
							}
						}
						freeaddrinfo(result);
					}
				}
			} else {
				wchar_t url[128];
				wsprintfW(url, L"http://%s:%d", addrBuf, port);
				SendMessage(hUrlCombo, CB_ADDSTRING, 0, (LPARAM)url);
			}
			wchar_t url[128];
			wsprintfW(url, L"http://127.0.0.1:%d", port);
			SendMessage(hUrlCombo, CB_ADDSTRING, 0, (LPARAM)url);
			SendMessage(hUrlCombo, CB_SETCURSEL, 0, 0);
			EnableWindow(hUrlCombo, TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_OPEN_URL), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_COPY_URL), TRUE);

			AddLog(hWnd, L"服务器启动");

			int parts2[] = { -1, 180 };
			SendMessage(hStatusBar, SB_SETPARTS, 2, (LPARAM)parts2);
			SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)L"运行中");
			wchar_t selUrl[256];
			GetDlgItemText(hWnd, IDC_EDIT_URL, selUrl, 256);
			SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)selUrl);
			break;
		}
		case IDC_BTN_STOP_WebServer:
			serverStop();
			SetDlgItemText(hWnd, IDC_BTN_START_WebServer, L"启动WEB服务");
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_STOP_WebServer), FALSE);
			SendMessage(GetDlgItem(hWnd, IDC_EDIT_URL), CB_RESETCONTENT, 0, 0);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT_URL), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_OPEN_URL), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_COPY_URL), FALSE);
			AddLog(hWnd, L"服务器关闭");
			SendMessage(hStatusBar, SB_SETTEXT, 0, (LPARAM)L"就绪");
			SendMessage(hStatusBar, SB_SETTEXT, 1, (LPARAM)L"");
			break;
		case IDC_BTN_SELECT_DIR: {
			if (SendMessage(GetDlgItem(hWnd, IDC_RADIO_DIR), BM_GETCHECK, 0, 0) == BST_CHECKED) {
				wchar_t path[MAX_PATH] = {0};
				BROWSEINFOW bi = { 0 };
				bi.hwndOwner = hWnd;
				bi.lpszTitle = L"选择Web服务器目录";
				bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
				LPITEMIDLIST pidl = SHBrowseForFolderW(&bi);
				if (pidl) {
					SHGetPathFromIDListW(pidl, path);
					SetDlgItemText(hWnd, IDC_EDIT_DIR, path);
					CoTaskMemFree(pidl);
				}
			} else {
				wchar_t path[MAX_PATH] = {0};
				OPENFILENAMEW ofn = { sizeof(ofn) };
				ofn.hwndOwner = hWnd;
				ofn.lpstrFilter = L"网页文件\0*.htm;*.html;*.js;*.css;*.png;*.jpg;*.svg;*.ico\0所有文件\0*.*\0";
				ofn.lpstrFile = path;
				ofn.nMaxFile = MAX_PATH;
				ofn.Flags = OFN_FILEMUSTEXIST | OFN_HIDEREADONLY | OFN_EXPLORER;
				if (GetOpenFileNameW(&ofn))
					SetDlgItemText(hWnd, IDC_EDIT_DIR, ofn.lpstrFile);
			}
			break;
		}
		case IDC_RADIO_DEFAULT:
			CheckRadioButton(hWnd, IDC_RADIO_DEFAULT, IDC_RADIO_FILE, IDC_RADIO_DEFAULT);
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_SELECT_DIR), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT_DIR), FALSE);
			EnableWindow(GetDlgItem(hWnd, IDC_CHK_SAFEMODE), FALSE);
			SetDlgItemText(hWnd, IDC_EDIT_DIR, L"");
			break;
		case IDC_RADIO_DIR:
			CheckRadioButton(hWnd, IDC_RADIO_DEFAULT, IDC_RADIO_FILE, IDC_RADIO_DIR);
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_SELECT_DIR), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT_DIR), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_CHK_SAFEMODE), FALSE);
			SetDlgItemText(hWnd, IDC_BTN_SELECT_DIR, L"选择目录");
			break;
		case IDC_RADIO_FILE:
			CheckRadioButton(hWnd, IDC_RADIO_DEFAULT, IDC_RADIO_FILE, IDC_RADIO_FILE);
			EnableWindow(GetDlgItem(hWnd, IDC_BTN_SELECT_DIR), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_EDIT_DIR), TRUE);
			EnableWindow(GetDlgItem(hWnd, IDC_CHK_SAFEMODE), TRUE);
			SetDlgItemText(hWnd, IDC_BTN_SELECT_DIR, L"选择文件");
			break;
		case IDC_BTN_OPEN_URL: {
			wchar_t url[256];
			GetDlgItemText(hWnd, IDC_EDIT_URL, url, 256);
			ShellExecuteW(nullptr, L"open", url, nullptr, nullptr, SW_SHOWNORMAL);
			break;
		}
		case IDC_BTN_COPY_URL: {
			wchar_t url[256];
			GetDlgItemText(hWnd, IDC_EDIT_URL, url, 256);
			int len = wcslen(url);
			HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, (len + 1) * sizeof(wchar_t));
			memcpy(GlobalLock(hMem), url, (len + 1) * sizeof(wchar_t));
			GlobalUnlock(hMem);
			if (OpenClipboard(hWnd)) {
				EmptyClipboard();
				SetClipboardData(CF_UNICODETEXT, hMem);
				CloseClipboard();
			}
			break;
		}
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
		}
	}
	break;
	case WM_CTLCOLORSTATIC:
	{
		HDC hdc = (HDC)wParam;
		SetBkMode(hdc, TRANSPARENT);
		return (LRESULT)GetStockObject(NULL_BRUSH);
	}
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		// TODO: 在此处添加任何绘图代码...
		EndPaint(hWnd, &ps);
	}
	break;
	case WM_DESTROY:
		serverStop();
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

// “关于”框的消息处理程序。
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch (message)
	{
	case WM_INITDIALOG:
		return (INT_PTR)TRUE;

	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
		{
			EndDialog(hDlg, LOWORD(wParam));
			return (INT_PTR)TRUE;
		}
		break;
	}
	return (INT_PTR)FALSE;
}

void AddLog(HWND hWnd, const wchar_t* msg) {
	wchar_t buf[256];
	wchar_t time[16];
	SYSTEMTIME st;
	GetLocalTime(&st);
	wsprintfW(time, L"[%02d:%02d:%02d] ", st.wHour, st.wMinute, st.wSecond);
	wsprintfW(buf, L"%s%s\r\n", time, msg);
	HWND hEdit = GetDlgItem(hWnd, IDC_Message_001);
	int len = GetWindowTextLength(hEdit);
	SendMessage(hEdit, EM_SETSEL, len, len);
	SendMessage(hEdit, EM_REPLACESEL, 0, (LPARAM)buf);
}