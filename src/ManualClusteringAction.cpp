#include "ManualClusteringAction.h"
#include "ScatterplotPlugin.h"
#include "Application.h"
#include "DataHierarchyItem.h"
#include "PointData.h"
#include "ClusterData.h"

#include <QHBoxLayout>
#include <QRandomGenerator>

using namespace hdps;
using namespace hdps::gui;

ManualClusteringAction::ManualClusteringAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Cluster"),
    EventListener(),
    _inputDataHierarchyItem(nullptr),
    _clusterDataHierarchyItem(nullptr),
    _targetAction(this, "Cluster set"),
    _nameAction(this, "Name"),
    _colorAction(this, "Color"),
    _addClusterAction(this, "Add")
{
    setText("Manual clustering");
    setIcon(Application::getIconFont("FontAwesome").getIcon("th-large"));

    const auto updateActions = [this]() -> void {
        const auto canAddCluster = _scatterplotPlugin->getNumberOfSelectedPoints() >= 1 && _targetAction.getCurrentIndex() >= 0 && !_nameAction.getString().isEmpty();

        _colorAction.setEnabled(canAddCluster);
        _addClusterAction.setEnabled(canAddCluster);
    };

    connect(&_nameAction, &StringAction::stringChanged, this, [this, updateActions](const QString& string) {
        updateActions();
    });

    connect(&_addClusterAction, &TriggerAction::triggered, this, [this]() {
        if (!_scatterplotPlugin->arePointsLoaded() || _clusterDataHierarchyItem == nullptr)
            return;

        auto clusters = _clusterDataHierarchyItem->getDataset<Clusters>();

        auto& points            = _scatterplotPlugin->getPointsDataHierarchyItem()->getDataset<Points>();
        auto& selection         = dynamic_cast<Points&>(points.getSelection());
        auto& clusterDataset    = _clusterDataHierarchyItem->getDataset<Clusters>();

        Cluster cluster;

        cluster._name       = _nameAction.getString();
        cluster._color      = _colorAction.getColor();
        cluster._indices    = selection.indices;

        clusterDataset.addCluster(cluster);

        _clusterDataHierarchyItem->notifyDataChanged();
    });

    connect(_scatterplotPlugin, &ScatterplotPlugin::currentColorsChanged, this, [this](const QString& datasetName) {
        if (_scatterplotPlugin->getColorDatasetDataHierarchyItem()->getDataset().getDataType() == ClusterType)
            _clusterDataHierarchyItem = _scatterplotPlugin->getColorDatasetDataHierarchyItem();

        updateTargets();
    });

    connect(_scatterplotPlugin, &ScatterplotPlugin::selectionChanged, this, updateActions);

    updateActions();
}

void ManualClusteringAction::updateTargets()
{
    auto clusterDataHierarchyItems = _scatterplotPlugin->getClusterDataHierarchyItems();

    QStringList clusterDatasetNames;

    for (auto clusterDataHierarchyItem : clusterDataHierarchyItems)
        clusterDatasetNames << clusterDataHierarchyItem->getDatasetName();

    _targetAction.setOptions(clusterDatasetNames);
    _targetAction.setCurrentText(_scatterplotPlugin->getColorDatasetName());
}

void ManualClusteringAction::createDefaultCustersSet()
{
    if (!_scatterplotPlugin->arePointsLoaded() || _clusterDataHierarchyItem != nullptr)
        return;

    auto clustersDatasetName = _scatterplotPlugin->getCore()->addData("Cluster", "annotation", _scatterplotPlugin->getPointsDatasetName());

    _scatterplotPlugin->loadColorData(clustersDatasetName);
}

ManualClusteringAction::Widget::Widget(QWidget* parent, ManualClusteringAction* manualClusteringAction, const WidgetActionWidget::State& state) :
    WidgetActionWidget(parent, manualClusteringAction, state)
{
    auto rng = QRandomGenerator::global();

    const auto randomHue        = rng->bounded(360);
    const auto randomSaturation = rng->bounded(150, 255);
    const auto randomLightness  = rng->bounded(100, 200);

    manualClusteringAction->createDefaultCustersSet();
    manualClusteringAction->updateTargets();

    manualClusteringAction->getNameAction().reset();
    manualClusteringAction->getColorAction().setColor(QColor::fromHsl(randomHue, randomSaturation, randomLightness));

    switch (state)
    {
        case Widget::State::Standard:
        {
            auto layout = new QHBoxLayout();

            auto targetWidget   = manualClusteringAction->_targetAction.createWidget(this);
            auto nameWidget     = manualClusteringAction->_nameAction.createWidget(this);
            auto colorWidget    = manualClusteringAction->_colorAction.createWidget(this);
            auto createWidget   = manualClusteringAction->_addClusterAction.createWidget(this);

            targetWidget->setFixedWidth(100);
            nameWidget->setFixedWidth(100);
            colorWidget->setFixedWidth(26);
            createWidget->setFixedWidth(50);

            layout->addWidget(targetWidget);
            layout->addWidget(nameWidget);
            layout->addWidget(colorWidget);
            layout->addWidget(createWidget);

            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            auto layout = new QGridLayout();

            layout->setColumnMinimumWidth(1, 200);

            layout->addWidget(manualClusteringAction->_targetAction.createLabelWidget(this), 0, 0);
            layout->addWidget(manualClusteringAction->_targetAction.createWidget(this), 0, 1);

            layout->addWidget(manualClusteringAction->_nameAction.createLabelWidget(this), 1, 0);
            layout->addWidget(manualClusteringAction->_nameAction.createWidget(this), 1, 1);

            layout->addWidget(manualClusteringAction->_colorAction.createLabelWidget(this), 2, 0);
            layout->addWidget(manualClusteringAction->_colorAction.createWidget(this), 2, 1);

            layout->addWidget(manualClusteringAction->_addClusterAction.createWidget(this), 3, 0);

            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }
}
