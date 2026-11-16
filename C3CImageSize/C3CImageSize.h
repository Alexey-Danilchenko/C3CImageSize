#ifndef C3CIMAGESIZE_H
#define C3CIMAGESIZE_H

#include <QWidget>
#include <QDialog>
#include <QCloseEvent>

#include "ui_c3cimagesize.h"

class C3CImageSize : public QDialog
{
    Q_OBJECT

public:
    C3CImageSize(QWidget *parent = 0, Qt::WindowFlags flags = Qt::WindowFlags(0));
    ~C3CImageSize();

private:
    Ui::C3CImageSizeClass ui;

    void closeEvent(QCloseEvent *event);
    void showEvent(QShowEvent *event);
    void WriteParameters(bool Ok);
    void setDimensions(void);
    void CalcNewSize(void);
    void SetupParams(struct C3CResizeParameters* p);

private Q_SLOTS:

    void accept();
    void reject();
    void setConstrain(bool checked);
    void setFit(bool checked);
    void setWidthUnit(int unit);
    void setHeightUnit(int unit);
    void setWidth(int w);
    void setHeight(int h);
    void sharpnessChanged(int);
    void setMethod(int);
    void setPreset(int index);
    void About(bool);
    void Reset(bool);

    void InitUIFromParams();
    void ParametersChange(void);
    void SetMaximumWidth(void);
    void SetMaximumHeight(void);
    void LoadUserPreset(QString *name, C3CResizeParameters *p);
    void WriteUIParametersToRegister(void);
    void ReadUIfromRegister(void);
};

void SetupFromParams();


#endif // C3CIMAGESIZE_H
