//
// Class Access - a Win32 GUI for the Search library demo app
//

#pragma once

// link with Common Controls v6
#pragma comment(linker,"\"/manifestdependency:type='win32' \
name='Microsoft.Windows.Common-Controls' version='6.0.0.0' \
processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")

#define STRICT
#define OEMRESOURCE
#define NOMINMAX

#include <windows.h>
#include <stdio.h>
#include <winuser.h>
#include <commctrl.h>
#include <cstdlib>
#include <assert.h>
#include <richedit.h>
#include <algorithm>

#include "search.hpp"
#include "resource.h"
#include "dcontext.hpp"
#include "error.hpp"

// Undef MS macro
#undef LoadString

using namespace search;

class Access : public GeneralErrorHandler {

	friend int WINAPI wWinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPWSTR,	_In_ int);

	Access(HINSTANCE, LPWSTR, int);
	Access() = delete;
	~Access();

	// Non-fatal error handler
	virtual void OnError(unsigned, GenErrCode);

	static LRESULT CALLBACK	MainWndProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK	OptionsDlgProc(HWND, UINT, WPARAM, LPARAM);
	static INT_PTR CALLBACK	HelpDlgProc(HWND, UINT, WPARAM, LPARAM);

	void InitApp();
	void CreateToolBar(bool); // bool==false: full-size buttons; ==true: list-style buttons
	void ResizeWnd(int = SW_SHOWNA);
	void LoadString(int, int = 0);
	void LoadEmptyString(int = 0);
	wchar_t* TextBuffer(int = 0);
	bool TestWndSize(int, int, int, int);
	void UpdateTitle(int, int);
	void TimerTick();
	bool ClientToApp(int, int, double&, double&) const;
	bool AppToClient(double, double, int&, int&) const;
	void RedrawWnd(GeneralError* external_err_ptr);
	void RedrawWndWrapper();
	void ProcessNew();
	void ProcessLeftClick(int, int);
	void ProcessContextMenu(int, int);
	void SearchParamDialog();
	void UpdateButtons();
	void ProcessFileOpen();
	void ProcessExport();
	void ProcessMax();
	void RemoveMax();
	void ProcessGraph();
	void ProcessHelp();

	void HelpDlgInit(HWND);

	void OptionsDlgInit(HWND);
	void OptionsDlgCommandCheck(HWND);
	void OptionsDlgCommandOK(HWND);

	void MainWndClose();
	void MainWndDestroy();
	void MainWndNotify(LPARAM);

	void FatalErrorMessage(const GeneralError&);
	void NonFatalErrorMessage(const GeneralError&);
	void OutOfMemoryMessage(const std::bad_alloc&) noexcept;
	void Catch(const GeneralError&) noexcept;
	void CatchBadAlloc(const std::bad_alloc&) noexcept;

	// System-related data members

	const HINSTANCE	hInst;
	const LPWSTR	pCmdLine;
	const int		nCmdShow;
	
	HWND			hMainWnd, hToolBarWnd;
	
	static constexpr auto
					uNumOfButtons{ 7 };

	int				uDrawingAreaWidth, uDrawingAreaHeight, uToolBarWidth;
	static constexpr auto
					dDrawingAreaSizeApp{ 1000.0 };
	static constexpr auto
					uDrawingAreaMargin{ 6 };

	const HCURSOR	hWaitCursor;
	const HMENU		hTopLevelMenu, hContextMenu;

	static constexpr auto
					uTextBufferSize{ 500 }; // check 500?
	wchar_t			szTextBuffer_0[uTextBufferSize + 2];
	wchar_t			szTextBuffer_1[uTextBufferSize + 2];

	static constexpr DWORD
					uFileTitleBufferSize{ 50 };
	wchar_t			szFileTitle[uFileTitleBufferSize + 1];
	
	bool			fFatalErrorOccured;
	bool			fFileIsOpen;

	// Application domain-specific data members

	convex_polygon	CP;
	convex_polygon_pf Graph;

	double			dPFmax;
	convex_polygon::point
					P, Q, Center;
	bool			fArc, fDrawEffPerimeter, fGraph, fBubbles;
	int				uNumSegments;
	double			dVelocityRatio, dRadiusOfDetection;
	bool			fShowFOverWLine;
	static constexpr WORD
					wOptDlgEditBufSize{ 10 };

	// RAII wrapper to manage wait cursor

	class AutoWaitCursor {
		friend class Access;

		explicit AutoWaitCursor(Access&);
		AutoWaitCursor() = delete;
		~AutoWaitCursor();

		HCURSOR	hSaveCursor;
		bool	fWaitCursorOn;
	};

	// RAII wrapper to manage file opening/closing

	class AutoFile {
		friend class Access;

		AutoFile(
			Access *const owner,
			const wchar_t *const file_name,
			const wchar_t *const mode);
		AutoFile() = delete;
		~AutoFile();

		operator FILE* () const;

		Access *const owner;
		FILE* file;
	};

	// Class to obtain a ptr to Access from within WndProcs/DlgProcs

	class StaticThis {
		friend class Access;

		StaticThis();
		~StaticThis() = default;

		// extract from WM_CREATE
		void ExtractFromWM_CREATE(LPARAM lparam);

		// extract from WM_INITDIALOG
		void ExtractFromWM_INITDIALOG(LPARAM lparam);

		Access* operator->() const;

		Access* This;
	};
};

// Support loading help file into a dialog (from MS online docs)

DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE lpBuff, LONG cb, PLONG pcb);
BOOL FillRichEditFromFile(HWND hwnd, LPCTSTR pszFile);

// Inlines

inline wchar_t* Access::TextBuffer(int uBufferID)
{
	assert(uBufferID == 0 || uBufferID == 1);

	switch (uBufferID)
	{
	case 0: return szTextBuffer_0;
	case 1: return szTextBuffer_1;
	}
	return nullptr;
}

inline Access* Access::StaticThis::operator->() const
{
	assert(This != nullptr);
	return This;
}