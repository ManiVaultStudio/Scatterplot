#include "PlotAction.h"
#include "ScatterplotWidget.h"
#include "Application.h"

using namespace hdps::gui;

PlotAction::PlotAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Plot"),
    _pointPlotAction(scatterplotPlugin),
    _densityPlotAction(scatterplotPlugin)
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("paint-brush"));

    const auto updateRenderMode = [this]() -> void {
        _pointPlotAction.setVisible(getScatterplotWidget()->getRenderMode() == ScatterplotWidget::SCATTERPLOT);
        _densityPlotAction.setVisible(getScatterplotWidget()->getRenderMode() != ScatterplotWidget::SCATTERPLOT);
    };

    connect(getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, updateRenderMode](const ScatterplotWidget::RenderMode& renderMode) {
        updateRenderMode();
    });

    updateRenderMode();
}

QMenu* PlotAction::getContextMenu()
{
    switch (getScatterplotWidget()->getRenderMode())
    {
        case ScatterplotWidget::RenderMode::SCATTERPLOT:
            return _pointPlotAction.getContextMenu();
            break;

        case ScatterplotWidget::RenderMode::DENSITY:
        case ScatterplotWidget::RenderMode::LANDSCAPE:
            return _densityPlotAction.getContextMenu();
            break;

        default:
            break;
    }

    return new QMenu("Plot");
}

PlotAction::Widget::Widget(QWidget* parent, PlotAction* plotAction, const Widget::State& state) :
    WidgetAction::Widget(parent, plotAction, state)
{
    QWidget* pointPlotWidget    = nullptr;
    QWidget* densityPlotWidget  = nullptr;

    switch (state)
    {
        case Widget::State::Standard:
        {
            pointPlotWidget     = plotAction->_pointPlotAction.getWidget(this, State::Standard);
            densityPlotWidget   = plotAction->_densityPlotAction.getWidget(this, State::Standard);
            
            auto layout = new QHBoxLayout();

            layout->setMargin(0);
            layout->addWidget(pointPlotWidget);
            layout->addWidget(densityPlotWidget);

            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            pointPlotWidget     = plotAction->_pointPlotAction.getWidget(this, State::Popup);
            densityPlotWidget   = plotAction->_densityPlotAction.getWidget(this, State::Popup);
            
            auto layout = new QVBoxLayout();

            layout->addWidget(pointPlotWidget);
            layout->addWidget(densityPlotWidget);

            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }

    const auto updateRenderMode = [plotAction, pointPlotWidget, densityPlotWidget]() -> void {
        const auto renderMode = plotAction->getScatterplotWidget()->getRenderMode();

        pointPlotWidget->setVisible(renderMode == ScatterplotWidget::RenderMode::SCATTERPLOT);
        densityPlotWidget->setVisible(renderMode != ScatterplotWidget::RenderMode::SCATTERPLOT);
    };

    connect(plotAction->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, updateRenderMode](const ScatterplotWidget::RenderMode& renderMode) {
        updateRenderMode();
    });

    updateRenderMode();
}
