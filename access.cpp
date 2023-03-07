#include "access.hpp"

Access::Access(HINSTANCE hInst, LPWSTR pCmdLine, int nCmdShow) :

	GeneralErrorHandler{ System },

	hInst{ hInst }, pCmdLine{ pCmdLine }, nCmdShow{ nCmdShow },
	hMainWnd{ NULL }, hToolBarWnd{ NULL },
	uDrawingAreaWidth{ 0 }, uDrawingAreaHeight{ 0 }, uToolBarWidth{ 0 },

	hWaitCursor{ HCURSOR(
		LoadImageW(NULL, MAKEINTRESOURCEW(OCR_WAIT),
		IMAGE_CURSOR, 0, 0, LR_SHARED)) },

	hTopLevelMenu{ LoadMenuW(NULL, MAKEINTRESOURCEW(IDR_CONTEXTMENU)) },

	hContextMenu{ GetSubMenu(hTopLevelMenu, 0) },

	szTextBuffer_0{}, szTextBuffer_1{}, szFileTitle{},
	fFatalErrorOccured{ false }, fFileIsOpen{ false },

	CP{}, Graph{ CP },
	dPFmax{}, P{}, Q{}, Center{},
	fArc{}, fDrawEffPerimeter{ false }, fGraph{ false }, fBubbles{},
	uNumSegments{},
	dVelocityRatio{ 0.5 }, dRadiusOfDetection{ dDrawingAreaSizeApp / 25.0 },
	fShowFOverWLine{ false }
{
}

Access::~Access()
{ 
	DestroyMenu(hTopLevelMenu); 
}

void Access::OnError(unsigned where, GenErrCode what)
{ 
	NonFatalErrorMessage(GeneralError(where, what, my_name()));
}

LRESULT CALLBACK Access::MainWndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
	static StaticThis This{};

	try
	{
		switch (msg)
		{
		case WM_CREATE:
			
			This.ExtractFromWM_CREATE(lParam);
			return 0;

		case WM_CLOSE:

			This->MainWndClose();
			return 0;

		case WM_DESTROY:

			This->MainWndDestroy();
			return 0;

		case WM_NOTIFY:

			This->MainWndNotify(lParam);
			return 0;

		case WM_SETTINGCHANGE:
			
			if (wParam == SPI_SETWORKAREA)
			{
				// Screen work area size has changed

				This->ResizeWnd();
				return 0;
			}
			else
				return DefWindowProcW(hWnd, msg, wParam, lParam);

		case WM_DISPLAYCHANGE:

			This->ResizeWnd();
			return 0;

		case WM_MOUSEMOVE:

			This->UpdateTitle(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_LBUTTONDOWN:

			This->ProcessLeftClick(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_CONTEXTMENU:

			This->ProcessContextMenu(LOWORD(lParam), HIWORD(lParam));
			return 0;

		case WM_TIMER:

			This->TimerTick();
			return 0;

		case WM_PAINT:

			This->RedrawWndWrapper();
			return 0;

		case WM_COMMAND:
		{
			if (HWND(lParam) == This->hToolBarWnd && HIWORD(wParam) == BN_CLICKED) // From toolbar
				switch (LOWORD(wParam))
				{
				case IDS_NEW:

					This->ProcessNew();
					return 0;

				case IDS_OPEN:

					This->ProcessFileOpen();
					return 0;

				case IDS_MAX:

					This->ProcessMax();
					return 0;

				case IDS_GRAPH:

					This->ProcessGraph();
					return 0;

				case IDS_EXPORT:

					This->ProcessExport();
					return 0;

				case IDS_HELP:

					This->ProcessHelp();
					return 0;

				case IDS_EXIT:

					SendMessageW(This->hMainWnd, WM_CLOSE, 0, 0);
					return 0;
				}

			if (HWND(lParam) == 0 && HIWORD(wParam) == 0) // From context menu
				switch (LOWORD(wParam))
				{
				case ID_SEARCHPARAMETERS_CHANGE:

					This->SearchParamDialog();
					return 0;
				}

			return DefWindowProcW(hWnd, msg, wParam, lParam);

		} // WM_COMMAND

		default: return DefWindowProcW(hWnd, msg, wParam, lParam);

		} // msg

	} // try

	catch (const GeneralError& exc)
	{
		This->Catch(exc);
	}

	catch (const std::bad_alloc& exc)
	{
		This->CatchBadAlloc(exc);
	}

	return 0;
}

INT_PTR CALLBACK Access::OptionsDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static StaticThis This{};

	try
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:

			This.ExtractFromWM_INITDIALOG(lParam);
			This->OptionsDlgInit(hWnd);
			return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
			case IDC_CHECK:

				This->OptionsDlgCommandCheck(hWnd);
				return TRUE;

			case IDOK:

				This->OptionsDlgCommandOK(hWnd);
				return TRUE;

			case IDCANCEL:

				EndDialog(hWnd, 1);
				return TRUE;

			default: return FALSE;
			}

		default: return FALSE;
		}
	}

	catch (const GeneralError& exc)
	{
		This->Catch(exc);
		return TRUE;
	}

	catch (const std::bad_alloc& exc)
	{
		This->CatchBadAlloc(exc);
		return TRUE;
	}
}

INT_PTR CALLBACK Access::HelpDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	static StaticThis This{};

	try
	{
		switch (uMsg)
		{
		case WM_INITDIALOG:

			This.ExtractFromWM_INITDIALOG(lParam);
			This->HelpDlgInit(hWnd);
			return TRUE;

		case WM_COMMAND:

			switch (LOWORD(wParam))
			{
			case IDOK:
			case IDCANCEL:
				EndDialog(hWnd, 1);
				return TRUE;
			default:
				return FALSE;
			}

		default: return FALSE;
		}
	}

	catch (const GeneralError& exc)
	{
		This->Catch(exc);
		return TRUE;
	}

	catch (const std::bad_alloc& exc)
	{
		This->CatchBadAlloc(exc);
		return TRUE;
	}
}

void Access::InitApp()
{
	// Check objects created in the constructor
	
	if (hWaitCursor == NULL) Throw(1010);

	if (hTopLevelMenu == NULL) Throw(1020);
	
	if (hContextMenu == NULL) Throw(1030);

	// Register main window class

	wchar_t szClassNameBuffer[] = L"Perimeter_MainWnd_Class";

	WNDCLASSEXW wcx{};
    wcx.cbSize			= sizeof(WNDCLASSEXW);
    wcx.style			= CS_OWNDC;
    wcx.lpfnWndProc		= MainWndProc;
	wcx.cbClsExtra		= 0;
    wcx.cbWndExtra		= sizeof(Access*); // store "this"
    wcx.hInstance		= hInst;
    wcx.hIcon			= HICON(LoadImageW(
		hInst, MAKEINTRESOURCEW(IDI_ICON32), IMAGE_ICON, 0, 0, LR_DEFAULTCOLOR));
    wcx.hCursor			= HCURSOR(LoadImageW(
		hInst, MAKEINTRESOURCEW(IDC_MYCURSOR), IMAGE_CURSOR, 0, 0, LR_DEFAULTCOLOR));
    wcx.hbrBackground	= NULL;
    wcx.lpszMenuName	= NULL;
    wcx.lpszClassName	= szClassNameBuffer;
    wcx.hIconSm			= NULL;

	if (!RegisterClassExW(&wcx)) Throw(1040);

	// Create main window

	hMainWnd = CreateWindowExW(
		0,
		szClassNameBuffer,
		NULL,
		WS_OVERLAPPED | WS_CAPTION | WS_SYSMENU | WS_MINIMIZEBOX,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		100, 100,
		NULL,
		NULL,
		hInst,
		this);

	if (hMainWnd == NULL) Throw(1050);

	// Set a timer

	if (SetTimer(hMainWnd, 1, 1000, NULL) == 0) Throw(1060);

	// Display main window

	ResizeWnd(nCmdShow);
}

void Access::CreateToolBar(bool fListStyle)
{
	if (hToolBarWnd != NULL)
	{
		if (DestroyWindow(hToolBarWnd))
			hToolBarWnd = NULL;
		else Throw(1070);
	}

	INITCOMMONCONTROLSEX icex{};
	icex.dwSize	= sizeof(INITCOMMONCONTROLSEX);
	icex.dwICC	= ICC_BAR_CLASSES;
	
	if (!InitCommonControlsEx(&icex)) Throw(1080, NoCommCtl);

	DWORD dwStyle{
		WS_CHILD | TBSTYLE_WRAPABLE | CCS_NODIVIDER |
		TBSTYLE_TOOLTIPS | CCS_NORESIZE |
		CCS_LEFT | CCS_VERT | TBSTYLE_FLAT };

	if (fListStyle) dwStyle |= TBSTYLE_LIST;

	hToolBarWnd = CreateWindowExW(
		0,
		TOOLBARCLASSNAMEW,
		NULL,
		dwStyle,
		0,0,0,0,
		hMainWnd,
		HMENU(ID_TOOLBAR),
		hInst,
		NULL);

	if (hToolBarWnd == NULL) Throw(1090);

	SendMessageW(hToolBarWnd, TB_BUTTONSTRUCTSIZE, WPARAM(sizeof(TBBUTTON)), 0);

	if (SendMessageW(hToolBarWnd, TB_SETBITMAPSIZE, 0, MAKELONG(24, 24)) == 0)
	{
		Throw(1100);
	}

 	TBADDBITMAP tbab{};
	tbab.hInst	= hInst;
	tbab.nID	= IDR_TOOLBAR;

	if (SendMessageW(
		hToolBarWnd, TB_ADDBITMAP,
		WPARAM(uNumOfButtons), LPARAM(&tbab)) == -1)
	{
		Throw(1110);
	}

	TBBUTTON tbb[uNumOfButtons]{};

	tbb[0].fsState = TBSTATE_ENABLED;
    tbb[0].fsStyle = TBSTYLE_BUTTON;
    tbb[0].dwData  = 0;

    tbb[6] = tbb[5] = tbb[4] = tbb[3] = tbb[2] = tbb[1] = tbb[0];

	tbb[3].fsStyle = TBSTYLE_CHECK;

	auto load_button = [=, &tbb](int index, int str_id, unsigned where)
	{
		tbb[index].iBitmap = index;
		tbb[index].idCommand = str_id;
		
		LoadString(str_id);
		
		if ((tbb[index].iString = SendMessageW(
			hToolBarWnd, TB_ADDSTRING,
			WPARAM(NULL), LPARAM(TextBuffer()))) == -1)
		{
			Throw(where);
		}
	};
	
	load_button(0, IDS_NEW, 1120);
	load_button(1, IDS_OPEN, 1130);
	load_button(2, IDS_MAX, 1140);
	load_button(3, IDS_GRAPH, 1150);
	load_button(4, IDS_EXPORT, 1160);
	load_button(5, IDS_HELP,1170);
	load_button(6, IDS_EXIT, 1180);

	if (SendMessageW(hToolBarWnd, TB_ADDBUTTONS, uNumOfButtons, LPARAM(&tbb)) == 0)
	{
		Throw(1190);
	}

	RECT unused{};
	SendMessageW(
		hToolBarWnd, TB_SETROWS, MAKEWPARAM(uNumOfButtons, TRUE), LPARAM(&unused));

	UpdateButtons();
}

void Access::ResizeWnd(int nCmdShow)
{
	// Calculate main window size based off system metrics

	auto fIsIconic{ IsIconic(hMainWnd) == TRUE };

	if (fIsIconic)
		if (OpenIcon(hMainWnd) == 0) Throw(1200);

	auto
		uScreenWidth{ GetSystemMetrics(SM_CXSCREEN) },
		uScreenHeight{ GetSystemMetrics(SM_CYSCREEN) };

	if (uScreenWidth == 0 || uScreenHeight == 0) Throw(1210);

	RECT WorkAreaRect{};

	if (!SystemParametersInfoW(SPI_GETWORKAREA, 0, PVOID(&WorkAreaRect), 0)) Throw(1220);

	int
		uWorkAreaWidth{ WorkAreaRect.right - WorkAreaRect.left },
		uWorkAreaHeight{ WorkAreaRect.bottom - WorkAreaRect.top },
		uWorkAreaLeft{ WorkAreaRect.left },
		uWorkAreaTop{ WorkAreaRect.top };

	auto xFactor{ 0.85 }, yFactor{ 0.85 };  // factors should be <= 1
	
	auto
		uMainWndWidthBase{ std::min(uScreenWidth, uWorkAreaWidth) },
		uMainWndHeightBase{ std::min(uScreenHeight, uWorkAreaHeight) },

		uMainWndWidth{ int(uMainWndWidthBase * xFactor) },
		uMainWndHeight{ int(uMainWndHeightBase * yFactor) },
		
		uMainWndLeft{ uWorkAreaLeft + std::min(
			20, int(0.5 * (1.0 - xFactor) * uMainWndWidthBase)) },

		uMainWndTop{ uWorkAreaTop + std::min(
			20, int(0.5 * (1.0 - yFactor) * uMainWndHeightBase)) };

	// Create toolbar

	CreateToolBar(false); // Try full-size buttons

	if (!TestWndSize(uMainWndWidth, uMainWndHeight, uMainWndLeft, uMainWndTop))
	{
		// Resolution too low for full-size buttons, try list-style buttons

		CreateToolBar(true);
	
		if (!TestWndSize(uMainWndWidth, uMainWndHeight, uMainWndLeft, uMainWndTop))
		{
			Throw(1230, ScrnRes);  // Resolution too low even for list-style buttons
		}
	}

	ShowWindow(hMainWnd, nCmdShow);
	ShowWindow(hToolBarWnd, SW_SHOW);
	InvalidateRgn(hMainWnd, NULL, TRUE);

	if (fIsIconic)
		if (CloseWindow(hMainWnd) == 0) Throw(1240);
}

void Access::LoadString(int uStrID, int uBufferID)
{
	auto result = ::LoadStringW(hInst, uStrID, TextBuffer(uBufferID), uTextBufferSize);

	assert(result != 0);

	// double null-terminate
	TextBuffer(uBufferID)[wcslen(TextBuffer(uBufferID)) + 1] = 0;
}

void Access::LoadEmptyString(int uBufferID)
{
	TextBuffer(uBufferID)[0] = TextBuffer(uBufferID)[1] = 0;
}

bool Access::TestWndSize(
	int uMainWndWidth, int uMainWndHeight, int uMainWndLeft, int uMainWndTop)
{
	// Determine whether the proposed window size is large enough,
	// and adjust the client area to make it square

	RECT rcWnd{}, rcClient{};

	if (GetWindowRect(hMainWnd, &rcWnd) == 0 || GetClientRect(hMainWnd, &rcClient) == 0)
	{
		Throw(1250);
	}

	int
		uButtonSize{ int(SendMessageW(hToolBarWnd, TB_GETBUTTONSIZE, 0, 0)) },
		uButtonWidth{ LOWORD(uButtonSize) },
		uButtonHeight{ HIWORD(uButtonSize) },
		uMinWndWidth{ GetSystemMetrics(SM_CXMIN) },
		uMinWndHeight{ GetSystemMetrics(SM_CYMIN) },
		uNonClientWidth{ rcWnd.right - rcWnd.left - rcClient.right + rcClient.left },
		uNonClientHeight{ rcWnd.bottom - rcWnd.top - rcClient.bottom + rcClient.top };

	constexpr auto uDrawingAreaThreshold{ 30 };

	if (uMinWndWidth == 0 || uMinWndHeight == 0) Throw(1260);

	auto
		NewDrawingAreaWidth{ uMainWndWidth - uButtonWidth - uNonClientWidth },
		NewDrawingAreaHeight{ uMainWndHeight - uNonClientHeight },
		iDelta{ NewDrawingAreaWidth - NewDrawingAreaHeight };

	if (NewDrawingAreaWidth <= uDrawingAreaThreshold ||
		NewDrawingAreaHeight <= uDrawingAreaThreshold)
	{
		Throw(1270, ScrnRes);
	}

	if (iDelta < 0)
	{
		NewDrawingAreaHeight += iDelta;
		uMainWndHeight		 += iDelta; 
	}
	else
	{
		NewDrawingAreaWidth	-= iDelta;
		uMainWndWidth		-= iDelta;
	}

	if (uMainWndWidth < uMinWndWidth || uMainWndHeight < uMinWndHeight)
	{
		Throw(1280, ScrnRes);
	}

	if (NewDrawingAreaHeight - uNumOfButtons * uButtonHeight >= 0)
	{
		if (MoveWindow(
				hMainWnd,
				uMainWndLeft, uMainWndTop,
				uMainWndWidth, uMainWndHeight,
				TRUE) == 0)
			Throw(1290);

		if (MoveWindow(
				hToolBarWnd,
				0, 0,
				uButtonWidth, NewDrawingAreaHeight,
				FALSE) == 0)
			Throw(1300);

		uToolBarWidth = uButtonWidth;

		RECT cli_rect{};
		GetClientRect(hMainWnd, &cli_rect);

		uDrawingAreaWidth = cli_rect.right - cli_rect.left - uToolBarWidth;
		uDrawingAreaHeight = cli_rect.bottom - cli_rect.top;

		return true;
	}
	else
	{
		uDrawingAreaWidth	= 0;
		uDrawingAreaHeight	= 0;

		return false;
	}
}

void Access::UpdateTitle(int uMouseX, int uMouseY)
{
	// Update main window title in different modes

	LoadString(IDS_APPNAME, 0);
	double dAppX{}, dAppY{};

	if (fGraph) // "Graph" mode
	{
		LoadString(IDS_GRAPHTITLE, 0);
		swprintf_s(
			TextBuffer(1), uTextBufferSize, L" : : %s (%u)",
			TextBuffer(0), uNumSegments);
	}
	else

	if (fDrawEffPerimeter) // "Maximum" mode
	{
		swprintf_s(TextBuffer(1), uTextBufferSize, L" : : max P(Z) = %lf", dPFmax);
	}
	else // Default mode

	if (fFileIsOpen) // Add file name
	{
		if (ClientToApp(uMouseX, uMouseY, dAppX, dAppY)) // Add cursor position
			swprintf_s(
				TextBuffer(1), uTextBufferSize,
				L"%s - %s (%u) (%.0f, %.0f)",
				szFileTitle,
				TextBuffer(0),
				CP.num_vertices(),
				dAppX, dAppY);
		else
			swprintf_s(
				TextBuffer(1), uTextBufferSize,
				L"%s - %s (%u)",
				szFileTitle,
				TextBuffer(0),
				CP.num_vertices());
	}
	else
	{
		if (ClientToApp(uMouseX, uMouseY, dAppX, dAppY))
			swprintf_s(
				TextBuffer(1), uTextBufferSize,
				L"%s (%u) (%.0f, %.0f)",
				TextBuffer(0),
				CP.num_vertices(),
				dAppX, dAppY);
		else
			swprintf_s(
				TextBuffer(1), uTextBufferSize,
				L"%s (%u)",
				TextBuffer(0),
				CP.num_vertices());
	}

	if (SetWindowTextW(hMainWnd, TextBuffer(1)) == 0) Throw(1310);
}

void Access::TimerTick()
{
	if (fFatalErrorOccured) return;

	POINT Mouse{};
	if (GetCursorPos(&Mouse) == 0) Throw(1320);
	if (ScreenToClient(hMainWnd, &Mouse) == 0) Throw(1330);
	
	UpdateTitle(Mouse.x, Mouse.y);
}

bool Access::ClientToApp(int iClientX, int iClientY, double& dAppX, double& dAppY) const
{
	dAppX = dDrawingAreaSizeApp * double(iClientX - uToolBarWidth - uDrawingAreaMargin)
			/ (uDrawingAreaWidth - 2 * uDrawingAreaMargin - 1);
	dAppY = dDrawingAreaSizeApp *
		(1.0 - (double(iClientY - uDrawingAreaMargin ) /
			(uDrawingAreaHeight - 2 * uDrawingAreaMargin - 1)));

	return (dAppX >= 0.0 && dAppX <= dDrawingAreaSizeApp &&
				dAppY >= 0.0 && dAppY <= dDrawingAreaSizeApp);
}

bool Access::AppToClient (double dAppX, double dAppY, int& iClientX, int& iClientY) const
{
	iClientX = uToolBarWidth + uDrawingAreaMargin +
		int(0.5 + dAppX * (uDrawingAreaWidth - 2 * uDrawingAreaMargin - 1) / dDrawingAreaSizeApp);
	iClientY = uDrawingAreaHeight - uDrawingAreaMargin - 1 -
		int(0.5 + (uDrawingAreaHeight - 2 * uDrawingAreaMargin - 1) * dAppY / dDrawingAreaSizeApp);

	return (dAppX >= 0 && dAppX <= dDrawingAreaSizeApp &&
				dAppY >= 0 && dAppY <= dDrawingAreaSizeApp);
}

void Access::RedrawWnd(GeneralError* external_err_ptr)
{
	constexpr COLORREF
		CLR_BACKGROUND{ RGB(255, 255, 255) },
		CLR_BORDER{ RGB(207, 207, 207) },
		CLR_POLYGON{ RGB(220, 220, 230) },
		CLR_POLYBORDER{ RGB(100, 100, 200) },
		CLR_AXES{ RGB(120, 120, 220) },
		CLR_GRAPH{ RGB(80, 80, 180) };

	constexpr auto uCircleSize{ 2 };

	DContext dc{ hMainWnd, external_err_ptr };

	// Paint background

	dc.Select(CreateSolidBrush(CLR_BACKGROUND));

	RECT DrawingArea{};
	DrawingArea.left	= uToolBarWidth;
	DrawingArea.top		= 0;
	DrawingArea.right	= uDrawingAreaWidth + uToolBarWidth;
	DrawingArea.bottom	= uDrawingAreaHeight;

	dc.FillRect(&DrawingArea);

	// Draw gridlines

	dc.Select(CreatePen(PS_DOT, 1, CLR_BORDER));
	
	int iClientX{}, iClientY{};

	for (auto uIndex = 0; uIndex <= 10; ++uIndex)
		if (fGraph && uIndex != 0 && uIndex != 10)
			continue;
		else
		{
			auto dApp{ uIndex * dDrawingAreaSizeApp / 10.0 };

			AppToClient(dApp, 0.0, iClientX, iClientY);
			dc.MoveTo(iClientX, iClientY);
			AppToClient(dApp, dDrawingAreaSizeApp, iClientX, iClientY);
			dc.LineTo(iClientX, iClientY);

			AppToClient(0.0, dApp, iClientX, iClientY);
			dc.MoveTo(iClientX, iClientY);
			AppToClient(dDrawingAreaSizeApp, dApp, iClientX, iClientY);
			dc.LineTo(iClientX, iClientY);
		}

	if (!fGraph) // Not in "Graph" mode
	{
		dc.Select(CreateSolidBrush(CLR_POLYGON));
		dc.Select(CreatePen(PS_SOLID, 1, CLR_POLYBORDER));

		auto upoints{ CP.num_vertices() };
		if (upoints > 0)
		{
			// Draw polygon

			std::vector<POINT> points{ upoints };
			auto uCount{ 0 };

			for (convex_polygon::const_iterator iter{ CP.begin() };
					iter != CP.end(); ++iter)
			{
				convex_polygon::point point{ *iter };
				AppToClient(point.x, point.y, iClientX, iClientY);
				points[uCount].x	= iClientX;
				points[uCount++].y	= iClientY;
			}

			if (upoints > 1) dc.Polygon(points.data(), upoints);

			// Draw bubbles at polygon's vertices

			dc.Select(CreateSolidBrush(CLR_BACKGROUND));

			POINT prev{};
			prev.x = prev.y = 2 * uDrawingAreaHeight;

			for (auto uIndex = 0; uIndex < uCount; ++uIndex)
			{
				if (sqrt(
					pow(double(abs(prev.x - points[uIndex].x)), 2.0) +
					pow(double(abs(prev.y - points[uIndex].y)), 2.0)) > 
						3.0 * uCircleSize ||
						upoints < 40)
				{
					dc.Ellipse(
						points[uIndex].x - uCircleSize,
						points[uIndex].y - uCircleSize,
						points[uIndex].x + uCircleSize + 1,
						points[uIndex].y + uCircleSize + 1);

					prev = points[uIndex];
				}
			}

			if (fDrawEffPerimeter) // In "Maximum" mode draw the curve delivering the maximum
			{
				bool fDone{ false };

				AppToClient(P.x, P.y, iClientX, iClientY);
				dc.MoveTo(iClientX, iClientY);

				if (fArc) // The curve is a circular arc
				{
					convex_polygon::point R{ 1.0, 0.0 };
					auto uNumSteps{ uDrawingAreaHeight / 4 + 1 };
					auto
						dRad{ (P - Center).abs() },
						dStartAng{ Center.angle(Center + R, P) },
						dFinishAng{ Center.angle(Center + R, Q) },
						dDelta{ dFinishAng - dStartAng };

					if (dDelta < 0.0) dDelta += 2.0 * pi;

					if (dDelta * uDrawingAreaHeight > 1.0)
					{
						dDelta /= uNumSteps;
						while (uNumSteps > 0)
						{
							dStartAng += dDelta;
							R.x = Center.x + dRad * cos(dStartAng);
							R.y = Center.y + dRad * sin(dStartAng);
							AppToClient(R.x, R.y, iClientX, iClientY);
							dc.LineTo(iClientX, iClientY);
							--uNumSteps;
						}
						fDone = true;
					}
				}

				if (!fDone) // The curve is a straight line (or nearly so)
				{
					AppToClient(Q.x, Q.y, iClientX, iClientY);
					dc.LineTo(iClientX, iClientY);
				}

				// Draw bubbles

				AppToClient(P.x, P.y, iClientX, iClientY);
				dc.Ellipse(
					iClientX - uCircleSize, iClientY - uCircleSize,
					iClientX + uCircleSize + 1, iClientY + uCircleSize + 1);

				AppToClient(Q.x, Q.y, iClientX, iClientY);
				dc.Ellipse(
					iClientX - uCircleSize, iClientY - uCircleSize,
					iClientX + uCircleSize + 1, iClientY + uCircleSize + 1);
			}
		}
	}
	else // "Graph" mode
	{
		auto
			dLeft{ dDrawingAreaSizeApp * 0.1 },
			dRight{ dDrawingAreaSizeApp * 0.9 },
			dTop{ dDrawingAreaSizeApp * 0.8 },
			dBottom{ dDrawingAreaSizeApp * 0.2 },
			dArrowLength{ dDrawingAreaSizeApp * 0.02 },
			dArrowWidth{ dDrawingAreaSizeApp * 0.005 },
			xMargin{ dDrawingAreaSizeApp * 0.04 },
			dFont{ dDrawingAreaSizeApp * 0.05 };

		int iLeft{}, iRight{}, iTop{}, iBottom{};

		AppToClient(dLeft, dBottom, iLeft, iBottom);
		AppToClient(dRight, dTop, iRight, iTop);
		if (iLeft == iRight) return;
		if (iBottom == iTop) return;
		if (Graph.maximum() == 0.0) return;

		// Draw axes with arrows

		dc.Select(CreatePen(PS_SOLID, 1, CLR_AXES));

		dc.MoveTo(iRight, iBottom);
		dc.LineTo(iLeft, iBottom);
		dc.LineTo(iLeft, iTop);

		dc.MoveTo(iRight, iBottom);
		AppToClient(
			dRight - dArrowLength, dBottom - dArrowWidth,
			iClientX, iClientY);
		dc.LineTo(iClientX, iClientY);
		dc.MoveTo(iRight, iBottom);
		AppToClient(
			dRight - dArrowLength, dBottom + dArrowWidth,
			iClientX, iClientY);
		dc.LineTo(iClientX, iClientY);

		dc.MoveTo(iLeft, iTop);
		AppToClient(
			dLeft - dArrowWidth, dTop - dArrowLength,
			iClientX, iClientY);
		dc.LineTo(iClientX, iClientY);
		dc.MoveTo(iLeft, iTop);
		AppToClient(
			dLeft + dArrowWidth, dTop - dArrowLength,
			iClientX, iClientY);
		dc.LineTo(iClientX, iClientY);

		// Draw P=F(w,r)/w line

		dc.Select(CreatePen(PS_SOLID, 1, CLR_GRAPH));

		auto dGain{ 0.0 }, dGraphFactor(Graph.maximum());
		auto fReallyShowLine{ false };

		if (fShowFOverWLine)
		{
			dGain = search::f(dVelocityRatio, dRadiusOfDetection) / dVelocityRatio;

			if (dGain <= dGraphFactor)
			{
				fReallyShowLine = true;
			}
			else
			if (dGain/50.0 < dGraphFactor)
			{
				fReallyShowLine = true;
				dGraphFactor = dGain;
			}

			dGain = std::min(dGain, 1000.0 * dDrawingAreaSizeApp * dDrawingAreaSizeApp);
		}

		if (fReallyShowLine)
		{
			dGraphFactor = std::max(dGraphFactor, 1.1 * dGain);

			auto iGain{ int(dGain * (iBottom - iTop) / dGraphFactor) };

			dc.MoveTo (iLeft, iBottom - iGain);
			dc.LineTo (iRight, iBottom - iGain);
		}

		// Draw graph

		dc.MoveTo(iLeft, iBottom);
		AppToClient(dRight - xMargin, 0, iClientX, iClientY);
		
		if (iClientX == iLeft) return;

		for (int iIndex = iLeft; iIndex <= iClientX; ++iIndex)
		{
			auto dZ{ (iIndex - iLeft) * Graph.area() / (iClientX - iLeft) };

			if (dZ < 0.0) dZ = 0.0;
			if (dZ > Graph.area()) dZ = Graph.area();

			auto dPF{ ((iBottom - iTop) / dGraphFactor) * Graph(dZ) };

			dc.LineTo(iIndex, iBottom - int(dPF));
		}

		// Draw graph bubbles

		if (fBubbles)
			for (unsigned uIndex = 0; uIndex <= Graph.num_segments (); ++uIndex)
			{
				auto iZ{
					int(iLeft + (iClientX - iLeft) * Graph.a(uIndex) / Graph.area()) };
				auto dPF{ Graph(Graph.a(uIndex)) };
				auto iPF{ int(dPF * (iBottom - iTop) / dGraphFactor) };

				dc.Ellipse(
					iZ - uCircleSize, iBottom - iPF - uCircleSize,
					iZ + uCircleSize + 1, iBottom - iPF + uCircleSize + 1);
			}

		// Output text

		AppToClient(0.0, dFont, iClientX, iClientY);
		auto iFont{ iClientY };
		AppToClient(0.0, 0.0, iClientX, iClientY);
		iFont = iClientY - iFont;

		if (iFont < 8) return;
		
		auto
			hFont{ CreateFontW(
							iFont, 0, 0, 0, 400, 0, 0, 0, ANSI_CHARSET,
							OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY, VARIABLE_PITCH,
							L"Segoe UI") },
			hFontItalic{ CreateFontW(
							iFont, 0, 0, 0, 400, TRUE, 0, 0, ANSI_CHARSET,
							OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY, VARIABLE_PITCH,
							L"Segoe UI") },
			hFontSpace{ CreateFontW(
							std::max(iFont / 6, 3), 0, 0, 0, 400, 0, 0, 0, ANSI_CHARSET,
							OUT_TT_ONLY_PRECIS, CLIP_DEFAULT_PRECIS,
							DEFAULT_QUALITY, VARIABLE_PITCH,
							L"Segoe UI") };

		if (hFont == NULL || hFontItalic == NULL) return;

		dc.Select(hFontItalic, false);
		AppToClient(dRight, dBottom - 2.5 * dArrowWidth, iClientX, iClientY);

		if (uDrawingAreaHeight - iClientY + 6 < iFont) return;

		dc.SetTextAlign(TA_TOP | TA_RIGHT);
		dc.SetTextColor(CLR_AXES);
		dc.TextOut(iClientX, iClientY, L"Z", 1);
		
		AppToClient(dLeft, dTop + 2.5 * dArrowWidth, iClientX, iClientY);
		
		if (iClientY + 6 < iFont) return;

		dc.SetTextAlign(TA_BOTTOM | TA_LEFT | TA_UPDATECP);

		dc.MoveTo(iClientX, iClientY);
		dc.TextOut(0, 0, L"P", 1);

		dc.Select(hFontSpace, false);
		dc.TextOut(0, 0, L"   ", 3);

		dc.Select(hFont, false);
		dc.TextOut(0, 0, L"(", 1);
		
		dc.Select(hFontSpace, false);
		dc.TextOut(0, 0, L"   ", 3);

		dc.Select(hFontItalic);
		dc.TextOut (0, 0, L"Z", 1);

		dc.Select(hFontSpace, false);
		dc.TextOut(0, 0, L"    ", 4);

		dc.Select(hFont);
		dc.TextOut(0, 0, L")", 1);
	}
}

void Access::RedrawWndWrapper()
{
	// This is needed to check for errors "posted" by ~DContext()

	GeneralError external_err{ 0, OK, nullptr };

	RedrawWnd(&external_err);

	if (external_err.what != OK) Throw(external_err);
}

void Access::ProcessNew()
{
	CP.reset();
	fFileIsOpen = false;
	UpdateButtons();
	TimerTick();
	RemoveMax();
}

void Access::ProcessLeftClick(int uMouseX, int uMouseY)
{
	if (fGraph) // In "Graph" mode toggle bubbles on/off
	{
		fBubbles = !fBubbles;
		InvalidateRgn(hMainWnd, NULL, FALSE);
		return;
	}

	double dAppX{}, dAppY{};

	if (ClientToApp(uMouseX, uMouseY, dAppX, dAppY)) // Add new vertex
	{
		AutoWaitCursor wc{ *this };

		CP.add_vertex(convex_polygon::point(dAppX, dAppY));
		CP.convex_hull();
		RemoveMax();
	}

	UpdateButtons();
	TimerTick();
}

void Access::ProcessContextMenu(int iMouseX, int iMouseY)
{
	// Display a popup menu in "Graph" mode

	if (!fGraph) return;

	if (iMouseX == 0xFFFF) // Invoked from keyboard
	{
		RECT rc{};
		if (GetWindowRect(hMainWnd, &rc) == 0) Throw(1340);
		
		iMouseX = (rc.left + rc.right)/2;
		iMouseY = (rc.top + rc.bottom)/2;
	}

	TrackPopupMenuEx(hContextMenu, TPM_LEFTALIGN | TPM_RIGHTBUTTON,
		iMouseX, iMouseY, hMainWnd, NULL);
}

void Access::SearchParamDialog()
{
	DialogBoxParamW(
		hInst,
		MAKEINTRESOURCEW(IDD_DIALOG_OPTIONS),
		hMainWnd,
		DLGPROC(OptionsDlgProc),
		LPARAM(this));
}

void Access::UpdateButtons()
{
	// Update the status of toolbar buttons based off the number of vertices

	auto enable_button = [=](WPARAM ctl_id, LPARAM enable, unsigned where,
		UINT msg = TB_ENABLEBUTTON)
	{
		if (SendMessageW(hToolBarWnd, msg, ctl_id, enable) == FALSE) Throw(where);
	};

	if (!fGraph)
	{
		bool fNewBut{}, fMaxBut{}, fGraphBut{};

		switch (CP.num_vertices())
		{
			case 0:
				fNewBut		= false;
				fMaxBut		= false;
				fGraphBut	= false;
				break;
			
			case 1:
			case 2:
				fNewBut		= true;
				fMaxBut		= false;
				fGraphBut	= false;
				break;
			
			default: // >= 3
				fNewBut		= true;
				fMaxBut		= true;
				fGraphBut	= true;
				break;
		}

		enable_button(IDS_NEW, LPARAM(fNewBut), 1350);
		enable_button(IDS_MAX, LPARAM(fMaxBut), 1360);
		enable_button(IDS_GRAPH, LPARAM(fGraphBut), 1370);
		enable_button(IDS_OPEN, true, 1380);
		enable_button(IDS_EXPORT, false, 1390);
		enable_button(IDS_GRAPH, false, 1400, TB_CHECKBUTTON);
	}
	else
	{
		enable_button(IDS_NEW, false, 1410);
		enable_button(IDS_OPEN, false, 1420);
		enable_button(IDS_MAX, false, 1430);
		enable_button(IDS_EXPORT, true, 1440);
		enable_button(IDS_GRAPH, true, 1450, TB_CHECKBUTTON);
	}
}

void Access::ProcessFileOpen()
{
	constexpr DWORD uFileNameBufferSize{ 260 };
	OPENFILENAMEW ofn{};
	wchar_t	
		FileNameBuffer[uFileNameBufferSize]{},
		FileTitleBuffer[uFileTitleBufferSize]{};

	ofn.lStructSize		= sizeof(OPENFILENAME);
	ofn.hwndOwner		= hMainWnd;
	ofn.lpstrFilter		= L"Text files\0*.txt\0All files\0*.*\0";
	ofn.nFilterIndex	= 1;
	ofn.lpstrFile		= FileNameBuffer;
	ofn.nMaxFile		= uFileNameBufferSize;
	ofn.lpstrFileTitle	= FileTitleBuffer;
	ofn.nMaxFileTitle	= uFileTitleBufferSize;
	ofn.Flags			= OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	if (GetOpenFileNameW(&ofn) == 0) return;

	AutoWaitCursor wc{ *this };
	AutoFile file{ this, FileNameBuffer, L"r" };
	
	if (file == nullptr)
	{ 
		OnError(1460, FileOpen);
		return;
	}

	double dAppX{}, dAppY{};
	wchar_t semi_col[2]{};
	int iResult{};
	convex_polygon New{};

	for (;;) 
	{
		iResult = fwscanf_s(file, L"%lf ,%lf %1ws", &dAppX, &dAppY, semi_col, 2);

		if (iResult != 3) break;

		if ((dAppX < 0.0) || (dAppX > dDrawingAreaSizeApp) ||
			(dAppY < 0.0) || (dAppY > dDrawingAreaSizeApp))
		{
			OnError(1470, AppOutOfRangeFile);
			return;
		}

		if(semi_col[0] != L';')
		{
			OnError(1475, AppWrongFileFormat);
			return;
		}

		New.add_vertex(convex_polygon::point(dAppX, dAppY));
	}

	if (iResult != EOF)
	{
		OnError(1480, AppWrongFileFormat);
		return;
	}

	if (New.num_vertices() < 3)
	{
		OnError(1490, AppNotEnoughVertices);
		return;
	}

	fFileIsOpen	= true;
	wcscpy_s(szFileTitle, FileTitleBuffer);

	if (wcslen(szFileTitle) + 4 > uFileTitleBufferSize)
	{
		szFileTitle[uFileTitleBufferSize - 1] = 0;
		szFileTitle[uFileTitleBufferSize - 2] = '.';
		szFileTitle[uFileTitleBufferSize - 3] = '.';
		szFileTitle[uFileTitleBufferSize - 4] = '.';
	}

	CP = New;
	CP.convex_hull();

	RemoveMax();
	UpdateButtons();
	TimerTick();
}

void Access::ProcessExport()
{
	constexpr DWORD uFileNameBufferSize{ 260 };
	OPENFILENAMEW ofn{};
	wchar_t	
		FileNameBuffer[uFileNameBufferSize]{},
		FileTitleBuffer[uFileTitleBufferSize]{};

	ofn.lStructSize		= sizeof(OPENFILENAMEW);
	ofn.hwndOwner		= hMainWnd;
	ofn.lpstrFilter		= L"Text files\0*.txt\0All files\0*.*\0";
	ofn.nFilterIndex	= 1;
	ofn.lpstrFile		= FileNameBuffer;
	ofn.nMaxFile		= uFileNameBufferSize;
	ofn.lpstrFileTitle	= FileTitleBuffer;
	ofn.nMaxFileTitle	= uFileTitleBufferSize;
	ofn.Flags			= OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt		= L"txt";

	if (GetSaveFileNameW(&ofn) == 0) return;

	AutoWaitCursor wc{ *this };
	AutoFile file{ this, FileNameBuffer, L"w" };

	if (file == nullptr)
	{
		OnError(1500, FileOpen);
		return;
	}

	if (fwprintf_s(file, L"NumSegments = %u\n", Graph.num_segments()) < 0)
	{
		OnError(1510, FileWrite);
		return;
	}

	for (unsigned uIndex = 1; uIndex <= Graph.num_segments(); ++uIndex)
		if (fwprintf_s(
			file,
			L"    a [%u] = %.15lf\nTheta [%u] = %.15lf\n Zeta [%u] = %.15lf\n",
			uIndex, Graph.a(uIndex),
			uIndex, Graph.theta(uIndex),
			uIndex, Graph.zeta(uIndex)) < 0)
		{
			OnError(1520, FileWrite);
			return;
		}
}

void Access::ProcessMax()
{
	AutoWaitCursor wc{ *this };

	CP.convex_hull();
	convex_polygon_pf PF{ CP };

	dPFmax = PF.shortest(fArc, P, Q, Center);

	if (dPFmax > 0.0) fDrawEffPerimeter = true;

	InvalidateRgn(hMainWnd, NULL, FALSE);
	TimerTick();
}

void Access::RemoveMax()
{
	fDrawEffPerimeter = false;
	InvalidateRgn(hMainWnd, NULL, FALSE);
	TimerTick();
}

void Access::ProcessGraph()
{
	auto iState{ SendMessageW(hToolBarWnd, TB_GETSTATE, IDS_GRAPH, 0) };

	if (iState == -1) Throw(1530);

	fGraph = bool(iState & TBSTATE_CHECKED);

	AutoWaitCursor wc{ *this };

	if (fGraph)
	{
		CP.convex_hull();
		Graph.~convex_polygon_pf();
		Graph.convex_polygon_pf::convex_polygon_pf(CP);
		uNumSegments = Graph.num_segments();
		fBubbles = true;
	}

	InvalidateRgn(hMainWnd, NULL, FALSE);
	UpdateButtons();
	TimerTick();
}

void Access::ProcessHelp()
{
	if (LoadLibraryW(L"Riched20.dll") == NULL)
		OnError(1540, NoRichEdit);
	else
		DialogBoxParamW(hInst, MAKEINTRESOURCEW(IDD_DIALOG_HELP),
			hMainWnd, DLGPROC(HelpDlgProc), LPARAM(this));
}

void Access::HelpDlgInit(HWND hWnd)
{
	if (!FillRichEditFromFile(GetDlgItem(hWnd, IDC_RICHEDIT_HELP), L"perimeter.rtf"))
	{
		OnError(1550, NoHelpFile);
		EndDialog(hWnd, 1);
	}
}

void Access::OptionsDlgInit(HWND hWnd)
{
	auto
		hEditW{ GetDlgItem(hWnd, IDC_EDITW) },
		hEditR{ GetDlgItem(hWnd, IDC_EDITR) },
		hCheck{ GetDlgItem(hWnd, IDC_CHECK) };

	SendMessageW(hEditW, EM_LIMITTEXT, wOptDlgEditBufSize, 0);
	SendMessageW(hEditR, EM_LIMITTEXT, wOptDlgEditBufSize, 0);

	constexpr size_t buf_size{ 64 };
	wchar_t szBuffer[buf_size]{};

	swprintf_s(szBuffer, buf_size, L"%lg", dVelocityRatio);
	SendMessageW(hEditW, WM_SETTEXT, 0, LPARAM(szBuffer));

	swprintf_s(szBuffer, buf_size, L"%lg", dRadiusOfDetection);
	SendMessageW(hEditR, WM_SETTEXT, 0, LPARAM(szBuffer));

	SendMessageW(hCheck, BM_SETCHECK,
		fShowFOverWLine ? BST_CHECKED : BST_UNCHECKED, 0);

	EnableWindow(hEditW, fShowFOverWLine);
	EnableWindow(hEditR, fShowFOverWLine);
}

void Access::OptionsDlgCommandCheck(HWND hWnd)
{
	auto
		hEditW{ GetDlgItem(hWnd, IDC_EDITW) },
		hEditR{ GetDlgItem(hWnd, IDC_EDITR) },
		hCheck{ GetDlgItem(hWnd, IDC_CHECK) };

	bool fChecked{ SendMessageW(hCheck, BM_GETCHECK, 0, 0) == BST_CHECKED };

	EnableWindow(hEditW, fChecked);
	EnableWindow(hEditR, fChecked);
}

void Access::OptionsDlgCommandOK(HWND hWnd)
{
	auto
		hEditW{ GetDlgItem(hWnd, IDC_EDITW) },
		hEditR{ GetDlgItem(hWnd, IDC_EDITR) },
		hCheck{ GetDlgItem(hWnd, IDC_CHECK) };

	auto fShowLine{ SendMessageW(hCheck, BM_GETCHECK, 0, 0) == BST_CHECKED };

	wchar_t
		MainBuffer[wOptDlgEditBufSize + 1]{},
		AuxBuffer[wOptDlgEditBufSize + 1]{};

	double ret_w{}, ret_r{};
	LRESULT text_len{};
	int num_fields{};

	// Retrieve w

	*(WORD*)(MainBuffer) = wOptDlgEditBufSize;
	text_len = SendMessageW(hEditW, EM_GETLINE, 0, LPARAM(MainBuffer));
	MainBuffer[text_len] = 0;

	num_fields = 
			swscanf_s(MainBuffer, L"%lg%s", &ret_w, AuxBuffer, wOptDlgEditBufSize);

	if (ret_w < 0.0 || 1.0 < ret_w || num_fields != 1)
	{
		OnError(1560, AppOutOfRangeW);
		SetFocus(hEditW);
		SendMessageW(hEditW, EM_SETSEL, 0, -1);

		return;
	}

	// Retrieve r

	*(WORD*)(MainBuffer) = wOptDlgEditBufSize;
	text_len = SendMessageW(hEditR, EM_GETLINE, 0, LPARAM(MainBuffer));
	MainBuffer[text_len] = 0;

	num_fields =
		swscanf_s(MainBuffer, L"%lg%s", &ret_r, AuxBuffer, wOptDlgEditBufSize);

	// If changing these boundary values (10 and 500),
	// make sure to also correct them in the
	// resource file - the string IDS_R_OUTOFRANGE

	if (ret_r < 10.0 || 500.0 < ret_r || num_fields != 1)
	{
		OnError(1570, AppOutOfRangeR);
		SetFocus(hEditR);
		SendMessageW(hEditR, EM_SETSEL, 0, -1);

		return;
	}
	
	fShowFOverWLine = fShowLine;
	dVelocityRatio = ret_w;
	dRadiusOfDetection = ret_r;

	InvalidateRgn(hMainWnd, NULL, FALSE);
	EndDialog(hWnd, 1);
}

void Access::MainWndClose()
{
	DestroyWindow(hMainWnd);
}

void Access::MainWndDestroy()
{
	KillTimer(hMainWnd, 1);
	PostQuitMessage(0);
}

void Access::MainWndNotify(LPARAM lParam)
{
	switch (LPNMHDR(lParam)->code)
	{
	case TTN_GETDISPINFO:
	{
		LPTOOLTIPTEXT lpttt = LPTOOLTIPTEXT(lParam);
		assert(lpttt != nullptr);
		lpttt->hinst = hInst;

		switch (lpttt->hdr.idFrom)
		{
		case IDS_NEW:
			LoadString(IDS_TT_NEW);
			break;
		case IDS_OPEN:
			LoadString(IDS_TT_OPEN);
			break;
		case IDS_MAX:
			LoadString(IDS_TT_MAX);
			break;
		case IDS_GRAPH:
			LoadString(IDS_TT_GRAPH);
			break;
		case IDS_EXPORT:
			LoadString(IDS_TT_EXPORT);
			break;
		case IDS_HELP:
			LoadString(IDS_TT_HELP);
			break;
		case IDS_EXIT:
			LoadString(IDS_TT_EXIT);
			break;

		default: // should not be reachable
			LoadEmptyString();
		}

		lpttt->lpszText = TextBuffer();

		return;
	}

	default: return;
	}
}

void Access::FatalErrorMessage(const GeneralError& exc)
{
	unsigned str_id{};

	switch (exc.what)
	{
	case NoCommCtl:
		str_id = IDS_FE_COMMCTL;
		break;
	case ScrnRes:
		str_id = IDS_FE_STRANGERESOLUTION;
		break;
	case GDI:
		str_id = IDS_FE_GDI;
		break;
	case System:
		str_id = IDS_FE_SYSERROR;
		break;

	default:
		str_id = IDS_UNKNOWN; // stub - shold be unreachable
	}

	LoadString(IDS_FATALERROR, 1);
	swprintf_s(TextBuffer(0), uTextBufferSize, TextBuffer(1), exc.where, exc.class_name);

	LoadString(str_id, 1);
	wcscat_s(TextBuffer(1), uTextBufferSize, TextBuffer(0));
	
	LoadString(IDS_FATALERROR_CAP, 0);

	MessageBoxW(hMainWnd, TextBuffer(1), TextBuffer(0), MB_ICONSTOP | MB_APPLMODAL);
}

void Access::NonFatalErrorMessage(const GeneralError& exc)
{
	unsigned str_id{};

	switch (exc.what)
	{
	case FileOpen:
		str_id = IDS_OPENFILE;
		break;
	case FileClose:
		str_id = IDS_CLOSEFILE;
		break;
	case FileWrite:
		str_id = IDS_FILEOUTPUT;
		break;
	case NoRichEdit:
		str_id = IDS_NO_RICHEDIT;
		break;
	case NoHelpFile:
		str_id = IDS_NO_HELPFILE;
		break;
	case AppWrongFileFormat:
		str_id = IDS_WRONGFILEFORMAT;
		break;
	case AppNotEnoughVertices:
		str_id = IDS_NOTENOUGHVERTICES;
		break; 
	case AppOutOfRangeFile:
		str_id = IDS_VALUEOUTOFRANGE;
		break; 
	case AppOutOfRangeW:
		str_id = IDS_W_OUTOFRANGE;
		break; 
	case AppOutOfRangeR:
		str_id = IDS_R_OUTOFRANGE;
		break;

	default:
		str_id = IDS_UNKNOWN; // stub - shold be unreachable
	}

	LoadString(IDS_NONFATALERROR, 1);
	swprintf_s(TextBuffer(0), uTextBufferSize, TextBuffer(1), exc.where);

	LoadString(str_id, 1);
	wcscat_s(TextBuffer(1), uTextBufferSize, TextBuffer(0));

	LoadString(IDS_NONFATALERROR_CAP, 0);
	
	MessageBoxW(hMainWnd, TextBuffer(1), TextBuffer(0), MB_ICONEXCLAMATION | MB_APPLMODAL);
}

void Access::OutOfMemoryMessage(const std::bad_alloc&) noexcept
{
	LoadString(IDS_FE_OUTOFMEM, 0);
	LoadString(IDS_FATALERROR_CAP, 1);

	MessageBoxW(hMainWnd, TextBuffer(0), TextBuffer(1), MB_ICONSTOP | MB_APPLMODAL);
}

void Access::Catch(const GeneralError& exc) noexcept
{
	try
	{
		if (fFatalErrorOccured) return;
		fFatalErrorOccured = true;	

		FatalErrorMessage(exc);

		if (hMainWnd != NULL)
		{
			DestroyWindow(hMainWnd);
			hMainWnd = NULL;
		}

		::exit(exc.where);
	}
	catch (...)
	{
	
	}
}

void Access::CatchBadAlloc(const std::bad_alloc& exc) noexcept
{
	try
	{
		if (fFatalErrorOccured) return;
		fFatalErrorOccured = true;	

		OutOfMemoryMessage(exc);

		if (hMainWnd != NULL)
		{
			DestroyWindow(hMainWnd);
			hMainWnd = NULL;
		}

		::exit(-1);
	}
	catch (...)
	{
	
	}
}

Access::AutoWaitCursor::AutoWaitCursor(Access& acc)
	: hSaveCursor{ NULL }, fWaitCursorOn{}
{
	if (acc.CP.num_vertices() < 10)
	{
		// should be a quick action - no need for WaitCursor

		fWaitCursorOn = false;
		return;
	}

	hSaveCursor = SetCursor(acc.hWaitCursor);
	fWaitCursorOn = true;
}

Access::AutoWaitCursor::~AutoWaitCursor()
{
	if (fWaitCursorOn) SetCursor(hSaveCursor);
}

Access::AutoFile::AutoFile(
	Access *const owner, const wchar_t *const file_name, const wchar_t *const mode)
	: owner{ owner }, file{ nullptr }
{
	if (_wfopen_s(&file, file_name, mode) != 0) file = nullptr;
}

Access::AutoFile::~AutoFile()
{
	if (file != nullptr)
		if (fclose(file) != 0)
			if (owner != nullptr)
			{ 
				owner->OnError(1580, FileClose); 
			}
}

Access::AutoFile::operator FILE* () const
{
	return file;
}

Access::StaticThis::StaticThis() : This{ nullptr }
{
}

void Access::StaticThis::ExtractFromWM_CREATE(LPARAM lparam)
{
	assert(lparam != 0);
	This = (Access*)(((LPCREATESTRUCTW)(lparam))->lpCreateParams);
}

void Access::StaticThis::ExtractFromWM_INITDIALOG(LPARAM lparam)
{
	assert(lparam != 0);
	This = (Access*)(lparam);
}

DWORD CALLBACK EditStreamCallback(DWORD_PTR dwCookie, LPBYTE lpBuff, LONG cb, PLONG pcb)
{
	HANDLE hFile = (HANDLE)dwCookie;
	return !ReadFile(hFile, lpBuff, cb, (DWORD*)pcb, NULL);
}

BOOL FillRichEditFromFile(HWND hwnd, LPCTSTR pszFile)
{
	BOOL fSuccess = FALSE;

	HANDLE hFile = CreateFile(pszFile, GENERIC_READ, FILE_SHARE_READ,
		0, OPEN_EXISTING,
		FILE_FLAG_SEQUENTIAL_SCAN, NULL);

	if (hFile != INVALID_HANDLE_VALUE)
	{
		EDITSTREAM es = { (DWORD_PTR)hFile, 0, EditStreamCallback };

		if (SendMessageW(hwnd, EM_STREAMIN, SF_RTF, (LPARAM)&es) &&
			es.dwError == 0)
		{
			fSuccess = TRUE;
		}
		CloseHandle(hFile);
	}
	return fSuccess;
}