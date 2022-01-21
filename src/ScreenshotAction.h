#pragma once

#include <actions/IntegralAction.h>
#include <actions/ToggleAction.h>
#include <actions/TriggerAction.h>
#include <actions/ColorAction.h>

#include "BatchScreenshotAction.h"

using namespace hdps::util;

class ScatterplotPlugin;

namespace hdps {

/**
 * Screenshot action class
 *
 * Action for creating screenshots
 *
 * @author Thomas Kroes
 */
class ScreenshotAction : public hdps::gui::WidgetAction
{

protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, ScreenshotAction* screenshotAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     * @param scatterplotPlugin Reference to the scatter plot plugin
     */
    ScreenshotAction(QObject* parent, ScatterplotPlugin& scatterplotPlugin);

    /** Grab target size from scatter plot widget */
    void initializeTargetSize();

    /**
     * Create screenshot
     * @param defaultSettings Use default settings for creating the screenshot
     */
    void createScreenshot(bool defaultSettings = false);

protected:

    /** Callback which is invoked when the current position dataset changes */
    void positionDatasetChanged();

public: // Action getters

    IntegralAction& getTargetWidthAction() { return _targetWidthAction; }
    IntegralAction& getTargetHeightAction() { return _targetHeightAction; }
    ToggleAction& getLockAspectRatioAction() { return _lockAspectRatioAction; }
    TriggerAction& getScaleQuarterAction() { return _scaleQuarterAction; }
    TriggerAction& getScaleHalfAction() { return _scaleHalfAction; }
    TriggerAction& getScaleOneAction() { return _scaleOneAction; }
    TriggerAction& getScaleTwiceAction() { return _scaleTwiceAction; }
    TriggerAction& getScaleFourAction() { return _scaleFourAction; }
    ColorAction& getBackgroundColorAction() { return _backgroundColorAction; }
    BatchScreenshotAction& getBatchScreenshotAction() { return _batchScreenshotAction; }
    TriggerAction& getCreateAction() { return _createAction; }
    TriggerAction& getCreateDefaultAction() { return _createDefaultAction; }
    ToggleAction& getOpenAfterCreationAction() { return _openAfterCreationAction; }

protected:
    ScatterplotPlugin&          _scatterplotPlugin;         /** Reference to the scatter plot plugin */
    IntegralAction              _targetWidthAction;         /** Screenshot target width action */
    IntegralAction              _targetHeightAction;        /** Screenshot target height action */
    ToggleAction                _lockAspectRatioAction;     /** Lock aspect ration action */
    TriggerAction               _scaleQuarterAction;        /** Scale 1/4 action */
    TriggerAction               _scaleHalfAction;           /** Scale 1/2 action */
    TriggerAction               _scaleOneAction;            /** Scale 100% of widget action */
    TriggerAction               _scaleTwiceAction;          /** Scale two times action */
    TriggerAction               _scaleFourAction;           /** Scale four times action */
    ColorAction                 _backgroundColorAction;     /** Background color action */
    BatchScreenshotAction       _batchScreenshotAction;     /** Batch screenshot action */
    TriggerAction               _createAction;              /** Create action */
    TriggerAction               _createDefaultAction;       /** Create with default settings action */
    ToggleAction                _openAfterCreationAction;   /** Open screenshot after creation action */
    float                       _aspectRatio;               /** Screenshot aspect ratio */

    /** Setting prefixes */
    static QString SETTING_KEY_OUTPUT_DIR;              /** Default output directory */
    static QString SETTING_KEY_LOCK_ASPECT_RATIO;       /** Lock the image aspect ratio */
    static QString SETTING_KEY_BACKGROUND_COLOR;        /** Screenshot background color */
    static QString SETTING_KEY_OPEN_AFTER_CREATION;     /** Whether screenshot images should be opened after creation */
};

}
