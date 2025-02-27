#include "PositionAction.h"
#include "ScatterplotPlugin.h"

#include <QMenu>

using namespace mv::gui;

PositionAction::PositionAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _xDimensionPickerAction(this, "X"),
    _yDimensionPickerAction(this, "Y"),
    _dontUpdateScatterplot(false)
{
    setIconByName("ruler-combined");
    setLabelSizingType(LabelSizingType::Auto);

    addAction(&_xDimensionPickerAction);
    addAction(&_yDimensionPickerAction);

    _xDimensionPickerAction.setToolTip("X dimension");
    _yDimensionPickerAction.setToolTip("Y dimension");

    auto scatterplotPlugin = dynamic_cast<ScatterplotPlugin*>(parent->parent());

    if (scatterplotPlugin == nullptr)
        return;

    connect(&_xDimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, [this, scatterplotPlugin](const std::uint32_t& currentDimensionIndex) {
        if (_dontUpdateScatterplot)
            return;

        scatterplotPlugin->setXDimension(currentDimensionIndex);
    });

    connect(&_yDimensionPickerAction, &DimensionPickerAction::currentDimensionIndexChanged, [this, scatterplotPlugin](const std::uint32_t& currentDimensionIndex) {
        if (_dontUpdateScatterplot)
            return;

        scatterplotPlugin->setYDimension(currentDimensionIndex);
    });

    const auto updateReadOnly = [this, scatterplotPlugin]() -> void {
        setEnabled(scatterplotPlugin->getPositionDataset().isValid());
        };

    updateReadOnly();

    auto onDimensionsChanged = [this, scatterplotPlugin](int32_t xDim, int32_t yDim, auto& currentData) {
        // Ensure that we never access non-existing dimensions
        // Data is re-drawn for each setPointsDataset call
        // with the dimension index of the respective _xDimensionPickerAction set to 0
        // This prevents the _yDimensionPickerAction dimension being larger than 
        // the new numDimensions during the first setPointsDataset call
        _dontUpdateScatterplot = true;
        _yDimensionPickerAction.setCurrentDimensionIndex(yDim);

        _xDimensionPickerAction.setPointsDataset(currentData);
        _yDimensionPickerAction.setPointsDataset(currentData);

        // setXDimension() ignores its argument and will update the 
        // scatterplot with both current dimension indices
        _xDimensionPickerAction.setCurrentDimensionIndex(xDim);
        _yDimensionPickerAction.setCurrentDimensionIndex(yDim);
        scatterplotPlugin->setXDimension(xDim);
        _dontUpdateScatterplot = false;
    };

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::dataDimensionsChanged, this, [this, updateReadOnly, scatterplotPlugin, onDimensionsChanged]() {
        updateReadOnly();
        
        // if the new number of dimensions allows it, keep the previous dimension indices
        auto xDim = _xDimensionPickerAction.getCurrentDimensionIndex();
        auto yDim = _yDimensionPickerAction.getCurrentDimensionIndex();
        
        const auto& currentData = scatterplotPlugin->getPositionDataset();
        const auto numDimensions = static_cast<int32_t>(currentData->getNumDimensions());
        
        if (xDim >= numDimensions || yDim >= numDimensions)
        {
            xDim = 0;
            yDim = numDimensions >= 2 ? 1 : 0;
        }
        
        onDimensionsChanged(xDim, yDim, currentData);
    });

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this, scatterplotPlugin, updateReadOnly, onDimensionsChanged]([[maybe_unused]] mv::DatasetImpl* dataset) {
        if (scatterplotPlugin->getPositionDataset().isValid()) {
            updateReadOnly();

            const auto& currentData = scatterplotPlugin->getPositionDataset();
            const auto numDimensions = static_cast<int32_t>(currentData->getNumDimensions());

            const int32_t xDim = 0;
            const int32_t yDim = numDimensions >= 2 ? 1 : 0;

            onDimensionsChanged(xDim, yDim, currentData);
        }
        else {
            _xDimensionPickerAction.setPointsDataset(Dataset<Points>());
            _yDimensionPickerAction.setPointsDataset(Dataset<Points>());
        }
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
