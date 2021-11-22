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

    // Update dataset picker when the position dataset changes
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this]() {
        updateDatasetPickerAction(_scatterplotPlugin->getPositionDataset());
    });

    // Update dataset picker when the position source dataset changes
    connect(&_scatterplotPlugin->getPositionSourceDataset(), &Dataset<Points>::changed, this, [this]() {
        updateDatasetPickerAction(_scatterplotPlugin->getPositionSourceDataset());
    });

    // Update dataset picker when the colors dataset changes
    connect(&_scatterplotPlugin->getColorsDataset(), &Dataset<Points>::changed, this, [this]() {
        updateDatasetPickerAction(_scatterplotPlugin->getColorsDataset());
    });

    // Update dataset picker when the position dataset changes
    connect(&_scatterplotPlugin->getClustersDataset(), &Dataset<Clusters>::changed, this, [this]() {
        updateDatasetPickerAction(_scatterplotPlugin->getClustersDataset());
    });
    
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
            _scatterplotPlugin->loadColors(currentDataset.get<Clusters>());
        else
            _scatterplotPlugin->loadColors(currentDataset.get<Points>(), _pointsDimensionPickerAction.getCurrentDimensionIndex());
    };

    // Update dimensions action when a dataset is picked
    connect(&_datasetPickerAction, &DatasetPickerAction::datasetPicked, this, [this, updateColors](const Dataset<DatasetImpl>& pickedDataset) {
        _pointsDimensionPickerAction.setPointsDataset(pickedDataset);
        updateColors();
    });

    // Update dimensions action when a dataset dimension is picked
    connect(&_pointsDimensionPickerAction, &PointsDimensionPickerAction::currentDimensionIndexChanged, this, [this, updateColors](const std::int32_t& currentDimensionIndex) {
        updateColors();
    });

    connect(&_scatterplotPlugin->getClustersDataset(), &Dataset<Clusters>::dataChanged, this, [this, updateColors]() {

        // Get current dataset
        const auto currentDataset = _datasetPickerAction.getCurrentDataset();

        // Only proceed if we have a valid dataset for coloring
        if (!currentDataset.isValid())
            return;

        if (currentDataset == _scatterplotPlugin->getClustersDataset())
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

void ColorByDataAction::updateDatasetPickerAction(const Dataset<DatasetImpl>& datasetToSelect)
{
    QVector<Dataset<DatasetImpl>> datasets;

    // Get reference to position dataset
    const auto positionDataset = _scatterplotPlugin->getPositionDataset();

    // Do not update if no position dataset is loaded
    if (!positionDataset.isValid())
        return;

    datasets << positionDataset;

    // Get smart pointer to position source dataset
    const auto positionSourceDataset = _scatterplotPlugin->getPositionSourceDataset();

    // Add source position dataset (if position dataset is derived)
    if (positionSourceDataset.isValid())
        datasets << positionSourceDataset;

    // Get smart pointer to colors dataset
    const auto colorsDataset = _scatterplotPlugin->getColorsDataset();

    // Add option to color the points with a colors dataset if one is loaded
    if (colorsDataset.isValid())
        datasets << colorsDataset;

    // Get smart pointer to clusters dataset
    const auto clustersDataset = _scatterplotPlugin->getClustersDataset();

    // Add option to color the points with a colors dataset if one is loaded
    if (clustersDataset.isValid())
        datasets << clustersDataset;

    // Assign options
    _datasetPickerAction.setDatasets(datasets);

    if (datasetToSelect.isValid())
        _datasetPickerAction.setCurrentDataset(datasetToSelect);
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
