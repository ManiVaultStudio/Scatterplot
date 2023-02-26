#include "PlotAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "Application.h"

#include <QVBoxLayout>
#include <QHBoxLayout>

using namespace hdps::gui;

PlotAction::PlotAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, scatterplotPlugin, "Plot"),
    _pointPlotAction(this, scatterplotPlugin),
    _densityPlotAction(this, scatterplotPlugin)
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("paint-brush"));

    const auto updateRenderMode = [this]() -> void {
        _pointPlotAction.setVisible(getScatterplotWidget().getRenderMode() == ScatterplotWidget::SCATTERPLOT);
        _densityPlotAction.setVisible(getScatterplotWidget().getRenderMode() != ScatterplotWidget::SCATTERPLOT);
    };

    connect(&getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, updateRenderMode](const ScatterplotWidget::RenderMode& renderMode) {
        updateRenderMode();
    });

    updateRenderMode();
}

QMenu* PlotAction::getContextMenu()
{
    switch (getScatterplotWidget().getRenderMode())
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

void PlotAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _pointPlotAction.fromParentVariantMap(variantMap);
    _densityPlotAction.fromParentVariantMap(variantMap);
}

QVariantMap PlotAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _pointPlotAction.insertIntoVariantMap(variantMap);
    _densityPlotAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

PlotAction::Widget::Widget(QWidget* parent, PlotAction* plotAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, plotAction, widgetFlags)
{
    QWidget* pointPlotWidget    = nullptr;
    QWidget* densityPlotWidget  = nullptr;

    if (widgetFlags & PopupLayout) {
        pointPlotWidget     = plotAction->_pointPlotAction.createWidget(this, WidgetActionWidget::PopupLayout);
        densityPlotWidget   = plotAction->_densityPlotAction.createWidget(this, WidgetActionWidget::PopupLayout);

        auto layout = new QVBoxLayout();

        layout->addWidget(pointPlotWidget);
        layout->addWidget(densityPlotWidget);

        setPopupLayout(layout);
    }
    else {
        pointPlotWidget = plotAction->_pointPlotAction.createWidget(this);
        densityPlotWidget = plotAction->_densityPlotAction.createWidget(this);

        auto layout = new QHBoxLayout();

        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(pointPlotWidget);
        layout->addWidget(densityPlotWidget);

        setLayout(layout);
    }

    const auto updateRenderMode = [plotAction, pointPlotWidget, densityPlotWidget]() -> void {
        const auto renderMode = plotAction->getScatterplotWidget().getRenderMode();

        pointPlotWidget->setVisible(renderMode == ScatterplotWidget::RenderMode::SCATTERPLOT);
        densityPlotWidget->setVisible(renderMode != ScatterplotWidget::RenderMode::SCATTERPLOT);
    };

    connect(&plotAction->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, updateRenderMode](const ScatterplotWidget::RenderMode& renderMode) {
        updateRenderMode();
    });

    updateRenderMode();
}
