#include "ColoringAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "DataHierarchyItem.h"

#include <PointData/PointData.h>
#include <ClusterData/ClusterData.h>

using namespace mv::gui;

const QColor ColoringAction::DEFAULT_CONSTANT_COLOR = qRgb(93, 93, 225);

ColoringAction::ColoringAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _colorByModel(this),
    _colorByAction(this, "Color by"),
    _constantColorAction(this, "Constant color", DEFAULT_CONSTANT_COLOR),
    _colorSpaceAction(this, "Color space", { "Scalar (1D)", "Duo (2D)", "RGB" }, "Scalar (1D)"),
    _dimensionAction(this, "Dimension"),
    _dimensionAction2(this, "Dimension 2"),
    _dimensionAction3(this, "Dimension 3"),
    _colorMap1DAction(this, "1D Color map"),
    _colorMap2DAction(this, "2D Color map")
{
    setIconByName("palette");
    setLabelSizingType(LabelSizingType::Auto);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);

    addAction(&_colorByAction);
    addAction(&_constantColorAction);
    addAction(&_colorSpaceAction);
    addAction(&_colorMap2DAction);
    addAction(&_colorMap1DAction);
    addAction(&_dimensionAction);
    addAction(&_dimensionAction2);
    addAction(&_dimensionAction3);

    _colorSpaceAction.setToolTip("Color space for data-driven coloring");

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

        _constantColorAction.setEnabled(currentIndex == 0);

        const auto currentColorDataset = getCurrentColorDataset();

        if (_currentColorPointsDataset.isValid()) {
            disconnect(&_currentColorPointsDataset, &Dataset<Points>::dataDimensionsChanged, this, nullptr);
        }

        _currentColorPointsDataset = Dataset<Points>();

        if (currentColorDataset.isValid()) {
            if (currentColorDataset->getDataType() == PointType) {
                _currentColorPointsDataset = currentColorDataset.get<Points>();

                if (_currentColorPointsDataset.isValid()) {
                    connect(&_currentColorPointsDataset, &Dataset<Points>::dataDimensionsChanged, this, [this]() {
                        if (_currentColorPointsDataset.isValid()) {
                            _dimensionAction.setPointsDataset(_currentColorPointsDataset);
                            _dimensionAction2.setPointsDataset(_currentColorPointsDataset);
                            _dimensionAction3.setPointsDataset(_currentColorPointsDataset);
                            updateScatterPlotWidgetColors();
                        }
					});

                    _dimensionAction.setPointsDataset(_currentColorPointsDataset);
                    _dimensionAction2.setPointsDataset(_currentColorPointsDataset);
                    _dimensionAction3.setPointsDataset(_currentColorPointsDataset);

                    // Auto-select the color space for datasets with exactly two or three channels
                    if (!_restoringState) {
                        const auto numDimensions = static_cast<std::int32_t>(_currentColorPointsDataset->getNumDimensions());

                        if (numDimensions == 2) {
                            _colorSpaceAction.setCurrentIndex(1);   // Duo (2D)
                            applyDefaultChannels();                 // also apply defaults when the index was already Duo
                        }
                        else if (numDimensions == 3) {
                            _colorSpaceAction.setCurrentIndex(2);   // RGB
                            applyDefaultChannels();                 // also apply defaults when the index was already RGB
                        }
                    }
                }
                else {
                    _dimensionAction.setPointsDataset(Dataset<Points>());
                    _dimensionAction2.setPointsDataset(Dataset<Points>());
                    _dimensionAction3.setPointsDataset(Dataset<Points>());
                }
            }
            else {
                _dimensionAction.setPointsDataset(Dataset<Points>());
                _dimensionAction2.setPointsDataset(Dataset<Points>());
                _dimensionAction3.setPointsDataset(Dataset<Points>());
            }

            emit currentColorDatasetChanged(currentColorDataset);
        }
        else {
            _dimensionAction.setPointsDataset(Dataset<Points>());
            _dimensionAction2.setPointsDataset(Dataset<Points>());
            _dimensionAction3.setPointsDataset(Dataset<Points>());
        }

        updateChannelActionsReadOnly();
        updateScatterPlotWidgetColors();
        updateScatterplotWidgetColorMap();
        updateColorMapActionScalarRange();
        updateColorMapActionsReadOnly();
    });

    connect(&_colorByModel, &ColorSourceModel::dataChanged, this, [this](const Dataset<DatasetImpl>& dataset) {
        const auto currentColorDataset = getCurrentColorDataset();

        if (!(currentColorDataset.isValid() && dataset.isValid()))
            return;

        if (currentColorDataset == dataset)
            updateScatterPlotWidgetColors();
        });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::childAdded, this, &ColoringAction::updateColorByActionOptions);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::childRemoved, this, &ColoringAction::updateColorByActionOptions);

    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, [this](const ScatterplotWidget::ColoringMode& coloringMode) {
        updateScatterPlotWidgetColors();
        updateColorMapActionsReadOnly();
        });

    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this](const ScatterplotWidget::RenderMode& renderMode) {
        updateScatterPlotWidgetColors();
        updateColorMapActionsReadOnly();
        });

    connect(&_dimensionAction, &DimensionPickerAction::currentDimensionIndexChanged, this, [this](const int32_t& currentDimensionIndex) {
        updateScatterPlotWidgetColors();
        updateColorMapActionsReadOnly();
        updateColorMapActionScalarRange();
        });

    connect(&_dimensionAction2, &DimensionPickerAction::currentDimensionIndexChanged, this, [this](const int32_t& currentDimensionIndex) {
        updateScatterPlotWidgetColors();
        });

    connect(&_dimensionAction3, &DimensionPickerAction::currentDimensionIndexChanged, this, [this](const int32_t& currentDimensionIndex) {
        updateScatterPlotWidgetColors();
        });

    connect(&_colorSpaceAction, &OptionAction::currentIndexChanged, this, [this](const std::int32_t& currentIndex) {
        if (!_restoringState)
            applyDefaultChannels();

        updateChannelActionsReadOnly();
        updateScatterPlotWidgetColors();
        updateScatterplotWidgetColorMap();
        updateColorMapActionsReadOnly();
        });

    connect(&_constantColorAction, &ColorAction::colorChanged, this, &ColoringAction::updateScatterplotWidgetColorMap);
    connect(&_colorMap1DAction, &ColorMapAction::imageChanged, this, &ColoringAction::updateScatterplotWidgetColorMap);
    connect(&_colorMap2DAction, &ColorMapAction::imageChanged, this, &ColoringAction::updateScatterplotWidgetColorMap);

    connect(&_colorMap1DAction.getRangeAction(ColorMapAction::Axis::X), &DecimalRangeAction::rangeChanged, this, &ColoringAction::updateScatterPlotWidgetColorMapRange);
    connect(&_colorMap2DAction.getRangeAction(ColorMapAction::Axis::X), &DecimalRangeAction::rangeChanged, this, &ColoringAction::updateScatterPlotWidgetColorMapRange);

    const auto updateReadOnly = [this]() {
        setEnabled(_scatterplotPlugin->getPositionDataset().isValid() && _scatterplotPlugin->getScatterplotWidget().getRenderMode() == ScatterplotWidget::SCATTERPLOT);
    };

    updateReadOnly();

    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, updateReadOnly);

    updateScatterplotWidgetColorMap();
    updateColorMapActionScalarRange();
    updateChannelActionsReadOnly();

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
    if (!colorDataset.isValid())
        return;

    if (hasColorDataset(colorDataset))
        return;

    _colorByModel.addDataset(colorDataset);
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
    if (!colorDataset.isValid())
        return;

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
        const auto dimension1 = _dimensionAction.getCurrentDimensionIndex();

        if (dimension1 < 0)
            return;

        switch (_colorSpaceAction.getCurrentIndex())
        {
            case 1: // Duo (2D)
            {
                const auto dimension2 = _dimensionAction2.getCurrentDimensionIndex();

                if (dimension2 >= 0)
                    _scatterplotPlugin->loadColors2D(currentColorDataset.get<Points>(), dimension1, dimension2);

                break;
            }

            case 2: // RGB
            {
                const auto dimension2 = _dimensionAction2.getCurrentDimensionIndex();
                const auto dimension3 = _dimensionAction3.getCurrentDimensionIndex();

                if (dimension2 >= 0 && dimension3 >= 0)
                    _scatterplotPlugin->loadColorsRGB(currentColorDataset.get<Points>(), dimension1, dimension2, dimension3);

                break;
            }

            default: // Scalar (1D)
                _scatterplotPlugin->loadColors(currentColorDataset.get<Points>(), dimension1);
                break;
        }
    }

    updateScatterplotWidgetColorMap();
}

void ColoringAction::updateColorMapActionScalarRange()
{
    const auto colorMapRange    = _scatterplotPlugin->getScatterplotWidget().getColorMapRange();
    const auto colorMapRangeMin = colorMapRange.x;
    const auto colorMapRangeMax = colorMapRange.y;

    auto& colorMapRangeAction = _colorMap1DAction.getRangeAction(ColorMapAction::Axis::X);

    colorMapRangeAction.initialize({ colorMapRangeMin, colorMapRangeMax }, { colorMapRangeMin, colorMapRangeMax });
	
	_colorMap1DAction.getDataRangeAction(ColorMapAction::Axis::X).setRange({ colorMapRangeMin, colorMapRangeMax });
}

void ColoringAction::updateScatterplotWidgetColorMap()
{
    auto& scatterplotWidget = _scatterplotPlugin->getScatterplotWidget();

    switch (scatterplotWidget.getRenderMode())
    {
        case ScatterplotWidget::SCATTERPLOT:
        {
            const int32_t currentIndex = _colorByAction.getCurrentIndex();
            if (currentIndex == 0) {
                QPixmap colorPixmap(1, 1);

                colorPixmap.fill(_constantColorAction.getColor());

                scatterplotWidget.setColorMap(colorPixmap.toImage());
                scatterplotWidget.setScalarEffect(PointEffect::Color);
                scatterplotWidget.setColoringMode(ScatterplotWidget::ColoringMode::Constant);
            }
            else if (currentIndex == 1) {
                scatterplotWidget.setColorMap(_colorMap2DAction.getColorMapImage());
                scatterplotWidget.setScalarEffect(PointEffect::Color2D);
                scatterplotWidget.setColoringMode(ScatterplotWidget::ColoringMode::Scatter);
            }
            else {
                const auto currentColorDataset = getCurrentColorDataset();
                const bool isDuo = currentColorDataset.isValid() && currentColorDataset->getDataType() == PointType && _colorSpaceAction.getCurrentIndex() == 1;

                if (isDuo)
                    scatterplotWidget.setColorMap(_colorMap2DAction.getColorMapImage().mirrored(false, true));
                else
                    scatterplotWidget.setColorMap(_colorMap1DAction.getColorMapImage().mirrored(false, true));
            }

            break;
        }

        case ScatterplotWidget::DENSITY:
            break;

        case ScatterplotWidget::LANDSCAPE:
        {
            scatterplotWidget.setScalarEffect(PointEffect::Color);
            scatterplotWidget.setColoringMode(ScatterplotWidget::ColoringMode::Scatter);
            scatterplotWidget.setColorMap(_colorMap1DAction.getColorMapImage());
            
            break;
        }

        default:
            break;
    }

    updateScatterPlotWidgetColorMapRange();
}

void ColoringAction::updateScatterPlotWidgetColorMapRange()
{
    auto& scatterplotWidget = _scatterplotPlugin->getScatterplotWidget();

    // The adjustable 1D color-map range only drives the (channel 1) scalar range for 1D scalar coloring.
    // In Duo/RGB the color channels each use their own automatically-computed range, so leave channel 1
    // untouched here (otherwise identical channels would normalize differently and produce a color tint).
    if (scatterplotWidget.getRenderMode() == ScatterplotWidget::SCATTERPLOT) {
        const auto currentColorDataset = getCurrentColorDataset();
        const bool isPointsSource = currentColorDataset.isValid() && currentColorDataset->getDataType() == PointType;

        if (isPointsSource && _colorSpaceAction.getCurrentIndex() != 0)   // Duo (1) or RGB (2)
            return;
    }

    const auto& rangeAction = _colorMap1DAction.getRangeAction(ColorMapAction::Axis::X);

    scatterplotWidget.setColorMapRange(rangeAction.getMinimum(), rangeAction.getMaximum());
}

bool ColoringAction::shouldEnableColorMap() const
{
    if (!_scatterplotPlugin->getPositionDataset().isValid())
        return false;

    const auto currentColorDataset = getCurrentColorDataset();

    if (currentColorDataset.isValid() && currentColorDataset->getDataType() == ClusterType)
        return false;

    if (_scatterplotPlugin->getScatterplotWidget().getRenderMode() == ScatterplotWidget::LANDSCAPE)
        return true;

    if (_scatterplotPlugin->getScatterplotWidget().getRenderMode() == ScatterplotWidget::SCATTERPLOT && _colorByAction.getCurrentIndex() > 0)
        return true;

    return false;
}

void ColoringAction::updateColorMapActionsReadOnly()
{
    const auto currentIndex = _colorByAction.getCurrentIndex();
    const bool isPointsSource = currentIndex >= 2 && _currentColorPointsDataset.isValid();
    const bool isDuo = isPointsSource && _colorSpaceAction.getCurrentIndex() == 1;
    const bool isRGB = isPointsSource && _colorSpaceAction.getCurrentIndex() == 2;

    _colorMap1DAction.setEnabled(shouldEnableColorMap() && (currentIndex >= 2) && !isDuo && !isRGB);
    _colorMap2DAction.setEnabled(shouldEnableColorMap() && (currentIndex == 1 || isDuo));
}

void ColoringAction::updateChannelActionsReadOnly()
{
    const auto currentIndex = _colorByAction.getCurrentIndex();
    const auto colorSpace   = _colorSpaceAction.getCurrentIndex();

    const bool isPointsSource = currentIndex >= 2 && _currentColorPointsDataset.isValid();

    const bool isDuo = isPointsSource && colorSpace == 1;   // Duo (2D)
    const bool isRGB = isPointsSource && colorSpace == 2;   // RGB

    // All actions remain visible; only their enabled state reflects the current coloring mode.

    // Constant color: only usable in constant mode
    _constantColorAction.setEnabled(currentIndex == 0);

    // Color space selector: only usable for a points color source
    _colorSpaceAction.setEnabled(isPointsSource);

    // Dimension pickers: channel 1 for any points source, channel 2 for Duo/RGB, channel 3 for RGB only
    _dimensionAction.setEnabled(isPointsSource);
    _dimensionAction2.setEnabled(isDuo || isRGB);
    _dimensionAction3.setEnabled(isRGB);

    // Dimension picker labels reflect their role in the current color space
    if (isRGB) {
        _dimensionAction.setText("Red");
        _dimensionAction2.setText("Green");
        _dimensionAction3.setText("Blue");
    }
    else if (isDuo) {
        _dimensionAction.setText("Dimension 1 (x)");
        _dimensionAction2.setText("Dimension 2 (y)");
        _dimensionAction3.setText("Dimension 3");
    }
    else {
        _dimensionAction.setText("Dimension");
        _dimensionAction2.setText("Dimension 2");
        _dimensionAction3.setText("Dimension 3");
    }
}

void ColoringAction::applyDefaultChannels()
{
    if (!_currentColorPointsDataset.isValid())
        return;

    const auto numDimensions = static_cast<std::int32_t>(_currentColorPointsDataset->getNumDimensions());

    switch (_colorSpaceAction.getCurrentIndex())
    {
        case 1: // Duo (2D): default to the first two channels
        {
            if (numDimensions >= 2) {
                _dimensionAction.setCurrentDimensionIndex(0);
                _dimensionAction2.setCurrentDimensionIndex(1);
            }

            break;
        }

        case 2: // RGB: default to the first three channels
        {
            if (numDimensions >= 3) {
                _dimensionAction.setCurrentDimensionIndex(0);
                _dimensionAction2.setCurrentDimensionIndex(1);
                _dimensionAction3.setCurrentDimensionIndex(2);
            }

            break;
        }

        default:
            break;
    }
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
        actions().connectPrivateActionToPublicAction(&_colorSpaceAction, &publicColoringAction->getColorSpaceAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_dimensionAction, &publicColoringAction->getDimensionAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_dimensionAction2, &publicColoringAction->getDimensionAction2(), recursive);
        actions().connectPrivateActionToPublicAction(&_dimensionAction3, &publicColoringAction->getDimensionAction3(), recursive);
        actions().connectPrivateActionToPublicAction(&_colorMap1DAction, &publicColoringAction->getColorMap1DAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_colorMap2DAction, &publicColoringAction->getColorMap2DAction(), recursive);
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
        actions().disconnectPrivateActionFromPublicAction(&_colorSpaceAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_dimensionAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_dimensionAction2, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_dimensionAction3, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_colorMap2DAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void ColoringAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _colorByAction.fromParentVariantMap(variantMap);
    _constantColorAction.fromParentVariantMap(variantMap);
    _dimensionAction.fromParentVariantMap(variantMap);
    _colorMap1DAction.fromParentVariantMap(variantMap);
    _colorMap2DAction.fromParentVariantMap(variantMap);
}

QVariantMap ColoringAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _colorByAction.insertIntoVariantMap(variantMap);
    _constantColorAction.insertIntoVariantMap(variantMap);
    _dimensionAction.insertIntoVariantMap(variantMap);
    _colorMap1DAction.insertIntoVariantMap(variantMap);
    _colorMap2DAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
