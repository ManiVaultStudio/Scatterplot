#pragma once

#include "actions/PixelSelectionAction.h"
#include "util/PixelSelectionTool.h"
#include <actions/ToggleAction.h>
#include <actions/DecimalAction.h>

#include <QActionGroup>
#include <QDebug>

class ScatterplotPlugin;

using namespace hdps::gui;

class SelectionAction : public PixelSelectionAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, SelectionAction* selectionAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:
    SelectionAction(ScatterplotPlugin& scatterplotPlugin);

public:
    ToggleAction& getOutlineEnabledAction() { return _outlineEnabledAction; }
    DecimalAction& getOutlineScaleAction() { return _outlineScaleAction; }
    ToggleAction& getHaloEnabledAction() { return _haloEnabledAction; }
    DecimalAction& getHaloScaleAction() { return _haloScaleAction; }

protected:
    ScatterplotPlugin&  _scatterplotPlugin;     /** Reference to scatter plot plugin */
    ToggleAction        _outlineEnabledAction;  /** Selection outline enabled action */
    DecimalAction       _outlineScaleAction;    /** Selection outline scale action */
    ToggleAction        _haloEnabledAction;     /** Selection halo enabled action */
    DecimalAction       _haloScaleAction;       /** Selection halo scale action */
};