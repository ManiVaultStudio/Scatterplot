#pragma once

#include "PluginAction.h"

#include <QLabel>

using namespace hdps::gui;

class DensityPlotAction : public PluginAction
{
protected:
    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, DensityPlotAction* densityPlotAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    DensityPlotAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    DecimalAction       _sigmaAction;

    static constexpr double DEFAULT_SIGMA = 25.0;

    friend class Widget;
    friend class PlotAction;
};