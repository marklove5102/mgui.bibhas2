#include "../../include/mgui.h"
#include "../../include/canvas.h"
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <commctrl.h>
#include <math.h>
#include <stdio.h>
#include "resource.h"

#include "control.h"

#define BTNW 75
#define BTNH 25

class CDrawBox;
class Tool {
public:
	CDrawBox *pDrawBox;
	Tool() {
		pDrawBox = NULL;
	}
	void setDrawBox(CDrawBox* d) {
		pDrawBox = d;
	}
	virtual void onBtnDown(int x, int y) = 0;
	virtual void onBtnUp(int x, int y) = 0;
	virtual void onMove(int x, int y) = 0;
};
class CDrawBox : public CCanvas {
public:
	int gridw;
	vector <Control*> controlList;
	Tool *tool;
	Control *selControl;
	int xfactor, yfactor;
	int nextId;
	DialogArea *dlgBox;
	DialogArea *selDialog;
	Control* clipBoard;
	Tool *defaultTool;
	bool bDirty;
public:
	CDrawBox() {
		gridw = 8;
		selControl = NULL;
		selDialog = NULL;
		tool = NULL;
		LONG l = ::GetDialogBaseUnits();
		yfactor = HIWORD(l);
		xfactor = LOWORD(l);
		nextId = 10001;
		clipBoard = NULL;
		bDirty = false;

		dlgBox = new DialogArea();
		dlgBox->x = dlgBox->y = 0;
		dlgBox->w = dlgBox->h = 250;
	}

	void setDirty(bool val) {
		bDirty = val;
	}
	bool isDirty() {
		return bDirty;
	}
	void addControl(Control *c) {
		controlList.push_back(c);
		setDirty(true);
	}
	void addNewControl(Control *c) {
		c->intId = generateId();
		c->x += dlgBox->x;
		c->y += dlgBox->y;
		addControl(c);
	}
	int generateId() {
		return nextId++;
	}
	int getNextId() {
		return nextId;
	}
	void setNextId(int i) {
		nextId = i;
	}
	int xConv(int i) {
		return (i * 4) / xfactor;
	}
	int yConv(int i) {
		return (i * 8) / yfactor;
	}

	void snap(int& x, int& y) {
		double d, fd, cd;

		d = ((double) x) / gridw;
		fd = ::floor(d);
		cd = ::ceil(d);
		if ((d - fd) < (cd - d)) {
			x = (int) (fd * gridw);
		} else {
			x = (int) (cd * gridw);
		}

		d = ((double) y) / gridw;
		fd = ::floor(d);
		cd = ::ceil(d);
		if ((d - fd) < (cd - d)) {
			y = (int) (fd * gridw);
		} else {
			y = (int) (cd * gridw);
		}
		/*
		int dx = x % gridw;
		int dy = y % gridw;
		if (dx < gridw / 2) {
			x -= dx;
		} else {
			x += gridw;
		}
		if (dy < gridw / 2) {
			y -= dy;
		} else {
			y += gridw;
		}
		*/
	}

	void onLeftMouseDown(int x, int y) {
		if (tool == NULL) {
			return;
		}
		tool->onBtnDown(x, y);
	}
	void onLeftMouseUp(int x, int y) {
		if (tool == NULL) {
			return;
		}
		tool->onBtnUp(x, y);
	}
	void onMouseMove(int x, int y) {
		if (tool == NULL) {
			return;
		}
		tool->onMove(x, y);
	}
	void onPaint(PAINTSTRUCT& ps) {
		dlgBox->draw(ps.hdc);
		for (int i = 0; i < controlList.size(); ++i) {
			controlList[i]->draw(ps.hdc);
		}
		if (selControl != NULL) {
			selControl->drawHandles(ps.hdc);
		} else if (selDialog != NULL) {
			selDialog->drawHandles(ps.hdc);
		}
	}

	void transformControls(int dx, int dy) {
		if (dx == 0 && dy == 0) {
			return;
		}

		for (int i = 0; i < controlList.size(); ++i) {
			Control *c = controlList[i];

			c->x += dx;
			c->y += dy;
		}
		redraw();
	}

	void setDefaultTool(Tool *d) {
		defaultTool = d;
	}
	void switchToDefaultTool() {
		setTool(defaultTool);
	}
	void setTool(Tool* t) {
		tool = t;
		tool->setDrawBox(this);
	}
	bool ptInRect(int x, int y, int rx, int ry, int rw, int rh, int grace) {
		if (x >= rx - grace && 
		    y >= ry - grace &&
		    x <= rx + rw + grace &&
		    y <= ry + rh + grace) {
		    return true;
		}
		return false;
	}
	Control* hitTest(int x, int y) {

		for (int i = controlList.size() - 1; i >= 0; --i) {
			Control *c = controlList[i];

			if (ptInRect(x, y, 
				c->x, c->y,
				c->w, c->h, MARKERW)) {
				return c;
			}
		}

		return NULL;
	}

	DialogArea* hitDialogTest(int x, int y) {
		if (ptInRect(x, y, 
			dlgBox->x, dlgBox->y,
			dlgBox->w, dlgBox->h, MARKERW)) {
			return dlgBox;
		}

		return NULL;
	}
	void select(Control *c) {
		if (selControl != NULL) {
			selControl->setSelected(false);
			redraw(selControl);
		}
		selControl = c;
		if (selControl != NULL) {
			selControl->setSelected(true);
			redraw(selControl);
			selectDialog(NULL);
		}
	}
	void selectDialog(DialogArea* d) {
		if (d == NULL) {
			if (selDialog != NULL) {
				redraw(selDialog);
			}
			selDialog = NULL;
			return;
		}

		if (d != dlgBox) {
			throw "Invalid dialog selected.";
		}
		select(NULL);
		selDialog = d;
		redraw(selDialog);
	}
	Control* getSelected() {
		return selControl;
	}
	DialogArea* getSelectedDialog() {
		return selDialog;
	}
	void erase(Box *ec) {
		if (ec == NULL) {
			return;
		}
		int count = controlList.size();
		for (int i = 0; i < count; ++i) {
			Control *c = controlList[i];
			if (c == ec) {
				controlList.erase(controlList.begin() + i);
				if (c == getSelected()) {
					select(NULL);
				}
				delete c;
				break;
			}
		}
		setDirty(true);
	}
	void redraw() {
		::InvalidateRect(m_wnd, NULL, TRUE);
		cout << "Complete redraw" << endl;
	}
	void redraw(Box *c) {
		RECT r;

		r.left = c->x - MARKERW;
		r.top = c->y - MARKERW;
		r.right = c->x + c->w + MARKERW;
		r.bottom = c->y + c->h + MARKERW;
		::InvalidateRect(m_wnd, &r, TRUE);
	}
	void clearList() {
		int count = controlList.size();
		for (int i = 0; i < count; ++i) {
			Control *c = controlList[i];

			delete c;
		}
		controlList.clear();
		select(NULL);
		setDirty(true);
	}

	void initClipBoard() {
		if (clipBoard == NULL) {
			clipBoard = new Control();
		}
	}
	void clearClipBoard() {
		if (clipBoard != NULL) {
			delete clipBoard;
			clipBoard = NULL;
		}
	}
	void copy() {
		Control *c = getSelected();

		clearClipBoard();
		if (c != NULL) {
			clipBoard = cloneTemplate(c);
		}
	}
	void paste() {
		if (clipBoard == NULL) {
			return;
		}
		//Move it slightly
		clipBoard->x += gridw;
		clipBoard->y += gridw;

		Control *c = cloneTemplate(clipBoard);
		addControl(c);
		select(c);
	}
	void cut() {
		copy();
		erase(getSelected());
	}

	void openFile(const std::wstring& fileName) {
		std::filesystem::path path(fileName);
		std::ifstream is(path);

		if (!is.is_open()) {
			throw "Could not open file.";
		}

		std::string magic;

		is >> magic;

		if (magic != "MGUI") {
			throw "Invalid file format.";
		}

		int count, nextId, version;

		is >> version;

		if (version != 1) {
			throw "Wrong file version.";
		}

		is >> count;
		is >> nextId;

		setNextId(nextId);

		is >> dlgBox->x >> dlgBox->y >>
			dlgBox->w >> dlgBox->h;

		clearList();

		Control c; 
		//Read controls
		for (int i = 0; i < count; ++i) {
			is >> c.type;
			std::getline(is >> std::ws, c.text); //Consume newline
			is >> c.id >>
				c.x >> c.y >>
				c.w >> c.h;
			
			Control *pC = cloneTemplate(&c);

			addControl(pC);

			if (pC->isSelected) {
				select(pC);
			}
		}

		setDirty(false);
	}

	Control* cloneTemplate(Control* c) {
		Control *pC = NULL;

		if (c->type == "LABEL") {
			pC = new Label();
		} else if (c->type == "BUTTON") {
			pC = new Button();
		} else if (c->type == "EDIT") {
			pC = new EditSingle();
		} else if (c->type == "EDIT_MULTI") {
			pC = new EditMulti();
		} else if (c->type == "LISTBOX") {
			pC = new ListBox();
		} else if (c->type == "COMBO") {
			pC = new ComboBox();
		} else if (c->type == "CHECKBOX") {
			pC = new CheckBox();
		} else if (c->type == "RADIO") {
			pC = new RadioButton();
		}
		if (pC == NULL) {
			throw "Invalid control type.";
		}
		pC->copyFrom(c);

		return pC;
	}

	void saveFile(const std::wstring& fileName) {
		std::filesystem::path path(fileName);
		std::ofstream os(path);

		if (!os.is_open()) {
			throw "Could not open file to save.";
		}

		size_t count = controlList.size();
		int nextId = getNextId();
		int version = 1;

		os << "MGUI" << std::endl;
		os << version << std::endl;
		os << count << std::endl;
		os << nextId << std::endl;
		os << dlgBox->x << " " << dlgBox->y << " " <<
			dlgBox->w << " " << dlgBox->h << std::endl;

		cout << xConv(dlgBox->x) <<
			", " << yConv(dlgBox->y) <<
			", " << xConv(dlgBox->w) <<
			", " << yConv(dlgBox->h) << endl;

		for (int i = 0; i < count; ++i) {
			Control *c = controlList[i];

			os << c->type << "\n" << c->text << "\n"
				<< c->id <<
				" " << xConv(c->x) <<
				" " << yConv(c->y) <<
				" " << xConv(c->w) <<
				" " << yConv(c->h) << endl;

		}

		setDirty(false);
	}
};

class CreateTool : public Tool {
	bool bDrawing;
	int ix, iy, px, py;
public:
	CreateTool() {
		bDrawing = false;
	}

	void onBtnDown(int x, int y) {
		pDrawBox->snap(x, y);
		ix = px = x;
		iy = py = y;
		bDrawing = true;
		::SetCapture(pDrawBox->m_wnd);
	}

	void onBtnUp(int x, int y) {
		pDrawBox->snap(x, y);
		px = x;
		py = y;
		bDrawing = false;
		::ReleaseCapture();

		Control *c = new Control();
		c->x = ix < px ? ix : px;
		c->y = iy < py ? iy : py;
		c->w = abs(ix - px);
		c->h = abs(iy - py);

		pDrawBox->addNewControl(c);
		pDrawBox->select(c);
		pDrawBox->switchToDefaultTool();
	}
	void onMove(int x, int y) {
		if (bDrawing == false)
			return;
		
		pDrawBox->snap(x, y);
		HDC dc = ::GetDC(pDrawBox->m_wnd);
		::SetROP2(dc, R2_NOT);
		::Rectangle(dc, ix, iy, px, py);
		px = x;
		py = y;
		::Rectangle(dc, ix, iy, px, py);
		::ReleaseDC(pDrawBox->m_wnd, dc);
	}
};

class SelectTool : public Tool {
	bool bDragMode, bMoveMode;
	int ix, iy, px, py;
	int fx, fy, fw, fh; //factors
public:
	SelectTool() {
		bDragMode = false;
		bMoveMode = false;
		ix = iy = px = py = 0;
		fx = fy = fx = fy = 0;
	}

	void computeFactors(int x, int y, Box *c) {
		int grace = 4;

		fx = fy = fw = fh = 0;
		if (pDrawBox->ptInRect(x, y, c->x, c->y, c->w, 0, grace)) {
			//Top line was clicked
			fy = 1;
			fh = -1;
		}
		if (pDrawBox->ptInRect(x, y, c->x, c->y, 0, c->h, grace)) {
			//Left line was clicked
			fx = 1;
			fw = -1;
		}
		if (pDrawBox->ptInRect(x, y, c->x, c->y + c->h, c->w, 0, grace)) {
			//Bottom line was clicked
			fh = 1;
		}
		if (pDrawBox->ptInRect(x, y, c->x + c->w, c->y, 0, c->h, grace)) {
			//Right line was clicked
			fw = 1;
		}
		cout << fx << fy << fw << fh << endl;
		if (fx == 0 && fy == 0 && fw == 0 && fh == 0) {
			//Clicked right in middle
			fx = fy = 1;
		}
	}

	void onBtnDown(int x, int y) {
		//Hit test
		Box *c = pDrawBox->hitTest(x, y);
		pDrawBox->select(NULL); //Clear
		pDrawBox->selectDialog(NULL); //Clear
		if (c != NULL) {
			pDrawBox->select((Control*) c);
		} else {
			DialogArea *d = pDrawBox->hitDialogTest(x, y);
			if (d != NULL) {
				pDrawBox->selectDialog(d);

				c = d;
			} else {
				return;
			}
		}
		//See where we clicked.
		computeFactors(x, y, c);

		pDrawBox->snap(x, y);
		bDragMode = true;
		bMoveMode = false;
		ix = px = x;
		iy = py = y;
	}
	void onBtnUp(int x, int y) {
		bDragMode = false;
		
		if (bMoveMode == false) {
			return;
		}

		bMoveMode = false;

		Box *c = pDrawBox->getSelected();
		if (c == NULL) {
			c = pDrawBox->getSelectedDialog();
			if (c == NULL) {
				return;
			}
		}
		pDrawBox->snap(x, y);
		int dx = px - ix;
		int dy = py - iy;

		pDrawBox->redraw(c);
		c->x = c->x + dx * fx;
		c->y = c->y + dy * fy;
		c->w = c->w + dx * fw;
		c->h = c->h + dy * fh;
		pDrawBox->redraw(c);
		
		if (c == pDrawBox->getSelectedDialog()) {
			//The dialog was moved
			pDrawBox->transformControls(dx * fx, dy * fy);
		}
	}
	void onMove(int x, int y) {
		Box *c = pDrawBox->getSelected();
		if (c == NULL) {
			c = pDrawBox->getSelectedDialog();
			if (c == NULL) {
				return;
			}
		}

		if (bDragMode == false || c == NULL) {
			return;
		}

		if (bMoveMode == false) {
			if (::abs(x - ix) > 4 || ::abs(y - iy) > 4) {
				bMoveMode = true;
			}
		}
		if (bMoveMode == false) {
			return;
		}

		pDrawBox->snap(x, y);
		HDC dc = ::GetDC(pDrawBox->m_wnd);
		int dx = px - ix;
		int dy = py - iy;

		::SetROP2(dc, R2_NOT);
		::Rectangle(dc, 
			c->x + dx * fx, c->y + dy * fy, 
			c->x + dx * fx + c->w + dx * fw, c->y + dy * fy + c->h + dy * fh);
		px = x;
		py = y;

		dx = px - ix;
		dy = py - iy;
		::Rectangle(dc, 
			c->x + dx * fx, c->y + dy * fy, 
			c->x + dx * fx + c->w + dx * fw, c->y + dy * fy + c->h + dy * fh);
		::ReleaseDC(pDrawBox->m_wnd, dc);
	}
};

class CPropertyDialog : public CDialog {
public:
	CEdit id, text;
	Control *control;
	CComboBox types;

	CPropertyDialog(CWindow *p, Control *c) :
		CDialog("PropertyDialog", p) {
		control = c;
	}
	void onInitDialog() {
		bind(id, ID_CONTROL_ID);
		bind(text, ID_CONTROL_TEXT);
		bind(types, ID_CONTROL_TYPE);

		const char *typeName[] = {
			"Label",
			"Edit",
			"Button",
			"Combo box",
			"List box",
			NULL
		};
		const char *typeId[] = {
			"LABEL",
			"EDIT",
			"PUSHBUTTON",
			"COMBO",
			"LISTBOX",
			NULL
		};

		id.setText(control->id);
		text.setText(control->text);
		for (int i = 0; typeName[i] != NULL; ++i) {
			types.addItem(typeName[i]);
			types.setItemData(i, (void*) typeId[i]);
			if (control->type == typeId[i]) {
				types.setSel(i);
			}
		}
		if (types.getSel() < 0) {
			types.setSel(0);
		}
	}
	void onOK() {
		string str;
		id.getText(control->id);
		if (control->id.length() == 0) {
			control->id = "0";
		}
		text.getText(control->text);
		//Get type ID
		control->type =
			(const char*) types.getItemData(types.getSel());
		CDialog::onOK();
	}
};

class CMyFrame : public CFrame {
	CDrawBox cv;
	CreateTool createTool;
	SelectTool selectTool;
public:
void create() {
	CFrame::create("Dialog Editor", 700, 450, "MainMenu");
	int i = 0;

	cv.create(0, 0, 0, 0, this);
	cv.setDefaultTool(&selectTool);
	cv.switchToDefaultTool();
}

void onClose() {
	if (cv.isDirty()) {
		if (questionBox("The design has changed.\nWould you like to save the file?")) {
			saveFile();
		}
	}
	stop();
}

void saveFile() {
	std::vector<COMDLG_FILTERSPEC> filter = {
		{L"Dialog files", L"*.dlg"},
		{L"All files", L"*.*"}
	};
	std::wstring fileName;

	if (saveFileName(L"Save file", filter, fileName)) {
		cv.saveFile(fileName);
	}
}

void openFile() {
	std::vector<COMDLG_FILTERSPEC> filter = {
		{L"Dialog files", L"*.dlg"},
		{L"All files", L"*.*"}
	};

	std::wstring fileName;

	if (openFileName(L"Open file", filter, fileName)) {
		cv.openFile(fileName);
		cv.redraw();
	}
}

void onCommand(int id, int type, CWindow* source) {
try {
	if (id == _EXIT) {
		onClose();
	} else if (id == _SAVE) {
		saveFile();
	} else if (id == _OPEN) {
		openFile();
	} else if (id == _PROPERTIES) {
		Box *c = cv.getSelected();
		if (c == NULL) {
			throw "Please select a control.";
		}
		CPropertyDialog dlg(this, (Control*) c);

		if (dlg.doModal() == true) {
			cv.redraw(c);
		}
	} else if (id == ID_NEW_LABEL) {
		Control *c = new Label();
		cv.addNewControl(c);
		cv.select(c);
	} else if (id == ID_NEW_RADIO) {
		Control *c = new RadioButton();
		cv.addNewControl(c);
		cv.select(c);
	} else if (id == ID_NEW_EDIT_MULTI) {
		Control *c = new EditMulti();
		cv.addNewControl(c);
		cv.select(c);
	} else if (id == ID_NEW_LISTBOX) {
		Control *c = new ListBox();
		cv.addNewControl(c);
		cv.select(c);
	} else if (id == ID_NEW_BUTTON) {
		Control *c = new Button();
		cv.addNewControl(c);
		cv.select(c);
	} else if (id == ID_NEW_CHECKBOX) {
		Control *c = new CheckBox();
		cv.addNewControl(c);
		cv.select(c);
	} else if (id == ID_NEW_COMBO) {
		Control *c = new ComboBox();
		cv.addNewControl(c);
		cv.select(c);
	} else if (id == ID_NEW_EDIT) {
		Control *c = new EditSingle();
		cv.addNewControl(c);
		cv.select(c);
	} else if (id == _SELECT) {
		cv.setTool(&selectTool);
	} else if (id == _DELETE) {
		cv.cut();
		cv.redraw();
	} else if (id == _COPY) {
		cv.copy();
	} else if (id == _PASTE) {
		cv.paste();
	}
} catch (const char* msg) {
	errorBox(msg);
}
}

void onSize(int w, int h) {
	cv.resize(5, BTNH + 10, w - 10, h - 65);
}
void onDestroy() {
}

};

int PASCAL WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
try {
	CWindow::init(hInstance);
	CMyFrame wnd;
	wnd.create();
	wnd.show();
	CWindow::loop();
} catch (const char* mesg) {
	cout << "GetLastError():" << ::GetLastError() << endl;
	Msg(mesg);
}
	return 0;
}
