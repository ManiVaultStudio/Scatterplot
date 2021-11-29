#include "PointPlotAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "DataHierarchyItem.h"

using namespace hdps::gui;

PointPlotAction::PointPlotAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Point"),
    _sizeByModel(this),
    _sizeByAction(this, "Point size by"),
    _pointSizeDimensionAction(this, "Point size dimension"),
    _sizeAction(this, "Point size", 1.0, 50.0, DEFAULT_POINT_SIZE, DEFAULT_POINT_SIZE),
    _opacityAction(this, "Point opacity", 0.0, 100.0, DEFAULT_POINT_OPACITY, DEFAULT_POINT_OPACITY),
    _pointSizeDatasets()
{
    _scatterplotPlugin->addAction(&_sizeAction);
    _scatterplotPlugin->addAction(&_opacityAction);

    _sizeByAction.setCustomModel(&_sizeByModel);

    _sizeAction.setSuffix("px");
    _opacityAction.setSuffix("%");

    const auto updatePointSize = [this]() -> void {
        getScatterplotWidget()->setPointSize(_sizeAction.getValue());
    };

    const auto updatePointOpacity = [this]() -> void {
        getScatterplotWidget()->setAlpha(0.01 * _opacityAction.getValue());
    };

    connect(&_sizeAction, &DecimalAction::valueChanged, this, updatePointSize);
    connect(&_opacityAction, &DecimalAction::valueChanged, this, updatePointOpacity);

    // Update size by action when the position dataset changes
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this]() {

        // Get reference to position dataset
        const auto positionDataset = _scatterplotPlugin->getPositionDataset();

        // Do not update if no position dataset is loaded
        if (!positionDataset.isValid())
            return;

        // Reset the datasets
        _pointSizeDatasets.clear();

        // Add the position dataset
        addPointSizeDataset(positionDataset);

        // Get smart pointer to position source dataset
        const auto positionSourceDataset = _scatterplotPlugin->getPositionSourceDataset();

        // Add source position dataset (if position dataset is derived)
        if (positionSourceDataset.isValid())
            addPointSizeDataset(positionSourceDataset);

        // Update the color by action
        updateSizeByActionOptions();

        // Reset the color by option
        _sizeByAction.setCurrentIndex(0);
    });

    updatePointSize();
    updatePointOpacity();
}

QMenu* PointPlotAction::getContextMenu()
{
    auto menu = new QMenu("Plot settings");

    const auto renderMode = getScatterplotWidget()->getRenderMode();

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
    // Avoid duplicates
    if (_pointSizeDatasets.contains(pointSizeDataset))
        return;

    // Add point size dataset to the list of candidate point size datasets
    _pointSizeDatasets << pointSizeDataset;

    // Get reference to the last added dataset
    auto& addedDataset = _pointSizeDatasets.last();

    // Connect to the data changed signal so that we can update the scatter plot point size appropriately
    connect(&addedDataset, &Dataset<DatasetImpl>::dataChanged, this, [this, addedDataset]() {

        // Get smart pointer to current point size dataset
        const auto currentPointSizeDataset = getCurrentPointSizeDataset();

        // Only proceed if we have a valid point size dataset
        if (!currentPointSizeDataset.isValid())
            return;

        // Update scatter plot widget point size if the dataset matches
        if (currentPointSizeDataset == addedDataset)
            updateScatterPlotWidgetPointSize();
    });
}

hdps::Dataset<hdps::DatasetImpl> PointPlotAction::getCurrentPointSizeDataset() const
{
    // Get current point size by option index
    const auto sizeByIndex = _sizeByAction.getCurrentIndex();

    // Only proceed if we have a valid point size dataset row index
    if (sizeByIndex < 1)
        return Dataset<DatasetImpl>();

    return _sizeByModel.getDataset(sizeByIndex);
}

void PointPlotAction::updateSizeByActionOptions()
{
    // Get smart pointer to the position dataset
    auto positionDataset = _scatterplotPlugin->getPositionDataset();

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
        if (dataType == PointType)
            addPointSizeDataset(childDataset);
    }

    // Set the datasets in the color-by model
    _sizeByModel.setDatasets(_pointSizeDatasets);
}

void PointPlotAction::updateScatterPlotWidgetPointSize()
{

}

PointPlotAction::Widget::Widget(QWidget* parent, PointPlotAction* pointPlotAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, pointPlotAction, widgetFlags)
{
    setToolTip("Point plot settings");

    // Create action widgets
    auto pointSizeBylabel           = pointPlotAction->getSizeByAction().createLabelWidget(this);
    auto pointSizeByWidget          = pointPlotAction->getSizeByAction().createWidget(this);
    auto pointSizeDimensionWidget   = pointPlotAction->getPointSizeDimensionAction().createWidget(this);
    auto pointSizeWidget            = pointPlotAction->getSizeAction().createWidget(this);
    auto pointOpacitylabel          = pointPlotAction->getOpacityAction().createLabelWidget(this);
    auto pointOpacityWidget         = pointPlotAction->getOpacityAction().createWidget(this);

    // Adjust size of the combo boxes to the contents
    pointSizeByWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    // Add widgets
    if (widgetFlags & PopupLayout) {
        auto layout = new QGridLayout();

        layout->setMargin(0);
        layout->addWidget(pointSizeBylabel, 0, 0);
        layout->addWidget(pointSizeByWidget, 0, 0);
        layout->addWidget(pointSizeDimensionWidget, 0, 0);
        layout->addWidget(pointSizeWidget, 0, 2);
        layout->addWidget(pointOpacitylabel, 1, 0);
        layout->addWidget(pointOpacityWidget, 1, 2);

        setLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        layout->setMargin(0);
        layout->addWidget(pointSizeBylabel);
        layout->addWidget(pointSizeByWidget);
        layout->addWidget(pointSizeDimensionWidget);
        layout->addWidget(pointSizeWidget);
        layout->addWidget(pointOpacitylabel);
        layout->addWidget(pointOpacityWidget);

        setLayout(layout);
    }
}
