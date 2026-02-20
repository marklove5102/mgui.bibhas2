#include "../include/mgui.h"

void
CLabel::create(const wchar_t* label, int x, int y, int w, int h, CWindow* parent) {
	DWORD style = WS_VISIBLE | WS_CHILD | SS_LEFT; 
	CWindow::create(0, L"STATIC", label, 
		style,
		x, y,
		w, h,
		parent->m_wnd,
		(HMENU) NULL);
	setFont((HFONT) GetStockObject(DEFAULT_GUI_FONT));
}
