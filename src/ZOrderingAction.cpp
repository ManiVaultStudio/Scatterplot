#include "ZOrderingAction.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <QMenu>

ZOrderingAction::ZOrderingAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _modeAction(this, "Mode", { "Insertion order", "Dimension", "Randomized" }),
    _dimensionPickerAction(this, "Dimension")
{
    setIconByName("layer-group");
    setLabelSizingType(LabelSizingType::Auto);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);

    addAction(&_modeAction, OptionAction::HorizontalButtons);
    addAction(&_dimensionPickerAction);

    _modeAction.setToolTip("Choose how overlapping points are ordered");
    _dimensionPickerAction.setToolTip("Dimension whose numerical values determine point depth");
    _dimensionPickerAction.setEnabled(false);
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
            updateScatterplotWidget();
            return;
        }

        auto dimensionIndex = static_cast<std::int32_t>(_dimensionPickerAction.getCurrentDimensionIndex());
        const auto numberOfDimensions = static_cast<std::int32_t>(positionDataset->getNumDimensions());

        if (dimensionIndex < 0 || dimensionIndex >= numberOfDimensions)
            dimensionIndex = 0;

        _dimensionPickerAction.setPointsDataset(positionDataset);
        _dimensionPickerAction.setCurrentDimensionIndex(dimensionIndex);
        updateScatterplotWidget();
    };

    connect(&_modeAction, &OptionAction::currentIndexChanged, this, [this]() {
        updateScatterplotWidget();
    });

    connect(&_dimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, this, [this]() {
        updateScatterplotWidget();
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateDataset);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataDimensionsChanged, this, updateDataset);
    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataChanged, this, [this]() {
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
        _scatterplotPlugin->setZOrderDimension(_dimensionPickerAction.getCurrentDimensionIndex());
    else if (!hasDataset)
        _scatterplotPlugin->setZOrderDimension(-1);
}

QMenu* ZOrderingAction::getContextMenu(QWidget* parent)
{
    auto menu = new QMenu("Z ordering", parent);

    menu->addAction(&_modeAction);
    menu->addAction(&_dimensionPickerAction);

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
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void ZOrderingAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _modeAction.fromParentVariantMap(variantMap);
    _dimensionPickerAction.fromParentVariantMap(variantMap);
    updateScatterplotWidget();
}

QVariantMap ZOrderingAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _modeAction.insertIntoVariantMap(variantMap);
    _dimensionPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
