#include <sstream>
#include "../include/mgui.h"

void
CEdit::create(int x, int y, int w, int h, 
	CWindow* parent, bool bMultiLine) {

	DWORD style = WS_CHILD | WS_VISIBLE;

	if (bMultiLine) {
		style = style | WS_VSCROLL | 
                    ES_WANTRETURN | ES_LEFT | ES_MULTILINE | ES_AUTOVSCROLL;
        }
	CWindow::create(WS_EX_CLIENTEDGE, L"EDIT", L"",
		style,
		x, y,
		w, h,
		parent->m_wnd,
		(HMENU) NULL);
	setFont((HFONT) GetStockObject(DEFAULT_GUI_FONT));
}

bool CEdit::getDouble(double& value) {
	std::wstring str;
	size_t count = 0;

	getText(str);

	double result = std::stod(str, &count);

	if (count == 0) {
		return false;
	}

	value = result;

	return true;
}

bool CEdit::getInt(int& value) {
	std::wstring str;
	size_t count = 0;

	getText(str);

	int result = std::stoi(str, &count);

	if (count == 0) {
		return false;
	}

	value = result;

	return true;
}

void CEdit::setDouble(double d) {
	std::wstringstream ws;

	ws << d;

	setText(ws.str());
}

void CEdit::setInt(int d) {
	std::wstringstream ws;

	ws << d;

	setText(ws.str());
}
