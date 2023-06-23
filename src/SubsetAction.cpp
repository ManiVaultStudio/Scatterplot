#include "SubsetAction.h"
#include "ScatterplotPlugin.h"

#include <PointData/PointData.h>

#include <Application.h>

#include <QMenu>

using namespace hdps;
using namespace hdps::gui;

SubsetAction::SubsetAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _subsetNameAction(this, "Subset name"),
    _sourceDataAction(this, "Source data"),
    _createSubsetAction(this, "Create subset")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("crop"));
    setDefaultWidgetFlags(GroupAction::Horizontal);
    setConnectionPermissionsToForceNone(true);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_subsetNameAction);
    addAction(&_sourceDataAction);
    addAction(&_createSubsetAction);

    _subsetNameAction.setToolTip("Name of the subset");
    _createSubsetAction.setToolTip("Create subset from selected data points");

    const auto updateReadOnly = [this]() -> void {
        _createSubsetAction.setEnabled(!_subsetNameAction.getString().isEmpty());
    };

    updateReadOnly();

    connect(&_subsetNameAction, &StringAction::stringChanged, this, updateReadOnly);
}

void SubsetAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    connect(&_createSubsetAction, &QAction::triggered, this, [this]() {
        _scatterplotPlugin->createSubset(_sourceDataAction.getCurrentIndex() == 1, _subsetNameAction.getString());
    });

    const auto onCurrentDatasetChanged = [this]() -> void {
        if (!_scatterplotPlugin->getPositionDataset().isValid())
            return;

        const auto datasetGuiName = _scatterplotPlugin->getPositionDataset()->text();

        QStringList sourceDataOptions;

        if (!datasetGuiName.isEmpty()) {
            const auto sourceDatasetGuiName = _scatterplotPlugin->getPositionDataset()->getSourceDataset<Points>()->text();

            sourceDataOptions << QString("From: %1").arg(datasetGuiName);

            if (sourceDatasetGuiName != datasetGuiName)
                sourceDataOptions << QString("From: %1 (source data)").arg(sourceDatasetGuiName);
        }

        _sourceDataAction.setOptions(sourceDataOptions);
        _sourceDataAction.setEnabled(sourceDataOptions.count() >= 2);
    };

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, onCurrentDatasetChanged);

    onCurrentDatasetChanged();
}

QMenu* SubsetAction::getContextMenu()
{
    auto menu = new QMenu("Subset");

    menu->addAction(&_createSubsetAction);
    menu->addAction(&_sourceDataAction);

    return menu;
}

void SubsetAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _subsetNameAction.fromParentVariantMap(variantMap);
}

QVariantMap SubsetAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _subsetNameAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
