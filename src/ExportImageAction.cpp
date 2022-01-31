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
QString ExportImageAction::SETTING_KEY_ENABLED_DIMENSION    = "Export/Image/EnabledDimensions";

ExportImageAction::ExportImageAction(QObject* parent, ScatterplotPlugin& scatterplotPlugin) :
    GroupAction(parent),
    _scatterplotPlugin(scatterplotPlugin),
    _dimensionSelectionAction(this),
    _setDefaultDimensionsAction(this),
    _targetWidthAction(this, "Width ", 1, 10000),
    _targetHeightAction(this, "Height", 1, 10000),
    _lockAspectRatioAction(this, "Lock aspect ratio", true, true),
    _scaleAction(this, "Scale", triggers.values().toVector()),
    _backgroundColorAction(this, "Background color", QColor(Qt::white), QColor(Qt::white)),
    _overrideRangesAction(this, "Override ranges", false, false),
    _fixedRangeAction(this, "Fixed range"),
    _fileNamePrefixAction(this, "Filename prefix", scatterplotPlugin.getPositionDataset()->getGuiName() + "_", scatterplotPlugin.getPositionDataset()->getGuiName() + "_"),
    _statusAction(this, "Status"),
    _outputDirectoryAction(this, "Save to"),
    _exportCancelAction(this, "", { TriggersAction::Trigger("Export", "Export dimensions"), TriggersAction::Trigger("Cancel", "Cancel export")  }),
    _aspectRatio()
{
    _dimensionSelectionAction.setMayReset(false);
    _targetWidthAction.setMayReset(false);
    _targetHeightAction.setMayReset(false);
    _lockAspectRatioAction.setMayReset(false);
    _scaleAction.setMayReset(false);
    _backgroundColorAction.setMayReset(false);
    _overrideRangesAction.setMayReset(false);
    _fixedRangeAction.setMayReset(false);
    _fileNamePrefixAction.setMayReset(false);
    _outputDirectoryAction.setMayReset(false);
    _exportCancelAction.setMayReset(false);

    _targetWidthAction.setSuffix("px");
    _targetHeightAction.setSuffix("px");

    // Update dimensions picker when the position dataset changes
    connect(&scatterplotPlugin.getPositionDataset(), &Dataset<Points>::changed, this, &ExportImageAction::updateDimensionsPickerAction);

    // Update the state of the target height action
    const auto updateTargetHeightAction = [this]() -> void {
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
    _outputDirectoryAction.setDirectory(_scatterplotPlugin.getSetting(SETTING_KEY_OUTPUT_DIR).toString());

    // Save directory to settings when the current directory changes
    connect(&_outputDirectoryAction, &DirectoryPickerAction::directoryChanged, this, [this](const QString& directory) {
        _scatterplotPlugin.setSetting(SETTING_KEY_OUTPUT_DIR, directory);
    });

    // Create the screenshot when the create action is triggered
    connect(&_exportCancelAction, &TriggersAction::triggered, this, [this](std::int32_t triggerIndex) {
        switch (triggerIndex)
        {
            case 0:
                exportImages();
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

    // Updates the export trigger when the file name prefix, output directory or the dimension model changes
    connect(&_fileNamePrefixAction, &StringAction::stringChanged, this, &ExportImageAction::updateExportTrigger);
    connect(&_outputDirectoryAction, &DirectoryPickerAction::directoryChanged, this, &ExportImageAction::updateExportTrigger);

    // Save selected dimensions when the action is triggered
    connect(&_setDefaultDimensionsAction, &TriggerAction::triggered, this, &ExportImageAction::setDefaultDimensions);

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

void ExportImageAction::exportImages()
{
    // Get output dir from settings
    const auto outputDir = _scatterplotPlugin.getSetting(SETTING_KEY_OUTPUT_DIR, "/").toString();

    // Get reference to the coloring action
    auto& coloringAction = _scatterplotPlugin.getSettingsAction().getColoringAction();

    // Cache the coloring type and dimension index
    const auto colorByIndex     = coloringAction.getColorByAction().getCurrentIndex();
    const auto dimensionIndex   = coloringAction.getDimensionAction().getCurrentDimensionIndex();

    // Set the coloring type to dimension
    coloringAction.getColorByAction().setCurrentIndex(1);

    auto numberOfExportedImages = 0;

    // Update status message
    _statusAction.setMessage("Exporting...");

    QApplication::setOverrideCursor(Qt::WaitCursor);
    {
        // Temporarily disable the export trigger
        _exportCancelAction.setTriggerEnabled(0, false);

        // Get enabled dimensions from dimension picker action
        const auto enabledDimensions = _dimensionSelectionAction.getEnabledDimensions();

        // Get screenshot dimensions and background color
        const auto width            = _targetWidthAction.getValue();
        const auto height           = _targetHeightAction.getValue();
        const auto backgroundColor  = _backgroundColorAction.getColor();
        const auto dimensionNames   = _scatterplotPlugin.getPositionDataset()->getDimensionNames();

        _statusAction.setStatus(StatusAction::Info);

        for (std::int32_t dimensionIndex = 0; dimensionIndex < enabledDimensions.size(); dimensionIndex++) {

            // Continue if the dimension is not enabled (not flagged for export)
            if (!enabledDimensions[dimensionIndex])
                continue;

            // Establish file name
            const auto fileName = _fileNamePrefixAction.getString() + dimensionNames[dimensionIndex] + ".png";

            // Update status
            _statusAction.setMessage("Export " + fileName + " (" + QString::number(numberOfExportedImages + 1) + "/" + QString::number(getNumberOfSelectedDimensions()) + ")");

            // Ensure status is updated properly
            QCoreApplication::processEvents();

            // Establish file path of the output image
            const auto imageFilePath = _outputDirectoryAction.getDirectory() + "/" + fileName;

            // Only proceed if the directory exists
            if (!QDir(_outputDirectoryAction.getDirectory()).exists()) {
                _statusAction.setStatus(StatusAction::Error);
                _statusAction.setMessage(_outputDirectoryAction.getDirectory() + "does not exist, aborting", true);
                break;
            }

            // Cache reference to the scatterplot widget
            auto& scatterplotWidget = _scatterplotPlugin.getScatterplotWidget();

            // Set the current dimension name
            coloringAction.getDimensionAction().setCurrentDimensionName(dimensionNames[dimensionIndex]);

            if (_overrideRangesAction.isChecked()) {
                auto& rangeAction = coloringAction.getColorMapAction().getSettingsAction().getHorizontalAxisAction().getRangeAction();

                rangeAction.initialize(_fixedRangeAction.getMinimum(), _fixedRangeAction.getMaximum(), _fixedRangeAction.getMinimum(), _fixedRangeAction.getMaximum());
            }

            // Create and save the image
            scatterplotWidget.createScreenshot(width, height, imageFilePath, backgroundColor);

            numberOfExportedImages++;
        }

        // Turn the export trigger back on
        _exportCancelAction.setTriggerEnabled(0, true);
    }
    QApplication::restoreOverrideCursor();

    // Reset the coloring type and dimension index
    coloringAction.getColorByAction().setCurrentIndex(colorByIndex);
    coloringAction.getDimensionAction().setCurrentDimensionIndex(dimensionIndex);

    // Update status message
    _statusAction.setMessage("Exported " + QString::number(numberOfExportedImages) + " image" + (numberOfExportedImages > 1 ? "s" : ""), true);
}

void ExportImageAction::updateDimensionsPickerAction()
{
    _dimensionSelectionAction.setPointsDataset(_scatterplotPlugin.getPositionDataset());

    // Update export trigger when the item model data changes or the model is reset
    connect(&_dimensionSelectionAction.getItemModel(), &QAbstractItemModel::dataChanged, this, &ExportImageAction::updateExportTrigger);
    connect(&_dimensionSelectionAction.getProxyModel(), &QAbstractItemModel::modelReset, this, &ExportImageAction::updateExportTrigger);

    // Initial update of export trigger
    updateExportTrigger();

    // Update the set default dimensions action to reflect the new name of the position dataset
    _setDefaultDimensionsAction.setText("Set default dimensions for " + _scatterplotPlugin.getPositionDataset()->getGuiName());
}

bool ExportImageAction::mayExport() const
{
    if (_fileNamePrefixAction.getString().isEmpty())
        return false;

    if (!QDir(_outputDirectoryAction.getDirectory()).exists())
        return false;

    if (getNumberOfSelectedDimensions() == 0)
        return false;

    return true;
}

std::int32_t ExportImageAction::getNumberOfSelectedDimensions() const
{
    const auto enabledDimensions = _dimensionSelectionAction.getEnabledDimensions();

    return std::count_if(enabledDimensions.begin(), enabledDimensions.end(), [](bool value) {
        return value;
    });
}

void ExportImageAction::updateExportTrigger()
{
    _exportCancelAction.setTriggerText(0, getNumberOfSelectedDimensions() == 0 ? "Nothing to export" : "Export (" + QString::number(getNumberOfSelectedDimensions()) + ")");
    _exportCancelAction.setTriggerTooltip(0, getNumberOfSelectedDimensions() == 0 ? "There are no images selected to export" : "Export " + QString::number(getNumberOfSelectedDimensions()) + " image" + (getNumberOfSelectedDimensions() >= 2 ? "s" : "") + " to disk");
    _exportCancelAction.setTriggerEnabled(0, mayExport());
}

void ExportImageAction::setDefaultDimensions()
{
    QVariantList enabledDimensions;

    for (const auto& enabledDimension : _dimensionSelectionAction.getEnabledDimensions())
        enabledDimensions.push_back(QVariant(enabledDimension));

    _scatterplotPlugin.setSetting(getEnabledDimensionsSettingsKey(), enabledDimensions);
}

QString ExportImageAction::getEnabledDimensionsSettingsKey() const
{
    return SETTING_KEY_ENABLED_DIMENSION + "/" + _scatterplotPlugin.getPositionDataset()->getGuiName();
}
