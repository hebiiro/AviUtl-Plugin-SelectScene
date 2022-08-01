#pragma once

//--------------------------------------------------------------------

struct CommandID
{
	static const UINT CONFIG = 2022;
};

struct LayoutMode
{
	static const int Vert = 0;
	static const int Horz = 1;
};

static const int MaxSceneCount = 50;

//--------------------------------------------------------------------

extern AviUtlInternal g_auin;
extern AviUtl::FilterPlugin* g_fp;
extern HTHEME g_themeWindow;
extern HTHEME g_themeButton;
extern int g_hotScene;
extern int g_dragScene;

extern int g_layoutMode;
extern int g_rowCount;
extern int g_colCount;
extern int g_sceneCount;
extern int g_voice;

//--------------------------------------------------------------------

inline BOOL isSceneIndexValid(int sceneIndex)
{
	return sceneIndex >= 0 && sceneIndex < g_sceneCount;
}

void playVoice(int voice);

int hitTest(HWND hwnd, POINT point);
int hitTestVert(HWND hwnd, POINT point);
int hitTestHorz(HWND hwnd, POINT point);

void onPaint(HWND hwnd, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);
void onPaintVert(HDC dc, LPCRECT clientRect);
void onPaintHorz(HDC dc, LPCRECT clientRect);
void onPaintButton(HDC dc, LPCRECT clientRect, int row, int rowCount, int col, int colCount, int sceneIndex);
void onContextMenu(HWND hwnd);
void onConfigDialog(HWND hwnd);

void loadConfig();
void saveConfig();

//--------------------------------------------------------------------
