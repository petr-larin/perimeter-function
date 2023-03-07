#include "dcontext.hpp"

DContext::DContext(HWND hwnd, GeneralError* external_err_ptr)
	: GeneralErrorHandler{ GDI, external_err_ptr }, hwnd{ hwnd },
		ps{}, hdc{ BeginPaint(hwnd, &ps) }, brush{ hdc, external_err_ptr},
		pen{ hdc, external_err_ptr }, font{ hdc, external_err_ptr }
{
	if (hdc == NULL) Throw(2010);
}

DContext::~DContext()
{
	brush.CleanUp(false, 2020);
	pen.CleanUp(false, 2030);
	font.CleanUp(false, 2040);

	if (hdc != NULL)
	{
		EndPaint(hwnd, &ps);
	}
}

DContext::operator HDC() const { return hdc; }
DContext::operator HBRUSH() const { return brush; }
DContext::operator HPEN() const { return pen; }
DContext::operator HFONT() const { return font; }

void DContext::Select(HBRUSH hbr, bool fCanDestroy)
{
	brush.Select(HGDIOBJ(hbr), fCanDestroy);
}

void DContext::Select(HPEN hpen, bool fCanDestroy)
{
	pen.Select(HGDIOBJ(hpen), fCanDestroy);
}

void DContext::Select(HFONT hfont, bool fCanDestroy)
{
	font.Select(HGDIOBJ(hfont), fCanDestroy);
}

void DContext::MoveTo(int iX, int iY)
{
	if (::MoveToEx(*this, iX, iY, NULL) == 0) Throw(2050);
}

void DContext::LineTo(int iX, int iY)
{
	if (::LineTo(*this, iX, iY) == 0) Throw(2060);
}

void DContext::Polygon(const POINT* lpPoints, int iCount)
{
	if (::Polygon(*this, lpPoints, iCount) == 0) Throw(2070);
}

void DContext::Ellipse(int iLeft, int iTop, int iRight, int iBottom)
{
	if (::Ellipse(*this, iLeft, iTop, iRight, iBottom) == 0) Throw(2080);
}

void DContext::FillRect(const RECT* lprc)
{
	if (::FillRect(*this, lprc, *this) == 0) Throw(2090);
}

/*
void DContext::Arc(
	int nLeftRect, int nTopRect, int nRightRect, int nBottomRect,
	int nXStartArc, int nYStartArc, int nXEndArc, int nYEndArc)
{
	if (::Arc(
		*this, nLeftRect, nTopRect, nRightRect, nBottomRect,
		nXStartArc, nYStartArc, nXEndArc, nYEndArc) == 0)
			Throw(2100);
}

void DContext::AngleArc(
	int iX, int iY, DWORD dwRadius, FLOAT eStartAngle, FLOAT eSweepAngle)
{
	if (::AngleArc(
		*this, iX, iY, dwRadius,
		eStartAngle, eSweepAngle) == 0) Throw(2110);
} */

void DContext::TextOut(int nXStart, int nYStart, LPCWSTR lpString, int cbString)
{
	if (::TextOutW(*this, nXStart, nYStart, lpString, cbString) == 0) Throw(2120);
}

void DContext::SetTextAlign(UINT fMode)
{
	if (::SetTextAlign(*this, fMode) == GDI_ERROR) Throw(2130);
}

/*
void DContext::SetBkMode(int iBkMode)
{
	if (::SetBkMode(*this, iBkMode) == 0) Throw(2140);
} */

void DContext::SetTextColor(COLORREF crColor)
{
	if (::SetTextColor(*this, crColor) == CLR_INVALID) Throw(2150);
}

DContext::GDIobj::GDIobj(HDC hdc, GeneralError* external_err_ptr)
	: GeneralErrorHandler{ GDI, external_err_ptr },
		hObj{ NULL }, hOldObj{ NULL }, hdc{ hdc }, fCanDestroy{}
{
}

DContext::GDIobj::operator HBRUSH() const { return HBRUSH(hObj); }
DContext::GDIobj::operator HPEN() const { return HPEN(hObj); }
DContext::GDIobj::operator HFONT() const { return HFONT(hObj); }

void DContext::GDIobj::CleanUp(bool fCanThrow, unsigned where) // can be called from a destructor
{
	if (hObj != NULL)
	{
		auto
			fSelObjFailed{ SelectObject(hdc, hOldObj) == NULL },
			fDelObjFailed{ false };

		if (fCanDestroy) if (DeleteObject(hObj) == 0) fDelObjFailed = true;

		hOldObj = hObj = NULL;

		if (fSelObjFailed) fCanThrow ? Throw(where) : Post(where);

		if (fDelObjFailed) fCanThrow ? Throw(where + 1) : Post(where + 1);
	}
}

void DContext::GDIobj::Select(HGDIOBJ hObj, bool fCanDestroy)
{
	if (hObj == NULL) Throw(2160);
	else
	{
		CleanUp(true, 2170);

		hOldObj = SelectObject(hdc, hObj);

		if (hOldObj == NULL) Throw(2180);
		else
		{
			this->hObj = hObj;
			this->fCanDestroy = fCanDestroy;
		}
	}
}