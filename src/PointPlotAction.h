#pragma once

#include "PluginAction.h"

#include "ScalarAction.h"

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

    /** Update default datasets (candidates are children of points type and with matching number of points) */
    void updateDefaultDatasets();

    /** Update the scatter plot widget point size */
    void updateScatterPlotWidgetPointSize();

    /** Update the scatter plot widget point size scalars */
    void updateScatterPlotWidgetPointSizeScalars();

    /** Update the scatter plot widget point opacity */
    void updateScatterPlotWidgetPointOpacity();

    /** Update the scatter plot widget point opacity scalars */
    void updateScatterPlotWidgetPointOpacityScalars();

public: // Action getters

    ScalarAction& getSizeAction() { return _sizeAction; }
    ScalarAction& getOpacityAction() { return _opacityAction; }

protected:
    ScalarAction    _sizeAction;        /** Point size action */
    ScalarAction    _opacityAction;     /** Point opacity action */

    static constexpr double DEFAULT_POINT_SIZE      = 10.0;     /** Default point size */
    static constexpr double DEFAULT_POINT_OPACITY   = 50.0;     /** Default point opacity */

    friend class PlotAction;
};