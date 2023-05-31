#include "LoadedDatasetsAction.h"
#include "ScatterplotPlugin.h"

#include "PointData/PointData.h"
#include "ClusterData/ClusterData.h"
#include "ColorData/ColorData.h"
#include "ScatterplotPlugin.h"

#include <QMenu>

using namespace hdps;
using namespace hdps::gui;

LoadedDatasetsAction::LoadedDatasetsAction(QObject* parent, const QString& title) :
    WidgetAction(parent, title),
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

    auto scatterplotPlugin = dynamic_cast<ScatterplotPlugin*>(parent);

    if (scatterplotPlugin == nullptr)
        return;

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, scatterplotPlugin](Dataset<DatasetImpl> pickedDataset) -> void {
        scatterplotPlugin->getPositionDataset() = pickedDataset;
    });

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });
    
    connect(&_colorDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, scatterplotPlugin](Dataset<DatasetImpl> pickedDataset) -> void {
        scatterplotPlugin->getSettingsAction().getColoringAction().setCurrentColorDataset(pickedDataset);
    });
    
    connect(&scatterplotPlugin->getSettingsAction().getColoringAction(), &ColoringAction::currentColorDatasetChanged, this, [this](Dataset<DatasetImpl> currentColorDataset) -> void {
        _colorDatasetPickerAction.setCurrentDataset(currentColorDataset);
    });
}

void LoadedDatasetsAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicLoadedDatasetsAction = dynamic_cast<LoadedDatasetsAction*>(publicAction);

    Q_ASSERT(publicLoadedDatasetsAction != nullptr);

    if (publicLoadedDatasetsAction == nullptr)
        return;

    if (recursive) {
        _positionDatasetPickerAction.connectToPublicAction(&publicLoadedDatasetsAction->_positionDatasetPickerAction, recursive);
        _colorDatasetPickerAction.connectToPublicAction(&publicLoadedDatasetsAction->_colorDatasetPickerAction, recursive);
    }

    WidgetAction::connectToPublicAction(publicAction, recursive);
}

void LoadedDatasetsAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        _positionDatasetPickerAction.disconnectFromPublicAction(recursive);
        _colorDatasetPickerAction.disconnectFromPublicAction(recursive);
    }

    WidgetAction::disconnectFromPublicAction(recursive);
}

void LoadedDatasetsAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _positionDatasetPickerAction.fromParentVariantMap(variantMap);
    _colorDatasetPickerAction.fromParentVariantMap(variantMap);
}

QVariantMap LoadedDatasetsAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _positionDatasetPickerAction.insertIntoVariantMap(variantMap);
    _colorDatasetPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
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