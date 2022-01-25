#include "ScreenshotAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include "Application.h"

#include <QMenu>
#include <QFileDialog>
#include <QFileInfo>
#include <QDesktopServices>
#include <QDialogButtonBox>

namespace hdps {

using namespace gui;

QString ScreenshotAction::SETTING_KEY_OUTPUT_DIR            = "OutputDir";
QString ScreenshotAction::SETTING_KEY_LOCK_ASPECT_RATIO     = "LockAspectRatio";
QString ScreenshotAction::SETTING_KEY_BACKGROUND_COLOR      = "BackgroundColor";
QString ScreenshotAction::SETTING_KEY_OPEN_AFTER_CREATION   = "OpenScreenshotAfterCreation";

ScreenshotAction::ScreenshotAction(QObject* parent, ScatterplotPlugin& scatterplotPlugin) :
    WidgetAction(parent),
    _scatterplotPlugin(scatterplotPlugin),
    _targetWidthAction(this, "Width ", 1, 10000),
    _targetHeightAction(this, "Height", 1, 10000),
    _lockAspectRatioAction(this, "Lock aspect ratio", true, true),
    _scaleQuarterAction(this, "25%"),
    _scaleHalfAction(this, "50%"),
    _scaleOneAction(this, "100%"),
    _scaleTwiceAction(this, "200%"),
    _scaleFourAction(this, "400%"),
    _backgroundColorAction(this, "Background color", QColor(Qt::white), QColor(Qt::white)),
    _batchScreenshotAction(scatterplotPlugin),
    _directoryPickerAction(this, "Save to"),
    _createAction(this, "Create"),
    _createDefaultAction(this, "Create"),
    _openAfterCreationAction(this, "Open"),
    _aspectRatio()
{
    setText("Create screenshot");
    setIcon(Application::getIconFont("FontAwesome").getIcon("camera"));

    _targetWidthAction.setToolTip("Width of the screenshot");
    _targetHeightAction.setToolTip("Height of the screenshot");
    _lockAspectRatioAction.setToolTip("Lock the aspect ratio");
    _scaleQuarterAction.setToolTip("Scale to 25% of the view");
    _scaleHalfAction.setToolTip("Scale to 50% of the view");
    _scaleOneAction.setToolTip("Scale to 100% of the view");
    _scaleTwiceAction.setToolTip("Scale to 200% of the view");
    _scaleFourAction.setToolTip("Scale to 400% of the view");
    _backgroundColorAction.setToolTip("Background color of the screenshot");
    _createAction.setToolTip("Create the screenshot");
    _createDefaultAction.setToolTip("Create the screenshot with default settings");
    _openAfterCreationAction.setToolTip("Open screenshot image file after creation");

    _targetWidthAction.setSuffix("px");
    _targetHeightAction.setSuffix("px");

    _createAction.setIcon(Application::getIconFont("FontAwesome").getIcon("camera"));
    _createDefaultAction.setIcon(Application::getIconFont("FontAwesome").getIcon("camera"));

    

    
    /*
    // Scale by a quarter
    connect(&_scaleQuarterAction, &TriggerAction::triggered, this, [this, scale]() {
        scale(0.25f);
    });

    // Scale by a half
    connect(&_scaleHalfAction, &TriggerAction::triggered, this, [this, scale]() {
        scale(0.5f);
    });

    // Scale by factor one
    connect(&_scaleOneAction, &TriggerAction::triggered, this, [this, scale]() {
        scale(1.0f);
    });

    // Scale by factor two
    connect(&_scaleTwiceAction, &TriggerAction::triggered, this, [this, scale]() {
        scale(2.0f);
    });

    // Scale by a quarter
    connect(&_scaleFourAction, &TriggerAction::triggered, this, [this, scale]() {
        scale(4.0f);
    });
    */

    

    
}

ScreenshotAction::Widget::Widget(QWidget* parent, ScreenshotAction* screenshotAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, screenshotAction, widgetFlags)
{
    setToolTip("Screenshot settings");

    //screenshotAction->initializeTargetSize();

    //if (widgetFlags & PopupLayout) {
    //    auto layout = new QGridLayout();

    //    layout->addWidget(screenshotAction->getTargetWidthAction().createLabelWidget(this), 0, 0);
    //    layout->addWidget(screenshotAction->getTargetWidthAction().createWidget(this), 0, 1);
    //    layout->addWidget(screenshotAction->getTargetHeightAction().createLabelWidget(this), 1, 0);
    //    layout->addWidget(screenshotAction->getTargetHeightAction().createWidget(this), 1, 1);
    //    layout->addWidget(screenshotAction->getLockAspectRatioAction().createWidget(this), 2, 1);

    //    auto scaleLayout = new QHBoxLayout();

    //    scaleLayout->addWidget(screenshotAction->getScaleQuarterAction().createWidget(this));
    //    scaleLayout->addWidget(screenshotAction->getScaleHalfAction().createWidget(this));
    //    scaleLayout->addWidget(screenshotAction->getScaleOneAction().createWidget(this));
    //    scaleLayout->addWidget(screenshotAction->getScaleTwiceAction().createWidget(this));
    //    scaleLayout->addWidget(screenshotAction->getScaleFourAction().createWidget(this));

    //    layout->addLayout(scaleLayout, 3, 1);

    //    layout->addWidget(screenshotAction->getBackgroundColorAction().createLabelWidget(this), 4, 0);
    //    layout->addWidget(screenshotAction->getBackgroundColorAction().createWidget(this), 4, 1);

    //    layout->addWidget(screenshotAction->getBatchScreenshotAction().createWidget(this), 6, 1);
    //    
    //    layout->addWidget(screenshotAction->getDirectoryPickerAction().createLabelWidget(this), 7, 0);
    //    layout->addWidget(screenshotAction->getDirectoryPickerAction().createWidget(this), 7, 1);

    //    auto createLayout = new QHBoxLayout();

    //    createLayout->addWidget(screenshotAction->getCreateAction().createWidget(this), 1);
    //    createLayout->addWidget(screenshotAction->getOpenAfterCreationAction().createWidget(this, ToggleAction::WidgetFlag::CheckBox));

    //    layout->addLayout(createLayout, 8, 1);

    //    setPopupLayout(layout);
    //}
    //else {
    //    auto layout = new QHBoxLayout();

    //    layout->setMargin(0);

    //    layout->addWidget(screenshotAction->getCreateDefaultAction().createWidget(this, TriggerAction::WidgetFlag::IconText));

    //    setLayout(layout);
    //}
}

}
