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

    // Update the state of the target height action
    const auto updateTargetHeightAction = [this]() -> void {

        // Disable when the aspect ratio is locked
        _targetHeightAction.setEnabled(!_lockAspectRatioAction.isChecked());
    };

    // Updates the aspect ratio
    const auto updateAspectRatio = [this]() -> void {
        _scatterplotPlugin.setSetting(SETTING_KEY_LOCK_ASPECT_RATIO, _lockAspectRatioAction.isChecked());
        _aspectRatio = static_cast<float>(_targetHeightAction.getValue()) / static_cast<float>(_targetWidthAction.getValue());
    };

    // Disable target height action when the aspect ratio is locked
    connect(&_lockAspectRatioAction, &ToggleAction::toggled, this, updateTargetHeightAction);
    connect(&_lockAspectRatioAction, &ToggleAction::toggled, this, updateAspectRatio);

    // Update target height action when the target width changed
    connect(&_targetWidthAction, &IntegralAction::valueChanged, this, [this]() {

        // Scale the target height when the aspect ratio is locked
        if (_lockAspectRatioAction.isChecked())
            _targetHeightAction.setValue(static_cast<std::int32_t>(_aspectRatio * static_cast<float>(_targetWidthAction.getValue())));
    });

    // Scale the screenshot
    const auto scale = [this](float scaleFactor) {
        _targetWidthAction.setValue(scaleFactor * static_cast<float>(_scatterplotPlugin.getScatterplotWidget().width()));
        _targetHeightAction.setValue(scaleFactor * static_cast<float>(_scatterplotPlugin.getScatterplotWidget().height()));
    };

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

    // Create the screenshot when the create action is triggered
    connect(&_createAction, &TriggerAction::triggered, this, [this]() {
        createScreenshot();
    });

    // Create the screenshot with default settings when the create default action is triggered
    connect(&_createDefaultAction, &TriggerAction::triggered, this, [this]() {
        createScreenshot(true);
    });

    // Load from settings
    _lockAspectRatioAction.setChecked(_scatterplotPlugin.getSetting(SETTING_KEY_LOCK_ASPECT_RATIO, true).toBool());
    _backgroundColorAction.setColor(_scatterplotPlugin.getSetting(SETTING_KEY_BACKGROUND_COLOR, QVariant::fromValue(QColor(Qt::white))).value<QColor>());
    _openAfterCreationAction.setChecked(_scatterplotPlugin.getSetting(SETTING_KEY_OPEN_AFTER_CREATION, true).toBool());

    // Save the background color setting when the action is changed
    connect(&_backgroundColorAction, &ColorAction::colorChanged, this, [this](const QColor& color) {
        _scatterplotPlugin.setSetting(SETTING_KEY_BACKGROUND_COLOR, color);
    });

    // Save the open after creation setting when the action is toggled
    connect(&_openAfterCreationAction, &ToggleAction::toggled, this, [this](const bool& toggled) {
        _scatterplotPlugin.setSetting(SETTING_KEY_OPEN_AFTER_CREATION, toggled);
    });

    // Perform initialization of actions
    updateAspectRatio();
    updateTargetHeightAction();
}

void ScreenshotAction::initializeTargetSize()
{
    _targetWidthAction.setValue(_scatterplotPlugin.getScatterplotWidget().width());
    _targetWidthAction.setValue(_scatterplotPlugin.getScatterplotWidget().width());
    _targetHeightAction.setValue(_scatterplotPlugin.getScatterplotWidget().height());
    _targetHeightAction.setValue(_scatterplotPlugin.getScatterplotWidget().height());

    _aspectRatio = static_cast<float>(_targetHeightAction.getValue()) / static_cast<float>(_targetWidthAction.getValue());
}

void ScreenshotAction::createScreenshot(bool defaultSettings /*= false*/)
{
    // Get output dir from settings
    const auto outputDir = _scatterplotPlugin.getSetting(SETTING_KEY_OUTPUT_DIR, "/").toString();

    // Get screenshot image file name (*.png *.jpg *.bmp)
    const auto fileName = QFileDialog::getSaveFileName(nullptr, tr("Save screenshot image"), outputDir, tr("Image Files (*.jpg)"));

    // Save if we have a valid filename
    if (!fileName.isEmpty()) {

        QApplication::setOverrideCursor(Qt::WaitCursor);
        {
            // Get screenshot dimensions and background color
            const auto width            = defaultSettings ? _scatterplotPlugin.getScatterplotWidget().width() : _targetWidthAction.getValue();
            const auto height           = defaultSettings ? _scatterplotPlugin.getScatterplotWidget().height() : _targetHeightAction.getValue();
            const auto backgroundColor  = defaultSettings ? QColor(Qt::white) : _backgroundColorAction.getColor();

            // Create and save the screenshot
            _scatterplotPlugin.getScatterplotWidget().createScreenshot(width, height, fileName, backgroundColor);

            // Save new output dir to settings
            _scatterplotPlugin.setSetting(SETTING_KEY_OUTPUT_DIR, QFileInfo(fileName).absolutePath());

            // Open the image file in an external program if the user requested this
            if (_openAfterCreationAction.isChecked())
                QDesktopServices::openUrl(fileName);
        }
        QApplication::restoreOverrideCursor();
    }
}

ScreenshotAction::Widget::Widget(QWidget* parent, ScreenshotAction* screenshotAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, screenshotAction, widgetFlags)
{
    setToolTip("Screenshot settings");

    screenshotAction->initializeTargetSize();

    if (widgetFlags & PopupLayout) {
        auto layout = new QGridLayout();

        layout->addWidget(screenshotAction->getTargetWidthAction().createLabelWidget(this), 0, 0);
        layout->addWidget(screenshotAction->getTargetWidthAction().createWidget(this), 0, 1);
        layout->addWidget(screenshotAction->getTargetHeightAction().createLabelWidget(this), 1, 0);
        layout->addWidget(screenshotAction->getTargetHeightAction().createWidget(this), 1, 1);
        layout->addWidget(screenshotAction->getLockAspectRatioAction().createWidget(this), 2, 1);

        auto scaleLayout = new QHBoxLayout();

        scaleLayout->addWidget(screenshotAction->getScaleQuarterAction().createWidget(this));
        scaleLayout->addWidget(screenshotAction->getScaleHalfAction().createWidget(this));
        scaleLayout->addWidget(screenshotAction->getScaleOneAction().createWidget(this));
        scaleLayout->addWidget(screenshotAction->getScaleTwiceAction().createWidget(this));
        scaleLayout->addWidget(screenshotAction->getScaleFourAction().createWidget(this));

        layout->addLayout(scaleLayout, 3, 1);

        layout->addWidget(screenshotAction->getBackgroundColorAction().createLabelWidget(this), 4, 0);
        layout->addWidget(screenshotAction->getBackgroundColorAction().createWidget(this), 4, 1);

        layout->addWidget(screenshotAction->getBatchScreenshotAction().createWidget(this), 6, 1);

        auto createLayout = new QHBoxLayout();

        createLayout->addWidget(screenshotAction->getCreateAction().createWidget(this), 1);
        createLayout->addWidget(screenshotAction->getOpenAfterCreationAction().createWidget(this, ToggleAction::WidgetFlag::CheckBox));

        layout->addLayout(createLayout, 7, 1);

        setPopupLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        layout->setMargin(0);

        layout->addWidget(screenshotAction->getCreateDefaultAction().createWidget(this, TriggerAction::WidgetFlag::IconText));

        setLayout(layout);
    }
}

}
