#include "pch.h"
#include "SelectScene.h"
#include "ConfigDialog.h"

//--------------------------------------------------------------------

// デバッグ用コールバック関数。デバッグメッセージを出力する。
void ___outputLog(LPCTSTR text, LPCTSTR output)
{
	::OutputDebugString(output);
}

//--------------------------------------------------------------------

AviUtlInternal g_auin;
HINSTANCE g_instance = 0;
HTHEME g_themeWindow = 0;
HTHEME g_themeButton = 0;
int g_hotScene = -1;
int g_dragScene = -1;
std::vector<RECT> g_buttonRectArray;

int g_layoutMode = LayoutMode::Horz;
int g_rowCount = 10;
int g_colCount = 10;
int g_sceneCount = MaxSceneCount;
int g_voice = 1;
BOOL g_fixedSize = TRUE;
int g_buttonWidth = 100;
int g_buttonHeight = 24;

//--------------------------------------------------------------------

void playVoice(int voice)
{
	if (voice == 0) return;

	// フォルダ名を取得する。
	TCHAR folderName[MAX_PATH] = {};
	::GetModuleFileName(g_instance, folderName, MAX_PATH);
	::PathRemoveExtension(folderName);
	MY_TRACE_TSTR(folderName);

	// wav ファイルのパスを取得する。
	TCHAR wavFileName[MAX_PATH] = {};
	::StringCbPrintf(wavFileName, sizeof(wavFileName), _T("%s\\%d.wav"), folderName, voice);
	MY_TRACE_TSTR(wavFileName);

	g_auin.voice(wavFileName);
}

//--------------------------------------------------------------------

void calcLayout(HWND hwnd, BOOL onSize)
{
	if (onSize && g_fixedSize)
		return; // 固定サイズの場合は WM_SIZE では何もしない。

	switch (g_layoutMode)
	{
	case LayoutMode::Vert: calcLayoutVert(hwnd); break;
	case LayoutMode::Horz: calcLayoutHorz(hwnd); break;
	}
}

void calcLayoutVert(HWND hwnd)
{
	g_buttonRectArray.resize(g_sceneCount);

	RECT clientRect; ::GetClientRect(hwnd, &clientRect);
	int clientX = clientRect.left;
	int clientY = clientRect.top;
	int clientW = clientRect.right - clientRect.left;
	int clientH = clientRect.bottom - clientRect.top;

	int rowCount = (g_rowCount > 0) ? g_rowCount : 5;
	int colCount = (g_sceneCount - 1) / rowCount + 1;

	int sceneIndex = 0;

	for (int col = 0; col < colCount; col++)
	{
		for (int row = 0; row < rowCount; row++, sceneIndex++)
		{
			if (sceneIndex >= g_sceneCount) break;

			RECT rc = {};

			if (g_fixedSize)
			{
				rc.left = clientX + g_buttonWidth * col;
				rc.top = clientY + g_buttonHeight * row;
				rc.right = rc.left + g_buttonWidth;
				rc.bottom = rc.top + g_buttonHeight;
			}
			else
			{
				rc.left = clientX + ::MulDiv(clientW, col + 0, colCount);
				rc.right = clientX + ::MulDiv(clientW, col + 1, colCount);
				rc.top = clientY + ::MulDiv(clientH, row + 0, rowCount);
				rc.bottom = clientY + ::MulDiv(clientH, row + 1, rowCount);
			}

			g_buttonRectArray[sceneIndex] = rc;
		}
	}

	if (g_fixedSize)
	{
		RECT rc = clientRect;
		rc.right = rc.left + g_buttonWidth * colCount;
		rc.bottom = rc.top + g_buttonHeight * rowCount;
		clientToWindow(hwnd, &rc);

		::SetWindowPos(hwnd, 0, 0, 0, getWidth(rc), getHeight(rc), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}

void calcLayoutHorz(HWND hwnd)
{
	g_buttonRectArray.resize(g_sceneCount);

	RECT clientRect; ::GetClientRect(hwnd, &clientRect);
	int clientX = clientRect.left;
	int clientY = clientRect.top;
	int clientW = clientRect.right - clientRect.left;
	int clientH = clientRect.bottom - clientRect.top;

	int colCount = (g_colCount > 0) ? g_colCount : 5;
	int rowCount = (g_sceneCount - 1) / colCount + 1;

	int sceneIndex = 0;

	for (int row = 0; row < rowCount; row++)
	{
		for (int col = 0; col < colCount; col++, sceneIndex++)
		{
			if (sceneIndex >= g_sceneCount) break;

			RECT rc = {};

			if (g_fixedSize)
			{
				rc.left = clientX + g_buttonWidth * col;
				rc.top = clientY + g_buttonHeight * row;
				rc.right = rc.left + g_buttonWidth;
				rc.bottom = rc.top + g_buttonHeight;
			}
			else
			{
				rc.left = clientX + ::MulDiv(clientW, col + 0, colCount);
				rc.right = clientX + ::MulDiv(clientW, col + 1, colCount);
				rc.top = clientY + ::MulDiv(clientH, row + 0, rowCount);
				rc.bottom = clientY + ::MulDiv(clientH, row + 1, rowCount);
			}

			g_buttonRectArray[sceneIndex] = rc;
		}
	}

	if (g_fixedSize)
	{
		RECT rc = clientRect;
		rc.right = rc.left + g_buttonWidth * colCount;
		rc.bottom = rc.top + g_buttonHeight * rowCount;
		::MapWindowPoints(hwnd, 0, (LPPOINT)&rc, 2);
		clientToWindow(hwnd, &rc);

		::SetWindowPos(hwnd, 0, 0, 0, getWidth(rc), getHeight(rc), SWP_NOZORDER | SWP_NOMOVE | SWP_NOACTIVATE);
	}
}

//--------------------------------------------------------------------

int hitTest(HWND hwnd, POINT point)
{
	for (int i = 0; i < (int)g_buttonRectArray.size(); i++)
	{
		if (::PtInRect(&g_buttonRectArray[i], point))
			return i;
	}

	return -1;
}

//--------------------------------------------------------------------

void onPaint(HWND hwnd, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp)
{
	if (!g_auin.GetExEdit())
		return;

	ClientDC _dc(hwnd);
	RECT clientRect; ::GetClientRect(hwnd, &clientRect);
	UxDC dc(_dc, &clientRect);

	if (!dc.isValid())
		return;

	AviUtl::SysInfo si = {};
	fp->exfunc->get_sys_info(editp, &si);
	GdiObjSelector fontSelector(dc, si.hfont);

	{
		// 背景を塗りつぶす。
		HBRUSH brush = (HBRUSH)::SendMessage(hwnd, WM_CTLCOLORDLG, (WPARAM)(HDC)dc, (LPARAM)hwnd);
		::FillRect(dc, &clientRect, brush);
	}

	for (int i = 0; i < (int)g_buttonRectArray.size(); i++)
	{
		int sceneIndex = i;
		const RECT& rc = g_buttonRectArray[i];

		// 描画するテキストを取得する。
		WCHAR text[MAX_PATH] = {};

		ExEdit::SceneSetting* scene = g_auin.GetSceneSetting(sceneIndex);

		if (scene->name)
		{
			::StringCbPrintfW(text, sizeof(text), L"%hs", scene->name);
		}
		else
		{
			if (sceneIndex == 0)
				::StringCbCopyW(text, sizeof(text), L"Root");
			else
				::StringCbPrintfW(text, sizeof(text), L"%d", sceneIndex);
		}

		// パートとステートを取得する。
		int partId = BP_PUSHBUTTON;
		int stateId = PBS_NORMAL;

		if (sceneIndex == g_auin.GetCurrentSceneIndex())
		{
			stateId = PBS_PRESSED;
		}
		else if (sceneIndex == g_dragScene)
		{
			if (sceneIndex == g_hotScene)
			{
				stateId = PBS_PRESSED;
			}
			else
			{
				stateId = PBS_HOT;
			}
		}
		else if (sceneIndex == g_hotScene)
		{
			stateId = PBS_HOT;
		}

		// テーマを使用して描画する。
		::DrawThemeBackground(g_themeButton, dc, partId, stateId, &rc, 0);
		::DrawThemeText(g_themeButton, dc, partId, stateId,
			text, ::lstrlenW(text), DT_CENTER | DT_VCENTER | DT_SINGLELINE, 0, &rc);
	}
}

//--------------------------------------------------------------------

void onContextMenu(HWND hwnd)
{
	MY_TRACE(_T("onContextMenu(0x%08X)\n"), hwnd);

	POINT cursorPos; ::GetCursorPos(&cursorPos);

	HMENU menu = ::CreatePopupMenu();

	::AppendMenu(menu, MF_STRING, CommandID::CONFIG, _T("設定"));

	int id = ::TrackPopupMenu(menu, TPM_NONOTIFY | TPM_RETURNCMD, cursorPos.x, cursorPos.y, 0, hwnd, 0);

	switch (id)
	{
	case CommandID::CONFIG:
		{
			onConfigDialog(hwnd);

			break;
		}
	}

	::DestroyMenu(menu);
}

//--------------------------------------------------------------------

void onConfigDialog(HWND hwnd)
{
	MY_TRACE(_T("onConfigDialog()\n"));

	ConfigDialog dialog(hwnd);

	::SendDlgItemMessage(dialog, IDC_ROW_COUNT_SPIN, UDM_SETRANGE32, 1, 50);
	::SendDlgItemMessage(dialog, IDC_COL_COUNT_SPIN, UDM_SETRANGE32, 1, 50);
	::SendDlgItemMessage(dialog, IDC_SCENE_COUNT_SPIN, UDM_SETRANGE32, 1, 50);
	::SendDlgItemMessage(dialog, IDC_VOICE_SPIN, UDM_SETRANGE32, 0, 10);

	HWND hwndLayoutMode = ::GetDlgItem(dialog, IDC_LAYOUT_MODE);
	ComboBox_AddString(hwndLayoutMode, _T("垂直方向"));
	ComboBox_AddString(hwndLayoutMode, _T("水平方向"));
	ComboBox_SetCurSel(hwndLayoutMode, g_layoutMode);
	::SetDlgItemInt(dialog, IDC_ROW_COUNT, g_rowCount, FALSE);
	::SetDlgItemInt(dialog, IDC_COL_COUNT, g_colCount, FALSE);
	::SetDlgItemInt(dialog, IDC_SCENE_COUNT, g_sceneCount, FALSE);
	::SetDlgItemInt(dialog, IDC_VOICE, g_voice, FALSE);
	HWND hwndFixedSize = ::GetDlgItem(dialog, IDC_FIXED_SIZE);
	Button_SetCheck(hwndFixedSize, g_fixedSize);
	::SetDlgItemInt(dialog, IDC_BUTTON_WIDTH, g_buttonWidth, FALSE);
	::SetDlgItemInt(dialog, IDC_BUTTON_HEIGHT, g_buttonHeight, FALSE);

	int retValue = dialog.doModal();

	if (IDOK != retValue)
		return;

	g_layoutMode = ComboBox_GetCurSel(hwndLayoutMode);
	g_rowCount = ::GetDlgItemInt(dialog, IDC_ROW_COUNT, 0, FALSE);
	g_colCount = ::GetDlgItemInt(dialog, IDC_COL_COUNT, 0, FALSE);
	g_sceneCount = ::GetDlgItemInt(dialog, IDC_SCENE_COUNT, 0, FALSE);
	g_voice = ::GetDlgItemInt(dialog, IDC_VOICE, 0, FALSE);
	g_fixedSize = Button_GetCheck(hwndFixedSize);
	g_buttonWidth = ::GetDlgItemInt(dialog, IDC_BUTTON_WIDTH, 0, FALSE);
	g_buttonHeight = ::GetDlgItemInt(dialog, IDC_BUTTON_HEIGHT, 0, FALSE);

	calcLayout(hwnd);

	::InvalidateRect(hwnd, 0, FALSE);
}

//--------------------------------------------------------------------

void loadConfig()
{
	MY_TRACE(_T("loadConfig()\n"));

	WCHAR fileName[MAX_PATH] = {};
	::GetModuleFileNameW(g_instance,  fileName, MAX_PATH);
	::PathRenameExtensionW(fileName, L".ini");
	MY_TRACE_WSTR(fileName);

	getPrivateProfileInt(fileName, L"Config", L"layoutMode", g_layoutMode);
	getPrivateProfileInt(fileName, L"Config", L"rowCount", g_rowCount);
	getPrivateProfileInt(fileName, L"Config", L"colCount", g_colCount);
	getPrivateProfileInt(fileName, L"Config", L"sceneCount", g_sceneCount);
	getPrivateProfileInt(fileName, L"Config", L"voice", g_voice);
	getPrivateProfileBool(fileName, L"Config", L"fixedSize", g_fixedSize);
	getPrivateProfileInt(fileName, L"Config", L"buttonWidth", g_buttonWidth);
	getPrivateProfileInt(fileName, L"Config", L"buttonHeight", g_buttonHeight);
}

void saveConfig()
{
	MY_TRACE(_T("saveConfig()\n"));

	WCHAR fileName[MAX_PATH] = {};
	::GetModuleFileNameW(g_instance,  fileName, MAX_PATH);
	::PathRenameExtensionW(fileName, L".ini");
	MY_TRACE_WSTR(fileName);

	setPrivateProfileInt(fileName, L"Config", L"layoutMode", g_layoutMode);
	setPrivateProfileInt(fileName, L"Config", L"rowCount", g_rowCount);
	setPrivateProfileInt(fileName, L"Config", L"colCount", g_colCount);
	setPrivateProfileInt(fileName, L"Config", L"sceneCount", g_sceneCount);
	setPrivateProfileInt(fileName, L"Config", L"voice", g_voice);
	setPrivateProfileBool(fileName, L"Config", L"fixedSize", g_fixedSize);
	setPrivateProfileInt(fileName, L"Config", L"buttonWidth", g_buttonWidth);
	setPrivateProfileInt(fileName, L"Config", L"buttonHeight", g_buttonHeight);
}

//--------------------------------------------------------------------
