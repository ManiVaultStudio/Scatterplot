#include "PositionAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"

#include <QMenu>

using namespace hdps::gui;

PositionAction::PositionAction(QObject* parent, const QString& title) :
    WidgetAction(parent, title),
    _xDimensionPickerAction(this, "X"),
    _yDimensionPickerAction(this, "Y")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("ruler-combined"));

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

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this, scatterplotPlugin]() {
        _xDimensionPickerAction.setPointsDataset(scatterplotPlugin->getPositionDataset());
        _yDimensionPickerAction.setPointsDataset(scatterplotPlugin->getPositionDataset());

        _xDimensionPickerAction.setCurrentDimensionIndex(0);
        _xDimensionPickerAction.setDefaultDimensionIndex(0);

        const auto yIndex = _xDimensionPickerAction.getNumberOfDimensions() >= 2 ? 1 : 0;

        _yDimensionPickerAction.setCurrentDimensionIndex(yIndex);
        _yDimensionPickerAction.setDefaultDimensionIndex(yIndex);
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
        _xDimensionPickerAction.connectToPublicAction(&publicPositionAction->_xDimensionPickerAction, recursive);
        _yDimensionPickerAction.connectToPublicAction(&publicPositionAction->_yDimensionPickerAction, recursive);
    }

    WidgetAction::connectToPublicAction(publicAction, recursive);
}

void PositionAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        _xDimensionPickerAction.disconnectFromPublicAction(recursive);
        _yDimensionPickerAction.disconnectFromPublicAction(recursive);
    }

    WidgetAction::disconnectFromPublicAction(recursive);
}

void PositionAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _xDimensionPickerAction.fromParentVariantMap(variantMap);
    _yDimensionPickerAction.fromParentVariantMap(variantMap);
}

QVariantMap PositionAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _xDimensionPickerAction.insertIntoVariantMap(variantMap);
    _yDimensionPickerAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

PositionAction::Widget::Widget(QWidget* parent, PositionAction* positionAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, positionAction, widgetFlags)
{
    auto xDimensionLabel    = positionAction->_xDimensionPickerAction.createLabelWidget(this);
    auto yDimensionLabel    = positionAction->_yDimensionPickerAction.createLabelWidget(this);
    auto xDimensionWidget   = positionAction->_xDimensionPickerAction.createWidget(this);
    auto yDimensionWidget   = positionAction->_yDimensionPickerAction.createWidget(this);

    xDimensionWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    yDimensionWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    if (widgetFlags & PopupLayout) {
        auto layout = new QGridLayout();

        layout->addWidget(xDimensionLabel, 0, 0);
        layout->addWidget(xDimensionWidget, 0, 1);
        layout->addWidget(yDimensionLabel, 1, 0);
        layout->addWidget(yDimensionWidget, 1, 1);

        setPopupLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(xDimensionLabel);
        layout->addWidget(xDimensionWidget);
        layout->addWidget(yDimensionLabel);
        layout->addWidget(yDimensionWidget);

        setLayout(layout);
    }
}
