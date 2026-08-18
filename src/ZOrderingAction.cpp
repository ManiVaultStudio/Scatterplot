#include "ZOrderingAction.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <QMenu>

#include <algorithm>
#include <cmath>
#include <limits>

ZOrderingAction::ZOrderingAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _modeAction(this, "Mode", { "Insertion order", "Dimension", "Randomized" }),
    _dimensionPickerAction(this, "Dimension"),
    _selectionThresholdEnabledAction(this, "Restrict selection by Z order", false),
    _selectionThresholdAction(this, "Minimum selectable value", 0.0f, 1.0f, 0.0f, 3)
{
    setIconByName("layer-group");
    setLabelSizingType(LabelSizingType::Auto);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);

    addAction(&_modeAction, OptionAction::HorizontalButtons);
    addAction(&_dimensionPickerAction);
    addAction(&_selectionThresholdEnabledAction);
    addAction(&_selectionThresholdAction);

    _modeAction.setToolTip("Choose how overlapping points are ordered");
    _dimensionPickerAction.setToolTip("Dimension whose numerical values determine point depth");
    _selectionThresholdEnabledAction.setToolTip("Prevent points below a minimum Z-order value from being selected in this scatterplot");
    _selectionThresholdAction.setToolTip("Points below this dimension value are excluded from selection");
    _dimensionPickerAction.setEnabled(false);
    _selectionThresholdEnabledAction.setEnabled(false);
    _selectionThresholdAction.setEnabled(false);
}

void ZOrderingAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    const auto updateDataset = [this]() {
        auto& positionDataset = _scatterplotPlugin->getPositionDataset();

        if (!positionDataset.isValid()) {
            _dimensionPickerAction.setPointsDataset(Dataset<Points>());
            _zOrderScalars.clear();
            updateScatterplotWidget();
            return;
        }

        auto dimensionIndex = static_cast<std::int32_t>(_dimensionPickerAction.getCurrentDimensionIndex());
        const auto numberOfDimensions = static_cast<std::int32_t>(positionDataset->getNumDimensions());

        if (dimensionIndex < 0 || dimensionIndex >= numberOfDimensions)
            dimensionIndex = 0;

        _dimensionPickerAction.setPointsDataset(positionDataset);
        _dimensionPickerAction.setCurrentDimensionIndex(dimensionIndex);
        updateZOrderScalars(true);
        updateScatterplotWidget();
    };

    connect(&_modeAction, &OptionAction::currentIndexChanged, this, [this]() {
        updateScatterplotWidget();
    });

    connect(&_dimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, this, [this]() {
        updateZOrderScalars(true);
        updateScatterplotWidget();
    });

    connect(&_selectionThresholdEnabledAction, &ToggleAction::toggled, this, [this]() {
        updateSelectionExclusions();
    });

    connect(&_selectionThresholdAction, &DecimalAction::valueChanged, this, [this]() {
        updateSelectionExclusions();
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateDataset);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataDimensionsChanged, this, updateDataset);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChanged, this, [this]() {
        updateZOrderScalars(false);
        updateScatterplotWidget();
    });

    _modeAction.setCurrentIndex(static_cast<std::int32_t>(Mode::InsertionOrder));
    updateDataset();
}

void ZOrderingAction::updateScatterplotWidget()
{
    if (_scatterplotPlugin == nullptr)
        return;

    const auto mode = static_cast<Mode>(_modeAction.getCurrentIndex());
    const auto hasDataset = _scatterplotPlugin->getPositionDataset().isValid();

    setEnabled(hasDataset);
	
    _dimensionPickerAction.setEnabled(hasDataset && mode == Mode::Dimension);
    
	switch (mode) {
        case Mode::InsertionOrder:
            _scatterplotPlugin->getScatterplotWidget().setZOrderMode(PointZOrderMode::InsertionOrder);
            break;

        case Mode::Dimension:
            _scatterplotPlugin->getScatterplotWidget().setZOrderMode(PointZOrderMode::Dimension);
            break;

        case Mode::Randomized:
            _scatterplotPlugin->getScatterplotWidget().setZOrderMode(PointZOrderMode::Randomized);
            break;
    }

    if (mode == Mode::Dimension && hasDataset)
        updateZOrderScalars(false);

    updateSelectionExclusions();
}

void ZOrderingAction::updateZOrderScalars(bool resetThreshold)
{
    _zOrderScalars.clear();

    if (_scatterplotPlugin == nullptr)
        return;

    auto& positionDataset     = _scatterplotPlugin->getPositionDataset();
    const auto dimensionIndex = static_cast<std::int32_t>(_dimensionPickerAction.getCurrentDimensionIndex());

    if (positionDataset.isValid() && dimensionIndex >= 0 && dimensionIndex < static_cast<std::int32_t>(positionDataset->getNumDimensions()))
        positionDataset->extractDataForDimension(_zOrderScalars, dimensionIndex);

    _scatterplotPlugin->getScatterplotWidget().setZOrderScalars(_zOrderScalars);

    auto minimum = std::numeric_limits<float>::max();
    auto maximum = std::numeric_limits<float>::lowest();

    for (const auto value : _zOrderScalars) {
        if (!std::isfinite(value))
            continue;

        minimum = std::min(minimum, value);
        maximum = std::max(maximum, value);
    }

    if (minimum > maximum) {
        minimum = 0.0f;
        maximum = 1.0f;
    }

    const auto previousThreshold = _selectionThresholdAction.getValue();

    // NumericalAction clamps each endpoint against the other, so change the
    // endpoint that expands the range first.
    if (minimum > _selectionThresholdAction.getMaximum()) {
        _selectionThresholdAction.setMaximum(maximum);
        _selectionThresholdAction.setMinimum(minimum);
    }
    else {
        _selectionThresholdAction.setMinimum(minimum);
        _selectionThresholdAction.setMaximum(maximum);
    }

    _selectionThresholdAction.setValue(resetThreshold ? minimum : std::clamp(previousThreshold, minimum, maximum));
    updateSelectionExclusions();
}

void ZOrderingAction::updateSelectionExclusions()
{
    if (_scatterplotPlugin == nullptr)
        return;

    const auto mode = static_cast<Mode>(_modeAction.getCurrentIndex());
    const auto restrictionAvailable = mode == Mode::Dimension &&
        _scatterplotPlugin->getPositionDataset().isValid() &&
        !_zOrderScalars.empty();

    _selectionThresholdEnabledAction.setEnabled(restrictionAvailable);
    _selectionThresholdAction.setEnabled(restrictionAvailable && _selectionThresholdEnabledAction.isChecked());

    auto& scatterplotWidget = _scatterplotPlugin->getScatterplotWidget();
    auto& selectAllAction   = dynamic_cast<SettingsAction*>(parent())->getSelectionAction().getPixelSelectionAction().getSelectAllAction();
    
    const auto restrictionActive = restrictionAvailable && _selectionThresholdEnabledAction.isChecked();

    if (!restrictionActive) {
        selectAllAction.setText("Select all");
        selectAllAction.setToolTip("Select all points");
        scatterplotWidget.clearSelectionExcludedIndices();
        _scatterplotPlugin->refreshSelection();
        return;
    }

    std::vector<std::uint32_t> excludedIndices;
    excludedIndices.reserve(_zOrderScalars.size());

    const auto threshold = _selectionThresholdAction.getValue();

    for (std::uint32_t index = 0; index < _zOrderScalars.size(); ++index) {
        const auto value = _zOrderScalars[index];

        if (!std::isfinite(value) || value < threshold)
            excludedIndices.push_back(index);
    }

    scatterplotWidget.setSelectionExcludedIndices(excludedIndices);
    selectAllAction.setText("Select all selectable points");
    selectAllAction.setToolTip(QString("Select all points that meet the Z-order threshold (%1 excluded)").arg(excludedIndices.size()));
    _scatterplotPlugin->refreshSelection();
}

QMenu* ZOrderingAction::getContextMenu(QWidget* parent)
{
    auto menu = new QMenu("Z ordering", parent);

    menu->addAction(&_modeAction);
    menu->addAction(&_dimensionPickerAction);
    menu->addSeparator();
    menu->addAction(&_selectionThresholdEnabledAction);
    menu->addAction(&_selectionThresholdAction);

    return menu;
}

void ZOrderingAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicZOrderingAction = dynamic_cast<ZOrderingAction*>(publicAction);

    Q_ASSERT(publicZOrderingAction != nullptr);

    if (publicZOrderingAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_modeAction, &publicZOrderingAction->getModeAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_dimensionPickerAction, &publicZOrderingAction->getDimensionPickerAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_selectionThresholdEnabledAction, &publicZOrderingAction->getSelectionThresholdEnabledAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_selectionThresholdAction, &publicZOrderingAction->getSelectionThresholdAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void ZOrderingAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_modeAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_dimensionPickerAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_selectionThresholdEnabledAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_selectionThresholdAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void ZOrderingAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _modeAction.fromParentVariantMap(variantMap);
    _dimensionPickerAction.fromParentVariantMap(variantMap);
    _selectionThresholdEnabledAction.fromParentVariantMap(variantMap, true);
    _selectionThresholdAction.fromParentVariantMap(variantMap, true);
    updateScatterplotWidget();
}

QVariantMap ZOrderingAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _modeAction.insertIntoVariantMap(variantMap);
    _dimensionPickerAction.insertIntoVariantMap(variantMap);
    _selectionThresholdEnabledAction.insertIntoVariantMap(variantMap);
    _selectionThresholdAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
