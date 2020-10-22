// WindowsProject1.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "WindowsProject1.h"

#define MAX_LOADSTRING 100

// 全局变量:
HINSTANCE hInst;                                // 当前实例
WCHAR szTitle[MAX_LOADSTRING];                  // 标题栏文本
WCHAR szWindowClass[MAX_LOADSTRING];            // 主窗口类名

// 此代码模块中包含的函数的前向声明:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: 在此处放置代码。

    // 初始化全局字符串
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WINDOWSPROJECT1, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // 执行应用程序初始化:
    if (!InitInstance (hInstance, nCmdShow))
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

    return (int) msg.wParam;
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

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WINDOWSPROJECT1));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    //wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW);
	wcex.hbrBackground = (HBRUSH)(COLOR_GRAYTEXT);
    //wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WINDOWSPROJECT1);
	wcex.lpszMenuName = NULL;
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

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

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_SYSMENU,
      CW_USEDEFAULT, 0, 764, 573, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   return TRUE;
}


//
//   函数:  GetDerID(LPWSTR szDevID)
//
//   目标: 获取当前显示卡硬件ID
//
//   注释:成功 TRUE,失败FALSE
//
//   没安装驱动获取不了
BOOL GetDerID(LPWSTR szDevID)
{
	HKEY keyServ;
	HKEY keyEnum;
	HKEY key;
	HKEY key2;
	LONG lResult;

	//查询"SYSTEM\\CurrentControlSet\\Services"下的所有子键保存到keyServ
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Services", 0, KEY_READ, &keyServ);
	if (lResult != ERROR_SUCCESS)
		return FALSE;

	//查询"SYSTEM\\CurrentControlSet\\Enum"下的所有子键保存到keyEnum
	lResult = RegOpenKeyEx(HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Enum", 0, KEY_READ, &keyEnum);
	if (lResult != ERROR_SUCCESS)
		return FALSE;

	int i = 0;
	int count = 0;
	DWORD size = 0;
	DWORD type = 0;
	for (;; ++i)
	{

		Sleep(5);
		size = 512;
		WCHAR  name[512] = { 0 };//保存keyServ下各子项的字段名称

		//逐个枚举keyServ下的各子项字段保存到name中
		lResult = RegEnumKeyEx(keyServ, i, name, &size, NULL, NULL, NULL, NULL);

		//要读取的子项不存在，即keyServ的子项全部遍历完时跳出循环
		if (lResult == ERROR_NO_MORE_ITEMS)
			break;

		//打开keyServ的子项字段为name所标识的字段的值保存到key
		lResult = RegOpenKeyEx(keyServ, name, 0, KEY_READ, &key);
		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey(keyServ);
			continue;
		}

		size = 512;
		//查询key下的字段为Group的子键字段名保存到name
		lResult = RegQueryValueEx(key, L"Group", 0, &type, (LPBYTE)name, &size);
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(key);
			continue;
		}

		//如果查询到的name不是Video则说明该键不是显卡驱动项
		if (StrCmpW(L"Video", name) != 0)
		{
			RegCloseKey(key);
			continue;
		}

		//如果程序继续往下执行的话说明已经查到了有关显卡的信息，所以在下面的代码执行完之后要break第一个for循环，函数返回
		lResult = RegOpenKeyEx(key, L"Enum", 0, KEY_READ, &key2);
		if (lResult != ERROR_SUCCESS)
		{
			RegCloseKey(key);
			continue;
		}

		RegCloseKey(key);
		key = key2;
		size = sizeof(count);
		//查询Count字段（显卡数目）
		lResult = RegQueryValueEx(key, L"Count", 0, &type, (LPBYTE)&count, &size);
		if (lResult == ERROR_FILE_NOT_FOUND)
		{
			RegCloseKey(key);
			continue;
		}
		
		
		for (int j = 0; j < count; ++j)
		{
			WCHAR sz[512];
		
			wsprintfW(name, L"%d", j);
			size = sizeof(sz);
			lResult = RegQueryValueEx(key, name, 0, &type, (LPBYTE)sz, &size);
			if (lResult == ERROR_FILE_NOT_FOUND)
			{
				RegCloseKey(key);
				break;
			}

			lResult = RegOpenKeyEx(keyEnum, sz, 0, KEY_READ, &key2);
			if (ERROR_SUCCESS)
			{
				RegCloseKey(keyEnum);
				continue;
			}

			//获取显示硬件ID
			lResult = RegQueryValueEx(key2, L"HardwareID", 0, &type, (LPBYTE)sz, &size);
			if(lResult != ERROR_FILE_NOT_FOUND)
			{
				//判断是否为NVIDA或AMDL"PCI\\VEN_1002"PCI\\VEN_8086
				if ((StrStrW(sz, L"PCI\\VEN_1002") != NULL) || (StrStrW(sz, L"PCI\\VEN_10DE") != NULL))
				{
					StrCpyW(szDevID, sz);
					//StrCpyW(szDevID, L"PCI\\VEN_10DE&DEV_1C82&SUBSYS_11BF10DE&REV_A1");
					return TRUE;
				}
			}
			RegCloseKey(key2);
			key2 = NULL;
		}

		RegCloseKey(key);
		key = NULL;
	}
	return FALSE;
}


//
//   函数:  GetDerID(LPWSTR szDevID)
//
//   目标: 获取当前显示卡硬件ID
//
//   注释:成功 TRUE,失败FALSE
//
//   ****没安装驱动获取不了*****
//BOOL GetDerID(LPWSTR szDevID)
//{
//	DISPLAY_DEVICE dd;
//	dd.cb = sizeof(DISPLAY_DEVICE);
//	int i = 0;
//	while(EnumDisplayDevicesW(NULL, i, &dd, 0))
//	{
//		//if (dd.StateFlags & (DISPLAY_DEVICE_ACTIVE ))//| DISPLAY_DEVICE_PRIMARY_DEVICE
//		//{
//		//	
//		//}
//		if ((StrStrW(dd.DeviceID, L"PCI\\VEN_1002") != NULL) || (StrStrW(dd.DeviceID, L"PCI\\VEN_10DE") != NULL))
//		{
//			StrCpyW(szDevID, dd.DeviceID);
//			return TRUE;
//		}
//		i++;
//	}
//	return FALSE;
//}

//
//   函数:  GetWinVer(LPWSTR szWinVer)
//
//   目标: 获取当前系统版本信息
//
//   注释:成功 TRUE,失败FALSE
//
//
BOOL GetWinVer(LPWSTR szWinVer)
{
	/*	if (IsWindows10OrGreater())
	{
		StrCpy(szWinVer, L"WIN10_");
	}
	else if (IsWindows8Point1OrGreater)
	{
		StrCpy(szWinVer, L"WIN8.1_");
	}
	else */
	typedef void(__stdcall * NTPROC)(DWORD*, DWORD*, DWORD*);
	HINSTANCE hinst = LoadLibrary(L"ntdll.dll");
	DWORD dwMajor, dwMinor, dwBuildNumber;
	NTPROC proc = (NTPROC)GetProcAddress(hinst, "RtlGetNtVersionNumbers");
	proc(&dwMajor, &dwMinor, &dwBuildNumber);
	if (dwMajor == 6 && dwMinor == 3)	//win 8.1
	{
		StrCpy(szWinVer, L"Win81_");

	}
	else if (dwMajor == 10 && dwMinor == 0)	//win 10
	{
		StrCpy(szWinVer, L"Win10_");
	}

	else if (IsWindows8OrGreater())
	{
		StrCpy(szWinVer, L"Win8_");
	}
	else if (IsWindows7OrGreater)
	{
		StrCpy(szWinVer, L"Win7_");
	}
	else if (IsWindowsVistaOrGreater())
	{
		StrCpy(szWinVer, L"Vista");
	}
	else if (IsWindowsXPOrGreater())
	{
		StrCpy(szWinVer, L"XP");
	}
	else
	{
		return FALSE;
	}

	SYSTEM_INFO sys_info;

	GetSystemInfo(&sys_info);
	if (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || PROCESSOR_ARCHITECTURE_IA64)
	{
		size_t max_n = 100;
		wcscat_s(szWinVer, max_n, L"64");
	}
	else if (sys_info.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_INTEL)
	{
		size_t max_n = 100;
		wcscat_s(szWinVer, max_n, L"32");
	}
	else
	{
		return FALSE;
	}

	return TRUE;
	//StrCpyW(szWinVer, L"Win10_64");
	//return TRUE;
}

//
//   函数:  GetWinVer(LPWSTR szWinVer)
//
//   目标: 获取当前系统本地语言信息
//
//   注释:成功 TRUE,失败FALSE
//
//
BOOL GetLocal(LPWSTR szlocal)
{
	//StrCpyW(szlocal, L"CHS");
	
	LANGID lid=GetSystemDefaultLangID();
	switch (lid)
	{
	case 0x0804:
		StrCpyW(szlocal, L"CHS");//中国大陆
		break;
	case 0x0404:
		StrCpyW(szlocal, L"CHT");//中国台湾
		break;
	case 0x0c04:
		StrCpyW(szlocal, L"CHT");//中国香港
		break;
	case 0x0411:
		StrCpyW(szlocal, L"JPN");//日本
		break;
	case 0x0412:
		StrCpyW(szlocal, L"KOR");//韩国
		break;
	case 0x0419:
		StrCpyW(szlocal, L"ENU");//俄罗斯
	default:
		StrCpyW(szlocal, L"ENU");//默认英文
		break;
	}
	return TRUE;
}

//
//   函数: ExecDerPackge(LPWSTR szDevPath)
//
//   目标: 根据szDevPath对应的驱动安装路径，运行驱动安装包
//
//   注释:成功 TRUE,失败FALSE
//
//   
BOOL ExecDerPackge(LPWSTR szDevPath)
{
	STARTUPINFO si;
	PROCESS_INFORMATION pi;
	DWORD errorid = 0;
	ZeroMemory(&si, sizeof(si));
	si.cb = sizeof(si);
	ZeroMemory(&pi, sizeof(pi));
	//WCHAR cmdline[MAX_PATH] = L"";
	if (CreateProcessW(szDevPath, NULL, NULL, NULL, FALSE, NORMAL_PRIORITY_CLASS, NULL, NULL, &si, &pi))
	{
		CloseHandle(pi.hProcess);
		CloseHandle(pi.hThread);
		return TRUE;
	}
	else
	{
		//errorid = GetLastError();
		return FALSE;
	}
	

	
}

//
//   函数: ReadConfigFile(LPWSTR szFilename, LPWSTR szVerInfo, LPWSTR szViewName, LPWSTR szDevPath)
//
//   目标: 读取配置文件，输出 szVerInfo版本信息，szViewName显示卡名称，szDevPath对应的驱动安装路径
//
//   注释:成功 TRUE,失败FALSE
//
//   
BOOL ReadConfigFile(LPWSTR szFilename,LPWSTR szVerInfo, LPWSTR szViewName, LPWSTR szDevPath)
{
	WCHAR szDevID[MAX_LOADSTRING];
	WCHAR szWinVer[MAX_LOADSTRING];
	WCHAR szlocal[MAX_LOADSTRING];
	//WCHAR szFilename[MAX_PATH];
	//WCHAR szIDNum[MAX_LOADSTRING];
	WCHAR szKeyName[MAX_LOADSTRING]=L"Version";
	WCHAR szValName[MAX_LOADSTRING];

	//初始化信息
	GetWinVer(szWinVer);
	GetLocal(szlocal);


	//读取版本信息
	StrCpy(szVerInfo,L"Version:" );
	if (GetPrivateProfileString(L"Version", szKeyName, NULL, szValName, MAX_LOADSTRING, szFilename))
	{
		StrCatW(szVerInfo, szValName);
	}
	StrCatW(szVerInfo, L"\n\nDate:");
	StrCpy(szKeyName, L"Date");
	if (GetPrivateProfileString(L"Version", szKeyName, NULL, szValName, MAX_LOADSTRING, szFilename))
	{
		StrCatW(szVerInfo, szValName);
	}


	//读取硬件对应信息
	if (!GetDerID(szDevID))
		return FALSE;

	if (!GetPrivateProfileString(L"SupportID", szDevID, NULL, szValName, MAX_LOADSTRING, szFilename))
	{
		LPWSTR lpend;
		BOOL flag = FALSE;
		lpend=StrRStrIW(szDevID, NULL, L"&REV");
		if (lpend != NULL)
		{
			StrCpyW(lpend, L"\0");
			if (GetPrivateProfileString(L"SupportID", szDevID, NULL, szValName, MAX_LOADSTRING, szFilename))
			{
				flag = TRUE;
			}
		}
		
		lpend = StrRStrIW(szDevID, NULL, L"&SUBSYS");
		if ((lpend != NULL) && (!flag))
		{
			StrCpyW(lpend, L"\0");
			if (GetPrivateProfileString(L"SupportID", szDevID, NULL, szValName, MAX_LOADSTRING, szFilename))
			{
				flag = TRUE;
			}
		}

		if (!flag)
		{
			return FALSE;
		}

	}

	//遍历"Device"节，查找对应DevID
	int intID = -1;
	WCHAR  szID[MAX_LOADSTRING];
	size_t Max_Size = 4;

		
	int i = 0;
	int j = 0;
	int MatchID[7] = { -1,-1,-1,-1,-1,-1,-1 };
	while (TRUE)
	{	
		_snwprintf_s(szID, Max_Size, L"%04d", i);
		//遍历"Device"节，查找对应DevID
		if (GetPrivateProfileString(L"Device", szID, NULL, szValName, MAX_LOADSTRING, szFilename))
		{
			if (_wcsicmp(szDevID, szValName)== 0)
			{
				MatchID[j]=i;
				j++;
			}
			i++;
		}
		else
		{
			break;
		}

	}

	for (i = 0; i < 8; i++)
	{
		if (MatchID[i] == -1)
		{
			break;
		}

		_snwprintf_s(szID, Max_Size, L"%04d", MatchID[i]);
		if (GetPrivateProfileString(L"Support", szID, NULL, szValName, MAX_LOADSTRING, szFilename))
		{
			if (wcsstr(szValName, szWinVer) != NULL)
			{
				intID = MatchID[i];
				break;
			}
		}
	}

	if(intID !=-1)
	{ 
		_snwprintf_s(szID, Max_Size, L"%04d", intID);
		GetPrivateProfileString(L"Root", szID, NULL, szDevPath, MAX_LOADSTRING, szFilename);
		GetPrivateProfileString(szlocal, szID, NULL, szViewName, MAX_LOADSTRING, szFilename);
		return TRUE;
	}
	
	return FALSE;
}




//
//   函数: HitTestRect(RECT rectDes, int xPiont,int yPiont)
//
//   目标: 测试坐标点是否在矩形内
//
//   注释:
//
//       
BOOL HitTestRect(RECT rectDes, int xPiont,int yPiont)
{
		return ((xPiont >= rectDes.left) & (xPiont <= rectDes.right) & (yPiont >= rectDes.top) & (yPiont <= rectDes.bottom));
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
RECT BtnRect;
CImage image;

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static WCHAR szDirName[MAX_PATH];
	static WCHAR szVerInfo[MAX_PATH]=L"";
	static WCHAR szViewName[MAX_PATH]=L"";
	static WCHAR szDevPath[MAX_PATH]=L"";

	static BOOL IsHitBtn = FALSE;
	static BOOL HitBtnstState = FALSE;
	static BOOL IsClickBtn = FALSE;
	static BOOL IsGetDev = FALSE;
	static HCURSOR hcursor;

    switch (message)
    {
	case WM_CREATE:
	{


		WCHAR szInfFileName[MAX_PATH];
		WCHAR szFilename[MAX_PATH] = L"";

		WCHAR szBgImagename[MAX_PATH] = L"";
		LoadStringW(hInst, IDS_BGLOGONAME, szBgImagename, MAX_LOADSTRING);
		GetCurrentDirectory(MAX_PATH, szDirName);
		wcscat_s(szFilename, szDirName);
		wcscat_s(szFilename, L"\\");
		wcscat_s(szFilename, szBgImagename);
		image.Load(szFilename);

		LoadStringW(hInst, IDS_INFFILENAME, szInfFileName, MAX_LOADSTRING);
		StrCpy(szFilename, szDirName);
		wcscat_s(szFilename, L"\\");
		wcscat_s(szFilename, szInfFileName);

		if (ReadConfigFile(szFilename, szVerInfo, szViewName, szDevPath))
		{
			IsGetDev = TRUE;
		}
		else
		{
			MessageBoxExW(NULL, L"ERROR:can't find the Graphic Card!", NULL, MB_OK, NULL);
		}

			
		
		return 0;
		}
		break;
    case WM_COMMAND:
        {
        }
        break;
	case WM_LBUTTONDOWN:
		{
		int xPos = GET_X_LPARAM(lParam);
		int yPos = GET_Y_LPARAM(lParam);
			 //鼠标点击
		if (HitTestRect(BtnRect, xPos, yPos))
		{
			//DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
			WCHAR szFilename[MAX_PATH] = L"";
			StrCatW(szFilename, szDirName);
			StrCatW(szFilename, L"\\");
			StrCatW(szFilename,szDevPath);

			if (!ExecDerPackge(szFilename))
			{
				
				MessageBoxEx(NULL,L"ERROR:EXEC device packge error!",NULL,MB_OK,NULL);
			}
		}
		return 0;
		}
		break;
	case WM_MOUSEMOVE:
		{
			//鼠标移动
			int xPos = GET_X_LPARAM(lParam);
			int yPos = GET_Y_LPARAM(lParam);
			if (HitTestRect(BtnRect, xPos, yPos))
			{
				hcursor=SetCursor(LoadCursor(NULL, IDC_HAND));

				IsHitBtn = TRUE;
				if(!HitBtnstState)
				{
					InvalidateRect(hWnd, &BtnRect, FALSE);
				}
		}
		else
		{
			
			IsHitBtn = FALSE;
			if (HitBtnstState)
			{
				SetCursor(hcursor);
				InvalidateRect(hWnd, &BtnRect, FALSE);
				

			}
		}
		return 0;
		}
		break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
			RECT rectWin;
			HBRUSH br;
			//HFONT hfUser;
			
			
			GetClientRect(hWnd, &rectWin);
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: 在此处添加使用 hdc 的任何绘图代码...
			//画背景

			if (!image.IsNull())
			{
				int oldmode;
				oldmode = GetStretchBltMode(hdc);
				SetStretchBltMode(hdc, HALFTONE);
				image.StretchBlt(hdc, rectWin, SRCCOPY);
				SetStretchBltMode(hdc, oldmode);

			}

			//画安装按钮
			SIZE size;
			if(IsGetDev)
			{ 
				GetTextExtentExPointW(hdc, szViewName, lstrlenW(szViewName) + 1, 512, NULL, NULL, &size);
				BtnRect.left = rectWin.left + 50;
				BtnRect.top = rectWin.top + 100;
				BtnRect.right = rectWin.left + 50+(int)size.cx;
				BtnRect.bottom = rectWin.top + 100+(int)size.cy;
				InflateRect(&BtnRect, 3, 3);
				if (IsHitBtn)
				{
					SetBkMode(hdc, TRANSPARENT);
					SetTextColor(hdc, RGB(255, 255, 255));
					RECT rText;
					rText = BtnRect;
					OffsetRect(&rText, 5, 1);

					DrawText(hdc, szViewName, -1, &rText, DT_WORDBREAK | DT_LEFT | DT_SINGLELINE | DT_VCENTER);

					br = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
					RECT rFrame;
					rFrame = BtnRect;
					FrameRect(hdc, &rFrame, br);
					DeleteObject(br);

					HitBtnstState = TRUE;
				}
				else
				{
					SetBkMode(hdc, TRANSPARENT);
					SetTextColor(hdc, RGB(255, 255, 255));
					RECT rText;
					rText = BtnRect;
					OffsetRect(&rText, 5, 0);

					DrawText(hdc, szViewName, -1, &rText, DT_WORDBREAK | DT_LEFT | DT_SINGLELINE | DT_VCENTER);
				
					//br = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
					//RECT rFrame;
					//rFrame = BtnRect;
					//FrameRect(hdc, &rFrame, br);
					//DeleteObject(br);
					HitBtnstState = FALSE;
				}

			}


			GetTextExtentExPointW(hdc, szVerInfo, lstrlenW(szVerInfo) + 1, 512, NULL, NULL, &size);
			SetBkMode(hdc, TRANSPARENT);
			SetTextColor(hdc, RGB(255, 255, 255));
			RECT rVerInfo;
			rVerInfo = rectWin;
			rVerInfo.left = rectWin.left + 50;
			rVerInfo.top = rectWin.bottom - 200;
			rVerInfo.right = rectWin.left + 50+(int)size.cx;
			rVerInfo.bottom = rectWin.bottom - 200+(int)size.cy*3;

			DrawText(hdc, szVerInfo, -1, &rVerInfo, DT_WORDBREAK | DT_LEFT);
			br = CreateSolidBrush(GetSysColor(COLOR_BTNHIGHLIGHT));
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

