#include "C3CImageSize.h"
#include "about.h"
#include "C3CUtils.h"
#include "definitions.h"
#include "global.h"
#include "C3CImageSizeScripting.h"


#include <QSettings>

#define PRESET_NUM 12
#define USER_PRESET 10
#define DELETE_PRESET 11
#define LINE_PRESET 9

static C3CResizeParameters presets[PRESET_NUM];

void SetupFromParams()
{
    pluginParams.pixelResizeWidth = 0;
    pluginParams.pixelResizeHeight = 0;

    if (pluginParams.paramsOk)
    {
        CalculateNewSize(glWidth,
                         glHeight,
                         pluginParams,
                         &pluginParams.pixelResizeWidth,
                         &pluginParams.pixelResizeHeight);

        // set method
        switch (pluginParams.resizeMethod)
        {
            case resizeLinear:
                glMethod = LINEAR_METHOD;
                break;
             case resizeLinearContrast:
                glMethod = LINEAR_CONTRAST_METHOD;
                break;
        }

        // set sharpness
        glSharpness = pluginParams.sharpnessIndex;
    }

    if (pluginParams.pixelResizeWidth <= 0 ||
        pluginParams.pixelResizeHeight <= 0)
        resultCode = getResultCode("invalid plugin parameters in action");
}

void C3CImageSize::SetupParams(C3CResizeParameters* p)
{
    p->constrainProportions = ui.constrainCheckBox->isChecked();
    p->fitTo = ui.fitCheckBox->isChecked();

    // set units
    switch (ui.widthUnitComboBox->currentIndex())
    {
        case PIXELS:
            p->resizeUnits = runitsPixels;
            break;
        case PERCENT:
            p->resizeUnits = runitsPercents;
            break;
    }

     // set method
     switch (ui.methodComboBox->currentIndex())
     {
        case LINEAR_METHOD:
            p->resizeMethod = resizeLinear;
            break;
        case LINEAR_CONTRAST_METHOD:
            p->resizeMethod = resizeLinearContrast;
            break;
    }

    // set sharpness //c3c
    p->sharpnessIndex = ui.sharpnessSpinBox->value();

    p->resizeWidth = ui.widthSpinBox->value();
    p->resizeHeight = ui.heightSpinBox->value();
}

void C3CImageSize::LoadUserPreset(QString *name, C3CResizeParameters *p)
{
    QSettings settings;

    settings.beginGroup("User Preset");

    *name = settings.value("name", "Store Preset").toString();

    if (*name != "Store Preset")
    {

        p->resizeUnits = settings.value("units", runitsPixels).toInt();

        p->resizeWidth = settings.value("width", -1).toInt();
        p->resizeHeight = settings.value("height", -1).toInt();

        p->constrainProportions = settings.value("constrain", true).toBool();
        p->fitTo = settings.value("fit", false).toBool();

        p->sharpnessIndex = settings.value("sharpness", 0).toInt();
        p->resizeMethod = settings.value("method", 0).toInt();
    }

    settings.endGroup();
}

C3CImageSize::C3CImageSize(QWidget *parent, Qt::WindowFlags flags)
    : QDialog(parent, flags)
{
    ui.setupUi(this);

    connect(ui.constrainCheckBox, SIGNAL(clicked(bool)), this, SLOT(setConstrain(bool)));
    connect(ui.constrainCheckBox, SIGNAL(toggled(bool)), this, SLOT(setConstrain(bool)));

    connect(ui.fitCheckBox, SIGNAL(clicked(bool)), this, SLOT(setFit(bool)));

    connect(ui.widthUnitComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setWidthUnit(int)));
    connect(ui.heightUnitComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setHeightUnit(int)));

    connect(ui.methodComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setMethod(int)));
    connect(ui.presetComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(setPreset(int)));

    connect(ui.heightSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setHeight(int)));
    connect(ui.widthSpinBox, SIGNAL(valueChanged(int)), this, SLOT(setWidth(int)));

    connect(ui.sharpnessSpinBox, SIGNAL(valueChanged(int)), this, SLOT(sharpnessChanged(int)));
    connect(ui.sharpnessSpinBox, SIGNAL(valueChanged(int)), ui.sharpnessSlider, SLOT(setValue(int)));

    connect(ui.sharpnessSlider, SIGNAL(valueChanged(int)), ui.sharpnessSpinBox, SLOT(setValue(int)));
    connect(ui.sharpnessSlider, SIGNAL(valueChanged(int)), this, SLOT(sharpnessChanged(int)));

    ui.widthSpinBox->setMaximum(glWidth);
    ui.heightSpinBox->setMaximum(glHeight);

    ui.widthSpinBox->setValue(glWidth);
    ui.widthSpinBox->setFocus();
    ui.widthSpinBox->selectAll();

    ui.heightSpinBox->setValue(glHeight);

    connect(ui.buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(ui.buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    connect(ui.aboutButton, SIGNAL(clicked(bool)), this, SLOT(About(bool)));


    newWidth = glWidth;
    newHeight = glHeight;

    InitUIFromParams();

    if (ui.constrainCheckBox->isChecked() && !ui.fitCheckBox->isChecked())
        ui.linkLabel->show();
    else
        ui.linkLabel->hide();

    for (int i = 0; i < PRESET_NUM; ++i)
    {
        presets[i].Init();
        presets[i].paramsOk = true;
        presets[i].constrainProportions = 1;
        presets[i].fitTo = 1;
        presets[i].resizeMethod = resizeLinear;
        presets[i].resizeUnits = runitsPixels;
        presets[i].sharpnessIndex = 2;
    }

    presets[1].resizeWidth = 900;
    presets[1].resizeHeight = 900;

    presets[2].resizeWidth = 1200;
    presets[2].resizeHeight = 1200;

    presets[3].resizeWidth = 1280;
    presets[3].resizeHeight = 720;

    presets[4].resizeWidth = 1280;
    presets[4].resizeHeight = 800;

    presets[5].resizeWidth = 1366;
    presets[5].resizeHeight = 768;

    presets[6].resizeWidth = 1920;
    presets[6].resizeHeight = 1080;

    presets[7].resizeWidth = 1920;
    presets[7].resizeHeight = 1200;

    presets[8].resizeWidth = 2560;
    presets[8].resizeHeight = 1440;

    ui.presetComboBox->insertSeparator(LINE_PRESET);

    QString name;
    LoadUserPreset(&name, &presets[USER_PRESET]);
    ui.presetComboBox->setItemText(USER_PRESET, name);


#if defined(Q_OS_MACOS)
    // app font
    QFont tmpFont = font();
    tmpFont.setPointSize(11);
    setFont(tmpFont);
#else
    QFont tmpFont = font();
    int size = tmpFont.pixelSize();
//    tmpFont.setPointSize(10);
    tmpFont.setPixelSize(13);
    setFont(tmpFont);

#endif
}

C3CImageSize::~C3CImageSize()
{
}

void C3CImageSize::ParametersChange(void)
{
    ui.presetComboBox->setCurrentIndex(0);
}

void C3CImageSize::setMethod(int)
{
    ui.widthSpinBox->setFocus();
    ui.widthSpinBox->selectAll();
    ParametersChange();
}

void C3CImageSize::setPreset(int index)
{
    if (index > 0 && index < LINE_PRESET)
    {
        pluginParams = presets[index];
        InitUIFromParams();
        ui.presetComboBox->setCurrentIndex(index);
    }
    else if (index == USER_PRESET)
    {
        if (ui.presetComboBox->itemText(USER_PRESET) == "Store Preset")
        {
            SetupParams(&presets[USER_PRESET]);
            QString string;

            if (presets[USER_PRESET].fitTo)
                string = "Fit To ";
            else
                string = "Resize To ";
            if (ui.widthUnitComboBox->currentIndex() == PIXELS)
                string += QString("%1").arg(presets[USER_PRESET].resizeWidth)
                            + "x"
                            + QString("%1").arg(presets[USER_PRESET].resizeHeight);
            else
                string += QString("%1%").arg(presets[USER_PRESET].resizeWidth)
                            + "x"
                            + QString("%1%").arg(presets[USER_PRESET].resizeHeight);

            ui.presetComboBox->setItemText(index, string);

            QSettings settings;

            settings.beginGroup("User Preset");

            settings.setValue("name", string);

            settings.setValue("width", ui.widthSpinBox->value());
            settings.setValue("height", ui.heightSpinBox->value());
            if (ui.widthUnitComboBox->currentIndex() == PIXELS)
                settings.setValue("units", runitsPixels);
            else
                settings.setValue("units", runitsPercents);
            settings.setValue("constrain", ui.constrainCheckBox->isChecked());
            settings.setValue("fit", ui.fitCheckBox->isChecked());
            settings.setValue("sharpness", ui.sharpnessSpinBox->value());
            settings.setValue("method", ui.methodComboBox->currentIndex());

            settings.endGroup();
        }
        else
        {
            QString name;
            LoadUserPreset(&name, &presets[index]);
            if (name != "Store Preset")
            {
                pluginParams = presets[index];
                InitUIFromParams();
                ui.presetComboBox->setCurrentIndex(USER_PRESET);
            }
        }
    }
    else if (index == DELETE_PRESET)
    {
        ui.presetComboBox->setItemText(USER_PRESET, "Store Preset");
        ui.presetComboBox->setCurrentIndex(0);

        QSettings settings;
        settings.beginGroup("User Preset");
        settings.setValue("name", "Store Preset");
        settings.endGroup();

    }
    else if (index > 0)
        ui.presetComboBox->setCurrentIndex(0);
}

void C3CImageSize::setConstrain(bool checked)
{
    SetMaximumWidth();
    SetMaximumHeight();

    if (checked && !ui.fitCheckBox->isChecked())
    {
        ui.linkLabel->show();

        ui.heightSpinBox->blockSignals(true);

        if (ui.widthUnitComboBox->currentIndex() == PIXELS)
            ui.heightSpinBox->setValue(roundi((float)ui.widthSpinBox->value() / glWidth * glHeight));
        else
            ui.heightSpinBox->setValue(ui.widthSpinBox->value());

        ui.heightSpinBox->blockSignals(false);
    }
    else
        ui.linkLabel->hide();

    ui.widthSpinBox->setFocus();
    ui.widthSpinBox->selectAll();

    ParametersChange();
}

void C3CImageSize::SetMaximumWidth(void)
{
    int maxSize = glWidth > glHeight ? glWidth: glWidth;

    if (ui.widthUnitComboBox->currentIndex() == PERCENT)
        ui.widthSpinBox->setMaximum(100);
    else if (ui.fitCheckBox->isChecked() && ui.constrainCheckBox->isChecked())
    {
        if (ui.heightSpinBox->value() <= glHeight)
            ui.widthSpinBox->setMaximum(99999);
        else
            ui.widthSpinBox->setMaximum(maxSize);
    }
    else
        ui.widthSpinBox->setMaximum(glWidth);
}

void C3CImageSize::SetMaximumHeight(void)
{
    int maxSize = glWidth > glHeight ? glWidth : glHeight;

    if (ui.widthUnitComboBox->currentIndex() == PERCENT)
        ui.heightSpinBox->setMaximum(100);
    else if (ui.fitCheckBox->isChecked() && ui.constrainCheckBox->isChecked())
    {
        if (ui.widthSpinBox->value() <= glWidth)
            ui.heightSpinBox->setMaximum(99999);
        else
            ui.heightSpinBox->setMaximum(maxSize);
    }
    else
        ui.heightSpinBox->setMaximum(glHeight);
}

void C3CImageSize::setFit(bool checked)
{
    setConstrain(ui.constrainCheckBox->isChecked());
    ParametersChange();
    SetMaximumWidth();
    SetMaximumHeight();
}

static bool LockValueChange = false;

void C3CImageSize::setWidthUnit(int unit)
{
    LockValueChange = true;

    ui.heightUnitComboBox->setCurrentIndex(unit);

    ui.widthSpinBox->setMaximum(99999);

    if (unit == PIXELS)
    { //PERCENT to PIXELS
        ui.widthSpinBox->setSingleStep(100);
        ui.widthSpinBox->setValue(roundi(ui.widthSpinBox->value() / 100.0 * glWidth));
    }
    else
    { //PIXELS to PERCENT
        ui.widthSpinBox->setSingleStep(1);
        ui.widthSpinBox->setValue(roundi(ui.widthSpinBox->value() * 100.0 / glWidth));
    }

    SetMaximumWidth();

    ui.widthSpinBox->setFocus();
    ui.widthSpinBox->selectAll();

    LockValueChange = false;
    ParametersChange();
}

void C3CImageSize::setHeightUnit(int unit)
{
    LockValueChange = true;

    ui.widthUnitComboBox->setCurrentIndex(unit);

    ui.heightSpinBox->setMaximum(99999);

    if (unit == PIXELS)
    { //PERCENT to PIXELS
        ui.heightSpinBox->setSingleStep(100);
        ui.heightSpinBox->setValue(roundi(ui.heightSpinBox->value() / 100.0 * glHeight));
        SetMaximumHeight();
    }
    else
    { //PIXELS to PERCENT
        ui.heightSpinBox->setSingleStep(1);
        ui.heightSpinBox->setValue(roundi(ui.heightSpinBox->value() * 100.0 / glHeight));
    }

    SetMaximumHeight();

    ui.widthSpinBox->setFocus();
    ui.widthSpinBox->selectAll();

    LockValueChange = false;
    ParametersChange();
}

void C3CImageSize::setWidth(int w)
{
    if (!LockValueChange)
    {
        if (/*ui.constrainCheckBox->isChecked()*/ ui.linkLabel->isVisible())
        {
            if (ui.heightUnitComboBox->currentIndex() == PIXELS)
            {
                ui.heightSpinBox->blockSignals(true);
                ui.heightSpinBox->setValue(roundi((float)w / glWidth * glHeight));
                ui.heightSpinBox->blockSignals(false);
            }
            else
                ui.heightSpinBox->setValue(w);
        }

        setDimensions();
        SetMaximumHeight();
        ParametersChange();
    }
}

void Normalize(double x, double &d, int &n, QString &suffix, int &precision)
{
    n = 0;
    d = x;
    while (d > 1000)
    {
        d = d / 1024;
        n++;
    }

    if (n == 0)
        suffix = "b";
    else if (n == 1)
        suffix = "K";
    else if (n == 2)
        suffix = "M";
    else if (n == 3)
        suffix = "G";
    else if (n == 4)
        suffix = "T";

    if (n == 0)
        precision = 0;
    else if (d < 10)
        precision = 2;
    else if (d < 100)
        precision = 1;
    else
        precision = 0;
}

void C3CImageSize::setDimensions(void)
{
    CalcNewSize();

    int sizeOf;

    if (glMAX == MAX8)
        sizeOf = 1;
    else
        sizeOf = 2;

    double dNew = newWidth * newHeight * glPlanes * sizeOf;
    double dOld = glWidth * glHeight * glPlanes * sizeOf;

    double d;
    int n;
    QString suffix;
    int precision;

    Normalize(dNew, d, n, suffix, precision);

    QString s = QString(tr(" Pixels dimensions: %L1")).arg(d, 0, 'f', precision) + suffix + " ";
    QString s2;

    if (dNew != dOld)
    {
        Normalize(dOld, d, n, suffix, precision);
        s2 = QString(tr(" (was %L1") + suffix + ") ").arg(d, 0, 'f', precision);
    }
    else
        s2 = "";

    ui.groupBox->setTitle(s + s2);
}

void C3CImageSize::setHeight(int h)
{
    if (!LockValueChange)
    {
        if (ui.linkLabel->isVisible())
        {
            if (ui.widthUnitComboBox->currentIndex() == PIXELS)
            {
                ui.widthSpinBox->blockSignals(true);
                ui.widthSpinBox->setValue(roundi((float)h / glHeight * glWidth));
                ui.widthSpinBox->blockSignals(false);
            }
            else
                ui.widthSpinBox->setValue(h);
        }

        setDimensions();
        SetMaximumWidth();
        ParametersChange();
    }
}

void C3CImageSize::WriteUIParametersToRegister(void)
{
    QSettings settings;
    settings.beginGroup("UI");
    settings.setValue("store", ui.storeCheckBox->isChecked());

    if (ui.storeCheckBox->isChecked())
    {
        settings.setValue("width", ui.widthSpinBox->value());
        settings.setValue("height", ui.heightSpinBox->value());
        settings.setValue("units", ui.widthUnitComboBox->currentIndex());
        settings.setValue("constrain", ui.constrainCheckBox->isChecked());
        settings.setValue("fit", ui.fitCheckBox->isChecked());
        settings.setValue("method", ui.methodComboBox->currentIndex());
        settings.setValue("sharpness", ui.sharpnessSpinBox->value());
    }

    settings.endGroup();
}

void C3CImageSize::WriteParameters(bool Ok)
{
    QSettings settings;

    settings.setValue("result", Ok);
    settings.beginGroup("Parameters");
    settings.setValue("width", newWidth);
    settings.setValue("height", newHeight);
    settings.endGroup();

    if (Ok)
        WriteUIParametersToRegister();

    // record plugin parameters // Alex
    if (Ok)
    {
        SetupParams(&pluginParams); //c3c
        CalculateNewSize(glWidth,
                         glHeight,
                         pluginParams,
                         &pluginParams.pixelResizeWidth,
                         &pluginParams.pixelResizeHeight); //c3c
    }

    pluginParams.paramsOk = Ok;
}

void C3CImageSize::InitUIFromParams()
{
    if (pluginParams.paramsOk)
    {
        // set constraints
        ui.constrainCheckBox->setChecked(pluginParams.constrainProportions);

        // set fitTo
        ui.fitCheckBox->setChecked(pluginParams.fitTo);

        if (ui.constrainCheckBox->isChecked() && !ui.fitCheckBox->isChecked())
            ui.linkLabel->show();
        else
            ui.linkLabel->hide();

        SetMaximumWidth();
        SetMaximumHeight();

        // set units - height is updated automatically
        switch (pluginParams.resizeUnits)
        {
            case runitsPixels:
                ui.widthUnitComboBox->setCurrentIndex(PIXELS);
                break;
            case runitsPercents:
                ui.widthUnitComboBox->setCurrentIndex(PERCENT);
                break;
        }

        SetMaximumWidth();
        SetMaximumHeight();

        // set method
        switch (pluginParams.resizeMethod)
        {
            case resizeLinear:
                ui.methodComboBox->setCurrentIndex(LINEAR_METHOD);
                break;
            case resizeLinearContrast:
                ui.methodComboBox->setCurrentIndex(LINEAR_CONTRAST_METHOD);
                break;
        }

        // set sharpness
        ui.sharpnessSpinBox->setValue(pluginParams.sharpnessIndex);

        if (pluginParams.resizeWidth > 0)
            ui.widthSpinBox->setValue(pluginParams.resizeWidth);
        if (pluginParams.resizeHeight > 0)
            ui.heightSpinBox->setValue(pluginParams.resizeHeight);
    }
}

void C3CImageSize::CalcNewSize(void)
{
    C3CResizeParameters p;
    SetupParams(&p);
    CalculateNewSize(glWidth, glHeight, p, (int32*)&newWidth, (int32*)&newHeight);
}

void C3CImageSize::accept()
{
    CalcNewSize();

    glMethod = ui.methodComboBox->currentIndex();
    glSharpness = ui.sharpnessSpinBox->value(); //c3c

    WriteParameters(true);
    QDialog::accept();
}

void C3CImageSize::reject()
{
    WriteParameters(false);
    QDialog::reject();
}

void C3CImageSize::closeEvent(QCloseEvent *event)
{
   WriteParameters(false);
   QDialog::closeEvent(event);
}

void C3CImageSize::ReadUIfromRegister(void)
{
    QSettings settings;

    settings.beginGroup("UI");

    if (ui.storeCheckBox->isChecked())
    {
        ui.widthUnitComboBox->setCurrentIndex(settings.value("units", PERCENT).toInt());
        ui.widthSpinBox->setValue(settings.value("width", 100).toInt());
        ui.heightSpinBox->setValue(settings.value("height", 100).toInt());

        ui.constrainCheckBox->setChecked(settings.value("constrain", true).toBool());
        ui.fitCheckBox->setChecked(settings.value("fit", false).toBool());

        ui.sharpnessSpinBox->setValue(settings.value("sharpness", 0).toInt());
        ui.methodComboBox->setCurrentIndex(settings.value("method", LINEAR_METHOD).toInt());
    }
    else
    {
        Reset(true);
        ui.storeCheckBox->setChecked(false);
    }

    settings.endGroup();

    SetMaximumWidth();
    SetMaximumHeight();
}

void C3CImageSize::showEvent(QShowEvent *event)
{
    QSettings settings;

    settings.beginGroup("UI");
    ui.storeCheckBox->setChecked(settings.value("store", true).toBool());
    settings.endGroup();

    bool scripting;
    scripting = settings.value("scripting", false).toBool();

    if (!scripting || !ui.storeCheckBox->isChecked())
        ReadUIfromRegister();

    double v = ui.widthSpinBox->value();
    ui.widthSpinBox->setValue(v - 0.01);
    ui.widthSpinBox->setValue(v);

    double h = ui.heightSpinBox->value();

    bool check = ui.constrainCheckBox->isChecked();
    ui.constrainCheckBox->setChecked(!check);
    ui.constrainCheckBox->setChecked(check);

    ui.widthSpinBox->setValue(v);
    ui.heightSpinBox->setValue(h);

    ui.widthSpinBox->setFocus();
    ui.widthSpinBox->selectAll();
}

void C3CImageSize::About(bool)
{
    ::About about(this);
    about.exec();
}

void C3CImageSize::Reset(bool)
{
    ui.constrainCheckBox->setChecked(true);
    ui.fitCheckBox->setChecked(false);
    ui.linkLabel->show();
    ui.widthUnitComboBox->setCurrentIndex(PIXELS);
    ui.methodComboBox->setCurrentIndex(LINEAR_METHOD);
    ui.sharpnessSpinBox->setValue(0);
    ui.widthSpinBox->setValue(glWidth);
    ui.heightSpinBox->setValue(glHeight);
    ui.storeCheckBox->setChecked(true);
}

void C3CImageSize::sharpnessChanged(int)
{
    ParametersChange();
}
