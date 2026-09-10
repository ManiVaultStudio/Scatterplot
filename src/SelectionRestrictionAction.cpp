#include "SelectionRestrictionAction.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "SelectionAction.h"
#include "SettingsAction.h"

#include <algorithm>
#include <cmath>
#include <limits>

SelectionRestrictionAction::SelectionRestrictionAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _enabledAction(this, "Restrict selection by dimension", false),
    _dimensionPickerAction(this, "Dimension"),
    _rangeAction(this, "Selectable range", util::NumericalRange<float>(0.0f, 1.0f), util::NumericalRange<float>(0.0f, 1.0f), 3)
{
    setIconByName("filter");
    setLabelSizingType(LabelSizingType::Auto);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);
    setShowLabels(false);

    addAction(&_enabledAction);
    addAction(&_dimensionPickerAction);
    addAction(&_rangeAction);

    _enabledAction.setToolTip("Only allow points within a dimension value range to be selected");
    _dimensionPickerAction.setToolTip("Dimension whose values determine whether points are selectable");
    _rangeAction.setToolTip("Inclusive range of dimension values for selectable points");
}

void SelectionRestrictionAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    connect(&_enabledAction, &ToggleAction::toggled, this, [this]() {
        updateActionsReadOnly();
        updateSelectionExclusions();
    });

    connect(&_dimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, this, [this]() {
        updateDimensionValues(true);
    });

    connect(&_rangeAction, &DecimalRangeAction::rangeChanged, this, [this]() {
        if (!_updatingRange)
            updateSelectionExclusions();
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, &SelectionRestrictionAction::updateDataset);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataDimensionsChanged, this, &SelectionRestrictionAction::updateDataset);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChanged, this, [this]() {
        updateDimensionValues(false);
    });

    updateDataset();
}

void SelectionRestrictionAction::updateDataset()
{
    if (_scatterplotPlugin == nullptr)
        return;

    auto& positionDataset = _scatterplotPlugin->getPositionDataset();

    if (!positionDataset.isValid()) {
        _dimensionPickerAction.setPointsDataset(Dataset<Points>());
        _dimensionValues.clear();
        updateActionsReadOnly();
        updateSelectionExclusions();
        return;
    }

    auto dimensionIndex = _dimensionPickerAction.getCurrentDimensionIndex();
    const auto numberOfDimensions = static_cast<std::int32_t>(positionDataset->getNumDimensions());

    if (dimensionIndex < 0 || dimensionIndex >= numberOfDimensions)
        dimensionIndex = 0;

    _dimensionPickerAction.setPointsDataset(positionDataset);
    _dimensionPickerAction.setCurrentDimensionIndex(dimensionIndex);
    updateDimensionValues(true);
}

void SelectionRestrictionAction::updateDimensionValues(bool resetRange)
{
    _dimensionValues.clear();

    if (_scatterplotPlugin == nullptr)
        return;

    auto& positionDataset     = _scatterplotPlugin->getPositionDataset();
    const auto dimensionIndex = _dimensionPickerAction.getCurrentDimensionIndex();

    if (positionDataset.isValid() && dimensionIndex >= 0 && dimensionIndex < static_cast<std::int32_t>(positionDataset->getNumDimensions()))
        positionDataset->extractDataForDimension(_dimensionValues, dimensionIndex);

    auto minimum = std::numeric_limits<float>::max();
    auto maximum = std::numeric_limits<float>::lowest();

    for (const auto value : _dimensionValues) {
        if (!std::isfinite(value))
            continue;

        minimum = std::min(minimum, value);
        maximum = std::max(maximum, value);
    }

    if (minimum > maximum) {
        minimum = 0.0f;
        maximum = 1.0f;
    }

    const auto previousMinimum = _rangeAction.getMinimum();
    const auto previousMaximum = _rangeAction.getMaximum();

    _updatingRange = true;

    if (minimum > _rangeAction.getLimitsMaximum()) {
        _rangeAction.setLimitsMaximum(maximum);
        _rangeAction.setLimitsMinimum(minimum);
    }
    else {
        _rangeAction.setLimitsMinimum(minimum);
        _rangeAction.setLimitsMaximum(maximum);
    }

    if (resetRange) {
        _rangeAction.setRange(util::NumericalRange<float>(minimum, maximum));
    }
    else {
        const auto rangeMinimum = std::clamp(previousMinimum, minimum, maximum);
        const auto rangeMaximum = std::clamp(previousMaximum, rangeMinimum, maximum);

        _rangeAction.setRange(util::NumericalRange<float>(rangeMinimum, rangeMaximum));
    }

    _updatingRange = false;

    updateActionsReadOnly();
    updateSelectionExclusions();
}

void SelectionRestrictionAction::updateActionsReadOnly()
{
    const auto restrictionAvailable = _scatterplotPlugin != nullptr &&
        _scatterplotPlugin->getPositionDataset().isValid() &&
        !_dimensionValues.empty();

    setEnabled(_scatterplotPlugin != nullptr && _scatterplotPlugin->getPositionDataset().isValid());
    _enabledAction.setEnabled(restrictionAvailable);
    _dimensionPickerAction.setEnabled(restrictionAvailable);
    _rangeAction.setEnabled(restrictionAvailable && _enabledAction.isChecked());
}

void SelectionRestrictionAction::updateSelectionExclusions()
{
    if (_scatterplotPlugin == nullptr)
        return;

    auto& scatterplotWidget = _scatterplotPlugin->getScatterplotWidget();
    auto& selectAllAction   = dynamic_cast<SelectionAction*>(parent())->getPixelSelectionAction().getSelectAllAction();
    const auto restrictionActive = _enabledAction.isEnabled() && _enabledAction.isChecked();

    if (!restrictionActive) {
        selectAllAction.setText("Select all");
        selectAllAction.setToolTip("Select all points");
        scatterplotWidget.clearSelectionExcludedIndices();
        _scatterplotPlugin->refreshSelection();
        return;
    }

    std::vector<std::uint32_t> excludedIndices;
    excludedIndices.reserve(_dimensionValues.size());

    const auto minimum = _rangeAction.getMinimum();
    const auto maximum = _rangeAction.getMaximum();

    for (std::uint32_t index = 0; index < _dimensionValues.size(); ++index) {
        const auto value = _dimensionValues[index];

        if (!std::isfinite(value) || value < minimum || value > maximum)
            excludedIndices.push_back(index);
    }

    scatterplotWidget.setSelectionExcludedIndices(excludedIndices);
    selectAllAction.setText("Select all selectable points");
    selectAllAction.setToolTip(QString("Select all points within the dimension range (%1 excluded)").arg(excludedIndices.size()));
    _scatterplotPlugin->refreshSelection();
}

void SelectionRestrictionAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicRestrictionAction = dynamic_cast<SelectionRestrictionAction*>(publicAction);

    Q_ASSERT(publicRestrictionAction != nullptr);

    if (publicRestrictionAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_enabledAction, &publicRestrictionAction->getEnabledAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_dimensionPickerAction, &publicRestrictionAction->getDimensionPickerAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_rangeAction, &publicRestrictionAction->getRangeAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void SelectionRestrictionAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_enabledAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_dimensionPickerAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_rangeAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void SelectionRestrictionAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _dimensionPickerAction.fromParentVariantMap(variantMap);
    _rangeAction.fromParentVariantMap(variantMap);
    _enabledAction.fromParentVariantMap(variantMap);
    updateSelectionExclusions();
}

QVariantMap SelectionRestrictionAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _enabledAction.insertIntoVariantMap(variantMap);
    _dimensionPickerAction.insertIntoVariantMap(variantMap);
    _rangeAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
