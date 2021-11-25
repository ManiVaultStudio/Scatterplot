#include "ColorByDataAction.h"
#include "ColoringAction.h"
#include "Application.h"
#include "DataHierarchyItem.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "PointData.h"
#include "ClusterData.h"

#include "event/Event.h"

#include <QMenu>
#include <QHBoxLayout>

using namespace hdps::gui;
using namespace hdps::util;

ColorByDataAction::ColorByDataAction(ScatterplotPlugin* scatterplotPlugin, ColoringAction& coloringAction) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _coloringAction(coloringAction),
    _datasetPickerAction(this, "Pick color dataset"),
    _pointsDimensionPickerAction(this, "Points"),
    _colorMapAction(this, "Color map"),
    _colorDatasets()
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

    // Update child color datasets when a child is added to or removed from the points position dataset
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildAdded, this, &ColorByDataAction::updateChildColorDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildRemoved, this, &ColorByDataAction::updateChildColorDatasets);

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

    // Update dimensions action when a dataset is picked and load point colors
    connect(&_datasetPickerAction, &DatasetPickerAction::datasetPicked, this, [this, updateColors](const Dataset<DatasetImpl>& pickedDataset) {
        
        // Update dimension picker in case the picked dataset contains points
        if (pickedDataset->getDataType() == PointType)
            _pointsDimensionPickerAction.setPointsDataset(pickedDataset);

        // Update colors in the scatter plot widget
        updateColors();
        //updateColorMapActionScalarRange();
    });

    // Update dimensions action when a dataset dimension is picked
    connect(&_pointsDimensionPickerAction, &PointsDimensionPickerAction::currentDimensionIndexChanged, this, [this, updateColors](const std::int32_t& currentDimensionIndex) {
        updateColors();
        updateColorMapActionScalarRange();
    });

    connect(&_colorMapAction, &ColorMapAction::imageChanged, this, &ColorByDataAction::updateScatterplotWidgetColorMap);
    connect(_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, &ColorByDataAction::updateScatterplotWidgetColorMap);
    connect(&_colorMapAction.getSettingsAction().getHorizontalAxisAction().getRangeAction(), &DecimalRangeAction::rangeChanged, this, &ColorByDataAction::updateScatterPlotWidgetColorMapRange);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, &ColorByDataAction::updateColorMapActionReadOnly);
    connect(_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, &ColorByDataAction::updateColorMapActionReadOnly);
    connect(&_pointsDimensionPickerAction, &PointsDimensionPickerAction::currentDimensionIndexChanged, this, &ColorByDataAction::updateColorMapActionReadOnly);

    // Do an initial update of the scalar ranges
    updateColorMapActionScalarRange();

    // Add compatible child color datasets
    updateChildColorDatasets();
}

QMenu* ColorByDataAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("Color dimension", parent);

    menu->addAction(&_datasetPickerAction);

    return menu;
}

void ColorByDataAction::updateDatasetPickerAction(const Dataset<hdps::DatasetImpl>& datasetToSelect /*= Dataset<DatasetImpl>()*/)
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

    // Add all foreign color datasets
    datasets << _colorDatasets;

    // Assign options
    _datasetPickerAction.setDatasets(datasets);

    // Update the picker
    if (datasetToSelect.isValid())
        _datasetPickerAction.setCurrentDataset(datasetToSelect);
}

void ColorByDataAction::addColorDataset(const Dataset<DatasetImpl>& colorDataset)
{
    // Do not add the same dataset twice
    if (_colorDatasets.contains(colorDataset))
        return;

    // Set the color by to data
    _coloringAction.getColorByDataTriggerAction().trigger();

    // Add the color set
    _colorDatasets << colorDataset;

    // Update the dataset picker action and select the added color dataset
    updateDatasetPickerAction(colorDataset);

    // Connect to the data changed signal so that we can update the scatter plot colors appropriately
    connect(&_colorDatasets.last(), &Dataset<DatasetImpl>::dataChanged, this, [this, colorDataset]() {

        // Get smart pointer to current dataset
        const auto currentDataset = _datasetPickerAction.getCurrentDataset();

        // Only proceed if we have a valid dataset for coloring
        if (!currentDataset.isValid())
            return;

        // Update colors if the dataset matches
        if (currentDataset == colorDataset)
            updateColors();
    });
}

void ColorByDataAction::updateChildColorDatasets()
{
    // Get smart pointer to the position dataset
    auto positionDataset = _scatterplotPlugin->getPositionDataset();

    // Only proceed if the position dataset is loaded
    if (!positionDataset.isValid())
        return;

    // Get child data hierarchy items of the position dataset
    const auto children = positionDataset->getDataHierarchyItem().getChildren();

    // Loop over all children and possibly add them to the color datasets vector
    for (auto child : children) {

        // Get smart pointer to child dataset
        const auto childDataset = child->getDataset();

        // Get the data type
        const auto dataType = childDataset->getDataType();

        // Add if points/clusters and not derived
        if (dataType == PointType || dataType == ClusterType) {

            // Add the child dataset to the vector (avoid adding duplicates)
            if (!_colorDatasets.contains(child->getDataset()))
                addColorDataset(childDataset);
        }
    }

    // Update the dataset picker action
    updateDatasetPickerAction();
}

bool ColorByDataAction::hasColorDataset(const Dataset<DatasetImpl>& colorDataset) const
{
    return _colorDatasets.contains(colorDataset);
}

Dataset<DatasetImpl> ColorByDataAction::getCurrentColorDataset() const
{
    return _datasetPickerAction.getCurrentDataset();
}

void ColorByDataAction::updateColors()
{
    // Get current dataset
    const auto currentDataset = _datasetPickerAction.getCurrentDataset();

    // Only proceed if we have a valid dataset
    if (!currentDataset.isValid())
        return;

    // Load colors either by points or clusters dataset
    if (currentDataset->getDataType() == ClusterType)
        _scatterplotPlugin->loadColors(currentDataset.get<Clusters>());
    else
        _scatterplotPlugin->loadColors(currentDataset.get<Points>(), _pointsDimensionPickerAction.getCurrentDimensionIndex());
}

void ColorByDataAction::updateColorMapActionScalarRange()
{
    // Get the color map range from the scatter plot widget
    const auto colorMapRange    = _scatterplotPlugin->getScatterplotWidget()->getColorMapRange();
    const auto colorMapRangeMin = colorMapRange.x;
    const auto colorMapRangeMax = colorMapRange.y;

    // Get reference to color map range action
    auto& colorMapRangeAction = _colorMapAction.getSettingsAction().getHorizontalAxisAction().getRangeAction();

    // Initialize the color map range action with the color map range from the scatter plot 
    colorMapRangeAction.initialize(colorMapRangeMin, colorMapRangeMax, colorMapRangeMin, colorMapRangeMax, colorMapRangeMin, colorMapRangeMax);
}

void ColorByDataAction::updateScatterplotWidgetColorMap()
{
    // Only update color map in scatter plot or landscape rendering mode
    if (_scatterplotPlugin->getScatterplotWidget()->getRenderMode() == ScatterplotWidget::DENSITY)
        return;

    // Only update color map in data coloring mode
    if (_scatterplotPlugin->getScatterplotWidget()->getRenderMode() == ScatterplotWidget::SCATTERPLOT && _scatterplotPlugin->getScatterplotWidget()->getColoringMode() == ScatterplotWidget::ColoringMode::Constant)
        return;

    // Only update color map when we have a valid position dataset
    if (!_scatterplotPlugin->getPositionDataset().isValid())
        return;

    // And update the scatter plot widget color map
    getScatterplotWidget()->setColorMap(_colorMapAction.getColorMapImage());
}

void ColorByDataAction::updateScatterPlotWidgetColorMapRange()
{
    // Get color map range action
    const auto& rangeAction = _colorMapAction.getSettingsAction().getHorizontalAxisAction().getRangeAction();

    // And assign scatter plot renderer color map range
    getScatterplotWidget()->setColorMapRange(rangeAction.getMinimum(), rangeAction.getMaximum());
}

bool ColorByDataAction::shouldEnableColorMap() const
{
    // Get render mode from the scatter plot widget
    const auto renderMode = _scatterplotPlugin->getScatterplotWidget()->getRenderMode();

    // Disable the color in density render mode
    if (renderMode == ScatterplotWidget::DENSITY)
        return false;

    // Disable the color map in color by data mode
    if (_scatterplotPlugin->getScatterplotWidget()->getRenderMode() == static_cast<std::int32_t>(ScatterplotWidget::ColoringMode::Constant))
        return false;

    // Get smart pointer to the current color dataset
    const auto currentColorDataset = getCurrentColorDataset();

    // Disable the color map when a clusters color dataset is loaded
    if (currentColorDataset.isValid() && currentColorDataset->getDataType() == ClusterType)
        return false;

    return true;
}

void ColorByDataAction::updateColorMapActionReadOnly()
{
    _colorMapAction.setEnabled(shouldEnableColorMap());
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
