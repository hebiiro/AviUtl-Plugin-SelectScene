#include "pch.h"
#include "SelectScene.h"

//--------------------------------------------------------------------

BOOL func_init(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("func_init()\n"));

	// 拡張編集関連のアドレスを取得する。
	g_auin.initExEditAddress();

	return TRUE;
}

BOOL func_exit(AviUtl::FilterPlugin* fp)
{
	MY_TRACE(_T("func_exit()\n"));

	return TRUE;
}

BOOL func_WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp)
{
//	MY_TRACE(_T("func_WndProc(0x%08X, 0x%08X, 0x%08X)\n"), message, wParam, lParam);

	switch (message)
	{
	case AviUtl::FilterPlugin::WindowMessage::Init:
		{
			MY_TRACE(_T("func_WndProc(Init, 0x%08X, 0x%08X)\n"), wParam, lParam);

			g_themeWindow = ::OpenThemeData(hwnd, VSCLASS_WINDOW);
			MY_TRACE_HEX(g_themeWindow);

			g_themeButton = ::OpenThemeData(hwnd, VSCLASS_BUTTON);
			MY_TRACE_HEX(g_themeButton);

			calcLayout(hwnd);

			// 再描画する。
			::InvalidateRect(hwnd, 0, FALSE);

			break;
		}
	case AviUtl::FilterPlugin::WindowMessage::Exit:
		{
			MY_TRACE(_T("func_WndProc(Exit, 0x%08X, 0x%08X)\n"), wParam, lParam);

			// 設定を保存する。
			saveConfig();

			::CloseThemeData(g_themeWindow), g_themeWindow = 0;
			::CloseThemeData(g_themeButton), g_themeButton = 0;

			break;
		}
	case AviUtl::FilterPlugin::WindowMessage::FileOpen:
		{
			MY_TRACE(_T("func_WndProc(FileOpen, 0x%08X, 0x%08X)\n"), wParam, lParam);

			// 再描画する。
			::InvalidateRect(hwnd, 0, FALSE);

			break;
		}
	case AviUtl::FilterPlugin::WindowMessage::Command:
		{
			MY_TRACE(_T("func_WndProc(Command, 0x%08X, 0x%08X)\n"), wParam, lParam);

			if (wParam == 0 && lParam == 0) return TRUE; // 再描画する。

			break;
		}
	case WM_SIZE:
		{
			MY_TRACE(_T("func_WndProc(WM_SIZE, 0x%08X, 0x%08X)\n"), wParam, lParam);

			calcLayout(hwnd, TRUE);

			break;
		}
	case WM_PAINT:
		{
			MY_TRACE(_T("func_WndProc(WM_PAINT, 0x%08X, 0x%08X)\n"), wParam, lParam);

			onPaint(hwnd, editp, fp);

			break;
		}
	case WM_LBUTTONDOWN:
		{
			MY_TRACE(_T("func_WndProc(WM_LBUTTONDOWN, 0x%08X, 0x%08X)\n"), wParam, lParam);

			// マウスカーソルの座標を取得する。
			POINT point = LP2PT(lParam);

			// ドラッグを開始するシーンを取得する。
			g_dragScene = hitTest(hwnd, point);
			MY_TRACE_INT(g_dragScene);

			// ドラッグシーンが無効なら
			if (!isSceneIndexValid(g_dragScene))
				break; // 何もしない。

			// マウスキャプチャを開始する。
			::SetCapture(hwnd);

			// 再描画する。
			onPaint(hwnd, editp, fp);

			break;
		}
	case WM_LBUTTONUP:
		{
			MY_TRACE(_T("func_WndProc(WM_LBUTTONUP, 0x%08X, 0x%08X)\n"), wParam, lParam);

			// マウスカーソルの座標を取得する。
			POINT point = LP2PT(lParam);

			// マウスキャプチャ中なら
			if (::GetCapture() == hwnd)
			{
				// マウスキャプチャを終了する。
				::ReleaseCapture();

				// ホットシーンを取得する。
				g_hotScene = hitTest(hwnd, point);
				MY_TRACE_INT(g_hotScene);

				// ドラッグシーンとホットシーンが同じなら
				if (g_dragScene == g_hotScene)
				{
					// ドラッグシーンが有効かつ現在のシーンと違うなら
					if (isSceneIndexValid(g_dragScene) && g_dragScene != g_auin.GetCurrentSceneIndex())
					{
						playVoice(g_voice);

						// ボタンが押されたのでシーンを変更する。
						g_auin.SetScene(g_dragScene, g_auin.GetFilter(fp, "拡張編集"), editp);

						// AviUtl のプレビューウィンドウを再描画する。
						::PostMessage(hwnd, AviUtl::FilterPlugin::WindowMessage::Command, 0, 0);
					}
				}

				// ドラッグシーンを初期値に戻す。
				g_dragScene = -1;

				// 再描画する。
				onPaint(hwnd, editp, fp);
			}

			break;
		}
	case WM_MOUSEMOVE:
		{
//			MY_TRACE(_T("func_WndProc(WM_MOUSEMOVE, 0x%08X, 0x%08X)\n"), wParam, lParam);

			// マウスカーソルの座標を取得する。
			POINT point = LP2PT(lParam);

			if (::GetCapture() == hwnd)
			{
				// マウス座標にあるシーンを取得する。
				int scene = hitTest(hwnd, point);

				// ドラッグシーンとマウス座標にあるシーンが異なるなら
				if (g_dragScene != scene)
					scene = -1; // マウス座標にあるシーンを無効にする。

				// ホットシーンとマウス座標にあるシーンが異なるなら
				if (g_hotScene != scene)
				{
					// ホットシーンを更新する。
					g_hotScene = scene;

					// 再描画する。
					onPaint(hwnd, editp, fp);
				}
			}
			else
			{
				// マウス座標にあるシーンを取得する。
				int scene = hitTest(hwnd, point);

				// ホットシーンとマウス座標にあるシーンが異なるなら
				if (g_hotScene != scene)
				{
					// ホットシーンを更新する。
					g_hotScene = scene;

					// ホットシーンが有効かつマウスキャプチャ中でないなら
					if (g_hotScene >= 0)
					{
						// WM_MOUSELEAVE が発行されるようにする。
						TRACKMOUSEEVENT tme = { sizeof(tme) };
						tme.dwFlags = TME_LEAVE;
						tme.hwndTrack = hwnd;
						::TrackMouseEvent(&tme);
					}

					// 再描画する。
					onPaint(hwnd, editp, fp);
				}
			}

			break;
		}
	case WM_MOUSELEAVE:
		{
//			MY_TRACE(_T("func_WndProc(WM_MOUSELEAVE, 0x%08X, 0x%08X)\n"), wParam, lParam);

			// ホットシーンが有効なら
			if (g_hotScene >= 0)
			{
				// ホットシーンを初期値に戻す。
				g_hotScene = -1;

				// 再描画する。
				onPaint(hwnd, editp, fp);
			}

			break;
		}
	case WM_RBUTTONUP:
		{
			MY_TRACE(_T("func_WndProc(WM_RBUTTONUP, 0x%08X, 0x%08X)\n"), wParam, lParam);

			onContextMenu(hwnd);

			break;
		}
	}

	return FALSE;
}

EXTERN_C BOOL APIENTRY DllMain(HINSTANCE instance, DWORD reason, LPVOID reserved)
{
	switch (reason)
	{
	case DLL_PROCESS_ATTACH:
		{
			// ロケールを設定する。
			// これをやらないと日本語テキストが文字化けするので最初に実行する。
			_tsetlocale(LC_CTYPE, _T(""));

			MY_TRACE(_T("DLL_PROCESS_ATTACH\n"));

			// この DLL のハンドルをグローバル変数に保存しておく。
			g_instance = instance;
			MY_TRACE_HEX(g_instance);

			break;
		}
	case DLL_PROCESS_DETACH:
		{
			MY_TRACE(_T("DLL_PROCESS_DETACH\n"));

			break;
		}
	}

	return TRUE;
}

EXTERN_C AviUtl::FilterPluginDLL* CALLBACK GetFilterTable()
{
	MY_TRACE(_T("GetFilterTable()\n"));

	// 設定を読み込む。
	loadConfig();

	LPCSTR name = "シーン簡単選択";
	LPCSTR information = "シーン簡単選択 2.1.0 by 蛇色";

	static AviUtl::FilterPluginDLL filter =
	{
		.flag =
			AviUtl::FilterPluginDLL::Flag::AlwaysActive |
			AviUtl::FilterPluginDLL::Flag::DispFilter |
			AviUtl::FilterPluginDLL::Flag::WindowThickFrame |
			AviUtl::FilterPluginDLL::Flag::WindowSize |
			AviUtl::FilterPluginDLL::Flag::ExInformation,
		.x = 400,
		.y = 400,
		.name = name,
		.func_init = func_init,
		.func_exit = func_exit,
		.func_WndProc = func_WndProc,
		.information = information,
	};

	if (g_fixedSize)
	{
		// 固定サイズモードのときはサイズ変更フレームを外す。
		filter.flag &= ~AviUtl::FilterPluginDLL::Flag::WindowThickFrame;
	}

	return &filter;
}

//--------------------------------------------------------------------
