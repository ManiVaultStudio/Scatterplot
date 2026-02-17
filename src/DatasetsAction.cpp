#include "DatasetsAction.h"
#include "ScatterplotPlugin.h"

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <ColorData/ColorData.h>

#include <QMenu>

using namespace mv;
using namespace mv::gui;

DatasetsAction::DatasetsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _positionDatasetPickerAction(this, "Position"),
    _colorDatasetPickerAction(this, "Color"),
    _pointSizeDatasetPickerAction(this, "Point size"),
    _pointOpacityDatasetPickerAction(this, "Point opacity")
{
    setIconByName("database");
    setToolTip("Manage loaded datasets for position and color");
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_positionDatasetPickerAction);
    addAction(&_colorDatasetPickerAction);
    addAction(&_pointSizeDatasetPickerAction);
    addAction(&_pointOpacityDatasetPickerAction);

    _colorDatasetPickerAction.setDefaultWidgetFlag(OptionAction::Clearable);
    _pointSizeDatasetPickerAction.setDefaultWidgetFlag(OptionAction::Clearable);
    _pointOpacityDatasetPickerAction.setDefaultWidgetFlag(OptionAction::Clearable);

    _positionDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
    });

    _colorDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return (dataset->getDataType() == PointType || dataset->getDataType() == ColorType || dataset->getDataType() == ClusterType);
    });

    _pointSizeDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
    });

    _pointOpacityDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
    });

    auto scatterplotPlugin = dynamic_cast<ScatterplotPlugin*>(parent->parent());

    if (scatterplotPlugin == nullptr)
        return;

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, scatterplotPlugin](Dataset<DatasetImpl> pickedDataset) -> void {
        scatterplotPlugin->getPositionDataset() = pickedDataset;
    });

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });

    auto& coloringAction = scatterplotPlugin->getSettingsAction().getColoringAction();

    connect(&_colorDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, &coloringAction](Dataset<DatasetImpl> pickedDataset) -> void {
        coloringAction.getColorByAction().setCurrentIndex(pickedDataset.isValid() ? 2 : 0);
        coloringAction.setCurrentColorDataset(pickedDataset);
    });
    
    connect(&scatterplotPlugin->getSettingsAction().getColoringAction(), &ColoringAction::currentColorDatasetChanged, this, [this](Dataset<DatasetImpl> currentColorDataset) -> void {
        _colorDatasetPickerAction.setCurrentDataset(currentColorDataset);
    });

    auto& pointPlotAction       = scatterplotPlugin->getSettingsAction().getPlotAction().getPointPlotAction();
    auto& pointSizeAction       = pointPlotAction.getSizeAction();
    auto& pointOpacityAction    = pointPlotAction.getOpacityAction();

    const auto pointSizeSourceChanged = [this, &pointSizeAction]() -> void {
        _pointSizeDatasetPickerAction.setCurrentDataset(pointSizeAction.isSourceDataset() ? pointSizeAction.getCurrentDataset() : nullptr);

        if (!pointSizeAction.isSourceDataset())
            _pointSizeDatasetPickerAction.setCurrentIndex(-1);
    };

    connect(&pointSizeAction, &ScalarAction::sourceSelectionChanged, this, pointSizeSourceChanged);
    connect(&pointSizeAction, &ScalarAction::sourceDataChanged, this, pointSizeSourceChanged);

    connect(&_pointSizeDatasetPickerAction, &DatasetPickerAction::currentIndexChanged, this, [this, &pointSizeAction](const int32_t& currentIndex) -> void {
        pointSizeAction.setCurrentDataset(_pointSizeDatasetPickerAction.getCurrentDataset());

        if (currentIndex < 0)
            pointSizeAction.setCurrentSourceIndex(ScalarSourceModel::DefaultRow::Constant);
    });

    const auto pointOpacitySourceChanged = [this, &pointOpacityAction]() -> void {
        _pointOpacityDatasetPickerAction.setCurrentDataset(pointOpacityAction.isSourceDataset() ? pointOpacityAction.getCurrentDataset() : nullptr);

        if (!pointOpacityAction.isSourceDataset())
            _pointOpacityDatasetPickerAction.setCurrentIndex(-1);
    };

    connect(&pointOpacityAction, &ScalarAction::sourceSelectionChanged, this, pointOpacitySourceChanged);
    connect(&pointOpacityAction, &ScalarAction::sourceDataChanged, this, pointOpacitySourceChanged);

    connect(&_pointOpacityDatasetPickerAction, &DatasetPickerAction::currentIndexChanged, this, [this, &pointOpacityAction](const int32_t& currentIndex) -> void {
        pointOpacityAction.setCurrentDataset(_pointOpacityDatasetPickerAction.getCurrentDataset());

        if (currentIndex < 0)
            pointOpacityAction.setCurrentSourceIndex(ScalarSourceModel::DefaultRow::Constant);
    });
}

void DatasetsAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicDatasetsAction = dynamic_cast<DatasetsAction*>(publicAction);

    Q_ASSERT(publicDatasetsAction != nullptr);

    if (publicDatasetsAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_positionDatasetPickerAction, &publicDatasetsAction->getPositionDatasetPickerAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_colorDatasetPickerAction, &publicDatasetsAction->getColorDatasetPickerAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_pointSizeDatasetPickerAction, &publicDatasetsAction->getPointSizeDatasetPickerAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_pointOpacityDatasetPickerAction, &publicDatasetsAction->getPointOpacityDatasetPickerAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void DatasetsAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_positionDatasetPickerAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_colorDatasetPickerAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_pointSizeDatasetPickerAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_pointOpacityDatasetPickerAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void DatasetsAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _positionDatasetPickerAction.fromParentVariantMap(variantMap);
    _colorDatasetPickerAction.fromParentVariantMap(variantMap);
}

QVariantMap DatasetsAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _positionDatasetPickerAction.insertIntoVariantMap(variantMap);
    _colorDatasetPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}