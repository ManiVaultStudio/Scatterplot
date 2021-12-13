#include "PointPlotAction.h"
#include "ScalarSourceAction.h"

#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "DataHierarchyItem.h"

using namespace gui;

PointPlotAction::PointPlotAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Point"),
    _sizeAction(scatterplotPlugin, "Point size", 0.0, 100.0, DEFAULT_POINT_SIZE, DEFAULT_POINT_SIZE),
    _opacityAction(scatterplotPlugin, "Point opacity", 0.0, 100.0, DEFAULT_POINT_OPACITY, DEFAULT_POINT_OPACITY),
    _pointSizeScalars(),
    _pointOpacityScalars()
{
    _scatterplotPlugin->addAction(&_sizeAction);
    _scatterplotPlugin->addAction(&_opacityAction);

    _sizeAction.getMagnitudeAction().setSuffix("px");
    _opacityAction.getMagnitudeAction().setSuffix("%");

    _opacityAction.getSourceAction().getOffsetAction().setSuffix("%");

    // Update size by action when the position dataset changes
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this]() {

        // Get reference to position dataset
        const auto positionDataset = _scatterplotPlugin->getPositionDataset();

        // Do not update if no position dataset is loaded
        if (!positionDataset.isValid())
            return;

        // Remove all datasets from the models
        _sizeAction.removeAllDatasets();
        _opacityAction.removeAllDatasets();

        // Add the position dataset
        _sizeAction.addDataset(positionDataset);
        _opacityAction.addDataset(positionDataset);

        // Get smart pointer to position source dataset
        const auto positionSourceDataset = _scatterplotPlugin->getPositionSourceDataset();

        // Add source position dataset (if position dataset is derived)
        if (positionSourceDataset.isValid()) {

            // Add the position dataset
            _sizeAction.addDataset(positionSourceDataset);
            _opacityAction.addDataset(positionSourceDataset);
        }

        // Update the color by action
        updateDefaultDatasets();

        // Reset the point size and opacity scalars
        updateScatterPlotWidgetPointSizeScalars();
        updateScatterPlotWidgetPointOpacityScalars();

        // Reset
        _sizeAction.getSourceAction().getPickerAction().setCurrentIndex(0);
        _opacityAction.getSourceAction().getPickerAction().setCurrentIndex(0);
    });

    // Enable/disable opacity when focus selection is turned on/off
    connect(&_scatterplotPlugin->getSettingsAction().getSelectionAction().getFocusSelectionAction(), &ToggleAction::toggled, this, &PointPlotAction::onFocusSelectionChanged);

    // Update default datasets when a child is added to or removed from the position dataset
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildAdded, this, &PointPlotAction::updateDefaultDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildRemoved, this, &PointPlotAction::updateDefaultDatasets);

    // Update scatter plot widget point size scalars
    connect(&_sizeAction, &ScalarAction::magnitudeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::offsetChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::sourceSelectionChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::sourceDataChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::scalarRangeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);

    // Update scatter plot widget point opacity scalars
    connect(&_opacityAction, &ScalarAction::magnitudeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::offsetChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::sourceSelectionChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::sourceDataChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::scalarRangeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);

    // Update the point size and opacity scalars if in selection scalar source mode when the selection of the position dataset changes
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataSelectionChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataSelectionChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);

    // For convenience, set the offset to double the magnitude in case of a selection source
    connect(&_sizeAction, &ScalarAction::sourceSelectionChanged, this, [this](const std::uint32_t& sourceSelectionIndex) {
        _sizeAction.getSourceAction().getOffsetAction().setValue(sourceSelectionIndex == ScalarSourceModel::DefaultRow::Selection ? _sizeAction.getMagnitudeAction().getValue() : 0.0f);
    });
}

QMenu* PointPlotAction::getContextMenu()
{
    auto menu = new QMenu("Plot settings");

    const auto renderMode = getScatterplotWidget().getRenderMode();

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_sizeAction);
    addActionToMenu(&_opacityAction);

    return menu;
}

void PointPlotAction::addPointSizeDataset(const Dataset<DatasetImpl>& pointSizeDataset)
{
    // Only proceed if we have a valid point size dataset
    if (!pointSizeDataset.isValid())
        return;

    // Add the dataset to the size action
    _sizeAction.addDataset(pointSizeDataset);
}

void PointPlotAction::addPointOpacityDataset(const Dataset<DatasetImpl>& pointOpacityDataset)
{
    // Only proceed if we have a valid point opacity dataset
    if (!pointOpacityDataset.isValid())
        return;

    // Add the dataset to the opacity action
    _opacityAction.addDataset(pointOpacityDataset);
}

void PointPlotAction::updateDefaultDatasets()
{
    // Get smart pointer to the position dataset
    auto positionDataset = Dataset<Points>(_scatterplotPlugin->getPositionDataset());

    // Only proceed if the position dataset is loaded
    if (!positionDataset.isValid())
        return;

    // Get child data hierarchy items of the position dataset
    const auto children = positionDataset->getDataHierarchyItem().getChildren();

    // Loop over all children and possibly add them to the datasets vector
    for (auto child : children) {

        // Get smart pointer to child dataset
        const auto childDataset = child->getDataset();

        // Get the data type
        const auto dataType = childDataset->getDataType();

        // Add if points/clusters and not derived
        if (dataType != PointType)
            continue;

        // Convert child dataset to points smart pointer
        auto points = Dataset<Points>(childDataset);

        // Add datasets
        _sizeAction.addDataset(points);
        _opacityAction.addDataset(points);
    }
}

void PointPlotAction::updateScatterPlotWidgetPointSizeScalars()
{
    // Number of points
    const auto numberOfPoints = _scatterplotPlugin->getPositionDataset()->getNumPoints();

    // Resize to number of points if needed
    if (numberOfPoints != _pointSizeScalars.size()) {

        // Resize the point size scalars vector because the number of points changed
        _pointSizeScalars.resize(numberOfPoints);
    }

    // Fill with ones for constant point size
    std::fill(_pointSizeScalars.begin(), _pointSizeScalars.end(), _sizeAction.getMagnitudeAction().getValue());

    // Populate scalars with dataset if not in constant mode
    if (!_sizeAction.isSourceConstant()) {

        // Get current point size source dataset
        auto pointSizeSourceDataset = Dataset<Points>(_sizeAction.getCurrentDataset());

        // Only populate scalars from dataset if the number of points in the source and target dataset match and we have a valid input dataset
        if (pointSizeSourceDataset.isValid() && pointSizeSourceDataset->getNumPoints() == _scatterplotPlugin->getPositionDataset()->getNumPoints())
        {
            // Visit the points dataset to get access to the point values
            pointSizeSourceDataset->visitData([this, pointSizeSourceDataset, numberOfPoints](auto pointData) {

                // Get current dimension index
                const auto currentDimensionIndex = _sizeAction.getSourceAction().getDimensionPickerAction().getCurrentDimensionIndex();

                // Get range for selected dimension
                const auto rangeMin     = _sizeAction.getSourceAction().getRangeAction().getMinimum();
                const auto rangeMax     = _sizeAction.getSourceAction().getRangeAction().getMaximum();
                const auto rangeLength  = rangeMax - rangeMin;

                // Prevent zero division in normalization
                if (rangeLength > 0) {

                    // Loop over all points and compute the point size scalar
                    for (std::uint32_t pointIndex = 0; pointIndex < numberOfPoints; pointIndex++) {

                        // Get point value for dimension
                        auto pointValue = static_cast<float>(pointData[pointIndex][currentDimensionIndex]);

                        // Clamp the point value to the range
                        const auto pointValueClamped = std::max(rangeMin, std::min(rangeMax, pointValue));

                        // Compute normalized point value
                        const auto pointValueNormalized = (pointValueClamped - rangeMin) / rangeLength;

                        // Compute normalized point size scalar
                        _pointSizeScalars[pointIndex] = _sizeAction.getSourceAction().getOffsetAction().getValue() + (pointValueNormalized * _sizeAction.getMagnitudeAction().getValue());
                    }
                }
                else {

                    // Zero division, so reset all point size scalars to zero
                    std::fill(_pointSizeScalars.begin(), _pointSizeScalars.end(), 0.0f);
                }
            });
        }
    }

    if (_sizeAction.isSourceSelection()) {

        // Get smart pointer to current position dataset
        auto positionDataset = _scatterplotPlugin->getPositionDataset();

        // Get smart pointer to the position selection dataset
        auto selectionSet = positionDataset->getSelection<Points>();

        // Default point size for all
        std::fill(_pointSizeScalars.begin(), _pointSizeScalars.end(), _sizeAction.getMagnitudeAction().getValue());

        // Establish point size of selected points
        const auto pointSizeSelectedPoints = _sizeAction.getMagnitudeAction().getValue() + _sizeAction.getSourceAction().getOffsetAction().getValue();

        // And selected point size for selected points
        for (const auto& selectionIndex : selectionSet->indices)
            _pointSizeScalars[selectionIndex] = pointSizeSelectedPoints;
    }

    // Set scatter plot point size scalars
    _scatterplotPlugin->getScatterplotWidget().setPointSizeScalars(_pointSizeScalars);
}

void PointPlotAction::updateScatterPlotWidgetPointOpacityScalars()
{
    // Enable the opacity magnitude action in constant mode
    //_opacityAction.getMagnitudeAction().setEnabled(_opacityAction.isConstant());

    // Normalized point opacity scalars
    std::vector<float> pointOpacityScalars;

    // Number of points
    const auto numberOfPoints = _scatterplotPlugin->getPositionDataset()->getNumPoints();

    // Resize to number of points
    pointOpacityScalars.resize(numberOfPoints);

    // Establish opacity magnitude
    const auto opacityMagnitude = 0.01f * _opacityAction.getMagnitudeAction().getValue();

    // Fill with ones for constant point opacity
    std::fill(pointOpacityScalars.begin(), pointOpacityScalars.end(), opacityMagnitude);

    // Populate scalars with dataset if not in constant mode
    if (!_opacityAction.isSourceConstant()) {

        // Get current point opacity source dataset
        auto pointOpacitySourceDataset = Dataset<Points>(_opacityAction.getCurrentDataset());

        // Only proceed if we have a valid set
        if (!pointOpacitySourceDataset.isValid())
            return;

        // Only populate scalars from dataset if the number of points in the source and target dataset match
        if (pointOpacitySourceDataset->getNumPoints() == _scatterplotPlugin->getPositionDataset()->getNumPoints())
        {
            // Visit the points dataset to get access to the point values
            pointOpacitySourceDataset->visitData([this, pointOpacitySourceDataset, &pointOpacityScalars, numberOfPoints, opacityMagnitude](auto pointData) {

                // Get current dimension index
                const auto currentDimensionIndex = _opacityAction.getSourceAction().getDimensionPickerAction().getCurrentDimensionIndex();

                // Get opacity offset
                const auto opacityOffset = 0.01f * _opacityAction.getSourceAction().getOffsetAction().getValue();

                // Get range for selected dimension
                const auto rangeMin     = _opacityAction.getSourceAction().getRangeAction().getMinimum();
                const auto rangeMax     = _opacityAction.getSourceAction().getRangeAction().getMaximum();
                const auto rangeLength  = rangeMax - rangeMin;

                // Prevent zero division in normalization
                if (rangeLength > 0) {

                    // Loop over all points and compute the point size opacity
                    for (std::uint32_t pointIndex = 0; pointIndex < numberOfPoints; pointIndex++) {

                        // Get point value for dimension
                        auto pointValue = static_cast<float>(pointData[pointIndex][currentDimensionIndex]);

                        // Clamp the point value to the range
                        const auto pointValueClamped = std::max(rangeMin, std::min(rangeMax, pointValue));

                        // Compute normalized point value
                        const auto pointValueNormalized = (pointValueClamped - rangeMin) / rangeLength;

                        // Compute normalized point opacity scalar
                        if (opacityOffset == 1.0f)
                            pointOpacityScalars[pointIndex] = 1.0f;
                        else
                            pointOpacityScalars[pointIndex] = opacityMagnitude * (opacityOffset + (pointValueNormalized / (1.0f - opacityOffset)));
                    }
                }
                else {

                    // Get reference to range action
                    auto& rangeAction = _opacityAction.getSourceAction().getRangeAction();

                    // Handle zero division
                    if (rangeAction.getRangeMinAction().getValue() == rangeAction.getRangeMaxAction().getValue())
                        std::fill(pointOpacityScalars.begin(), pointOpacityScalars.end(), 0.0f);
                    else
                        std::fill(pointOpacityScalars.begin(), pointOpacityScalars.end(), 1.0f);
                }
            });
        }
    }

    // Set scatter plot point size scalars
    _scatterplotPlugin->getScatterplotWidget().setPointOpacityScalars(pointOpacityScalars);
}

void PointPlotAction::onFocusSelectionChanged(const bool& focusSelection)
{
    // Disable opacity action when focus selection is enabled
    _opacityAction.setEnabled(!focusSelection);
}

PointPlotAction::Widget::Widget(QWidget* parent, PointPlotAction* pointPlotAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, pointPlotAction, widgetFlags)
{
    setToolTip("Point plot settings");

    // Add widgets
    if (widgetFlags & PopupLayout) {
        auto layout = new QGridLayout();

        layout->setMargin(0);

        layout->addWidget(pointPlotAction->getSizeAction().createLabelWidget(this), 0, 0);
        layout->addWidget(pointPlotAction->getSizeAction().createWidget(this), 0, 1);

        layout->addWidget(pointPlotAction->getOpacityAction().createLabelWidget(this), 1, 0);
        layout->addWidget(pointPlotAction->getOpacityAction().createWidget(this), 1, 1);

        setLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        layout->setMargin(0);

        layout->addWidget(pointPlotAction->getSizeAction().createLabelWidget(this));
        layout->addWidget(pointPlotAction->getSizeAction().createWidget(this));
        layout->addWidget(pointPlotAction->getOpacityAction().createLabelWidget(this));
        layout->addWidget(pointPlotAction->getOpacityAction().createWidget(this));

        setLayout(layout);
    }
}
