#include "RenderModeAction.h"
#include "Application.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

RenderModeAction::RenderModeAction(QObject* parent, const QString& title) :
    OptionAction(parent, title, { "Scatter", "Density", "Contour" }),
    _scatterplotPlugin(nullptr),
    _scatterPlotAction(this, "Scatter"),
    _densityPlotAction(this, "Density"),
    _contourPlotAction(this, "Contour")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("image"));
    setDefaultWidgetFlags(OptionAction::HorizontalButtons);

    _scatterPlotAction.setConnectionPermissionsToForceNone(true);
    _densityPlotAction.setConnectionPermissionsToForceNone(true);
    _contourPlotAction.setConnectionPermissionsToForceNone(true);

    _scatterPlotAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _densityPlotAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _contourPlotAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);

    _scatterPlotAction.setShortcut(QKeySequence("S"));
    _densityPlotAction.setShortcut(QKeySequence("D"));
    _contourPlotAction.setShortcut(QKeySequence("C"));

    _scatterPlotAction.setToolTip("Set render mode to scatter plot (S)");
    _densityPlotAction.setToolTip("Set render mode to density plot (D)");
    _contourPlotAction.setToolTip("Set render mode to contour plot (C)");
}

void RenderModeAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    _scatterplotPlugin->getWidget().addAction(&_scatterPlotAction);
    _scatterplotPlugin->getWidget().addAction(&_densityPlotAction);
    _scatterplotPlugin->getWidget().addAction(&_contourPlotAction);

    const auto currentIndexChanged = [this]() {
        const auto renderMode = static_cast<RenderMode>(getCurrentIndex());

        _scatterPlotAction.setChecked(renderMode == RenderMode::ScatterPlot);
        _densityPlotAction.setChecked(renderMode == RenderMode::DensityPlot);
        _contourPlotAction.setChecked(renderMode == RenderMode::ContourPlot);

        _scatterplotPlugin->getScatterplotWidget().setRenderMode(static_cast<ScatterplotWidget::RenderMode>(getCurrentIndex()));
    };

    currentIndexChanged();

    connect(this, &OptionAction::currentIndexChanged, this, currentIndexChanged);

    connect(&_scatterPlotAction, &QAction::toggled, this, [this, scatterplotPlugin](bool toggled) {
        if (toggled)
            setCurrentIndex(static_cast<std::int32_t>(RenderMode::ScatterPlot));
    });

    connect(&_densityPlotAction, &QAction::toggled, this, [this, scatterplotPlugin](bool toggled) {
        if (toggled)
            setCurrentIndex(static_cast<std::int32_t>(RenderMode::DensityPlot));
    });

    connect(&_contourPlotAction, &QAction::toggled, this, [this, scatterplotPlugin](bool toggled) {
        if (toggled)
            setCurrentIndex(static_cast<std::int32_t>(RenderMode::ContourPlot));
    });

    setCurrentIndex(static_cast<std::int32_t>(RenderMode::ScatterPlot));
}

QMenu* RenderModeAction::getContextMenu()
{
    auto menu = new QMenu("Render mode");

    menu->addAction(&_scatterPlotAction);
    menu->addAction(&_densityPlotAction);
    menu->addAction(&_contourPlotAction);

    return menu;
}

void RenderModeAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    OptionAction::connectToPublicAction(publicAction, recursive);
}

void RenderModeAction::disconnectFromPublicAction(bool recursive)
{
    OptionAction::disconnectFromPublicAction(recursive);
}

void RenderModeAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _scatterPlotAction.fromParentVariantMap(variantMap);
    _densityPlotAction.fromParentVariantMap(variantMap);
    _contourPlotAction.fromParentVariantMap(variantMap);
}

QVariantMap RenderModeAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _scatterPlotAction.insertIntoVariantMap(variantMap);
    _densityPlotAction.insertIntoVariantMap(variantMap);
    _contourPlotAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
