#pragma once

#include "PluginAction.h"

#include <actions/StringAction.h>

#include <DimensionsPickerAction.h>

using namespace hdps::gui;

class BatchScreenshotAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, BatchScreenshotAction* batchScreenshotAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:
    BatchScreenshotAction(ScatterplotPlugin& scatterplotPlugin);

protected:

    /** Callback which is invoked when the current position dataset changes */
    void positionDatasetChanged();

protected: // Action getters

    ToggleAction& getBatchAction() { return _batchAction; }
    StringAction& getDimensionsPreviewAction() { return _dimensionsPreviewAction; }
    DimensionsPickerAction& getDimensionsPickerAction() { return _dimensionsPickerAction; }

protected:
    ToggleAction            _batchAction;                   /** Batch action */
    StringAction            _dimensionsPreviewAction;       /** Batch export preview dimensions action */
    DimensionsPickerAction  _dimensionsPickerAction;        /** Batch export dimensions action */

    friend class Widget;
};