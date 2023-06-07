#include "ClusteringAction.h"
#include "ScatterplotPlugin.h"

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>

#include <QHBoxLayout>

using namespace hdps;
using namespace hdps::gui;

ClusteringAction::ClusteringAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _nameAction(this, "Name"),
    _colorAction(this, "Color"),
    _addClusterAction(this, "Add cluster"),
    _targetClusterDataset(this, "Cluster set")
{
    setText("Manual clustering");
    setIcon(Application::getIconFont("FontAwesome").getIcon("th-large"));
    setConnectionPermissionsToForceNone();
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);

    addAction(&_nameAction);
    addAction(&_colorAction);
    addAction(&_addClusterAction);

    _nameAction.setToolTip("Name of the cluster");
    _colorAction.setToolTip("Color of the cluster");
    _addClusterAction.setToolTip("Add cluster");
    _targetClusterDataset.setToolTip("Target cluster set");

    connect(&_nameAction, &StringAction::stringChanged, this, &ClusteringAction::updateActions);

    connect(&_addClusterAction, &TriggerAction::triggered, this, [this]() {

        if (!_scatterplotPlugin->getPositionDataset().isValid())
            return;

        auto targetClusterDataset = _targetClusterDataset.getCurrentDataset<Clusters>();

        if (!targetClusterDataset.isValid())
            return;

        auto selection = _scatterplotPlugin->getPositionDataset()->getSelection<Points>();

        Cluster cluster;

        cluster.setName(_nameAction.getString());
        cluster.setColor(_colorAction.getColor());
        cluster.setIndices(selection->indices);

        targetClusterDataset->addCluster(cluster);

        events().notifyDatasetChanged(targetClusterDataset);

        _nameAction.reset();
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildAdded, this, &ClusteringAction::updateTargetClusterDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildRemoved, this, &ClusteringAction::updateTargetClusterDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, &ClusteringAction::updateTargetClusterDatasets);

    updateActions();
}

void ClusteringAction::createDefaultClusterDataset()
{
    if (!_scatterplotPlugin->getPositionDataset().isValid())
        return;

    if (!_scatterplotPlugin->getPositionDataset()->getChildren(ClusterType).isEmpty())
        return;

    const auto defaultClusters = Application::core()->addDataset<Clusters>("Cluster", "Clusters (manual)", _scatterplotPlugin->getPositionDataset());

    events().notifyDatasetAdded(defaultClusters);

    updateTargetClusterDatasets();

    _scatterplotPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(defaultClusters);
}

void ClusteringAction::updateTargetClusterDatasets()
{
    if (!_scatterplotPlugin->getPositionDataset().isValid())
        return;

    const auto clusterDatasets = _scatterplotPlugin->getPositionDataset()->getChildren(ClusterType);

    _targetClusterDataset.setDatasets(clusterDatasets);

    if (!clusterDatasets.isEmpty())
        _targetClusterDataset.setCurrentDataset(clusterDatasets.first());
}

void ClusteringAction::updateActions()
{
    const auto positionDataset          = _scatterplotPlugin->getPositionDataset();
    const auto numberOfSelectedPoints   = positionDataset.isValid() ? positionDataset->getSelectionSize() : 0;
    const auto hasSelection             = numberOfSelectedPoints >= 1;
    const auto canAddCluster            = hasSelection && !_nameAction.getString().isEmpty();

    _nameAction.setEnabled(hasSelection);
    _colorAction.setEnabled(hasSelection);
    _addClusterAction.setEnabled(canAddCluster);
}
