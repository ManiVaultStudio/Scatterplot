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

protected:
    ScatterplotPlugin&  _scatterplotPlugin;     /** Reference to scatter plot plugin */
    ToggleAction        _showOutlineAction;     /** Show outline action */
    DecimalAction       _outlineScaleAction;    /** Selection outline scale action */
};