#include "ManualClusteringAction.h"
#include "ScatterplotPlugin.h"
#include "Application.h"
#include "PointData/PointData.h"
#include "ClusterData/ClusterData.h"

#include <QHBoxLayout>

using namespace hdps;
using namespace hdps::gui;

ManualClusteringAction::ManualClusteringAction(QObject* parent, const QString& title) :
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

    // Update actions when the cluster name changed
    connect(&_nameAction, &StringAction::stringChanged, this, &ManualClusteringAction::updateActions);

    // Add cluster to dataset when the action is triggered
    connect(&_addClusterAction, &TriggerAction::triggered, this, [this]() {

        // Only add cluster id there is a valid position dataset
        if (!_scatterplotPlugin->getPositionDataset().isValid())
            return;

        // Get the target cluster dataset
        auto targetClusterDataset = _targetClusterDataset.getCurrentDataset<Clusters>();

        // Only add cluster id there is a valid target cluster dataset
        if (!targetClusterDataset.isValid())
            return;

        // Get points selection dataset
        auto selection = _scatterplotPlugin->getPositionDataset()->getSelection<Points>();

        Cluster cluster;

        // Adjust cluster parameters
        cluster.setName(_nameAction.getString());
        cluster.setColor(_colorAction.getColor());
        cluster.setIndices(selection->indices);

        // Add the cluster to the set
        targetClusterDataset->addCluster(cluster);

        events().notifyDatasetChanged(targetClusterDataset);

        // Reset the cluster name input
        _nameAction.reset();
    });

    // Update the current cluster picker action when the position dataset changes or children are added/removed
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildAdded, this, &ManualClusteringAction::updateTargetClusterDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildRemoved, this, &ManualClusteringAction::updateTargetClusterDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, &ManualClusteringAction::updateTargetClusterDatasets);

    // Do an initial update of the actions
    updateActions();
}

void ManualClusteringAction::createDefaultClusterDataset()
{
    // Only add a default clusters dataset when there is a position dataset
    if (!_scatterplotPlugin->getPositionDataset().isValid())
        return;

    // Only add a default clusters dataset when there are no child cluster datasets
    if (!_scatterplotPlugin->getPositionDataset()->getChildren(ClusterType).isEmpty())
        return;

    // Add default clusters dataset
    const auto defaultClusters = Application::core()->addDataset<Clusters>("Cluster", "Clusters (manual)", _scatterplotPlugin->getPositionDataset());

    // Notify others that the default set was added
    events().notifyDatasetAdded(defaultClusters);

    // Update picker
    updateTargetClusterDatasets();

    // Choose default clusters for coloring
    _scatterplotPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(defaultClusters);
}

void ManualClusteringAction::updateTargetClusterDatasets()
{
    // Only update targets when there is a position dataset
    if (!_scatterplotPlugin->getPositionDataset().isValid())
        return;

    const auto clusterDatasets = _scatterplotPlugin->getPositionDataset()->getChildren(ClusterType);

    // Update pickers
    _targetClusterDataset.setDatasets(clusterDatasets);

    if (!clusterDatasets.isEmpty())
        _targetClusterDataset.setCurrentDataset(clusterDatasets.first());
}

void ManualClusteringAction::updateActions()
{
    const auto positionDataset          = _scatterplotPlugin->getPositionDataset();
    const auto numberOfSelectedPoints   = positionDataset.isValid() ? positionDataset->getSelectionSize() : 0;
    const auto hasSelection             = numberOfSelectedPoints >= 1;
    const auto canAddCluster            = hasSelection && !_nameAction.getString().isEmpty();

    _nameAction.setEnabled(hasSelection);
    _colorAction.setEnabled(hasSelection);
    _addClusterAction.setEnabled(canAddCluster);
}
