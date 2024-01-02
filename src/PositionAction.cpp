#include "PositionAction.h"
#include "ScatterplotPlugin.h"

#include <QMenu>

using namespace mv::gui;

PositionAction::PositionAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _xDimensionPickerAction(this, "X"),
    _yDimensionPickerAction(this, "Y")
{
    setIcon(mv::Application::getIconFont("FontAwesome").getIcon("ruler-combined"));
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_xDimensionPickerAction);
    addAction(&_yDimensionPickerAction);

    _xDimensionPickerAction.setToolTip("X dimension");
    _yDimensionPickerAction.setToolTip("Y dimension");

    auto scatterplotPlugin = dynamic_cast<ScatterplotPlugin*>(parent->parent());

    if (scatterplotPlugin == nullptr)
        return;

    connect(&_xDimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, [this, scatterplotPlugin](const std::uint32_t& currentDimensionIndex) {
        scatterplotPlugin->setXDimension(currentDimensionIndex);
    });

    connect(&_yDimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, [this, scatterplotPlugin](const std::uint32_t& currentDimensionIndex) {
        scatterplotPlugin->setYDimension(currentDimensionIndex);
    });

    const auto updateReadOnly = [this, scatterplotPlugin]() -> void {
        setEnabled(scatterplotPlugin->getPositionDataset().isValid());
        };

    updateReadOnly();

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataDimensionsChanged, this, [this, scatterplotPlugin, updateReadOnly]() {
        updateReadOnly();

        // if the new number of dimensions allows it, keep the previous dimension indices
        auto xDim = _xDimensionPickerAction.getCurrentDimensionIndex();
        auto yDim = _yDimensionPickerAction.getCurrentDimensionIndex();

        const auto& currentData = scatterplotPlugin->getPositionDataset();
        const auto numDimensions = static_cast<int32_t>(currentData->getNumDimensions());

        if (xDim >= numDimensions || yDim >= numDimensions)
        {
            xDim = 0;
            yDim = _xDimensionPickerAction.getNumberOfDimensions() >= 2 ? 1 : 0;
            _xDimensionPickerAction.setCurrentDimensionIndex(xDim);
            _yDimensionPickerAction.setCurrentDimensionIndex(yDim);
        }

        _xDimensionPickerAction.setPointsDataset(currentData);
        _yDimensionPickerAction.setPointsDataset(currentData);

        _xDimensionPickerAction.setCurrentDimensionIndex(xDim);
        _yDimensionPickerAction.setCurrentDimensionIndex(yDim);

    });

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this, scatterplotPlugin, updateReadOnly](mv::DatasetImpl* dataset) {
        updateReadOnly();

        _xDimensionPickerAction.setPointsDataset(scatterplotPlugin->getPositionDataset());
        _yDimensionPickerAction.setPointsDataset(scatterplotPlugin->getPositionDataset());

        _xDimensionPickerAction.setCurrentDimensionIndex(0);

        const auto yIndex = _xDimensionPickerAction.getNumberOfDimensions() >= 2 ? 1 : 0;

        _yDimensionPickerAction.setCurrentDimensionIndex(yIndex);
    });

}

QMenu* PositionAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("Position", parent);

    auto xDimensionMenu = new QMenu("X dimension");
    auto yDimensionMenu = new QMenu("Y dimension");

    xDimensionMenu->addAction(&_xDimensionPickerAction);
    yDimensionMenu->addAction(&_yDimensionPickerAction);

    menu->addMenu(xDimensionMenu);
    menu->addMenu(yDimensionMenu);

    return menu;
}

std::int32_t PositionAction::getDimensionX() const
{
    return _xDimensionPickerAction.getCurrentDimensionIndex();
}

std::int32_t PositionAction::getDimensionY() const
{
    return _yDimensionPickerAction.getCurrentDimensionIndex();
}

void PositionAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicPositionAction = dynamic_cast<PositionAction*>(publicAction);

    Q_ASSERT(publicPositionAction != nullptr);

    if (publicPositionAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_xDimensionPickerAction, &publicPositionAction->getXDimensionPickerAction(), recursive);
        actions().connectPrivateActionToPublicAction(&_yDimensionPickerAction, &publicPositionAction->getYDimensionPickerAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void PositionAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_xDimensionPickerAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_yDimensionPickerAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void PositionAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _xDimensionPickerAction.fromParentVariantMap(variantMap);
    _yDimensionPickerAction.fromParentVariantMap(variantMap);
}

QVariantMap PositionAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _xDimensionPickerAction.insertIntoVariantMap(variantMap);
    _yDimensionPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}
