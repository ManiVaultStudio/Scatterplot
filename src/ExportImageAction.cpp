#include "ExportImageAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <Application.h>

const QMap<ExportImageAction::Scale, TriggersAction::Trigger> ExportImageAction::triggers = QMap<ExportImageAction::Scale, TriggersAction::Trigger>({
    { ExportImageAction::Eighth, TriggersAction::Trigger("12.5%", "Scale by 1/8th") },
    { ExportImageAction::Quarter, TriggersAction::Trigger("25%", "Scale by a quarter") },
    { ExportImageAction::Half, TriggersAction::Trigger("50%", "Scale by half") },
    { ExportImageAction::One, TriggersAction::Trigger("100%", "Keep the original size") },
    { ExportImageAction::Twice, TriggersAction::Trigger("200%", "Scale twice") },
    { ExportImageAction::Thrice, TriggersAction::Trigger("300%", "Scale thrice") },
    { ExportImageAction::Four, TriggersAction::Trigger("400%", "Scale four times") },
    { ExportImageAction::Eight, TriggersAction::Trigger("800%", "Scale eight times") }
});

const QMap<ExportImageAction::Scale, float> ExportImageAction::scaleFactors = QMap<ExportImageAction::Scale, float>({
    { ExportImageAction::Eighth, 0.125f },
    { ExportImageAction::Quarter, 0.25f },
    { ExportImageAction::Half, 0.5f },
    { ExportImageAction::One, 1.0f },
    { ExportImageAction::Twice, 2.0f },
    { ExportImageAction::Thrice, 3.0f },
    { ExportImageAction::Four, 4.0f },
    { ExportImageAction::Eight, 8.0f }
});

QString ExportImageAction::SETTING_KEY_OUTPUT_DIR           = "Export/Image/OutputDir";
QString ExportImageAction::SETTING_KEY_LOCK_ASPECT_RATIO    = "Export/Image/LockAspectRatio";
QString ExportImageAction::SETTING_KEY_BACKGROUND_COLOR     = "Export/Image/BackgroundColor";
QString ExportImageAction::SETTING_KEY_OPEN_AFTER_CREATION  = "Export/Image/OpenScreenshotAfterCreation";

ExportImageAction::ExportImageAction(QObject* parent, ScatterplotPlugin& scatterplotPlugin) :
    GroupAction(parent),
    _scatterplotPlugin(scatterplotPlugin),
    _dimensionsPickerAction(this),
    _targetWidthAction(this, "Width ", 1, 10000),
    _targetHeightAction(this, "Height", 1, 10000),
    _lockAspectRatioAction(this, "Lock aspect ratio", true, true),
    _scaleAction(this, "Scale", triggers.values().toVector()),
    _backgroundColorAction(this, "Background color", QColor(Qt::white), QColor(Qt::white)),
    _overrideRangesAction(this, "Override ranges", false, false),
    _fixedRangeAction(this, "Fixed range"),
    _fileNamePrefixAction(this, "Filename prefix", scatterplotPlugin.getPositionDataset()->getGuiName() + "_", scatterplotPlugin.getPositionDataset()->getGuiName() + "_"),
    _directoryPickerAction(this, "Save to"),
    _dialogAction(this, "", { TriggersAction::Trigger("Export", "Export dimensions"), TriggersAction::Trigger("Cancel", "Cancel export")  }),
    _aspectRatio()
{
    _dimensionsPickerAction.setMayReset(false);
    _targetWidthAction.setMayReset(false);
    _targetHeightAction.setMayReset(false);
    _lockAspectRatioAction.setMayReset(false);
    _scaleAction.setMayReset(false);
    _backgroundColorAction.setMayReset(false);
    _overrideRangesAction.setMayReset(false);
    _fixedRangeAction.setMayReset(false);
    _fileNamePrefixAction.setMayReset(false);
    _directoryPickerAction.setMayReset(false);
    _dialogAction.setMayReset(false);

    _targetWidthAction.setSuffix("px");
    _targetHeightAction.setSuffix("px");

    // Update dimensions picker when the position dataset changes
    connect(&scatterplotPlugin.getPositionDataset(), &Dataset<Points>::changed, this, &ExportImageAction::updateDimensionsPickerAction);

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

    // Scale when one of the scale buttons is clicked
    connect(&_scaleAction, &TriggersAction::triggered, this, [this, scale](std::int32_t triggerIndex) {
        scale(scaleFactors.values().at(triggerIndex));
    });

    // Load directory from settings
    _directoryPickerAction.setDirectory(_scatterplotPlugin.getSetting(SETTING_KEY_OUTPUT_DIR).toString());

    // Save directory to settings when the current directory changes
    connect(&_directoryPickerAction, &DirectoryPickerAction::directoryChanged, this, [this](const QString& directory) {
        _scatterplotPlugin.setSetting(SETTING_KEY_OUTPUT_DIR, directory);
    });

    // Create the screenshot when the create action is triggered
    connect(&_dialogAction, &TriggerAction::triggered, this, [this](std::int32_t triggerIndex) {
        switch (triggerIndex)
        {
            case 0:
                createScreenshot();
                break;

            case 1:
                break;

            default:
                break;
        }
        
    });

    // Load from settings
    _lockAspectRatioAction.setChecked(_scatterplotPlugin.getSetting(SETTING_KEY_LOCK_ASPECT_RATIO, true).toBool());
    _backgroundColorAction.setColor(_scatterplotPlugin.getSetting(SETTING_KEY_BACKGROUND_COLOR, QVariant::fromValue(QColor(Qt::white))).value<QColor>());

    // Save the background color setting when the action is changed
    connect(&_backgroundColorAction, &ColorAction::colorChanged, this, [this](const QColor& color) {
        _scatterplotPlugin.setSetting(SETTING_KEY_BACKGROUND_COLOR, color);
    });

    // Update fixed range read-only
    const auto updateFixedRangeReadOnly = [this]() {
        _fixedRangeAction.setEnabled(_overrideRangesAction.isChecked());
    };

    // Update fixed range read-only when override ranges is toggled 
    connect(&_overrideRangesAction, &ToggleAction::toggled, this, updateFixedRangeReadOnly);

    // Perform initialization of actions
    updateAspectRatio();
    updateTargetHeightAction();
    updateFixedRangeReadOnly();

    initializeTargetSize();
    updateDimensionsPickerAction();
}

void ExportImageAction::initializeTargetSize()
{
    // Get size of the scatterplot widget
    const auto scatterPlotWidgetSize = _scatterplotPlugin.getScatterplotWidget().size();

    _targetWidthAction.initialize(1, 8 * scatterPlotWidgetSize.width(), scatterPlotWidgetSize.width(), scatterPlotWidgetSize.width());
    _targetHeightAction.initialize(1, 8 * scatterPlotWidgetSize.height(), scatterPlotWidgetSize.height(), scatterPlotWidgetSize.height());

    _aspectRatio = static_cast<float>(_targetHeightAction.getValue()) / static_cast<float>(_targetWidthAction.getValue());
}

void ExportImageAction::createScreenshot(bool defaultSettings /*= false*/)
{
    // Get output dir from settings
    const auto outputDir = _scatterplotPlugin.getSetting(SETTING_KEY_OUTPUT_DIR, "/").toString();

    /*
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
        }
        QApplication::restoreOverrideCursor();
    }
    */
}

void ExportImageAction::updateDimensionsPickerAction()
{
    _dimensionsPickerAction.setPointsDataset(_scatterplotPlugin.getPositionDataset());
}
