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
extern HINSTANCE g_instance;
extern HTHEME g_themeWindow;
extern HTHEME g_themeButton;
extern int g_hotScene;
extern int g_dragScene;
extern std::vector<RECT> g_buttonRectArray;

extern int g_layoutMode;
extern int g_rowCount;
extern int g_colCount;
extern int g_sceneCount;
extern int g_voice;
extern BOOL g_fixedSize;
extern int g_buttonWidth;
extern int g_buttonHeight;

//--------------------------------------------------------------------

inline BOOL isSceneIndexValid(int sceneIndex)
{
	return sceneIndex >= 0 && sceneIndex < g_sceneCount;
}

void playVoice(int voice);

void calcLayout(HWND hwnd, BOOL onSize = FALSE);
void calcLayoutVert(HWND hwnd);
void calcLayoutHorz(HWND hwnd);

int hitTest(HWND hwnd, POINT point);

void onPaint(HWND hwnd, AviUtl::EditHandle* editp, AviUtl::FilterPlugin* fp);
void onContextMenu(HWND hwnd);
void onConfigDialog(HWND hwnd);

void loadConfig();
void saveConfig();

//--------------------------------------------------------------------
