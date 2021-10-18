#pragma once

#include "PluginAction.h"

#include <QLabel>

using namespace hdps::gui;

class PointPlotAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, PointPlotAction* pointPlotAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
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