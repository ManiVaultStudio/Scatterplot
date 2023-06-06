#include "PlotAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "Application.h"

using namespace hdps::gui;

PlotAction::PlotAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(nullptr),
    _pointPlotAction(this, "Point"),
    _densityPlotAction(this, "Density")
{
    setToolTip("Plot settings");
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("paint-brush"));
    setDefaultWidgetFlags(GroupAction::Horizontal);

    addAction(&_pointPlotAction.getSizeAction());
    addAction(&_pointPlotAction.getOpacityAction());
    addAction(&_pointPlotAction.getFocusSelection());
    
    addAction(&_densityPlotAction.getSigmaAction());
    addAction(&_densityPlotAction.getContinuousUpdatesAction());
}

void PlotAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    _pointPlotAction.initialize(_scatterplotPlugin);
    _densityPlotAction.initialize(_scatterplotPlugin);

    auto& scatterplotWidget = _scatterplotPlugin->getScatterplotWidget();

    const auto updateRenderMode = [this, &scatterplotWidget]() -> void {
        _pointPlotAction.setVisible(scatterplotWidget.getRenderMode() == ScatterplotWidget::SCATTERPLOT);
        _densityPlotAction.setVisible(scatterplotWidget.getRenderMode() != ScatterplotWidget::SCATTERPLOT);
    };

    updateRenderMode();

    connect(&scatterplotWidget, &ScatterplotWidget::renderModeChanged, this, updateRenderMode);
}

QMenu* PlotAction::getContextMenu()
{
    if (_scatterplotPlugin == nullptr)
        return nullptr;

    switch (_scatterplotPlugin->getScatterplotWidget().getRenderMode())
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
