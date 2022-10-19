#include "LoadedDatasetsAction.h"
#include "ScatterplotPlugin.h"

#include "PointData.h"
#include "ColorData.h"
#include "ClusterData.h"

#include <QMenu>

using namespace hdps;
using namespace hdps::gui;

LoadedDatasetsAction::LoadedDatasetsAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Loaded datasets"),
    _positionDatasetPickerAction(this, "Position"),
    _colorDatasetPickerAction(this, "Color")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("database"));
    setToolTip("Manage loaded datasets for position and/or color");

    _positionDatasetPickerAction.setDatasetsFilterFunction([](const hdps::Datasets& datasets) -> Datasets {
        Datasets pointDatasets;

        for (auto dataset : datasets)
            if (dataset->getDataType() == PointType)
                pointDatasets << dataset;

        return pointDatasets;
    });

    _colorDatasetPickerAction.setDatasetsFilterFunction([](const hdps::Datasets& datasets) -> Datasets {
        Datasets colorDatasets;

        for (auto dataset : datasets)
            if (dataset->getDataType() == PointType || dataset->getDataType() == ColorType || dataset->getDataType() == ClusterType)
                colorDatasets << dataset;

        return colorDatasets;
    });

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
        _scatterplotPlugin->getPositionDataset() = pickedDataset;
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });

    
    connect(&_colorDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this](Dataset<DatasetImpl> pickedDataset) -> void {
        _scatterplotPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(pickedDataset);
    });
    
    connect(&_scatterplotPlugin->getSettingsAction().getColoringAction(), &ColoringAction::currentColorDatasetChanged, this, [this](Dataset<DatasetImpl> currentColorDataset) -> void {
        _colorDatasetPickerAction.setCurrentDataset(currentColorDataset);
    });
}

LoadedDatasetsAction::Widget::Widget(QWidget* parent, LoadedDatasetsAction* currentDatasetAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, currentDatasetAction)
{
    setFixedWidth(300);

    auto layout = new QGridLayout();

    layout->addWidget(currentDatasetAction->_positionDatasetPickerAction.createLabelWidget(this), 0, 0);
    layout->addWidget(currentDatasetAction->_positionDatasetPickerAction.createWidget(this), 0, 1);
    layout->addWidget(currentDatasetAction->_colorDatasetPickerAction.createLabelWidget(this), 1, 0);
    layout->addWidget(currentDatasetAction->_colorDatasetPickerAction.createWidget(this), 1, 1);

    if (widgetFlags & PopupLayout)
    {
        setPopupLayout(layout);
            
    } else {
        layout->setContentsMargins(0, 0, 0, 0);
        setLayout(layout);
    }
}