//-------------------------------------------------------------------------------
// About dialog for the C3C Image Size plug-in
//
// Shared by the C3CImageSize filter and the C3CImageSizeAutomate
// automation plug-ins.
//-------------------------------------------------------------------------------
#ifndef ABOUT_H
#define ABOUT_H

#include <QDialog>
#include "ui_about.h"

class About : public QDialog
{
    Q_OBJECT

public:
    About(QWidget *parent = 0);
    ~About();

private:
    Ui::About ui;

    void mousePressEvent(QMouseEvent *event);
};

#endif // ABOUT_H
