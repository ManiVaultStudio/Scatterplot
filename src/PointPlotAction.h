#pragma once

#include "PluginAction.h"

#include <QLabel>

using namespace hdps::gui;

class PointPlotAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, PointPlotAction* pointPlotAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    PointPlotAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    DecimalAction   _pointSizeAction;
    DecimalAction   _pointOpacityAction;

    static constexpr double DEFAULT_POINT_SIZE = 10.0;
    static constexpr double DEFAULT_POINT_OPACITY = 50.0;

    friend class Widget;
    friend class PlotAction;
};