#include "ColoringAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "DataHierarchyItem.h"

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>

using namespace hdps::gui;

const QColor ColoringAction::DEFAULT_CONSTANT_COLOR = qRgb(93, 93, 225);

ColoringAction::ColoringAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _colorByModel(this),
    _colorByAction(this, "Color by"),
    _constantColorAction(this, "Constant color", DEFAULT_CONSTANT_COLOR, DEFAULT_CONSTANT_COLOR),
    _dimensionAction(this, "Dim"),
    _colorMapAction(this, "Color map")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("palette"));
    setDefaultWidgetFlags(GroupAction::Horizontal);
    setShowLabels(false);

    addAction(&_colorByAction);
    addAction(&_constantColorAction);
    addAction(&_dimensionAction);
    addAction(&_colorMapAction);

    _scatterplotPlugin->getWidget().addAction(&_colorByAction);
    _scatterplotPlugin->getWidget().addAction(&_dimensionAction);

    _colorByAction.setCustomModel(&_colorByModel);
    _colorByAction.setToolTip("Color by");

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this]() {
        const auto positionDataset = _scatterplotPlugin->getPositionDataset();

        if (!positionDataset.isValid())
            return;

        _colorByModel.removeAllDatasets();

        addColorDataset(positionDataset);

        const auto positionSourceDataset = _scatterplotPlugin->getPositionSourceDataset();

        if (positionSourceDataset.isValid())
            addColorDataset(positionSourceDataset);

        updateColorByActionOptions();

        _colorByAction.setCurrentIndex(0);
    });

    connect(&_scatterplotPlugin->getPositionSourceDataset(), &Dataset<Points>::changed, this, [this]() {
        const auto positionSourceDataset = _scatterplotPlugin->getPositionSourceDataset();

        if (positionSourceDataset.isValid())
            addColorDataset(positionSourceDataset);

        updateColorByActionOptions();
    });

    connect(&_colorByAction, &OptionAction::currentIndexChanged, this, [this](const std::int32_t& currentIndex) {
        _scatterplotPlugin->getScatterplotWidget().setColoringMode(currentIndex == 0 ? ScatterplotWidget::ColoringMode::Constant : ScatterplotWidget::ColoringMode::Data);

        _constantColorAction.setEnabled(_colorByAction.getCurrentIndex() == 0);
        _colorMapAction.setEnabled(_colorByAction.getCurrentIndex() >= 1);

        if (_colorByAction.getCurrentIndex() == 1)
            _colorMapAction.setColorMapType(ColorMap::Type::TwoDimensional);

        if (_colorByAction.getCurrentIndex() == 2)
            _colorMapAction.setColorMapType(ColorMap::Type::OneDimensional);

        const auto currentColorDataset = getCurrentColorDataset();

        if (currentColorDataset.isValid()) {
            const auto currentColorDatasetTypeIsPointType = currentColorDataset->getDataType() == PointType;

            _dimensionAction.setPointsDataset(currentColorDatasetTypeIsPointType ? Dataset<Points>(currentColorDataset) : Dataset<Points>());
            //_dimensionAction.setVisible(currentColorDatasetTypeIsPointType);

            emit currentColorDatasetChanged(currentColorDataset);
        }
        else {
            _dimensionAction.setPointsDataset(Dataset<Points>());
            //_dimensionAction.setVisible(false);
        }

        updateScatterPlotWidgetColors();
        updateScatterplotWidgetColorMap();
        updateColorMapActionScalarRange();
        updateColorMapActionReadOnly();
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildAdded, this, &ColoringAction::updateColorByActionOptions);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChildRemoved, this, &ColoringAction::updateColorByActionOptions);

    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, &ColoringAction::updateScatterPlotWidgetColors);
    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, &ColoringAction::updateScatterPlotWidgetColors);

    connect(&_dimensionAction, &DimensionPickerAction::currentDimensionIndexChanged, this, &ColoringAction::updateScatterPlotWidgetColors);
    connect(&_dimensionAction, &DimensionPickerAction::currentDimensionIndexChanged, this, &ColoringAction::updateColorMapActionScalarRange);
    
    connect(&_constantColorAction, &ColorAction::colorChanged, this, &ColoringAction::updateScatterplotWidgetColorMap);
    connect(&_colorMapAction, &ColorMapAction::imageChanged, this, &ColoringAction::updateScatterplotWidgetColorMap);
    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, &ColoringAction::updateScatterplotWidgetColorMap);
    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, &ColoringAction::updateScatterplotWidgetColorMap);

    connect(&_colorMapAction.getRangeAction(ColorMapAction::Axis::X), &DecimalRangeAction::rangeChanged, this, &ColoringAction::updateScatterPlotWidgetColorMapRange);

    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, &ColoringAction::updateColorMapActionReadOnly);
    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, &ColoringAction::updateColorMapActionReadOnly);

    const auto updateReadOnly = [this]() {
        setEnabled(_scatterplotPlugin->getScatterplotWidget().getRenderMode() == ScatterplotWidget::SCATTERPLOT);
    };

    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, updateReadOnly);

    updateReadOnly();

    updateScatterplotWidgetColorMap();
    updateColorMapActionScalarRange();

    _scatterplotPlugin->getScatterplotWidget().setColoringMode(ScatterplotWidget::ColoringMode::Constant);
}

QMenu* ColoringAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("Color", parent);

    const auto addActionToMenu = [menu](QAction* action) -> void {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    return menu;
}

void ColoringAction::addColorDataset(const Dataset<DatasetImpl>& colorDataset)
{
    if (hasColorDataset(colorDataset))
        return;

    _colorByModel.addDataset(colorDataset);

    auto& addedDataset = _colorByModel.getDatasets().last();

    for (const auto& dataset : _colorByModel.getDatasets()) {
        connect(&dataset, &Dataset<DatasetImpl>::dataChanged, this, [this, dataset]() {
            const auto currentColorDataset = getCurrentColorDataset();

            if (!currentColorDataset.isValid())
                return;

            if (currentColorDataset == dataset)
                updateScatterPlotWidgetColors();
        });
    }
}

bool ColoringAction::hasColorDataset(const Dataset<DatasetImpl>& colorDataset) const
{
    return _colorByModel.rowIndex(colorDataset) >= 0;
}

Dataset<DatasetImpl> ColoringAction::getCurrentColorDataset() const
{
    const auto colorByIndex = _colorByAction.getCurrentIndex();

    if (colorByIndex < 2)
        return Dataset<DatasetImpl>();

    return _colorByModel.getDataset(colorByIndex);
}

void ColoringAction::setCurrentColorDataset(const Dataset<DatasetImpl>& colorDataset)
{
    addColorDataset(colorDataset);

    const auto colorDatasetRowIndex = _colorByModel.rowIndex(colorDataset);

    if (colorDatasetRowIndex >= 0)
        _colorByAction.setCurrentIndex(colorDatasetRowIndex);

    emit currentColorDatasetChanged(colorDataset);
}

void ColoringAction::updateColorByActionOptions()
{
    auto positionDataset = _scatterplotPlugin->getPositionDataset();

    if (!positionDataset.isValid())
        return;

    const auto children = positionDataset->getDataHierarchyItem().getChildren();

    for (auto child : children) {
        const auto childDataset = child->getDataset();
        const auto dataType     = childDataset->getDataType();

        if (dataType == PointType || dataType == ClusterType)
            addColorDataset(childDataset);
    }
}

void ColoringAction::updateScatterPlotWidgetColors()
{
    if (_colorByAction.getCurrentIndex() <= 1)
        return;

    const auto currentColorDataset = getCurrentColorDataset();

    if (!currentColorDataset.isValid())
        return;

    if (currentColorDataset->getDataType() == ClusterType)
        _scatterplotPlugin->loadColors(currentColorDataset.get<Clusters>());
    else {
        const auto currentDimensionIndex = _dimensionAction.getCurrentDimensionIndex();

        if (currentDimensionIndex >= 0)
            _scatterplotPlugin->loadColors(currentColorDataset.get<Points>(), _dimensionAction.getCurrentDimensionIndex());
    }

    updateScatterplotWidgetColorMap();
}

void ColoringAction::updateColorMapActionScalarRange()
{
    const auto colorMapRange    = _scatterplotPlugin->getScatterplotWidget().getColorMapRange();
    const auto colorMapRangeMin = colorMapRange.x;
    const auto colorMapRangeMax = colorMapRange.y;

    auto& colorMapRangeAction = _colorMapAction.getRangeAction(ColorMapAction::Axis::X);

    colorMapRangeAction.initialize({ colorMapRangeMin, colorMapRangeMax }, { colorMapRangeMin, colorMapRangeMax });
	
	_colorMapAction.getDataRangeAction(ColorMapAction::Axis::X).setRange({ colorMapRangeMin, colorMapRangeMax });
}

void ColoringAction::updateScatterplotWidgetColorMap()
{
    auto& scatterplotWidget = _scatterplotPlugin->getScatterplotWidget();

    switch (scatterplotWidget.getRenderMode())
    {
        case ScatterplotWidget::SCATTERPLOT:
        {
            if (_colorByAction.getCurrentIndex() == 0) {
                QPixmap colorPixmap(1, 1);

                colorPixmap.fill(_constantColorAction.getColor());

                scatterplotWidget.setColorMap(colorPixmap.toImage());
                scatterplotWidget.setScalarEffect(PointEffect::Color);
                scatterplotWidget.setColoringMode(ScatterplotWidget::ColoringMode::Constant);
            }
            else if (_colorByAction.getCurrentIndex() == 1) {
                scatterplotWidget.setColorMap(_colorMapAction.getColorMapImage());
                scatterplotWidget.setScalarEffect(PointEffect::Color2D);
                scatterplotWidget.setColoringMode(ScatterplotWidget::ColoringMode::Scatter);
            }
            else {
                scatterplotWidget.setColorMap(_colorMapAction.getColorMapImage().mirrored(false, true));
            }

            break;
        }

        case ScatterplotWidget::DENSITY:
            break;

        case ScatterplotWidget::LANDSCAPE:
        {
            scatterplotWidget.setScalarEffect(PointEffect::Color);
            scatterplotWidget.setColoringMode(ScatterplotWidget::ColoringMode::Scatter);
            scatterplotWidget.setColorMap(_colorMapAction.getColorMapImage());
            
            break;
        }

        default:
            break;
    }

    updateScatterPlotWidgetColorMapRange();
}

void ColoringAction::updateScatterPlotWidgetColorMapRange()
{
    const auto& rangeAction = _colorMapAction.getRangeAction(ColorMapAction::Axis::X);

    _scatterplotPlugin->getScatterplotWidget().setColorMapRange(rangeAction.getMinimum(), rangeAction.getMaximum());
}

bool ColoringAction::shouldEnableColorMap() const
{
    const auto currentColorDataset = getCurrentColorDataset();

    if (currentColorDataset.isValid() && currentColorDataset->getDataType() == ClusterType)
        return false;

    if (_scatterplotPlugin->getScatterplotWidget().getRenderMode() == ScatterplotWidget::LANDSCAPE)
        return true;

    if (_scatterplotPlugin->getScatterplotWidget().getRenderMode() == ScatterplotWidget::SCATTERPLOT && _colorByAction.getCurrentIndex() > 0)
        return true;

    return false;
}

void ColoringAction::updateColorMapActionReadOnly()
{
    _colorMapAction.setEnabled(shouldEnableColorMap());
}

void ColoringAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicColoringAction = dynamic_cast<ColoringAction*>(publicAction);

    Q_ASSERT(publicColoringAction != nullptr);

    if (publicColoringAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_colorByAction, &publicColoringAction->getColorByAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_constantColorAction, &publicColoringAction->getConstantColorAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_dimensionAction, &publicColoringAction->getDimensionAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_colorMapAction, &publicColoringAction->getColorMapAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void ColoringAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_colorByAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_constantColorAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_dimensionAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_colorMapAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void ColoringAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _colorByAction.fromParentVariantMap(variantMap);
    _constantColorAction.fromParentVariantMap(variantMap);
    _dimensionAction.fromParentVariantMap(variantMap);
    _colorMapAction.fromParentVariantMap(variantMap);
}

QVariantMap ColoringAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _colorByAction.insertIntoVariantMap(variantMap);
    _constantColorAction.insertIntoVariantMap(variantMap);
    _dimensionAction.insertIntoVariantMap(variantMap);
    _colorMapAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
