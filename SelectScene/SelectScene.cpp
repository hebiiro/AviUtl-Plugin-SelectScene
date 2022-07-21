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
AviUtl::FilterPlugin* g_fp = 0;
HTHEME g_themeWindow = 0;
HTHEME g_themeButton = 0;
int g_hotScene = -1;
int g_dragScene = -1;

int g_layoutMode = LayoutMode::Horz;
int g_rowCount = 10;
int g_colCount = 10;
int g_voice = 1;

//--------------------------------------------------------------------

void playVoice(int voice)
{
	if (voice == 0) return;

	// フォルダ名を取得する。
	TCHAR folderName[MAX_PATH] = {};
	::GetModuleFileName(g_fp->dll_hinst, folderName, MAX_PATH);
	::PathRemoveExtension(folderName);
	MY_TRACE_TSTR(folderName);

	// wav ファイルのパスを取得する。
	TCHAR wavFileName[MAX_PATH] = {};
	::StringCbPrintf(wavFileName, sizeof(wavFileName), _T("%s\\%d.wav"), folderName, voice);
	MY_TRACE_TSTR(wavFileName);

	// ファイルが存在するなら
	if (::GetFileAttributes(wavFileName) != INVALID_FILE_ATTRIBUTES)
	{
		// wav ファイルを再生する。
		::PlaySound(wavFileName, 0, SND_FILENAME | SND_ASYNC);
	}
}

//--------------------------------------------------------------------

int hitTest(HWND hwnd, POINT point)
{
	switch (g_layoutMode)
	{
	case LayoutMode::Vert: return hitTestVert(hwnd, point);
	case LayoutMode::Horz: return hitTestHorz(hwnd, point);
	}

	return -1;
}

int hitTestVert(HWND hwnd, POINT point)
{
	RECT clientRect; ::GetClientRect(hwnd, &clientRect);
	int clientX = clientRect.left;
	int clientY = clientRect.top;
	int clientW = clientRect.right - clientRect.left;
	int clientH = clientRect.bottom - clientRect.top;

	int rowCount = (g_rowCount > 0) ? g_rowCount : 5;
	int colCount = (MaxSceneCount - 1) / rowCount + 1;

	int sceneIndex = 0;

	for (int col = 0; col < colCount; col++)
	{
		for (int row = 0; row < rowCount; row++, sceneIndex++)
		{
			if (sceneIndex >= MaxSceneCount) break;

			RECT rc = {};
			rc.left = clientX + ::MulDiv(clientW, col + 0, colCount);
			rc.right = clientX + ::MulDiv(clientW, col + 1, colCount);
			rc.top = clientY + ::MulDiv(clientH, row + 0, rowCount);
			rc.bottom = clientY + ::MulDiv(clientH, row + 1, rowCount);

			if (::PtInRect(&rc, point))
				return sceneIndex;
		}
	}

	return -1;
}

int hitTestHorz(HWND hwnd, POINT point)
{
	RECT clientRect; ::GetClientRect(hwnd, &clientRect);
	int clientX = clientRect.left;
	int clientY = clientRect.top;
	int clientW = clientRect.right - clientRect.left;
	int clientH = clientRect.bottom - clientRect.top;

	int colCount = (g_colCount > 0) ? g_colCount : 5;
	int rowCount = (MaxSceneCount - 1) / colCount + 1;

	int sceneIndex = 0;

	for (int row = 0; row < rowCount; row++)
	{
		for (int col = 0; col < colCount; col++, sceneIndex++)
		{
			if (sceneIndex >= MaxSceneCount) break;

			RECT rc = {};
			rc.left = clientX + ::MulDiv(clientW, col + 0, colCount);
			rc.right = clientX + ::MulDiv(clientW, col + 1, colCount);
			rc.top = clientY + ::MulDiv(clientH, row + 0, rowCount);
			rc.bottom = clientY + ::MulDiv(clientH, row + 1, rowCount);

			if (::PtInRect(&rc, point))
				return sceneIndex;
		}
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

	switch (g_layoutMode)
	{
	case LayoutMode::Vert: onPaintVert(dc, &clientRect); break;
	case LayoutMode::Horz: onPaintHorz(dc, &clientRect); break;
	}
}

void onPaintVert(HDC dc, LPCRECT clientRect)
{
	int rowCount = (g_rowCount > 0) ? g_rowCount : 5;
	int colCount = (MaxSceneCount - 1) / rowCount + 1;

	int sceneIndex = 0;

	for (int col = 0; col < colCount; col++)
	{
		for (int row = 0; row < rowCount; row++, sceneIndex++)
		{
			if (sceneIndex >= MaxSceneCount) break;

			onPaintButton(dc, clientRect, row, rowCount, col, colCount, sceneIndex);
		}
	}
}

void onPaintHorz(HDC dc, LPCRECT clientRect)
{
	int colCount = (g_colCount > 0) ? g_colCount : 5;
	int rowCount = (MaxSceneCount - 1) / colCount + 1;

	int sceneIndex = 0;

	for (int row = 0; row < rowCount; row++)
	{
		for (int col = 0; col < colCount; col++, sceneIndex++)
		{
			if (sceneIndex >= MaxSceneCount) break;

			onPaintButton(dc, clientRect, row, rowCount, col, colCount, sceneIndex);
		}
	}
}

void onPaintButton(HDC dc, LPCRECT clientRect, int row, int rowCount, int col, int colCount, int sceneIndex)
{
	int clientX = clientRect->left;
	int clientY = clientRect->top;
	int clientW = clientRect->right - clientRect->left;
	int clientH = clientRect->bottom - clientRect->top;

	// 描画矩形を取得する。
	RECT rc = {};
	rc.left = clientX + ::MulDiv(clientW, col + 0, colCount);
	rc.right = clientX + ::MulDiv(clientW, col + 1, colCount);
	rc.top = clientY + ::MulDiv(clientH, row + 0, rowCount);
	rc.bottom = clientY + ::MulDiv(clientH, row + 1, rowCount);

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

	::SendDlgItemMessage(dialog, IDC_ROW_COUNT_SPIN, UDM_SETRANGE32, 0, 50);
	::SendDlgItemMessage(dialog, IDC_COL_COUNT_SPIN, UDM_SETRANGE32, 0, 50);
	::SendDlgItemMessage(dialog, IDC_VOICE_SPIN, UDM_SETRANGE32, 0, 10);

	HWND hwndLayoutMode = ::GetDlgItem(dialog, IDC_LAYOUT_MODE);
	ComboBox_AddString(hwndLayoutMode, _T("垂直方向"));
	ComboBox_AddString(hwndLayoutMode, _T("水平方向"));
	ComboBox_SetCurSel(hwndLayoutMode, g_layoutMode);
	::SetDlgItemInt(dialog, IDC_ROW_COUNT, g_rowCount, FALSE);
	::SetDlgItemInt(dialog, IDC_COL_COUNT, g_colCount, FALSE);
	::SetDlgItemInt(dialog, IDC_VOICE, g_voice, FALSE);

	int retValue = dialog.doModal();

	if (IDOK != retValue)
		return;

	g_layoutMode = ComboBox_GetCurSel(hwndLayoutMode);
	g_rowCount = ::GetDlgItemInt(dialog, IDC_ROW_COUNT, 0, FALSE);
	g_colCount = ::GetDlgItemInt(dialog, IDC_COL_COUNT, 0, FALSE);
	g_voice = ::GetDlgItemInt(dialog, IDC_VOICE, 0, FALSE);

	::InvalidateRect(hwnd, 0, FALSE);
}

//--------------------------------------------------------------------

void loadConfig()
{
	MY_TRACE(_T("loadConfig()\n"));

	WCHAR fileName[MAX_PATH] = {};
	::GetModuleFileNameW(g_fp->dll_hinst,  fileName, MAX_PATH);
	::PathRenameExtensionW(fileName, L".ini");
	MY_TRACE_WSTR(fileName);

	getPrivateProfileInt(fileName, L"Config", L"layoutMode", g_layoutMode);
	getPrivateProfileInt(fileName, L"Config", L"rowCount", g_rowCount);
	getPrivateProfileInt(fileName, L"Config", L"colCount", g_colCount);
	getPrivateProfileInt(fileName, L"Config", L"voice", g_voice);
}

void saveConfig()
{
	MY_TRACE(_T("saveConfig()\n"));

	WCHAR fileName[MAX_PATH] = {};
	::GetModuleFileNameW(g_fp->dll_hinst,  fileName, MAX_PATH);
	::PathRenameExtensionW(fileName, L".ini");
	MY_TRACE_WSTR(fileName);

	setPrivateProfileInt(fileName, L"Config", L"layoutMode", g_layoutMode);
	setPrivateProfileInt(fileName, L"Config", L"rowCount", g_rowCount);
	setPrivateProfileInt(fileName, L"Config", L"colCount", g_colCount);
	setPrivateProfileInt(fileName, L"Config", L"voice", g_voice);
}

//--------------------------------------------------------------------
