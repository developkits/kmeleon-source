/*
*  Copyright (C) 2000 Brian Harris
*
*  This program is free software; you can redistribute it and/or modify
*  it under the terms of the GNU General Public License as published by
*  the Free Software Foundation; either version 2, or (at your option)
*  any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
*/

// functions exposed to kmeleon
int  Init();
void Create(HWND parent);
void Config(HWND parent);
void Quit();
void DoMenu(HMENU menu, char *param);
void DoRebar(HWND rebarWnd);
int  DoAccel(char *param);
void Save(const char *file);

extern "C" KMELEON_PLUGIN int DrawBitmap(DRAWITEMSTRUCT *dis);

// Window Procs
BOOL CALLBACK DlgProc (HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
CALLBACK EditProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

// Utils
BOOL BrowseForBookmarks(char *file);

void ParseBookmarks(char *bmFileBuffer, CBookmarkNode &node);
void BuildMenu(HMENU menu, CBookmarkNode *node, BOOL isContinuation);
void BuildRebar();
void Rebuild();

#define MENU_TO_COMMAND(x) (x+SUBMENU_OFFSET)
#define COMMAND_TO_MENU(x) (HMENU)(x-SUBMENU_OFFSET)

// Global vars

extern HIMAGELIST gImagelist;

extern HMENU gMenuBookmarks;
extern HWND ghWndTB;

extern UINT nConfigCommand;
extern UINT nAddCommand;
extern UINT nEditCommand;
extern UINT nDropdownCommand;
extern UINT nFirstBookmarkPosition;

extern CHAR gBookmarkFile[];
extern CHAR gToolbarFolder[];
extern CHAR gBookmarksTitle[];

extern BOOL gBookmarksModified;
extern BOOL gGeneratedByUs;
extern BOOL gToolbarEnabled;
extern int gMaxMenuLength;
extern int gMaxTBSize;

extern CBookmarkNode gBookmarkRoot;

extern kmeleonPlugin kPlugin;
