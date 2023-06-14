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
    _clusterDatasetPickerAction(this, "Add to"),
    _clusterDatasetNameAction(this, "Cluster dataset name"),
    _createClusterDatasetAction(this, "Create"),
    _clusterDatasetWizardAction(this, "Create cluster dataset"),
    _clusterDatasetAction(this, "Target Cluster")
{
    setText("Manual clustering");
    setIcon(Application::getIconFont("FontAwesome").getIcon("th-large"));
    setConnectionPermissionsToForceNone();
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);

    addAction(&_nameAction);
    addAction(&_colorAction);
    addAction(&_clusterDatasetAction);
    addAction(&_addClusterAction);

    //setPopupSizeHint(QSize(400, 0));

    _clusterDatasetAction.setShowLabels(false);
    _clusterDatasetAction.addAction(&_clusterDatasetPickerAction);
    _clusterDatasetAction.addAction(&_clusterDatasetWizardAction, TriggerAction::Icon);

    _nameAction.setToolTip("Name of the cluster");
    _colorAction.setToolTip("Color of the cluster");
    _addClusterAction.setToolTip("Add cluster");
    _clusterDatasetPickerAction.setToolTip("Target cluster set");
    
    _clusterDatasetNameAction.setToolTip("Name of the new cluster dataset");
    
    _createClusterDatasetAction.setToolTip("Create new cluster dataset");

    _clusterDatasetWizardAction.setIcon(Application::getIconFont("FontAwesome").getIcon("magic"));
    _clusterDatasetWizardAction.setToolTip("Create a new cluster dataset");
    _clusterDatasetWizardAction.setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    _clusterDatasetWizardAction.addAction(&_clusterDatasetNameAction);
    _clusterDatasetWizardAction.addAction(&_createClusterDatasetAction);

    _clusterDatasetPickerAction.setDatasetsFilterFunction([this](const hdps::Datasets& datasets) ->hdps::Datasets {
        Datasets clusterDatasets;

        for (auto dataset : datasets)
            if (dataset->getDataType() == ClusterType)
                clusterDatasets << dataset;

        return clusterDatasets;
    });

    connect(&_createClusterDatasetAction, &TriggerAction::triggered, this, [this]() -> void {
        const auto defaultClusters = Application::core()->addDataset<Clusters>("Cluster", _clusterDatasetNameAction.getString(), _scatterplotPlugin->getPositionDataset());

        events().notifyDatasetAdded(defaultClusters);

        _scatterplotPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(defaultClusters);
    });

    connect(&_addClusterAction, &TriggerAction::triggered, this, [this]() {
        if (!_scatterplotPlugin->getPositionDataset().isValid())
            return;

        auto targetClusterDataset = _clusterDatasetPickerAction.getCurrentDataset<Clusters>();

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

    const auto updateActionsReadOnly = [this]() -> void {
        const auto positionDataset          = _scatterplotPlugin->getPositionDataset();
        const auto numberOfSelectedPoints   = positionDataset.isValid() ? positionDataset->getSelectionSize() : 0;
        const auto hasSelection             = numberOfSelectedPoints >= 1;
        const auto canAddCluster            = hasSelection && !_nameAction.getString().isEmpty();
        
        setEnabled(hasSelection);

        _nameAction.setEnabled(_clusterDatasetPickerAction.hasSelection());
        _colorAction.setEnabled(_clusterDatasetPickerAction.hasSelection());
        _addClusterAction.setEnabled(!_nameAction.getString().isEmpty());
    };

    updateActionsReadOnly();

    connect(&_nameAction, &StringAction::stringChanged, this, updateActionsReadOnly);
    connect(&_clusterDatasetPickerAction, &DatasetPickerAction::datasetPicked, this, updateActionsReadOnly);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataSelectionChanged, this, updateActionsReadOnly);
}
