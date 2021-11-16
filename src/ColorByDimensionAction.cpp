#include "ColorByDimensionAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "PointData.h"

#include <QMenu>
#include <QHBoxLayout>

using namespace hdps::gui;
using namespace hdps::util;

ColorByDimensionAction::ColorByDimensionAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _datasetPickerAction(this),
    _pointsDimensionPickerAction(this)
{
    scatterplotPlugin->addAction(&_datasetPickerAction);

    // Get reference to points dataset
    auto& points = _scatterplotPlugin->getPositionDataset();

    // Update dataset picker when position (source) or colors dataset changes
    connect(&_scatterplotPlugin->getPositionDataset(), &DatasetRef<Points>::changed, this, &ColorByDimensionAction::updateDatasetPickerAction);
    connect(&_scatterplotPlugin->getPositionSourceDataset(), &DatasetRef<Points>::changed, this, &ColorByDimensionAction::updateDatasetPickerAction);
    connect(&_scatterplotPlugin->getColorsDataset(), &DatasetRef<Points>::changed, this, &ColorByDimensionAction::updateDatasetPickerAction);
    
    // Update dimensions action when a dataset is picked
    connect(&_datasetPickerAction, &DatasetPickerAction::datasetPicked, this, [this](const DatasetRef<hdps::DataSet>& pickedDataset) {
        _pointsDimensionPickerAction.setPointsDataset(DatasetRef<Points>(pickedDataset.get<Points>()));
    });
}

QMenu* ColorByDimensionAction::getContextMenu()
{
    auto menu = new QMenu("Color dimension");

    menu->addAction(&_datasetPickerAction);

    return menu;
}

void ColorByDimensionAction::updateDatasetPickerAction()
{
    QVector<DatasetRef<DataSet>> datasets;

    // Get reference to position dataset
    auto& positionDataset = _scatterplotPlugin->getPositionDataset();

    // Do not update if no position dataset is loaded
    if (!positionDataset.isValid())
        return;

    datasets << DatasetRef<DataSet>(positionDataset.get());

    // Get reference to position source dataset
    auto& positionSourceDataset = _scatterplotPlugin->getPositionSourceDataset();

    // Add source position dataset (if position dataset is derived)
    if (positionSourceDataset.isValid())
        datasets << DatasetRef<DataSet>(positionSourceDataset.get());

    // Get reference to colors dataset
    auto& colorsDataset = _scatterplotPlugin->getColorsDataset();

    // Add option to color the points with a colors dataset if one is loaded
    if (colorsDataset.isValid())
        datasets << DatasetRef<DataSet>(colorsDataset.get());

    // Assign options
    _datasetPickerAction.setDatasets(datasets);
}

ColorByDimensionAction::Widget::Widget(QWidget* parent, ColorByDimensionAction* colorDataAction) :
    WidgetActionWidget(parent, colorDataAction)
{
    auto layout = new QHBoxLayout();

    layout->setMargin(0);

    auto pickDatasetWidget              = colorDataAction->getDatasetPickerAction().createWidget(this);
    auto pointsDimensionPickerWidget    = colorDataAction->getPointsDimensionPickerAction().createWidget(this);

    pickDatasetWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    layout->addWidget(pickDatasetWidget);
    layout->addWidget(pointsDimensionPickerWidget);

    setLayout(layout);
}
