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

    auto scatterplotPlugin = dynamic_cast<ScatterplotPlugin*>(parent->parent());

    Q_ASSERT(scatterplotPlugin);

    if (scatterplotPlugin == nullptr)
        return;

    setupDatasetPickerActions(scatterplotPlugin);

    connect(&mv::projects(), &AbstractProjectManager::projectOpened, this, [this, scatterplotPlugin]() -> void {
        setupDatasetPickerActions(scatterplotPlugin);
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

void DatasetsAction::setupDatasetPickerActions(ScatterplotPlugin* scatterplotPlugin)
{
    setupPositionDatasetPickerAction(scatterplotPlugin);
    setupColorDatasetPickerAction(scatterplotPlugin);
    setupPointSizeDatasetPickerAction(scatterplotPlugin);
    setupPointOpacityDatasetPickerAction(scatterplotPlugin);
}

void DatasetsAction::setupPositionDatasetPickerAction(ScatterplotPlugin* scatterplotPlugin)
{
    _positionDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
    });

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, scatterplotPlugin](Dataset<DatasetImpl> pickedDataset) -> void {
        scatterplotPlugin->getPositionDataset() = pickedDataset;
    });

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this](DatasetImpl* dataset) -> void {
        _positionDatasetPickerAction.setCurrentDataset(dataset);
    });
}

void DatasetsAction::setupColorDatasetPickerAction(ScatterplotPlugin* scatterplotPlugin)
{
    auto& settingsAction = scatterplotPlugin->getSettingsAction();

    _colorDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return (dataset->getDataType() == PointType || dataset->getDataType() == ColorType || dataset->getDataType() == ClusterType);
    });

    auto& coloringAction = settingsAction.getColoringAction();

    connect(&_colorDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, &coloringAction, scatterplotPlugin](Dataset<DatasetImpl> pickedDataset) -> void {
        if (_colorDataset.isValid())
            disconnect(&_colorDataset, &Dataset<>::guiNameChanged, this, nullptr);

        _colorDataset = pickedDataset;

        connect(&_colorDataset, &Dataset<>::guiNameChanged, scatterplotPlugin, &ScatterplotPlugin::updateHeadsUpDisplay);

        coloringAction.setCurrentColorDataset(pickedDataset);

        if (!pickedDataset.isValid())
            coloringAction.getColorByAction().setCurrentIndex(0);
    });

    connect(&settingsAction.getColoringAction(), &ColoringAction::currentColorDatasetChanged, this, [this](Dataset<DatasetImpl> currentColorDataset) -> void {
        _colorDatasetPickerAction.setCurrentDataset(currentColorDataset);
    });
}

void DatasetsAction::setupPointSizeDatasetPickerAction(ScatterplotPlugin* scatterplotPlugin)
{
    auto& settingsAction = scatterplotPlugin->getSettingsAction();

    _pointSizeDatasetPickerAction.setFilterFunction([this](mv::Dataset<DatasetImpl> dataset) -> bool {
        return dataset->getDataType() == PointType;
	});

    auto& pointPlotAction   = settingsAction.getPlotAction().getPointPlotAction();
    auto& pointSizeAction   = pointPlotAction.getSizeAction();


    connect(&pointSizeAction, &ScalarAction::sourceSelectionChanged, this, [this, &pointSizeAction, scatterplotPlugin](const uint32_t& sourceSelectionIndex) -> void {
        if (_pointSizeDataset.isValid())
            disconnect(&_pointSizeDataset, &Dataset<>::guiNameChanged, this, nullptr);

        _pointSizeDataset = pointSizeAction.getCurrentDataset();

        connect(&_pointSizeDataset, &Dataset<>::guiNameChanged, scatterplotPlugin, &ScatterplotPlugin::updateHeadsUpDisplay);

        _pointSizeDatasetPickerAction.setCurrentDataset(pointSizeAction.isSourceDataset() ? pointSizeAction.getCurrentDataset() : nullptr);

        if (!pointSizeAction.isSourceDataset())
            _pointSizeDatasetPickerAction.setCurrentIndex(-1);
	});

    connect(&_pointSizeDatasetPickerAction, &DatasetPickerAction::currentIndexChanged, this, [this, &pointSizeAction](const int32_t& currentIndex) -> void {
        pointSizeAction.setCurrentDataset(_pointSizeDatasetPickerAction.getCurrentDataset());

        if (currentIndex < 0)
            pointSizeAction.setCurrentSourceIndex(ScalarSourceModel::DefaultRow::Constant);
    });
}

void DatasetsAction::setupPointOpacityDatasetPickerAction(ScatterplotPlugin* scatterplotPlugin)
{
    auto& settingsAction        = scatterplotPlugin->getSettingsAction();
    auto& pointPlotAction       = settingsAction.getPlotAction().getPointPlotAction();
    auto& pointOpacityAction    = pointPlotAction.getOpacityAction();

    _pointOpacityDatasetPickerAction.setFilterFunction([this, scatterplotPlugin](mv::Dataset<DatasetImpl> dataset) -> bool {
        if (dataset->getDataType() != PointType)
            return false;
        
        const auto positionDataset = scatterplotPlugin->getPositionSourceDataset();

        if (!positionDataset.isValid())
            return false;

        const mv::Dataset<Points> candidatePoints(dataset);

        if (candidatePoints->getNumPoints() != positionDataset->getNumPoints())
            return false;

        return true;
    });

    connect(&_pointOpacityDatasetPickerAction, &DatasetPickerAction::currentIndexChanged, this, [this, &pointPlotAction, &pointOpacityAction, scatterplotPlugin](const int32_t& currentIndex) -> void {
        const auto& pointOpacityDataset = _pointOpacityDatasetPickerAction.getCurrentDataset();

        if (pointOpacityDataset.isValid())
            disconnect(&_pointOpacityDataset, &Dataset<>::guiNameChanged, this, nullptr);

        _pointOpacityDataset = pointOpacityDataset;

        connect(&_pointOpacityDataset, &Dataset<>::guiNameChanged, scatterplotPlugin, &ScatterplotPlugin::updateHeadsUpDisplay);

        pointPlotAction.setCurrentPointOpacityDataset(_pointOpacityDataset);

        if (!_pointOpacityDataset.isValid())
            pointOpacityAction.setCurrentSourceIndex(ScalarSourceModel::DefaultRow::Constant);
    });

    connect(&pointOpacityAction, &ScalarAction::sourceSelectionChanged, this, [this, &pointOpacityAction](const uint32_t& sourceSelectionIndex) -> void {
        _pointOpacityDatasetPickerAction.setCurrentDataset(pointOpacityAction.isSourceDataset() ? pointOpacityAction.getCurrentDataset() : nullptr);

    	if (!pointOpacityAction.isSourceDataset())
            _pointOpacityDatasetPickerAction.setCurrentIndex(-1);
    });
}
