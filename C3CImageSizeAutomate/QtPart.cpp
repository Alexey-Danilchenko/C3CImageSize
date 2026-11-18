//#define UNICODE

#define QT_NO_ACCESSIBILITY

#include <QCoreApplication>
#include <QSettings>
#include <QLayout>
#include <QMessageBox>
#include "about.h"
#include "PIDefines.h"
#include "PITypes.h"
#include "QtPart.h"

#if defined(Q_OS_MACOS)
#define BUNDLE_ID CFSTR("C3CImageSizeAuto")
// For static QT builds - import the platform plugin. With dynamic Qt
// the plugin is loaded at runtime from Contents/PlugIns/platforms
// inside the plugin bundle
#if defined(_QT_STATIC_) && QT_VERSION >= 0x060000
#include <QtPlugin>
Q_IMPORT_PLUGIN (QCocoaIntegrationPlugin);
#endif
#endif

#if defined( Q_OS_WIN)
#include "psnative.h"
#if defined(_QT_STATIC_) && QT_VERSION >= 0x050000
#include <QtPlugin>
Q_IMPORT_PLUGIN (QWindowsIntegrationPlugin);
#endif
#endif

int ReadRegister(int32 &width, int32 &height)
{
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName(APPLICATION);

    QSettings settings;

    bool result = settings.value("result", false).toBool();

    if (!result) return -1;

    settings.beginGroup("Parameters");

    width = settings.value("width", -1).toInt();
    height = settings.value("height", -1).toInt();

    settings.endGroup();

    if (width < 0 || height < 0) return -1;

    return 0;
}

void WriteScripting(bool scripting) {
    QCoreApplication::setOrganizationName(ORGANIZATION);
    QCoreApplication::setApplicationName(APPLICATION);

     QSettings settings;
     settings.setValue("scripting", scripting);
}

void DoAbout() {

    int argc = 0;
    char* argv[1];

#if defined(Q_OS_MACOS)
    // MacOS X specific application attributes set
    // This is needed if QT compiled as frameworks
    // QCoreApplication::addLibraryPath(GetBundlePath(BUNDLE_ID) + "/PlugIns");
    QCoreApplication::setAttribute(Qt::AA_PluginApplication, true);
    QCoreApplication::setAttribute(Qt::AA_DontUseNativeMenuBar, true);
#endif

    QApplication a(argc, argv);

#if defined(Q_OS_MACOS)
    About w(0);
    w.setWindowFlags(Qt::Window | Qt::WindowTitleHint
                                | Qt::CustomizeWindowHint
                                | Qt::WindowCloseButtonHint);


#else

    HWND hwnd = GetActiveWindow();
    About w;
    w.setWindowFlags(Qt::Dialog | Qt::WindowTitleHint
                                | Qt::CustomizeWindowHint
                                | Qt::WindowCloseButtonHint
                                | Qt::MSWindowsFixedSizeDialogHint);
    // Embed the dialog in Photoshop's window (Qt 6 replacement for QWinWidget)
    QWindow* nativeParent = embedInNativeWindow(w, hwnd);

#endif

#if defined(Q_OS_WIN)
    // Keep Photoshop's window disabled while the dialog is modal
    // (the legacy QWinWidget did the same via EnableWindow)
    EnableWindow(hwnd, FALSE);
#endif
    w.exec();
#if defined(Q_OS_WIN)
    EnableWindow(hwnd, TRUE);
    // Safe: a foreign QWindow never destroys the wrapped native window
    delete nativeParent;
#endif
}

//#undef UNICODE
