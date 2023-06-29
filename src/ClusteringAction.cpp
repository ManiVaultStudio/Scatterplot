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
    _nameAction(this, "Cluster name"),
    _colorAction(this, "Cluster color"),
    _addClusterAction(this, "Add cluster"),
    _clusterDatasetPickerAction(this, "Add to"),
    _clusterDatasetNameAction(this, "Dataset name"),
    _createClusterDatasetAction(this, "Create"),
    _clusterDatasetWizardAction(this, "Create cluster dataset"),
    _clusterDatasetAction(this, "Target clusters dataset")
{
    setText("Manual clustering");
    setIcon(Application::getIconFont("FontAwesome").getIcon("th-large"));
    setConnectionPermissionsToForceNone();
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_nameAction);
    addAction(&_colorAction);
    addAction(&_clusterDatasetAction);
    addAction(&_addClusterAction);

    _clusterDatasetAction.setShowLabels(false);
    _clusterDatasetAction.addAction(&_clusterDatasetPickerAction);
    _clusterDatasetAction.addAction(&_clusterDatasetWizardAction, TriggerAction::Icon);

    _nameAction.setToolTip("Name of the cluster");
    _colorAction.setToolTip("Color of the cluster");
    _addClusterAction.setToolTip("Add cluster");
    _clusterDatasetPickerAction.setToolTip("Target cluster set");
    
    _clusterDatasetNameAction.setToolTip("Name of the new cluster dataset");
    _clusterDatasetNameAction.setClearable(true);
    
    _createClusterDatasetAction.setToolTip("Create new cluster dataset");
    _createClusterDatasetAction.setEnabled(false);

    _clusterDatasetWizardAction.setIcon(Application::getIconFont("FontAwesome").getIcon("magic"));
    _clusterDatasetWizardAction.setToolTip("Create a new cluster dataset");
    _clusterDatasetWizardAction.setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    _clusterDatasetWizardAction.setLabelSizingType(LabelSizingType::Auto);
    _clusterDatasetWizardAction.addAction(&_clusterDatasetNameAction);
    _clusterDatasetWizardAction.addAction(&_createClusterDatasetAction);

    _clusterDatasetPickerAction.setDatasetsFilterFunction([this](const hdps::Datasets& datasets) ->hdps::Datasets {
        Datasets clusterDatasets;

        for (auto dataset : datasets)
            if (dataset->getDataType() == ClusterType)
                clusterDatasets << dataset;

        return clusterDatasets;
    });

    connect(& _clusterDatasetNameAction, & StringAction::stringChanged, this, [this](const QString& string) -> void {
        _createClusterDatasetAction.setEnabled(!string.isEmpty());
    });

    connect(&_createClusterDatasetAction, &TriggerAction::triggered, this, [this]() -> void {
        const auto clustersDataset = Application::core()->addDataset<Clusters>("Cluster", _clusterDatasetNameAction.getString(), _scatterplotPlugin->getPositionDataset());

        events().notifyDatasetAdded(clustersDataset);

        _clusterDatasetPickerAction.setCurrentDataset(clustersDataset);
    });

    connect(&_clusterDatasetPickerAction, &DatasetPickerAction::datasetPicked, this, [this](Dataset<DatasetImpl> dataset) -> void {
        _scatterplotPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(dataset);
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

        events().notifyDatasetDataChanged(targetClusterDataset);

        _nameAction.reset();

        randomizeClusterColor();
    });

    const auto updateActionsReadOnly = [this]() -> void {
        const auto positionDataset          = _scatterplotPlugin->getPositionDataset();
        const auto numberOfSelectedPoints   = positionDataset.isValid() ? positionDataset->getSelectionSize() : 0;
        const auto hasSelection             = numberOfSelectedPoints >= 1;
        const auto canAddCluster            = hasSelection && !_nameAction.getString().isEmpty();
        
        setEnabled(_scatterplotPlugin->getPositionDataset().isValid() && hasSelection);

        _nameAction.setEnabled(_clusterDatasetPickerAction.hasSelection());
        _colorAction.setEnabled(_clusterDatasetPickerAction.hasSelection());
        _addClusterAction.setEnabled(!_nameAction.getString().isEmpty());
    };

    updateActionsReadOnly();

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateActionsReadOnly);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataSelectionChanged, this, updateActionsReadOnly);
    connect(&_nameAction, &StringAction::stringChanged, this, updateActionsReadOnly);
    connect(&_clusterDatasetPickerAction, &DatasetPickerAction::datasetPicked, this, updateActionsReadOnly);

    randomizeClusterColor();
}

void ClusteringAction::randomizeClusterColor()
{
    auto rng = QRandomGenerator::global();

    _colorAction.setColor(QColor::fromHsl(rng->bounded(360), rng->bounded(150, 255), rng->bounded(100, 200)));
}
