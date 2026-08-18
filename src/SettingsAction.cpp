#include "SettingsAction.h"

#include "ScatterplotPlugin.h"

#include <PointData/PointData.h>

#include <QMenu>

#include "ScatterplotWidget.h"

using namespace mv::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent)),
    _renderModeAction(this, "Render Mode"),
    _positionAction(this, "Position"),
    _selectionAction(this, "Selection"),
    _zOrderingAction(this, "Z ordering"),
    _plotAction(this, "Plot"),
    _coloringAction(this, "Coloring"),
    _subsetAction(this, "Subset"),
    _clusteringAction(this, "Clustering"),
    _exportAction(this, "Export"),
    _miscellaneousAction(this, "Miscellaneous"),
    _datasetsAction(this, "Datasets")
{
    setConnectionPermissionsToForceNone();

    _renderModeAction.initialize(_scatterplotPlugin);
    _selectionAction.initialize(_scatterplotPlugin);
    _zOrderingAction.initialize(_scatterplotPlugin);
    _plotAction.initialize(_scatterplotPlugin);
    _subsetAction.initialize(_scatterplotPlugin);
    _exportAction.initialize(_scatterplotPlugin);

    const auto updateEnabled = [this]() {
        const auto enabled = _scatterplotPlugin->getPositionDataset().isValid();

        _plotAction.setEnabled(enabled);
        _positionAction.setEnabled(enabled);
        _coloringAction.setEnabled(enabled);
        _zOrderingAction.setEnabled(enabled);
    };

    updateEnabled();

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateEnabled);
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu();

    menu->addMenu(_renderModeAction.getContextMenu());
    menu->addMenu(_plotAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_positionAction.getContextMenu());
    menu->addMenu(_zOrderingAction.getContextMenu());
    menu->addMenu(_coloringAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_subsetAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_miscellaneousAction.getContextMenu());

    return menu;
}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    const auto containsZOrderingSettings = variantMap.contains("Z ordering");

    _datasetsAction.fromParentVariantMap(variantMap);
    _plotAction.fromParentVariantMap(variantMap);
    _positionAction.fromParentVariantMap(variantMap);
    _zOrderingAction.fromParentVariantMap(variantMap, true);
    _coloringAction.fromParentVariantMap(variantMap);
    _subsetAction.fromParentVariantMap(variantMap, true);
    _clusteringAction.fromParentVariantMap(variantMap, true);
    _renderModeAction.fromParentVariantMap(variantMap);
    _selectionAction.fromParentVariantMap(variantMap);
    _miscellaneousAction.fromParentVariantMap(variantMap);

    // Migrate projects saved before z ordering became a dedicated action.
    if (!containsZOrderingSettings) {
        const auto miscellaneousMap = variantMap.value("Miscellaneous").toMap();
        const auto randomizedDepthMap = miscellaneousMap.value("Randomized depth").toMap();

        if (!randomizedDepthMap.isEmpty()) {
            const auto mode = randomizedDepthMap.value("Value").toBool() ? ZOrderingAction::Mode::Randomized : ZOrderingAction::Mode::InsertionOrder;

            _zOrderingAction.getModeAction().setCurrentIndex(static_cast<std::int32_t>(mode));
        }
    }

    if (variantMap.contains("PointRendererNavigation"))
        _scatterplotPlugin->getScatterplotWidget().getPointRendererNavigator().getNavigationAction().fromVariantMap(variantMap["PointRendererNavigation"].toMap());

    if (variantMap.contains("DensityRendererNavigation"))
        _scatterplotPlugin->getScatterplotWidget().getDensityRendererNavigator().getNavigationAction().fromVariantMap(variantMap["DensityRendererNavigation"].toMap());
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _datasetsAction.insertIntoVariantMap(variantMap);
    _renderModeAction.insertIntoVariantMap(variantMap);
    _plotAction.insertIntoVariantMap(variantMap);
    _positionAction.insertIntoVariantMap(variantMap);
    _zOrderingAction.insertIntoVariantMap(variantMap);
    _coloringAction.insertIntoVariantMap(variantMap);
    _subsetAction.insertIntoVariantMap(variantMap);
    _clusteringAction.insertIntoVariantMap(variantMap);
    _selectionAction.insertIntoVariantMap(variantMap);
    _miscellaneousAction.insertIntoVariantMap(variantMap);

    variantMap["PointRendererNavigation"]   = _scatterplotPlugin->getScatterplotWidget().getPointRendererNavigator().getNavigationAction().toVariantMap();
    variantMap["DensityRendererNavigation"] = _scatterplotPlugin->getScatterplotWidget().getDensityRendererNavigator().getNavigationAction().toVariantMap();

    return variantMap;
}
