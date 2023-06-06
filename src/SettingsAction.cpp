#include "SettingsAction.h"
#include "ExportImageDialog.h"

#include "Application.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "PointData/PointData.h"

#include <QMenu>

using namespace hdps::gui;

SettingsAction::SettingsAction(QObject* parent, const QString& title) :
    HorizontalToolbarAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent)),
    _datasetsAction(this, "Datasets"),
    _renderModeAction(this, "Render Mode"),
    _positionAction(this, "Position"),
    _coloringAction(this, "Coloring"),
    _subsetAction(this, "Subset"),
    _manualClusteringAction(this, "Clustering"),
    _selectionAction(this, "Selection"),
    _plotAction(this, "Plot"),
    _exportAction(this, "Export to image/video"),
    _miscellaneousAction(this, "Miscellaneous")
{
    setText("Settings");
    setConnectionPermissionsToForceNone();

    addAction(&_datasetsAction);
    addAction(&_renderModeAction, 100);
    addAction(&_positionAction, 50);
    addAction(&_plotAction, 50);
    addAction(&_coloringAction);
    addAction(&_subsetAction);
    addAction(&_manualClusteringAction);
    addAction(&_selectionAction);
    addAction(&_exportAction);

    _renderModeAction.initialize(_scatterplotPlugin);
    _selectionAction.initialize(_scatterplotPlugin);
    _subsetAction.initialize(_scatterplotPlugin);
    _plotAction.initialize(_scatterplotPlugin);

    const auto updateEnabled = [this]() {
        const auto enabled = _scatterplotPlugin->getPositionDataset().isValid();

        _renderModeAction.setEnabled(enabled);
        _plotAction.setEnabled(enabled);
        _positionAction.setEnabled(enabled);
        _coloringAction.setEnabled(enabled);
        _subsetAction.setEnabled(enabled);
        _manualClusteringAction.setEnabled(enabled);
        _selectionAction.setEnabled(enabled);
        _exportAction.setEnabled(enabled);
    };

    updateEnabled();

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateEnabled);

    const auto updateHighlights = [this](const bool& state) -> void {
        _scatterplotPlugin->getScatterplotWidget().showHighlights(state);
    };

    _exportAction.setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("camera"));
    _exportAction.setDefaultWidgetFlags(TriggerAction::Icon);

    connect(&_exportAction, &TriggerAction::triggered, this, [this]() {
        ExportImageDialog exportDialog(nullptr, *_scatterplotPlugin);

        exportDialog.exec();
    });
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu();

    menu->addMenu(_renderModeAction.getContextMenu());
    menu->addMenu(_plotAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_positionAction.getContextMenu());
    menu->addMenu(_coloringAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_subsetAction.getContextMenu());
    menu->addMenu(_selectionAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_miscellaneousAction.getContextMenu());

    return menu;
}

void SettingsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _datasetsAction.fromParentVariantMap(variantMap);
    _plotAction.fromParentVariantMap(variantMap);
    _positionAction.fromParentVariantMap(variantMap);
    _coloringAction.fromParentVariantMap(variantMap);
    _renderModeAction.fromParentVariantMap(variantMap);
    _selectionAction.fromParentVariantMap(variantMap);
}

QVariantMap SettingsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _datasetsAction.insertIntoVariantMap(variantMap);
    _renderModeAction.insertIntoVariantMap(variantMap);
    _plotAction.insertIntoVariantMap(variantMap);
    _positionAction.insertIntoVariantMap(variantMap);
    _coloringAction.insertIntoVariantMap(variantMap);
    _selectionAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
