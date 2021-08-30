#pragma once

#include "PluginAction.h"

#include <QActionGroup>
#include <QHBoxLayout>

using namespace hdps::gui;

class QMenu;

class RenderModeAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, RenderModeAction* renderModeAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    RenderModeAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    ToggleAction    _scatterPlotAction;
    ToggleAction    _densityPlotAction;
    ToggleAction    _contourPlotAction;
    QActionGroup    _actionGroup;

    friend class Widget;
};