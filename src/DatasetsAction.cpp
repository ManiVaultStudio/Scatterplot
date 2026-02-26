#include "DatasetsAction.h"
#include "ScatterplotPlugin.h"

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>
#include <ColorData/ColorData.h>

#include <actions/LabelProxyAction.h>

using namespace mv;
using namespace mv::gui;

DatasetsAction::DatasetsAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _positionDatasetPickerAction(this, "Position"),
    _colorDatasetPickerAction(this, "Color")
{
    setIconByName("database");
    setToolTip("Manage loaded datasets for position and color");
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    setLabelSizingType(LabelSizingType::Auto);

    auto scatterplotPlugin = dynamic_cast<ScatterplotPlugin*>(parent->parent());

    Q_ASSERT(scatterplotPlugin);

    if (scatterplotPlugin == nullptr)
        return;

    auto& settingsAction    = scatterplotPlugin->getSettingsAction();
    auto& plotAction        = settingsAction.getPlotAction();
    auto& pointPlotAction   = plotAction.getPointPlotAction();

	addAction(&_positionDatasetPickerAction);
    addAction(&_colorDatasetPickerAction);
    addAction(new LabelProxyAction(this, "Size", const_cast<DatasetPickerAction*>(&pointPlotAction.getSizeAction().getSourceDatasetPickerAction())));
    addAction(new LabelProxyAction(this, "Opacity", const_cast<DatasetPickerAction*>(&pointPlotAction.getOpacityAction().getSourceDatasetPickerAction())));

    _positionDatasetPickerAction.setDefaultWidgetFlag(OptionAction::Clearable);
    _colorDatasetPickerAction.setDefaultWidgetFlag(OptionAction::Clearable);
    
    setupDatasetPickerActions(scatterplotPlugin);

    const auto invalidateFilters = [this, scatterplotPlugin]() -> void {
        _colorDatasetPickerAction.invalidateFilter();
    };

    connect(&_positionDatasetPickerAction, &DatasetPickerAction::datasetPicked, [this, scatterplotPlugin, invalidateFilters](Dataset<DatasetImpl> pickedDataset) -> void {
        invalidateFilters();
    });

    const auto resetAuxilliaryDatasets = [this, &pointPlotAction]() -> void {
        _colorDatasetPickerAction.setCurrentIndex(-1);

        pointPlotAction.getSizeAction().setCurrentSourceIndex(ScalarSourceModel::DefaultRow::Constant);
		pointPlotAction.getOpacityAction().setCurrentSourceIndex(ScalarSourceModel::DefaultRow::Constant);
	};

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, resetAuxilliaryDatasets);
    connect(&scatterplotPlugin->getPositionSourceDataset(), &Dataset<Points>::changed, this, resetAuxilliaryDatasets);
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

    _colorDatasetPickerAction.setFilterFunction([this, scatterplotPlugin](mv::Dataset<DatasetImpl> dataset) -> bool {
        if (!(dataset->getDataType() == PointType || dataset->getDataType() == ColorType || dataset->getDataType() == ClusterType))
            return false;

        const auto positionDataset = scatterplotPlugin->getPositionDataset();

        if (!positionDataset.isValid())
            return false;

        return true;
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

void DatasetsAction::invalidateDatasetPickerActionFilters()
{
    _positionDatasetPickerAction.invalidateFilter();
    _colorDatasetPickerAction.invalidateFilter();
}
