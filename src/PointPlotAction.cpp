#include "PointPlotAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

PointPlotAction::PointPlotAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Point"),
    _pointSizeAction(this, "Point size", 1.0, 50.0, DEFAULT_POINT_SIZE, DEFAULT_POINT_SIZE),
    _pointOpacityAction(this, "Point opacity", 0.0, 100.0, DEFAULT_POINT_OPACITY, DEFAULT_POINT_OPACITY)
{
    _scatterplotPlugin->addAction(&_pointSizeAction);
    _scatterplotPlugin->addAction(&_pointOpacityAction);

    _pointSizeAction.setSuffix("px");
    _pointOpacityAction.setSuffix("%");

    const auto updatePointSize = [this]() -> void {
        getScatterplotWidget()->setPointSize(_pointSizeAction.getValue());
    };

    const auto updatePointOpacity = [this]() -> void {
        getScatterplotWidget()->setAlpha(0.01 * _pointOpacityAction.getValue());
    };

    connect(&_pointSizeAction, &DecimalAction::valueChanged, this, [this, updatePointSize](const double& value) {
        updatePointSize();
    });

    connect(&_pointOpacityAction, &DecimalAction::valueChanged, this, [this, updatePointOpacity](const double& value) {
        updatePointOpacity();
    });

    updatePointSize();
    updatePointOpacity();
}

QMenu* PointPlotAction::getContextMenu()
{
    auto menu = new QMenu("Plot settings");

    const auto renderMode = getScatterplotWidget()->getRenderMode();

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_pointSizeAction);
    addActionToMenu(&_pointOpacityAction);

    return menu;
}

PointPlotAction::Widget::Widget(QWidget* parent, PointPlotAction* pointPlotAction, const Widget::State& state) :
    WidgetActionWidget(parent, pointPlotAction, state)
{
    setToolTip("Point plot settings");

    auto pointSizelabel     = pointPlotAction->_pointSizeAction.createLabelWidget(this);
    auto pointOpacitylabel  = pointPlotAction->_pointOpacityAction.createLabelWidget(this);
    auto pointSizeWidget    = pointPlotAction->_pointSizeAction.createWidget(this);
    auto pointOpacityWidget = pointPlotAction->_pointOpacityAction.createWidget(this);

    switch (state)
    {
        case Widget::State::Standard:
        {
            auto layout = new QHBoxLayout();

            layout->setMargin(0);
            layout->addWidget(pointSizelabel);
            layout->addWidget(pointSizeWidget);
            layout->addWidget(pointOpacitylabel);
            layout->addWidget(pointOpacityWidget);

            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            auto layout = new QGridLayout();

            layout->setMargin(0);
            layout->addWidget(pointSizelabel, 0, 0);
            layout->addWidget(pointSizeWidget, 0, 2);
            layout->addWidget(pointOpacitylabel, 1, 0);
            layout->addWidget(pointOpacityWidget, 1, 2);

            setLayout(layout);
            break;
        }

        default:
            break;
    }
}