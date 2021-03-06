/*
*  Copyright (C) 2006 Dorian Boissonnade
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
//  Holds various preferences for k-meleon. also has the getters/setters

#include "StdAfx.h"
#include "kmeleonConst.h"
#include "kmeleon_plugin.h"

#include "MfcEmbed.h"
#include "BrowserFrmTab.h"
extern CMfcEmbedApp theApp;

#include "Preferences.h"
#include "nsDirectoryServiceUtils.h"
#include "nsIStyleSheetService.h"
#include "nsIIOService.h"
#include "MozUtils.h"


	CPrefString::operator CString() {
		return theApp.preferences.GetString(GetPrefName(), def);
	}

	CPrefString::operator LPCTSTR() {
		val = theApp.preferences.GetString(GetPrefName(), def);
		return val;
	}

	CString CPrefString::operator =(LPCTSTR s) {
		theApp.preferences.SetString(GetPrefName(), s);
		return s;
	}

	CPrefInt::operator int() {
		return theApp.preferences.GetInt(GetPrefName(), def);
	}

	int CPrefInt::operator =(int i) {
		theApp.preferences.SetInt(GetPrefName(), i);
		return i;
	}

	CPrefBool::operator BOOL() {
		return theApp.preferences.GetBool(GetPrefName(), def);
	}

	BOOL CPrefBool::operator =(BOOL b) {
		theApp.preferences.SetBool(GetPrefName(), b);
		return b;
	}

CString MakeAbsolutePath(LPCTSTR root, LPCTSTR path)
{
   // Need probably to remove ".." correctly 
   CString absPath = (*path=='/' || _tcschr(path, ':')) ? path : CString(root) + _T('\\') + path;

   // Remove slash at the end if any.
   if (absPath.GetAt(absPath.GetLength()-1) == '\\') {
      absPath.GetBuffer(0);
      absPath.ReleaseBuffer(absPath.GetLength()-1);
   }

   return absPath;
}

typedef void (CPreferences::*PrefChangedMethod)();

#include "nsiPrefBranch2.h"

class CPrefObserver : public nsIObserver
{
	PrefChangedMethod m_method;

public:
	
	
	CPrefObserver(char* prefname, PrefChangedMethod method)
	{
		nsCOMPtr<nsIPrefBranch2> pb = do_GetService(NS_PREFSERVICE_CONTRACTID);
		NS_ENSURE_TRUE(pb, );
	    pb->AddObserver(prefname, this, PR_FALSE);
		m_method = method;
	}

	~CPrefObserver() {}

	NS_DECL_ISUPPORTS;
	NS_DECL_NSIOBSERVER;

};

	NS_IMETHODIMP CPrefObserver::Observe(
		nsISupports* aSubject, const char* aTopic, const PRUnichar* aSomeData)
	{
		if (strcmp(aTopic, "nsPref:changed") != 0)
			return NS_OK;  

		(theApp.preferences.*m_method)();
		return NS_OK;
	}


NS_IMPL_ISUPPORTS(CPrefObserver, nsIObserver);

CPreferences::CPreferences() :
    iSaveType("kmeleon.general.saveType", 0),
    saveDir("kmeleon.download.saveDir", _T("")),   
    downloadDir("browser.download.dir", _T("")),
    lastDownloadDir("browser.download.lastDir", _T("")),
    bUseDownloadDir("browser.download.useDownloadDir", false),
    bAskOpenSave("kmeleon.download.askOpenSave", true),
    bShowMinimized("kmeleon.download.showMinimizedDialog", false),
    bFlashWhenCompleted("kmeleon.download.flashWhenCompleted", false),
    bCloseDownloadDialog("kmeleon.download.closeDownloadDialog", false),
	bSaveUseTitle("kmeleon.download.saveUseTitle", true),

    bMaximized("kmeleon.display.maximized", true),
    windowWidth("kmeleon.display.width", -1),
    windowHeight("kmeleon.display.height", -1),
    windowXPos("kmeleon.display.XPos", -1),
    windowYPos("kmeleon.display.YPos", -1),

    bFindMatchCase("kmeleon.find.matchCase", false),
    bFindHighlight("kmeleon.find.highlight", false),
    bFindSearchBackwards("kmeleon.find.searchBackwards", false),
    bFindWrapAround("kmeleon.find.wrapAround", true),

	MRUbehavior("kmeleon.MRU.behavior", 1),
	
	bOffline("kmeleon.general.offline", false),
	bGuestAccount("kmeleon.general.guest_account", false),

	bSiteIcons("kmeleon.favicons.show", PR_TRUE),
	bDisableResize("kmeleon.display.disableResize", false),
    bHideTaskBarButtons("kmeleon.display.hideTaskBarButtons", false),
	bToolbarBackground("kmeleon.display.backgroundImageEnabled", true),

	bStartHome("kmeleon.general.startHome", true),
	iNewWindowOpenAs("kmeleon.display.newWindowOpenAs", 0),
    newWindowURL("kmeleon.display.newWindowURL", _T("")),


//    searchEngine("kmeleon.general.searchEngine", _T("http://www.google.com/search?q=")),

    bSourceUseExternalCommand("kmeleon.general.sourceEnabled", false),
    sourceCommand("kmeleon.general.sourceCommand", _T("")),
	bNewWindowHasUrlFocus("kmeleon.display.NewWindowHasUrlFocus", false),
	bAutoHideTabControl("browser.tabs.autoHide", true),
	iTabOnMiddleClick("kmeleon.tabs.OnMiddleClick", 0),
	iTabOnDoubleClick("kmeleon.tabs.OnDoubleClick", 0),
	iTabOnRightClick("kmeleon.tabs.OnRightClick", 2),
	iOnCloseLastTab("kmeleon.tabs.onCloseLast", 1),
	iOnCloseTab("kmeleon.tabs.onCloseOption", 0),
	iOnOpenTab("kmeleon.tabs.onOpenOption", 1)
{
}

CPreferences::~CPreferences() {
}



void CPreferences::LocaleChanged()
{
	if (theApp.LoadLanguage())
		theApp.menus.RebuildAll();
	USES_CONVERSION;
	CString locale = GetString("general.useragent.locale", _T("en-US"));
	theApp.plugins.SendMessage("*", "*", "DoLocale", (long)T2CA(locale), 0);
}

void CPreferences::MRUListChanged()
{
	theApp.m_MRUList->RefreshURLs();
}

void CPreferences::BackgroundChanged()
{
	POSITION pos = theApp.m_FrameWndLst.GetHeadPosition();
	while( pos != NULL ) {
		CBrowserFrame* pBrowserFrame = (CBrowserFrame *) theApp.m_FrameWndLst.GetNext(pos);
		pBrowserFrame->SetBackImage();
		pBrowserFrame->m_wndReBar.RedrawWindow(0, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
	}
}

void CPreferences::SkinChanged()
{
	skinsCurrent = GetString("kmeleon.general.skinsCurrent", skinsCurrent);
	if (!theApp.skin.Init(skinsCurrent))
		return;

	CBrowserFrame* pBrowserFrame = NULL;
	POSITION pos = theApp.m_FrameWndLst.GetHeadPosition();
	while( pos != NULL ) {
		pBrowserFrame = (CBrowserFrame *) theApp.m_FrameWndLst.GetNext(pos);
		pBrowserFrame->SetBackImage();
		pBrowserFrame->m_wndReBar.RedrawWindow(0, 0, RDW_INVALIDATE | RDW_UPDATENOW | RDW_ERASE | RDW_ALLCHILDREN);
		if (pBrowserFrame->IsKindOf(RUNTIME_CLASS(CBrowserFrmTab))) {

		}
		/*pBrowserFrame->m_wndReBar.Invalidate();
		pBrowserFrame->m_wndReBar.UpdateWindow();
		CReBarCtrl& rc = pBrowserFrame->m_wndReBar.GetReBarCtrl ();

		REBARBANDINFO info;
		memset (&info, 0, sizeof (REBARBANDINFO));
		info.cbSize = sizeof (info);
		info.fMask = RBBIM_CHILD;

		int count = rc.GetBandCount();
		for (int i = 0; i < count; i++)  {
			CRect rectBand;
			rc.GetRect (i, rectBand);
			rc.InvalidateRect (rectBand);
			rc.UpdateWindow ();		
			rc.GetBandInfo (i, &info);
			if (info.hwndChild != NULL)
			{
				::InvalidateRect (info.hwndChild, NULL, TRUE);
				::UpdateWindow (info.hwndChild);
			}
		}*/
	}

	/*CString skinFile;
	CString skin = GetString("kmeleon.general.skinsCurrent", skinsCurrent);
	theApp.FindSkinFile(skinFile, _T("skin.js"), skin, false);
	if (!skinFile.IsEmpty()) 
	{
		//CString defaultDir = ::GetMozDirectory(NS_APP_DEFAULTS_50_DIR);
		//if (defaultDir.GetLength())  
		//	CopyFile(skinFile, defaultDir + _T("skin.js"), FALSE);
		CopyFile(skinFile, profileFolder + _T("\\skin.js"), FALSE);
	}*/
}

BOOL LoadStyleSheet(nsIURI* uri, BOOL load)
{
  nsCOMPtr<nsIStyleSheetService> ssService = do_GetService("@mozilla.org/content/style-sheet-service;1");
  NS_ENSURE_TRUE(ssService, FALSE);

  nsresult rv = NS_OK;
  bool alreadyRegistered = PR_FALSE;
  ssService->SheetRegistered(uri, nsIStyleSheetService::USER_SHEET, &alreadyRegistered);
  if (alreadyRegistered)
    rv = ssService->UnregisterSheet(uri, nsIStyleSheetService::USER_SHEET);
  
  if (load)
    rv = ssService->LoadAndRegisterSheet(uri, nsIStyleSheetService::USER_SHEET);

  return NS_SUCCEEDED(rv);
}

BOOL LoadStyleSheet(LPCTSTR path, BOOL load)
{
	nsresult rv;
	nsCOMPtr<nsIFile> adfile;
#ifdef _UNICODE
	rv = NS_NewLocalFile(nsDependentString(path), TRUE, getter_AddRefs(adfile));
#else
	rv = NS_NewNativeLocalFile(nsDependentCString(path), TRUE, getter_AddRefs(adfile));
#endif
	NS_ENSURE_SUCCESS(rv, FALSE);

	nsCOMPtr<nsIIOService> ios = do_GetService("@mozilla.org/network/io-service;1");
	NS_ENSURE_TRUE(ios, FALSE);

	nsCOMPtr<nsIURI> uri;
	rv = ios->NewFileURI(adfile, getter_AddRefs(uri));
	NS_ENSURE_SUCCESS(rv, FALSE);
 
	return LoadStyleSheet(uri, load);
}

void LoadAdBlock(BOOL load)
{
	CString adblockpath = theApp.GetFolder(ProfileFolder) + _T("\\chrome\\adblock.css");
	LoadStyleSheet(adblockpath, load);
}

void LoadFlashBlock(BOOL load)
{
	nsCOMPtr<nsIURI> fburi;
	nsresult rv = NewURI(getter_AddRefs(fburi), nsCString("chrome://flashblock/content/flashblock.css"));
	NS_ENSURE_SUCCESS(rv, ); 

	LoadStyleSheet(fburi, load);
}

void CPreferences::FlashBlockChanged()
{
	LoadFlashBlock(GetBool("kmeleon.flashblock", false));
}

void CPreferences::AdBlockChanged()
{
	LoadAdBlock(GetBool("kmeleon.adblocking", false));
}

void CPreferences::MenuChanged()
{
	theApp.menus.RebuildAll();
}

void CPreferences::Release() {
	m_prefservice = nullptr;
	m_prefs = nullptr;
}

void CPreferences::Load() {

   nsresult rv;
   m_prefservice = do_GetService(NS_PREFSERVICE_CONTRACTID, &rv);
   if (!m_prefservice) return;

   //m_prefservice->ReadUserPrefs(nullptr); // Done in XRE_NotifyProfile
   m_prefservice->GetBranch("", getter_AddRefs(m_prefs));
   if (NS_FAILED(rv)) {
      _ASSERTE(m_prefs && "Could not get preferences service");
      return;
   }

   bool inited;
   rv = m_prefs->GetBoolPref("kmeleon.prefs_inited", &inited);
   if (NS_FAILED(rv) || !inited) {
      // Set up prefs for first run
      rv = m_prefs->SetBoolPref("kmeleon.prefs_inited", PR_TRUE);
      rv = m_prefservice->SavePrefFile(nullptr);
   }

   // Well, since the observer own them, and release them at shutdown, we don't
   // have to care about them.
   new CPrefObserver("general.useragent.locale", &CPreferences::LocaleChanged);
   new CPrefObserver("kmeleon.flashblock", &CPreferences::FlashBlockChanged);
   new CPrefObserver("kmeleon.adblocking", &CPreferences::AdBlockChanged);
   new CPrefObserver("kmeleon.MRU", &CPreferences::MRUListChanged);
   new CPrefObserver("kmeleon.general.skinsCurrent", &CPreferences::SkinChanged);
   new CPrefObserver("kmeleon.display.backgroundImageEnabled", &CPreferences::BackgroundChanged);
   new CPrefObserver("kmeleon.display.backgroundImage", &CPreferences::BackgroundChanged);
   new CPrefObserver("kmeleon.display.bitmapInMenus", &CPreferences::MenuChanged);

   // -- Folders XXX have to put this somewhere else
   
   settingsDir = GetString("kmeleon.general.settingsDir", _T(""));
   pluginsDir = GetString("kmeleon.general.pluginsDir", _T(""));

   skinsDir = GetString("kmeleon.general.skinsDir", _T(""));
   skinsCurrent = GetString("kmeleon.general.skinsCurrent",  _T(""));

   profileFolder = GetMozDirectory(NS_APP_USER_PROFILE_50_DIR);
   settingsFolder = GetMozDirectory(NS_APP_DEFAULTS_50_DIR) + _T("\\Settings");
   CString rootFolder = theApp.GetFolder(RootFolder);

   userSettingsFolder = settingsDir.IsEmpty() ?
      profileFolder :
      MakeAbsolutePath(rootFolder, settingsDir);
   
   pluginsFolder = pluginsDir.IsEmpty() ?
      rootFolder + _T("\\kplugins") :
      MakeAbsolutePath(rootFolder, pluginsDir);

   userPluginsFolder = profileFolder + _T("\\kplugins"); 
  
   skinsFolder = skinsDir.IsEmpty() ?
      rootFolder + _T("\\skins") :
      MakeAbsolutePath(rootFolder, skinsDir);

   userSkinsFolder = profileFolder + _T("\\skins");
   resFolder = GetMozDirectory(NS_APP_RES_DIR);

   // XXX
   currentSkinFolder = userSkinsFolder + _T("\\") + skinsCurrent;
   WIN32_FIND_DATA FindData;
   HANDLE hFile = FindFirstFile(currentSkinFolder, &FindData);
	if(hFile != INVALID_HANDLE_VALUE)
      FindClose(hFile);
   else
      currentSkinFolder = skinsFolder + _T("\\") + skinsCurrent;
   
   CString skin;
   if (theApp.skin.FindSkinFile(skin, _T("skin.js"), skinsCurrent)) {
	  nsCOMPtr<nsIFile> file;
      NS_NewLocalFile(CStringToNSString(skin), false, getter_AddRefs(file));   
	  m_prefservice->SavePrefFile(nullptr);
      m_prefservice->ReadUserPrefs(file);
      m_prefservice->ReadUserPrefs(nullptr);
   }

   FlashBlockChanged();
   AdBlockChanged();
}

void CPreferences::Flush()
{
   if (!m_prefservice) return;
   m_prefservice->SavePrefFile(nullptr);      
}

int CPreferences::GetBool(const char *preference, int defaultVal)
{
   if (!m_prefs) return defaultVal;
   bool tempBool;
   nsresult rv = m_prefs->GetBoolPref(preference, &tempBool);
   if (NS_SUCCEEDED(rv))
      return tempBool;
   else
      return defaultVal;
}

int CPreferences::GetInt(const char *preference, int defaultVal)
{
	if (!m_prefs) return defaultVal;
	PRInt32 tempInt;
   nsresult rv = m_prefs->GetIntPref(preference, &tempInt);
   if (NS_SUCCEEDED(rv))
      return tempInt;
   else
      return defaultVal;
}

int CPreferences::GetString(const char *preference, char *retVal, const char *defaultVal)
{
	nsCString string;
    if (!m_prefs || !NS_SUCCEEDED(m_prefs->GetCharPref(preference, getter_Copies(string))))
		string = defaultVal;
		
	if (retVal)
      strcpy(retVal, string.get());
   return string.Length();
}

int CPreferences::GetString(const char *preference, wchar_t *retVal, const wchar_t *defaultVal)
{
	nsCString string;
	nsString unicode;
    if (!m_prefs || !NS_SUCCEEDED(m_prefs->GetCharPref(preference, getter_Copies(string))))
		unicode = defaultVal;
	else
		NS_CStringToUTF16(string, NS_CSTRING_ENCODING_UTF8, unicode);
	
	if (retVal)
      wcscpy(retVal, unicode.get());
   return unicode.Length();
/*
   nsString string;
   if (!m_prefs || !NS_SUCCEEDED(m_prefs->CopyUnicharPref(preference, getter_Copies(string))))
      string = defaultVal;	
   if (retVal)
      wcscpy(retVal, string.get());
   return string.Length();*/
}

CString CPreferences::GetString(const char *preference, LPCTSTR defaultVal)
{
	nsCString string;
    if (!m_prefs || !NS_SUCCEEDED(m_prefs->GetCharPref(preference, getter_Copies(string))))
		return defaultVal;
	return NSUTF8StringToCString(string);

  /* if (!m_prefs || !NS_SUCCEEDED(m_prefs->CopyUnicharPref(preference, getter_Copies(string))))
		return defaultVal;
	USES_CONVERSION;
	return W2CT(string.get());*/
}

#include "nsIPrefLocalizedString.h"

int CPreferences::GetLocaleString(const char *preference, char *retVal, const char *defaultVal)
{
	nsCString string;
	nsCOMPtr<nsIPrefLocalizedString> prefString;

    if (!m_prefs)
		string = defaultVal;
	else if (NS_SUCCEEDED(m_prefs->GetComplexValue(preference,
								NS_GET_IID(nsIPrefLocalizedString),
								getter_AddRefs(prefString))))
	{
		nsString unicode;
		prefString->GetData(getter_Copies(unicode));
		CopyUTF16toUTF8(unicode, string);
	}
	else if (!NS_SUCCEEDED(m_prefs->GetCharPref(preference, getter_Copies(string))))
		string = defaultVal;
	
	if (retVal)
      strcpy(retVal, string.get());
   return string.Length();
}

CString CPreferences::GetLocaleString(const char *preference, LPCTSTR defaultVal)
{
	nsCString string;
	nsString unicode;
	if (!m_prefs) return defaultVal;

	nsCOMPtr<nsIPrefLocalizedString> prefString;
	nsresult rv = m_prefs->GetComplexValue(preference,
								NS_GET_IID(nsIPrefLocalizedString),
								getter_AddRefs(prefString));

	if (NS_SUCCEEDED(rv)) {
		prefString->GetData(getter_Copies(unicode));
	}
	else {
		if (!NS_SUCCEEDED(m_prefs->GetCharPref(preference, getter_Copies(string))))
			return  defaultVal;
		CopyUTF8toUTF16(string, unicode);
	}

	return NSStringToCString(unicode);
}

inline void CPreferences::SetString(const char *preference, const wchar_t *value)
{
   if (!m_prefs) return;
   nsCString string;
   NS_UTF16ToCString(nsString(value), NS_CSTRING_ENCODING_UTF8, string);
   m_prefs->SetCharPref(preference, string.get());
   
//   m_prefs->SetUnicharPref(preference, value);
}

void CPreferences::SetString(const char *preference, const char *value)
{
   if (!m_prefs) return;
   m_prefs->SetCharPref(preference, value);
   //USES_CONVERSION;
   //SetString(preference, A2CW(value));
}

void CPreferences::Clear(const char *preference)
{
   if (!m_prefs) return;
   m_prefs->ClearUserPref(preference);
}

void CPreferences::DeleteBranch(const char *startingAt)
{
   if (!m_prefs) return;
   m_prefs->DeleteBranch(startingAt);
}
/*
void CPreferences::_GetBool(const char *preference, int& var, int defaultVal)
{
   ASSERT(m_prefs);
   PRBool tempBool;
   nsresult rv = m_prefs->GetBoolPref(preference, &tempBool);
   if (NS_SUCCEEDED(rv))
      var = tempBool;
   else
      var = defaultVal;
}

void CPreferences::_GetInt(const char *preference, int& var, int defaultVal)
{
   ASSERT(m_prefs);
   PRInt32 tempInt;
   nsresult rv = m_prefs->GetIntPref(preference, &tempInt);
   if (NS_SUCCEEDED(rv))
      var = tempInt;
   else
      var = defaultVal;
}

void CPreferences::_GetString(const char *preference, CString& var, LPCTSTR defaultVal)
{
   ASSERT(m_prefs);
   nsString string;
   nsresult rv = m_prefs->CopyUnicharPref(preference, getter_Copies(string));
   if (NS_SUCCEEDED(rv) && defaultVal) {
		USES_CONVERSION;
      var = W2CT(string.get());	
	}
   else if (defaultVal)
      var = defaultVal;
}

void CPreferences::_SetString(const char *preference, LPCTSTR value)
{
   ASSERT(m_prefs);
   USES_CONVERSION;
   m_prefs->SetUnicharPref(preference, T2CW(value));
}*/