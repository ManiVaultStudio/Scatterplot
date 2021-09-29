#pragma once

#include "actions/PixelSelectionAction.h"
#include "util/PixelSelectionTool.h"

#include <QActionGroup>
#include <QDebug>

class ScatterplotPlugin;

using namespace hdps::gui;

class SelectionAction : public PixelSelectionAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, SelectionAction* selectionAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    SelectionAction(ScatterplotPlugin& scatterplotPlugin);

protected:
    ScatterplotPlugin&  _scatterplotPlugin;     /** Reference to scatter plot plugin */
};