#pragma once

#include "PluginAction.h"
#include "PointPlotAction.h"
#include "DensityPlotAction.h"

using namespace hdps::gui;

class PlotAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, PlotAction* plotAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    PlotAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    PointPlotAction     _pointPlotAction;
    DensityPlotAction   _densityPlotAction;

    friend class Widget;
};