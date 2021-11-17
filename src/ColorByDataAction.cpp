#include "ColorByDataAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "PointData.h"
#include "ClusterData.h"

#include <QMenu>
#include <QHBoxLayout>

using namespace hdps::gui;
using namespace hdps::util;

ColorByDataAction::ColorByDataAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _datasetPickerAction(this),
    _pointsDimensionPickerAction(this, "Points")
{
    scatterplotPlugin->addAction(&_datasetPickerAction);

    // Get reference to points dataset
    auto& points = _scatterplotPlugin->getPositionDataset();

    // Update dataset picker when position (source) or colors dataset changes
    connect(&_scatterplotPlugin->getPositionDataset(), &DatasetRef<Points>::changed, this, &ColorByDataAction::updateDatasetPickerAction);
    connect(&_scatterplotPlugin->getPositionSourceDataset(), &DatasetRef<Points>::changed, this, &ColorByDataAction::updateDatasetPickerAction);
    connect(&_scatterplotPlugin->getColorsDataset(), &DatasetRef<Points>::changed, this, &ColorByDataAction::updateDatasetPickerAction);
    connect(&_scatterplotPlugin->getClustersDataset(), &DatasetRef<Clusters>::changed, this, &ColorByDataAction::updateDatasetPickerAction);
    
    // Update scalar range in color map
    const auto updateScalarRangeActions = [this]() {
        const auto colorMapRange    = _scatterplotPlugin->getScatterplotWidget()->getColorMapRange();
        const auto colorMapRangeMin = colorMapRange.x;
        const auto colorMapRangeMax = colorMapRange.y;

        auto& colorMapAction = _scatterplotPlugin->getSettingsAction().getColoringAction().getColorMapAction();

        //colorMapAction.getSettingsAction().getHorizontalAxisAction().getRangeAction().initialize(colorMapRangeMin, colorMapRangeMax, colorMapRangeMin, colorMapRangeMax, colorMapRangeMin, colorMapRangeMax);
    };

    // Update colors when the color by option changes or data changes
    const auto updateColors = [this]() {

        // Get current dataset
        const auto currentDataset = _datasetPickerAction.getCurrentDataset();

        // Only proceed if we have a valid dataset
        if (!currentDataset.isValid())
            return;

        if (currentDataset->getDataType() == ClusterType)
            _scatterplotPlugin->loadColors(DatasetRef<Clusters>(currentDataset.get<Clusters>()));
        else
            _scatterplotPlugin->loadColors(DatasetRef<Points>(currentDataset.get<Points>()), _pointsDimensionPickerAction.getCurrentDimensionIndex());
    };

    // Update dimensions action when a dataset is picked
    connect(&_datasetPickerAction, &DatasetPickerAction::datasetPicked, this, [this, updateColors](const DatasetRef<hdps::DataSet>& pickedDataset) {
        _pointsDimensionPickerAction.setPointsDataset(DatasetRef<Points>(pickedDataset.get<Points>()));
        updateColors();
    });

    // Update dimensions action when a dataset dimension is picked
    connect(&_pointsDimensionPickerAction, &PointsDimensionPickerAction::currentDimensionIndexChanged, this, [this, updateColors](const std::int32_t& currentDimensionIndex) {
        updateColors();
    });

    connect(&_scatterplotPlugin->getClustersDataset(), &DatasetRef<Clusters>::dataChanged, this, [this, updateColors]() {

        // Get current dataset
        const auto currentDataset = _datasetPickerAction.getCurrentDataset();

        // Only proceed if we have a valid dataset for coloring
        if (!currentDataset.isValid())
            return;

        if (currentDataset->getId() == _scatterplotPlugin->getClustersDataset()->getId())
            updateColors();
    });

    // Do an initial update of the scalar ranges
    updateScalarRangeActions();
}

QMenu* ColorByDataAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("Color dimension", parent);

    menu->addAction(&_datasetPickerAction);

    return menu;
}

void ColorByDataAction::updateDatasetPickerAction()
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

    // Get reference to clusters dataset
    auto& clustersDataset = _scatterplotPlugin->getClustersDataset();

    // Add option to color the points with a colors dataset if one is loaded
    if (clustersDataset.isValid())
        datasets << DatasetRef<DataSet>(clustersDataset.get());

    // Assign options
    _datasetPickerAction.setDatasets(datasets);
}

ColorByDataAction::Widget::Widget(QWidget* parent, ColorByDataAction* colorByDataAction) :
    WidgetActionWidget(parent, colorByDataAction)
{
    auto layout = new QHBoxLayout();

    layout->setMargin(0);

    auto pickDatasetWidget              = colorByDataAction->getDatasetPickerAction().createWidget(this);
    auto pointsDimensionPickerWidget    = colorByDataAction->getPointsDimensionPickerAction().createWidget(this);

    pickDatasetWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    layout->addWidget(pickDatasetWidget);
    layout->addWidget(pointsDimensionPickerWidget);

    setLayout(layout);
}
