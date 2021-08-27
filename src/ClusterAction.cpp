#include "ClusterAction.h"
#include "ScatterplotPlugin.h"
#include "Application.h"

#include "DataHierarchyItem.h"
//#include "PointData.h"
//#include "ClusterData.h"

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
    _createClusterAction(this, "create")
{
    setText("Add cluster");
    setIcon(Application::getIconFont("FontAwesome").getIcon("th-large"));

    const auto updateActions = [this]() -> void {
       //qDebug() << (_scatterplotPlugin->getNumberOfSelectedPoints() >= 1);
        //setEnabled(_scatterplotPlugin->getNumberOfSelectedPoints() >= 1);
    };

    connect(_scatterplotPlugin, &ScatterplotPlugin::selectionChanged, this, updateActions);

    updateActions();

    connect(&_createClusterAction, &TriggerAction::triggered, this, [this]() {
        /*
        if (_clusterDataHierarchyItem == nullptr) {
            const auto clusterDatasetName = _core->addData("Cluster", "Cluster", _inputDataHierarchyItem->getDatasetName());
            _clusterDataHierarchyItem = _core->getDataHierarchyItem(clusterDatasetName);
        }
        */
    });
}

QMenu* ClusterAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu(text(), parent);

    menu->addAction(&_createClusterAction);

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


ClusterAction::Widget::Widget(QWidget* parent, ClusterAction* clusterAction, const hdps::gui::WidgetActionWidget::State& state) :
    WidgetActionWidget(parent, clusterAction, state)
{
    auto rng = QRandomGenerator::global();

    const auto randomHue        = rng->bounded(360);
    const auto randomSaturation = rng->bounded(150, 255);
    const auto randomLightness  = rng->bounded(100, 200);

    clusterAction->getColorAction().setColor(QColor::fromHsl(randomHue, randomSaturation, randomLightness));

    auto layout = new QHBoxLayout();

    auto targetWidget   = clusterAction->_targetAction.createWidget(this);
    auto nameWidget     = clusterAction->_nameAction.createWidget(this);
    auto colorWidget    = clusterAction->_colorAction.createWidget(this);
    auto createWidget   = clusterAction->_createClusterAction.createWidget(this);

    targetWidget->setFixedWidth(100);
    nameWidget->setFixedWidth(100);
    colorWidget->setFixedWidth(26);
    createWidget->setFixedWidth(50);

    layout->addWidget(targetWidget);
    layout->addWidget(nameWidget);
    layout->addWidget(colorWidget);
    layout->addWidget(createWidget);

    switch (state)
    {
        case Widget::State::Standard:
        {
            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }
}
