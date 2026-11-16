//-------------------------------------------------------------------------------
// About dialog for the C3C Image Size plug-in
//
// Shared by the C3CImageSize filter and the C3CImageSizeAutomate
// automation plug-ins. Clicking a link opens the corresponding web
// page; clicking anywhere else closes the dialog.
//-------------------------------------------------------------------------------
#include "about.h"

#include <QDesktopServices>
#include <QMouseEvent>
#include <QUrl>

About::About(QWidget *parent)
    : QDialog(parent)
{
    ui.setupUi(this);
}

About::~About()
{
}

void About::mousePressEvent(QMouseEvent *e)
{
    QWidget* child = childAt(e->position());

    if (child == ui.GitHubLabel || child == ui.LicenseLabel)
        QDesktopServices::openUrl(QUrl("https://github.com/Alexey-Danilchenko/C3CImageSize"));
    else if (child == ui.QtLabel)
        QDesktopServices::openUrl(QUrl("https://www.qt.io/"));
    else
        done(0);
}
