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
    _clustersDataset(),
    _targetAction(this, "Cluster set"),
    _nameAction(this, "Name"),
    _colorAction(this, "Color"),
    _addClusterAction(this, "Add cluster")
{
    setText("Manual clustering");
    setIcon(Application::getIconFont("FontAwesome").getIcon("th-large"));

    _targetAction.setToolTip("Add cluster to");
    _nameAction.setToolTip("Name of the cluster");
    _colorAction.setToolTip("Color of the cluster");
    _addClusterAction.setToolTip("Add cluster");

    const auto updateActions = [this]() -> void {
        const auto numberOfSelectedPoints   = _scatterplotPlugin->getNumberOfSelectedPoints();
        const auto hasSelection             = numberOfSelectedPoints >= 1;
        const auto canAddCluster            = hasSelection && !_nameAction.getString().isEmpty();

        _targetAction.setEnabled(hasSelection && _targetAction.getOptions().count() >= 2);
        _nameAction.setEnabled(hasSelection);
        _colorAction.setEnabled(hasSelection);
        _addClusterAction.setEnabled(canAddCluster);
    };

    connect(&_nameAction, &StringAction::stringChanged, this, [this, updateActions](const QString& string) {
        updateActions();
    });

    connect(&_addClusterAction, &TriggerAction::triggered, this, [this]() {
        if (!_scatterplotPlugin->getPositionDataset().isValid() || !_clustersDataset.isValid())
            return;

        // Get points selection dataset
        auto selection = _scatterplotPlugin->getPositionDataset()->getSelection<Points>();

        Cluster cluster;

        cluster.setName(_nameAction.getString());
        cluster.setColor(_colorAction.getColor());
        cluster.setIndices(selection->indices);

        _clustersDataset->addCluster(cluster);

        Application::core()->notifyDataChanged(*_clustersDataset);

        _nameAction.reset();
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this, updateActions](DatasetImpl* dataset) {
        _targetAction.reset();
        _nameAction.reset();
        _clustersDataset.reset();
    });

    connect(&_scatterplotPlugin->getColorsDataset(), &Dataset<Points>::changed, this, [this, updateActions](DatasetImpl* dataset) {
        if (dataset == nullptr)
            return;

        if (_scatterplotPlugin->getColorsDataset()->getDataType() == ClusterType)
            _clustersDataset.set(dataset);

        updateActions();

        updateTargets();
    });

    connect(_scatterplotPlugin, &ScatterplotPlugin::selectionChanged, this, updateActions);

    updateActions();
}

void ManualClusteringAction::updateTargets()
{
    /* TODO
    auto clusterDatasetNames = _scatterplotPlugin->getClusterDatasetNames();

    _targetAction.setOptions(clusterDatasetNames);
    _targetAction.setCurrentText(_clustersDataset->getGuiName());
    */
}

void ManualClusteringAction::createDefaultCustersSet()
{
    if (!_scatterplotPlugin->getPositionDataset().isValid() || _clustersDataset.isValid())
        return;

    /*
    auto clustersDatasetName = _scatterplotPlugin->getCore()->addData("Cluster", "annotation", _scatterplotPlugin->getPointsDataset()->getName());

    _scatterplotPlugin->getColorsDataset().setDatasetName(clustersDatasetName);
    */
}

ManualClusteringAction::Widget::Widget(QWidget* parent, ManualClusteringAction* manualClusteringAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, manualClusteringAction, widgetFlags)
{
    auto rng = QRandomGenerator::global();

    const auto randomHue        = rng->bounded(360);
    const auto randomSaturation = rng->bounded(150, 255);
    const auto randomLightness  = rng->bounded(100, 200);

    manualClusteringAction->createDefaultCustersSet();
    manualClusteringAction->updateTargets();

    manualClusteringAction->getNameAction().reset();
    manualClusteringAction->getColorAction().setColor(QColor::fromHsl(randomHue, randomSaturation, randomLightness));

    if (widgetFlags & PopupLayout) {
        auto layout = new QGridLayout();

        layout->setColumnMinimumWidth(1, 200);

        layout->addWidget(manualClusteringAction->_targetAction.createLabelWidget(this), 0, 0);
        layout->addWidget(manualClusteringAction->_targetAction.createWidget(this), 0, 1);

        layout->addWidget(manualClusteringAction->_nameAction.createLabelWidget(this), 1, 0);
        layout->addWidget(manualClusteringAction->_nameAction.createWidget(this), 1, 1);

        layout->addWidget(manualClusteringAction->_colorAction.createLabelWidget(this), 2, 0);
        layout->addWidget(manualClusteringAction->_colorAction.createWidget(this), 2, 1);

        layout->addWidget(manualClusteringAction->_addClusterAction.createWidget(this), 3, 1);

        setPopupLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        auto targetWidget = manualClusteringAction->_targetAction.createWidget(this);
        auto nameWidget = manualClusteringAction->_nameAction.createWidget(this);
        auto colorWidget = manualClusteringAction->_colorAction.createWidget(this);
        auto createWidget = manualClusteringAction->_addClusterAction.createWidget(this);

        targetWidget->setFixedWidth(100);
        nameWidget->setFixedWidth(100);
        colorWidget->setFixedWidth(26);
        createWidget->setFixedWidth(50);

        layout->addWidget(targetWidget);
        layout->addWidget(nameWidget);
        layout->addWidget(colorWidget);
        layout->addWidget(createWidget);

        setLayout(layout);
    }
}
