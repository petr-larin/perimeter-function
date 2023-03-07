//
// Wrapper class for Win32 graphics API
//

#pragma once

#define STRICT

#include <windows.h>
#include "error.hpp"

// Undef MS macro
#undef TextOut

class DContext : public GeneralErrorHandler {
public:

	DContext() = delete;
	DContext(HWND hwnd, GeneralError* = nullptr);

	virtual ~DContext();

	operator HDC() const;
	operator HBRUSH() const;
	operator HPEN() const;
	operator HFONT() const;

	void Select(HBRUSH hbr, bool fCanDestroy = true);
	void Select(HPEN hpen, bool fCanDestroy = true);
	void Select(HFONT hfont, bool fCanDestroy = true);
	void MoveTo(int iX, int iY);
	void LineTo(int iX, int iY);
	void Polygon(const POINT* lpPoints, int iCount);
	void Ellipse(int iLeft, int iTop, int iRight, int iBottom);
	void FillRect(const RECT* lprc);
	//void Arc(
	//	int nLeftRect, int nTopRect, int nRightRect, int nBottomRect,
	//	int nXStartArc, int nYStartArc, int nXEndArc, int nYEndArc);
	//void AngleArc(
	//	int iX, int iY, DWORD dwRadius, FLOAT eStartAngle, FLOAT eSweepAngle);
	void TextOut(int nXStart, int nYStart, LPCWSTR lpString, int cbString);
	void SetTextAlign(UINT fMode);
	//void SetBkMode(int iBkMode);
	void SetTextColor(COLORREF crColor);

private:

	class GDIobj : public GeneralErrorHandler {
	friend DContext;

		GDIobj() = delete;
		GDIobj(HDC hdc, GeneralError* = nullptr);

		virtual ~GDIobj() = default; // CleanUp is done by DContext

		operator HBRUSH() const;
		operator HPEN() const;
		operator HFONT() const;

		void CleanUp(bool fCanThrow, unsigned where);
		void Select(HGDIOBJ hObj, bool fCanDestroy);

		HGDIOBJ		hObj, hOldObj;
		const HDC	hdc;
		bool		fCanDestroy;
	};

	const HWND	hwnd;
	PAINTSTRUCT	ps;
	const HDC	hdc;

	GDIobj		brush, pen, font;
};