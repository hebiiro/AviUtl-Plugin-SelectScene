#include "pch.h"
#include "ConfigDialog.h"
#include "SelectScene.h"

//---------------------------------------------------------------------

ConfigDialog::ConfigDialog(HWND hwnd)
	: Dialog(g_instance, MAKEINTRESOURCE(IDD_CONFIG), hwnd)
{
}

void ConfigDialog::onOK()
{
	Dialog::onOK();
}

void ConfigDialog::onCancel()
{
	Dialog::onCancel();
}

INT_PTR ConfigDialog::onDlgProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_NOTIFY:
		{
			if (wParam == IDC_VOICE_SPIN)
			{
				NMUPDOWN* nm = (NMUPDOWN*)lParam;

				playVoice(nm->iPos + nm->iDelta);
			}

			break;
		}
	}

	return Dialog::onDlgProc(hwnd, message, wParam, lParam);
}

//---------------------------------------------------------------------
