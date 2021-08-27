#include "ClusterAction.h"
#include "ScatterplotPlugin.h"
#include "Application.h"
#include "DataHierarchyItem.h"
#include "PointData.h"
#include "ClusterData.h"

#include <QHBoxLayout>
#include <QRandomGenerator>

using namespace hdps;
using namespace hdps::gui;

ClusterAction::ClusterAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Cluster"),
    EventListener(),
    _inputDataHierarchyItem(nullptr),
    _clusterDataHierarchyItem(nullptr),
    _targetAction(this, "Cluster set"),
    _nameAction(this, "Name"),
    _colorAction(this, "Color"),
    _addClusterAction(this, "Add")
{
    setText("Add cluster");
    setIcon(Application::getIconFont("FontAwesome").getIcon("th-large"));

    const auto updateActions = [this]() -> void {
        const auto canAddCluster = _targetAction.getCurrentIndex() >= 0 && !_nameAction.getString().isEmpty();

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

QMenu* ClusterAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu(text(), parent);

    menu->addAction(&_addClusterAction);

    return menu;
}

const std::vector<std::uint32_t>& ClusterAction::getSelectedIndices() const
{
    /*
    auto& points    = _inputDataHierarchyItem->getDataset<Points>();
    auto& selection = dynamic_cast<Points&>(points.getSelection());

    return selection.indices;
    */

    return std::vector<std::uint32_t>();
}

void ClusterAction::updateTargets()
{
    auto clusterDataHierarchyItems = _scatterplotPlugin->getClusterDataHierarchyItems();

    QStringList clusterDatasetNames;

    for (auto clusterDataHierarchyItem : clusterDataHierarchyItems)
        clusterDatasetNames << clusterDataHierarchyItem->getDatasetName();

    _targetAction.setOptions(clusterDatasetNames);
    _targetAction.setCurrentText(_scatterplotPlugin->getColorDatasetName());
}

void ClusterAction::createDefaultCustersSet()
{
    if (!_scatterplotPlugin->arePointsLoaded() || _clusterDataHierarchyItem != nullptr)
        return;

    auto clustersDatasetName = _scatterplotPlugin->getCore()->addData("Cluster", "Manual clustering", _scatterplotPlugin->getPointsDatasetName());

    _scatterplotPlugin->loadColorData(clustersDatasetName);
}

ClusterAction::Widget::Widget(QWidget* parent, ClusterAction* clusterAction, const hdps::gui::WidgetActionWidget::State& state) :
    WidgetActionWidget(parent, clusterAction, state)
{
    auto rng = QRandomGenerator::global();

    const auto randomHue        = rng->bounded(360);
    const auto randomSaturation = rng->bounded(150, 255);
    const auto randomLightness  = rng->bounded(100, 200);

    clusterAction->createDefaultCustersSet();
    clusterAction->updateTargets();
    clusterAction->getColorAction().setColor(QColor::fromHsl(randomHue, randomSaturation, randomLightness));

    switch (state)
    {
        case Widget::State::Standard:
        {
            auto layout = new QHBoxLayout();

            auto targetWidget   = clusterAction->_targetAction.createWidget(this);
            auto nameWidget     = clusterAction->_nameAction.createWidget(this);
            auto colorWidget    = clusterAction->_colorAction.createWidget(this);
            auto createWidget   = clusterAction->_addClusterAction.createWidget(this);

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

            layout->addWidget(clusterAction->_targetAction.createLabelWidget(this), 0, 0);
            layout->addWidget(clusterAction->_targetAction.createWidget(this), 0, 1);

            layout->addWidget(clusterAction->_nameAction.createLabelWidget(this), 1, 0);
            layout->addWidget(clusterAction->_nameAction.createWidget(this), 1, 1);

            layout->addWidget(clusterAction->_colorAction.createLabelWidget(this), 2, 0);
            layout->addWidget(clusterAction->_colorAction.createWidget(this), 2, 1);

            layout->addWidget(clusterAction->_addClusterAction.createLabelWidget(this), 3, 0);
            layout->addWidget(clusterAction->_addClusterAction.createWidget(this), 3, 1);

            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }
}
