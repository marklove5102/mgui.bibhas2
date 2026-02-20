#include "../include/mgui.h"

void
CTrackBar::create(int x, int y, int w, int h, CWindow* parent, HMENU id) {
	CWindow::create(0, TRACKBAR_CLASS, L"",
		WS_CHILD |
		WS_VISIBLE |
		TBS_AUTOTICKS,
		x, y,
		w, h,
		parent->m_wnd,
		id);
}

void CTrackBar::setMax(int64_t max) {
	::SendMessage(m_wnd, TBM_SETRANGEMAX, (WPARAM) TRUE, (LPARAM) max);
}

void CTrackBar::setMin(int64_t min) {
	::SendMessage(m_wnd, TBM_SETRANGEMIN, (WPARAM) TRUE, (LPARAM) min);
}

void CTrackBar::setPos(int64_t pos) {
	::SendMessage(m_wnd, TBM_SETPOS, (WPARAM) TRUE, (LPARAM) pos);
}

int64_t CTrackBar::getPos() {
	LRESULT res = ::SendMessage(m_wnd, TBM_GETPOS, (WPARAM) 0, (LPARAM) 0);

	return (int64_t) res;
}