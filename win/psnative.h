//
// psnative.h
//
// Qt 6 replacement for the legacy QWinWidget (QtWinMigrate) helper.
//
// Photoshop hands the plugin the HWND of its main window. To show a Qt
// dialog over it, we wrap the HWND in a QWindow (QWindow::fromWinId) and
// make it the dialog's transient parent (QWindow::setTransientParent).
// On Windows this sets Photoshop's window as the dialog's owner
// (GWL_HWNDPARENT): the dialog keeps its title bar, stays above and is
// centered over Photoshop's window - the same behavior the legacy
// QWinWidget provided.
//
// Copyright (c) 2026 Alexey Danilchenko
//
// Distributed under the OSI-approved BSD 3-Clause License.  See
// accompanying file Copyright.txt or https://cmake.org/licensing for
// details.
//

#ifndef PSNATIVE_H
#define PSNATIVE_H

#include <QWidget>
#include <QWindow>
#include <qt_windows.h>

// Embeds the top-level \a widget over \a hwnd.
// Returns the QWindow wrapping \a hwnd; keep it alive while \a widget is
// shown. Deleting it is safe: a foreign window does not own (and never
// destroys) the wrapped native handle, and the transient parent is not a
// QObject parent, so the widget's window is not affected. Returns nullptr
// if \a hwnd is invalid, in which case \a widget stays a regular
// top-level window.
inline QWindow *embedInNativeWindow(QWidget &widget, HWND hwnd)
{
    QWindow *native = QWindow::fromWinId(reinterpret_cast<WId>(hwnd));
    if (native)
        widget.windowHandle()->setTransientParent(native);
    return native;
}

#endif // PSNATIVE_H
