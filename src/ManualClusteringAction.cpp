#include "ManualClusteringAction.h"
#include "ScatterplotPlugin.h"
#include "Application.h"
#include "PointData.h"
#include "ClusterData.h"

#include <QHBoxLayout>
#include <QRandomGenerator>

using namespace hdps;
using namespace hdps::gui;

ManualClusteringAction::ManualClusteringAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Cluster"),
    _nameAction(this, "Name"),
    _colorAction(this, "Color"),
    _addClusterAction(this, "Add cluster"),
    _targetClusterDataset(this, "Cluster set")
{
    setText("Manual clustering");
    setIcon(Application::getIconFont("FontAwesome").getIcon("th-large"));

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

        // Notify others that the cluster data has changed
        Application::core()->notifyDataChanged(targetClusterDataset);

        // Reset the cluster name input
        _nameAction.reset();
    });

    // Update the current cluster picker action when the position dataset changes or children are added/removed
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildAdded, this, &ManualClusteringAction::updateTargetClusterDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildRemoved, this, &ManualClusteringAction::updateTargetClusterDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, &ManualClusteringAction::updateTargetClusterDatasets);

    // Update actions when the position dataset selection changes
    connect(_scatterplotPlugin, &ScatterplotPlugin::selectionChanged, this, &ManualClusteringAction::updateActions);

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
    Application::core()->notifyDataAdded(defaultClusters);

    // Update picker
    updateTargetClusterDatasets();

    // Choose default clusters for coloring
    _scatterplotPlugin->getSettingsAction().getColoringAction().getColorByDataAction().getDatasetPickerAction().setCurrentDataset(defaultClusters);
}

void ManualClusteringAction::updateTargetClusterDatasets()
{
    // Only update targets when there is a position dataset
    if (!_scatterplotPlugin->getPositionDataset().isValid())
        return;

    // Update pickers
    _targetClusterDataset.setDatasets(_scatterplotPlugin->getPositionDataset()->getChildren(ClusterType));
}

void ManualClusteringAction::updateActions()
{
    const auto numberOfSelectedPoints   = _scatterplotPlugin->getNumberOfSelectedPoints();
    const auto hasSelection             = numberOfSelectedPoints >= 1;
    const auto canAddCluster            = hasSelection && !_nameAction.getString().isEmpty();

    _nameAction.setEnabled(hasSelection);
    _colorAction.setEnabled(hasSelection);
    _addClusterAction.setEnabled(canAddCluster);
}

ManualClusteringAction::Widget::Widget(QWidget* parent, ManualClusteringAction* manualClusteringAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, manualClusteringAction, widgetFlags)
{
    auto rng = QRandomGenerator::global();

    const auto randomHue        = rng->bounded(360);
    const auto randomSaturation = rng->bounded(150, 255);
    const auto randomLightness  = rng->bounded(100, 200);

    // Add a cluster dataset if none exist
    manualClusteringAction->createDefaultClusterDataset();

    // Reset the cluster name
    manualClusteringAction->getNameAction().reset();

    // And adjust the color
    manualClusteringAction->getColorAction().setColor(QColor::fromHsl(randomHue, randomSaturation, randomLightness));

    if (widgetFlags & PopupLayout) {
        auto layout = new QGridLayout();

        layout->setColumnMinimumWidth(1, 200);

        layout->addWidget(manualClusteringAction->getTargetClusterDataset().createLabelWidget(this), 0, 0);
        layout->addWidget(manualClusteringAction->getTargetClusterDataset().createWidget(this), 0, 1);

        layout->addWidget(manualClusteringAction->getNameAction().createLabelWidget(this), 1, 0);
        layout->addWidget(manualClusteringAction->getNameAction().createWidget(this), 1, 1);

        layout->addWidget(manualClusteringAction->getColorAction().createLabelWidget(this), 2, 0);
        layout->addWidget(manualClusteringAction->getColorAction().createWidget(this), 2, 1);

        layout->addWidget(manualClusteringAction->getAddClusterAction().createWidget(this), 3, 1);

        setPopupLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        auto currentClusterWidget   = manualClusteringAction->getTargetClusterDataset().createWidget(this);
        auto nameWidget             = manualClusteringAction->getNameAction().createWidget(this);
        auto colorWidget            = manualClusteringAction->getColorAction().createWidget(this);
        auto createWidget           = manualClusteringAction->getAddClusterAction().createWidget(this);

        currentClusterWidget->setFixedWidth(100);
        nameWidget->setFixedWidth(100);
        colorWidget->setFixedWidth(26);
        createWidget->setFixedWidth(50);

        layout->addWidget(currentClusterWidget);
        layout->addWidget(nameWidget);
        layout->addWidget(colorWidget);
        layout->addWidget(createWidget);

        setLayout(layout);
    }
}
