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
    _opacityAction(scatterplotPlugin, "Point opacity", 0.0, 100.0, DEFAULT_POINT_OPACITY, DEFAULT_POINT_OPACITY)
{
    _scatterplotPlugin->addAction(&_sizeAction);
    _scatterplotPlugin->addAction(&_opacityAction);

    _sizeAction.getMagnitudeAction().setSuffix("px");
    _opacityAction.getMagnitudeAction().setSuffix("%");

    const auto updatePointSize = [this]() -> void {
        getScatterplotWidget()->setPointSize(_sizeAction.getMagnitudeAction().getValue());
    };

    const auto updatePointOpacity = [this]() -> void {
        getScatterplotWidget()->setAlpha(0.01 * _opacityAction.getMagnitudeAction().getValue());
    };

    connect(&_sizeAction.getMagnitudeAction(), &DecimalAction::valueChanged, this, updatePointSize);
    connect(&_opacityAction.getMagnitudeAction(), &DecimalAction::valueChanged, this, updatePointOpacity);

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

        // Reset
        _sizeAction.getSourceAction().getPickerAction().reset();
        _opacityAction.getSourceAction().getPickerAction().reset();
    });

    // Update default datasets when a child is added to or removed from the position dataset
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildAdded, this, &PointPlotAction::updateDefaultDatasets);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildRemoved, this, &PointPlotAction::updateDefaultDatasets);

    // Update scatter plot widget point sizing when the size magnitude or source data of the size action changed
    connect(&_sizeAction, &ScalarAction::magnitudeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSize);
    connect(&_sizeAction, &ScalarAction::sourceDataChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);
    connect(&_sizeAction, &ScalarAction::scalarRangeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointSizeScalars);

    // Update scatter plot widget point transparency when the size magnitude or source data of the opacity action changed
    connect(&_opacityAction, &ScalarAction::magnitudeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacity);
    connect(&_opacityAction, &ScalarAction::sourceDataChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);
    connect(&_opacityAction, &ScalarAction::scalarRangeChanged, this, &PointPlotAction::updateScatterPlotWidgetPointOpacityScalars);

    // Initial updates
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

        // Number of points must match
        if (points->getNumPoints() != positionDataset->getNumPoints())
            continue;

        // Add datasets
        _sizeAction.addDataset(points);
        _opacityAction.addDataset(points);
    }
}

void PointPlotAction::updateScatterPlotWidgetPointSize()
{
    qDebug() << "updateScatterPlotWidgetPointSize";
}

void PointPlotAction::updateScatterPlotWidgetPointSizeScalars()
{
    qDebug() << "updateScatterPlotWidgetPointSizeScalars";
}

void PointPlotAction::updateScatterPlotWidgetPointOpacity()
{
    qDebug() << "updateScatterPlotWidgetPointOpacity";
}

void PointPlotAction::updateScatterPlotWidgetPointOpacityScalars()
{
    qDebug() << "updateScatterPlotWidgetPointOpacityScalars";
}

PointPlotAction::Widget::Widget(QWidget* parent, PointPlotAction* pointPlotAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, pointPlotAction, widgetFlags)
{
    setToolTip("Point plot settings");

    // Add widgets
    if (widgetFlags & PopupLayout) {
        auto layout = new QVBoxLayout();

        layout->setMargin(0);

        layout->addWidget(pointPlotAction->getSizeAction().createWidget(this));
        layout->addWidget(pointPlotAction->getOpacityAction().createWidget(this));

        setLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        // Create action widgets
        //auto sizeByWidget = pointPlotAction->getSizeAction().createWidget(this);
        //auto opacityWidget = pointPlotAction->getOpacityAction().createWidget(this);

        layout->setMargin(0);
        layout->addWidget(pointPlotAction->getSizeAction().createWidget(this));
        layout->addWidget(pointPlotAction->getOpacityAction().createWidget(this));

        setLayout(layout);
    }
}
