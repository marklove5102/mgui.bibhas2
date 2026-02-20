#include "../../include/mgui.h"
#include <iostream>
#include <commctrl.h>
#include "resource.h"

class CMyPage : public CPropertyPage {
public:
	wstring name;

	CMyPage(const wchar_t* title) : CPropertyPage(title, IDD_PERSON) {
	}
	void onInitDialog() {
		CEdit m_edt;
		bind(m_edt, ID_TXT_NAME);
		m_edt.setText(L"Bibhas");
	}
	bool onApply() {
		CEdit m_edt;
		bind(m_edt, ID_TXT_NAME);

		m_edt.getText(name);

		return true;
	}
};

class CMyDialog : public CDialog {
CEdit m_edt;
public:

CMyDialog() : CDialog(TEST_DIALOG, NULL) {
}
CMyDialog(CWindow *p) : CDialog(TEST_DIALOG, p) {
}

void onOK() {
	/*
	CMyDialog mdlg(this);

	mdlg.doModal();
	*/
	CPropertySheet ps(L"Title", this);
	CMyPage p1(L"Person");
	CMyPage p2(L"Hello");

	ps.addPage(p1);
	ps.addPage(p2);

	if (ps.doModal()) {
		m_edt.setText(p1.name + L" " + p2.name);
	}
}

void onInitDialog() {
	bind(m_edt, ID_EDT1);

	m_edt.setText(L"Hello");
}

};

int PASCAL WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
try {
	CWindow::init(hInstance);
	CMyDialog mdlg;

	mdlg.doModal();

} catch (const wchar_t* mesg) {
	cout << "Exception: " << mesg << " GetLastError():" << ::GetLastError() << endl;
}
	return 0;
}
