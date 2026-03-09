## Overview
MGUI is a C++ library that wraps over the low level Win 32 GUI API. The library is minimal and not cross platform. It's purpose is to provide a way to quickly
create Windows GUI apps using the Visual Studio Community edition.

If you need cross platform support, there are better options like
Qt.

## Build Instructions

Clone this repo. Open the solution ``mgui.sln`` in Visual C++ Commuinity edition. Then build the solution. This will create ``mgui.lib`` under the ``x64`` folder.

## Using mgui
In Visual Studio create a Windows Desktop application.

Include the header file ``mgui/include/mgui.h``.

Link your application code with the following libraries:

```
-lmgui -lcomctl32 -lgdi32 -lcomdlg32
```

## Minimal Code Example

```cpp
class MainWindow : public CFrame {
public:
    void create() {
        CFrame::create(L"Hello World", 800, 600);
    }

    //Called when user closes the window
    void onClose() override {
        //Stop the event loop
        CWindow::stop();
	}
};

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
    _In_opt_ HINSTANCE hPrevInstance,
    _In_ LPWSTR    lpCmdLine,
    _In_ int       nCmdShow)
{
    CWindow::init(hInstance, 0);

    MainWindow mainWin;

    mainWin.create();
    mainWin.show();

    //Start the event loop
    CWindow::loop();

    return 0;
}
```