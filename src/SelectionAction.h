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
    ToggleAction& getOutlineOverrideColorAction() { return _outlineOverrideColorAction; }
    DecimalAction& getOutlineScaleAction() { return _outlineScaleAction; }
    DecimalAction& getOutlineOpacityAction() { return _outlineOpacityAction; }
    ToggleAction& getHaloEnabledAction() { return _haloEnabledAction; }

protected:
    ScatterplotPlugin&  _scatterplotPlugin;             /** Reference to scatter plot plugin */
    ToggleAction        _outlineEnabledAction;          /** Selection outline enabled action */
    ToggleAction        _outlineOverrideColorAction;    /** Selection outline override color action */
    DecimalAction       _outlineScaleAction;            /** Selection outline scale action */
    DecimalAction       _outlineOpacityAction;          /** Selection outline opacity action */
    ToggleAction        _haloEnabledAction;             /** Selection halo enabled action */
};