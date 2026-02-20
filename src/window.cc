#include "../include/mgui.h"
/*
 * Have to enable
 * #define _WIN32_IE	0x0300
 * in commctrl.h
 * Also link comctl32.
 */
#include <commctrl.h>
#include <iostream>
#include <shobjidl.h>
#include <atlbase.h>

HINSTANCE g_instance;

void trace(const wchar_t* msg) {
	/*
	 static FILE *pf = NULL;

	 if (!pf) {
	 pf = fopen("trace.log", "a");
	 if (pf) {
	 fputs("Initializing trace********\n", pf);
	 }
	 }
	 if (pf) {
	 fputs("TRACE: ", pf);
	 fputs(msg, pf);
	 fputs("\n", pf);
	 fflush(pf);
	 }
	 */
	cout << msg << endl;
}

HACCEL CWindow::accelerators = NULL;

CWindow::CWindow() {
	m_wnd = NULL;
}

CWindow::~CWindow() {
	detach();
}

void CWindow::detach() {
	//cout << "CWindow::detach this: " << this << " HWND: " << m_wnd << endl;
	if (m_wnd != NULL) {
::		RemoveProp(m_wnd, L"cwnd");
		m_wnd = NULL;
	}
}

void CWindow::attach(HWND w) {
	detach();
	//cout << "CWindow::attach this: " << this << " HWND: " << w << endl;
	m_wnd = w;
::	SetProp(m_wnd, L"cwnd", this);
}

void CWindow::create(DWORD exStyle, const wchar_t* wndClass, const wchar_t* title,
		DWORD style, int x, int y, int width, int height, HWND parent,
		HMENU idOrMenu) {

	HWND w =:: CreateWindowEx(exStyle,
			wndClass,
			title,
			style,
			x, y, width, height,
			parent, (HMENU) idOrMenu,
			g_instance,
			NULL);
	
	if (!w)
		throw "CreateWindowEx failed";

	attach(w);
}

void CWindow::show(bool bShow) {
::	ShowWindow(m_wnd, bShow ? SW_SHOW : SW_HIDE);
}

HWND CWindow::getWindow() {
	return m_wnd;
}

CWindow* CWindow::fromWindowSafe(HWND wnd) {
	CWindow *pw = (CWindow*):: GetProp(wnd, L"cwnd");

	if (pw == NULL) {
		throw "CWindow::fromWindow. No CWindow is associated.";
	}

	return pw;
}

CWindow* CWindow::fromWindow(HWND wnd) {
	CWindow *pw = (CWindow*):: GetProp(wnd, L"cwnd");

	return pw;
}

void CALLBACK timerProc(
		HWND hWnd, UINT message, UINT_PTR id, DWORD notused) {
	CWindow* w;

	w = CWindow::fromWindow(hWnd);
	if (w == NULL) {
		return;
	}
	w->onTimer((int) id);
}

LRESULT CALLBACK cwndProc(
		HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	CWindow* w;

	w = CWindow::fromWindow(hWnd);
	if (w == NULL) {
		//trace("cwndProc: No associated window");
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	if (w->handleEvent(message, wParam, lParam) == true) {
		return 0;
	} else {
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
}
void CWindow::onNotify(int id, NMHDR* pHdr) {
}

bool CWindow::handleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
	bool processed = false;
	switch (message) {
	case WM_SIZE:
		onSize(LOWORD(lParam), HIWORD(lParam));
		processed = true;
		break;
	case WM_MOVE:
		onMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
		processed = true;
		break;
	case WM_CLOSE:
		onClose();
		processed = true;
		break;
	case WM_DESTROY:
		onDestroy();
		processed = true;
		break;
	case WM_NOTIFY:
		onNotify((int) wParam, (NMHDR*) lParam);
		break;
	case WM_COMMAND: {
		CWindow *pw = NULL;
		if (lParam != 0) {
			pw = fromWindow((HWND) lParam);
		}
		onCommand(LOWORD(wParam), HIWORD(wParam), pw);
		processed = true;
	}
	}

	return processed;
}

void CWindow::onCommand(int id, int type, CWindow* source) {
	//cout << "COMMAND: ID: " << id << " Source: " << source << endl;
}

void CWindow::loop() {
	MSG msg;

	while (::GetMessage(&msg, (HWND)NULL, 0, 0)) {
		if (accelerators != NULL && TranslateAccelerator(msg.hwnd, accelerators, &msg)) {
			continue;
		}

		::TranslateMessage(&msg);
		::DispatchMessage(&msg);
	}
}

void CWindow::stop() {
	::PostQuitMessage(0);
}

void CWindow::setTimer(int id, int interval) {
::	SetTimer(m_wnd, (UINT_PTR) id, (UINT) interval, timerProc);
}

void CWindow::killTimer(int id) {
::	KillTimer(m_wnd, id);
}

void CWindow::acceptFiles(bool val) {
::	DragAcceptFiles(m_wnd, val == true ? TRUE : FALSE);
}

void CWindow::init(HINSTANCE hInstance, int acceleratorId) {
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);

	if (!SUCCEEDED(hr)) {
		throw "Failed to initialize COM.";
	}

	g_instance = hInstance;

	if (acceleratorId != 0) {
		accelerators = ::LoadAccelerators(g_instance,
				MAKEINTRESOURCE(acceleratorId));
	}

	WNDCLASS wndclass;

	static const long styleNormal = CS_HREDRAW | CS_VREDRAW | CS_DBLCLKS;
	static const long styleNoRedraw = CS_DBLCLKS;

	wndclass.lpfnWndProc = cwndProc; //DefWindowProc;//(WNDPROC)wxWndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = sizeof(DWORD); // VZ: what is this DWORD used for?
	wndclass.hInstance = g_instance;
	wndclass.hIcon = (HICON) NULL;
	wndclass.hCursor =:: LoadCursor((HINSTANCE)NULL, IDC_ARROW);
	wndclass.lpszMenuName = NULL;

	wndclass.hbrBackground = (HBRUSH)(COLOR_WINDOW);
	wndclass.lpszClassName = L"MGUIWindow";
	wndclass.style = styleNormal;

	if ( !RegisterClass(&wndclass) )
	{
		throw "Failed to register window";
	}

	//Initialize common controls
	INITCOMMONCONTROLSEX whatever;

	whatever.dwSize = sizeof(INITCOMMONCONTROLSEX);
	whatever.dwICC = ICC_LISTVIEW_CLASSES | ICC_DATE_CLASSES;
	cout << "Initializing..." << endl;
	if (::InitCommonControlsEx(&whatever) == FALSE) {
		throw "Failed to load common controls.";
	}
}

static void check_throw(HRESULT hr) {
	if (!SUCCEEDED(hr)) {
		throw hr;
	}
}

static bool select_file(HWND hWnd, bool is_open, const wchar_t* title, const std::vector<COMDLG_FILTERSPEC>& filter, std::wstring& selected_name) {
	CComPtr<IFileDialog> pFileSelector;

	HRESULT hr = S_OK;

	if (is_open) {
		hr = CoCreateInstance(CLSID_FileOpenDialog,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pFileSelector));
		check_throw(hr);
	}
	else {
		hr = CoCreateInstance(CLSID_FileSaveDialog,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_PPV_ARGS(&pFileSelector));
		check_throw(hr);
	}

	hr = pFileSelector->SetTitle(title);
	check_throw(hr);

	hr = pFileSelector->SetFileTypes((UINT) filter.size(), filter.data());
	check_throw(hr);

	hr = pFileSelector->SetFileTypeIndex(0);
	check_throw(hr);

	hr = pFileSelector->Show(hWnd);

	if (hr == HRESULT_FROM_WIN32(ERROR_CANCELLED)) {
		return false; // User cancelled the dialog
	}

	check_throw(hr);

	CComPtr<IShellItem> pItem;

	hr = pFileSelector->GetResult(&pItem);
	check_throw(hr);

	PWSTR pszFilePath;
	// Get the file system path from the Shell item
	hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
	check_throw(hr);

	selected_name = pszFilePath;

	CoTaskMemFree(pszFilePath);

	return true;
}

bool CWindow::openFileName(const wchar_t* title, const std::vector<COMDLG_FILTERSPEC>& filter, std::wstring& file_name) {
	return select_file(m_wnd, true, title, filter, file_name);
}
bool CWindow::saveFileName(const wchar_t* title, const std::vector<COMDLG_FILTERSPEC>& filter, std::wstring& file_name) {
	return select_file(m_wnd, false, title, filter, file_name);
}

void CFrame::create(const wchar_t* title, int width, int height) {
	CFrame::create(title, width, height, (const wchar_t*) NULL);
}

void CFrame::create(const wchar_t* title, int width, int height, int menuId) {
	CFrame::create(title, width, height, MAKEINTRESOURCE(menuId));
}

void CFrame::create(const wchar_t* title, int width, int height, const wchar_t* menuId) {
	HMENU hm = NULL;

	if (menuId != NULL) {
		hm =:: LoadMenu(g_instance, menuId);
		if (hm == NULL) {
			throw "Could not load menu.";
		}
	}

	CWindow::create(0, L"MGUIWindow", title,
		WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_SYSMENU,
		CW_USEDEFAULT, CW_USEDEFAULT, width, height, NULL, hm);

}

void CFrame::create(const wchar_t* title, int width, int height, int menuId, int iconId) {

	CFrame::create(title, width, height, menuId);

	HICON icon = LoadIcon(::g_instance, MAKEINTRESOURCE(iconId));
	::SendMessage(m_wnd, WM_SETICON, ICON_BIG, LPARAM(icon));
	::SendMessage(m_wnd, WM_SETICON, ICON_SMALL, LPARAM(icon));
}

void CFrame::create(const wchar_t* title, int width, int height,
		const wchar_t* menuId, const wchar_t* iconId) {

	CFrame::create(title, width, height, menuId);

	HICON icon = LoadIcon(::g_instance, iconId);
	::SendMessage(m_wnd, WM_SETICON, ICON_BIG, LPARAM(icon));
	::SendMessage(m_wnd, WM_SETICON, ICON_SMALL, LPARAM(icon));
}

CFrame::CFrame() {
	hOldCursor = NULL;
}

CFrame::~CFrame() {
::	DestroyWindow(m_wnd);
}

void CFrame::showWaitCursor(bool bShow) {
	if (bShow) {
		if (hOldCursor != NULL) {
			//We are already showing wait cursor
			return;
		}
		hOldCursor =:: SetCursor(
				::LoadCursor((HINSTANCE) NULL, IDC_WAIT));
	} else {
		::SetCursor(hOldCursor);
		hOldCursor = NULL;
	}
}

bool CFrame::handleEvent(UINT message, WPARAM wParam, LPARAM lParam) {
	bool processed = false;

	if (message != WM_DROPFILES) {
		return CWindow::handleEvent(message, wParam, lParam);
	}

	std::cout << "Processing file drop" << std::endl;
	HDROP hd = (HDROP) wParam;
	int count =:: DragQueryFile(hd, 0xFFFFFFFF, NULL, 0);
	std::wstring *list = new std::wstring[count];
	for (int i = 0; i < count; ++i) {
		int sz = ::DragQueryFile(hd, i, NULL, 0);
		std::wstring& str = list[i];
		str.resize(sz);
		::DragQueryFile(hd, i, (wchar_t*) str.data(), sz + 1);
	}
	onDropFiles(list, count);
	delete [] list;
	std::cout << "Done." << std::endl;

	return true;
}

void CFrame::onDropFiles(std::wstring* list, int count) {
}
