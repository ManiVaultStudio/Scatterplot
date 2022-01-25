#pragma once

#include <actions/GroupAction.h>
#include <actions/IntegralAction.h>
#include <actions/ToggleAction.h>
#include <actions/TriggerAction.h>
#include <actions/TriggersAction.h>
#include <actions/ColorAction.h>
#include <actions/StringAction.h>

#include <DimensionsPickerAction.h>

using namespace hdps::gui;

class ScatterplotPlugin;

/**
 * Export image action class
 *
 * Action for export to image
 *
 * @author Thomas Kroes
 */
class ExportImageAction : public GroupAction
{
public:

    /** Scale options */
    enum Scale {
        Eighth,         /** Scale by one-eight */
        Quarter,        /** Scale by a quarter */
        Half,           /** Scale by a half */
        One,            /** Do not scale */
        Twice,          /** Scale by a factor of two */
        Thrice,         /** Scale by a factor of three */
        Four,           /** Scale by a factor of four */
        Eight           /** Scale by a factor of eight */
    };

    static const QMap<Scale, TriggersAction::Trigger> triggers;     /** Maps scale enum to trigger */
    static const QMap<Scale, float> scaleFactors;                   /** Maps scale enum to scale factor */

public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     * @param scatterplotPlugin Reference to the scatter plot plugin
     */
    ExportImageAction(QObject* parent, ScatterplotPlugin& scatterplotPlugin);

    /** Grab target size from scatter plot widget */
    void initializeTargetSize();

    /**
     * Create screenshot
     * @param defaultSettings Use default settings for creating the screenshot
     */
    void createScreenshot(bool defaultSettings = false);

protected:

    /** Update the input points dataset of the dimensions picker action */
    void updateDimensionsPickerAction();

public: // Action getters

    DimensionsPickerAction& getDimensionsPickerAction() { return _dimensionsPickerAction; }
    IntegralAction& getTargetWidthAction() { return _targetWidthAction; }
    IntegralAction& getTargetHeightAction() { return _targetHeightAction; }
    ToggleAction& getLockAspectRatioAction() { return _lockAspectRatioAction; }
    TriggersAction& getScaleAction() { return _scaleAction; }
    ColorAction& getBackgroundColorAction() { return _backgroundColorAction; }
    ToggleAction& getOverrideRangesAction() { return _overrideRangesAction; }
    DecimalRangeAction& getFixedRangeAction() { return _fixedRangeAction; }
    DirectoryPickerAction& getDirectoryPickerAction() { return _directoryPickerAction; }
    TriggersAction& getDialogAction() { return _dialogAction; }

protected:
    ScatterplotPlugin&          _scatterplotPlugin;         /** Reference to scatterplot plugin */
    DimensionsPickerAction      _dimensionsPickerAction;    /** Dimensions picker action */
    IntegralAction              _targetWidthAction;         /** Screenshot target width action */
    IntegralAction              _targetHeightAction;        /** Screenshot target height action */
    ToggleAction                _lockAspectRatioAction;     /** Lock aspect ratio action */
    TriggersAction              _scaleAction;               /** Scale action */
    ColorAction                 _backgroundColorAction;     /** Background color action */
    ToggleAction                _overrideRangesAction;      /** Override ranges action */
    DecimalRangeAction          _fixedRangeAction;          /** Fixed range action */
    DirectoryPickerAction       _directoryPickerAction;     /** Directory picker action */
    StringAction                _fileNamePrefixAction;      /** File name prefix action */
    TriggersAction              _dialogAction;              /** Create action */
    float                       _aspectRatio;               /** Screenshot aspect ratio */

    /** Setting prefixes */
    static QString SETTING_KEY_OUTPUT_DIR;              /** Default output directory */
    static QString SETTING_KEY_LOCK_ASPECT_RATIO;       /** Lock the image aspect ratio */
    static QString SETTING_KEY_BACKGROUND_COLOR;        /** Screenshot background color */
    static QString SETTING_KEY_OPEN_AFTER_CREATION;     /** Whether screenshot images should be opened after creation */
};