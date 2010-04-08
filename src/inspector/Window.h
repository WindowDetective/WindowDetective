/////////////////////////////////////////////////////////////////////
// File: Window.h                                                  //
// Date: 5/3/10                                                    //
// Desc: Object that represents a real window or control           //
/////////////////////////////////////////////////////////////////////

#ifndef WINDOW_H
#define WINDOW_H

#include "inspector.h"

namespace inspector {

class Window : public QObject {
    Q_OBJECT
    /* Not sure if i really need properties.
       I could use them for SearchCriteria and maybe InfoWindow.
       Need to use Q_DECLARE_METATYPE for custom types
    // Qt property declarations. These access the member variables
    Q_PROPERTY(HWND handle READ getHandle)
    Q_PROPERTY(WindowClass* windowClass READ getWindowClass)
    Q_PROPERTY(String text READ getText WRITE setText)
    Q_PROPERTY(Window* parent READ getParent WRITE setParent)
    Q_PROPERTY(QList<Window*> children READ getChildren WRITE setChildren)
    Q_PROPERTY(uint styleBits READ getStyleBits)
    Q_PROPERTY(uint exStyleBits READ getExStyleBits)
    Q_PROPERTY(WindowStyleList styles READ getStyles WRITE setStyles STORED false)
    Q_PROPERTY(WindowStyleList standardStyles READ getStandardStyles)
    Q_PROPERTY(WindowStyleList extendedStyles READ getExtendedStyles)
    Q_PROPERTY(QIcon icon READ getIcon)
    Q_PROPERTY(QRect windowRect READ getDimensions WRITE setDimensions)
    Q_PROPERTY(QRect clientRect READ getClientDimensions)
    Q_PROPERTY(QPoint position READ getPosition WRITE setPosition STORED false)
    Q_PROPERTY(QSize size READ getSize WRITE setSize STORED false)
    Q_PROPERTY(bool visible READ isVisible WRITE setVisible)
    Q_PROPERTY(bool unicode READ isUnicode)
    Q_PROPERTY(Process* process READ getProcess)
    Q_PROPERTY(uint processId READ getProcessId STORED false)
    Q_PROPERTY(uint threadId READ getThreadId)*/
private:
    HWND handle;
    WindowClass* windowClass;     // The type of control this is
    Window* parent;
    QList<Window*> children;
    String text;                  // Window's title or control's text
    DWORD styleBits;              // The conbined bit-flags of each style
    DWORD exStyleBits;            // Bit-flags of each extended style
    WindowStyleList styles;       // List of styles applied to this window
    WindowStyleList exStyles;     // Extended styles applied to this window
    QIcon icon;                   // The window's icon (small and large)
    QRect windowRect;             // Coordinates of the window
    QRect clientRect;             // Coordinates of the client area
    bool visible;                 // If visible on screen (has WS_VISIBLE flag)
    bool enabled;                 // Is it enabled for mouse and keyboard input
    bool unicode;                 // Whether the window uses Unicode or ASCII
    Process* process;             // Application that created this window
    DWORD threadId;               // The thread in which it was created
public:
    Window() : handle(NULL) {}
    Window(HWND handle);

    // Getter methods. Parent and child variables, as well as process and
    // thread, are set by the WindowManager, everything else is set in it's
    // update method and the getter just returns the cached copy here.
    HWND getHandle() { return handle; }
    WindowClass* getWindowClass() { return windowClass; }
    String getText() { return text; }
    Window* getParent() { return parent; }
    HWND getParentHandle() { return parent->getHandle(); }
    QList<Window*> getChildren() { return children; }
    uint getStyleBits() { return styleBits; }
    uint getExStyleBits() { return exStyleBits; }
    WindowStyleList getStyles() { return styles + exStyles; }
    WindowStyleList getStandardStyles() { return styles; }
    WindowStyleList getExtendedStyles() { return exStyles; }
    const QIcon& getIcon() { return icon; }
    QRect getDimensions() { return windowRect; }
    QPoint getPosition() { return windowRect.topLeft(); }
    QSize getSize() { return windowRect.size(); }
    QRect getRelativeDimensions();
    QPoint getRelativePosition();
    QRect getClientDimensions() { return clientRect; }
    Process* getProcess() { return process; }
    uint getProcessId() { return process->getId(); }
    uint getThreadId() { return threadId; }
    bool isVisible() { return visible; }
    bool isEnabled() { return enabled; }
    bool isUnicode() { return unicode; }
    bool isChild() { return styleBits & WS_CHILD; }

    // Setter methods. Updates the object's variable and call the appropriate
    // Win32 function to update the real window
    void setParent(Window* p) { parent = p; }
    void setChildren(QList<Window*> c) { children = c; }
    void setText(String text);
    void setStyleBits(uint styleBits, uint exStyleBits);
    void setPosition(QPoint pos);
    void setPosition(int x, int y) { setPosition(QPoint(x, y)); }
    void setSize(QSize size);
    void setSize(int width, int height) { setSize(QSize(width, height)); }
    void setDimensions(QRect rect);
    void setDimensions(int x, int y, int w, int h) { setDimensions(QRect(x, y, w, h)); }
    void setVisible(bool isVisible) { isVisible ? show() : hide(); }
    void setEnabled(bool isEnabled);
    void setProcess(Process* p) { process = p; }
    void setThreadId(DWORD id) { threadId = id; }

    void addStyle(WindowStyle* style) { styles.append(style); }
    void removeStyle(WindowStyle* style) { styles.removeOne(style); }

    // Update methods. These update the specific variable/s with data from
    // the real window. That data can then be accessed using the getter.
    void update();
    void updateText();
    void updateWindowInfo();
    void updateWindowClass();
    void updateIcon();

    // Command methods. These perform a command on the window.
    void show(bool activate = false, bool stay = false);
    void hide();
    void maximise();
    void minimise();
    void close();
    void destroy();

    template <class ReturnType, class FirstType, class SecondType>
    ReturnType sendMessage(UINT msg, FirstType wParam, SecondType lParam);

    template <class ReturnType>
    ReturnType sendMessage(UINT msg);

signals:
    void updated();
};

};   //namespace inspector

#endif   // WINDOW_H