#include "CurrentDatasetAction.h"
#include "ScatterplotPlugin.h"
#include "PointData.h"
#include "ClusterData.h"

#include <QMenu>

using namespace hdps;
using namespace hdps::gui;

CurrentDatasetAction::CurrentDatasetAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Datasets"),
    _positionDatasetPickerAction(this, "Position"),
    _clusterDatasetPickerAction(this, "Cluster")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("database"));

    _positionDatasetPickerAction.setDatasetsFilterFunction([](const hdps::Datasets& datasets) -> Datasets {
        Datasets pointDatasets;

        for (auto dataset : datasets)
            if (dataset->getDataType() == PointType)
                pointDatasets << dataset;

        return pointDatasets;
    });

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
        _scatterplotPlugin->getPositionDataset() = pickedDataset;
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });
}

CurrentDatasetAction::Widget::Widget(QWidget* parent, CurrentDatasetAction* currentDatasetAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, currentDatasetAction)
{
    setFixedWidth(300);

    auto layout = new QHBoxLayout();

    layout->addWidget(currentDatasetAction->_positionDatasetPickerAction.createLabelWidget(this));
    layout->addWidget(currentDatasetAction->_positionDatasetPickerAction.createWidget(this));

    if (widgetFlags & PopupLayout)
    {
        setPopupLayout(layout);
            
    } else {
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
    }
}