#include "PositionAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"

#include <QMenu>
#include <QComboBox>

using namespace hdps::gui;

PositionAction::PositionAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Position"),
    _xDimensionAction(this, "X"),
    _yDimensionAction(this, "Y")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("ruler-combined"));

    _xDimensionAction.setToolTip("X dimension");
    _yDimensionAction.setToolTip("Y dimension");

    connect(&_xDimensionAction, &OptionAction::currentIndexChanged, [this, scatterplotPlugin](const std::uint32_t& currentIndex) {
        scatterplotPlugin->setXDimension(currentIndex);
    });

    connect(&_yDimensionAction, &OptionAction::currentIndexChanged, [this, scatterplotPlugin](const std::uint32_t& currentIndex) {
        scatterplotPlugin->setYDimension(currentIndex);
    });

    connect(&_yDimensionAction, &OptionAction::optionsChanged, [this, scatterplotPlugin](const QStringList& options) {
        _xDimensionAction.setCurrentIndex(0);
        _xDimensionAction.setDefaultIndex(0);

        const auto yIndex = options.count() >= 2 ? 1 : 0;

        _yDimensionAction.setCurrentIndex(yIndex);
        _yDimensionAction.setDefaultIndex(yIndex);
    });
}

QMenu* PositionAction::getContextMenu()
{
    auto menu = new QMenu("Position");

    auto xDimensionMenu = new QMenu("X dimension");
    auto yDimensionMenu = new QMenu("Y dimension");

    xDimensionMenu->addAction(&_xDimensionAction);
    yDimensionMenu->addAction(&_yDimensionAction);

    menu->addMenu(xDimensionMenu);
    menu->addMenu(yDimensionMenu);

    return menu;
}

void PositionAction::setDimensions(const std::uint32_t& numberOfDimensions, const std::vector<QString>& dimensionNames /*= std::vector<QString>()*/)
{
    auto dimensionNamesStringList = common::getDimensionNamesStringList(numberOfDimensions, dimensionNames);

    _xDimensionAction.setOptions(dimensionNamesStringList);
    _yDimensionAction.setOptions(dimensionNamesStringList);

    _xDimensionAction.setCurrentIndex(0);
    _yDimensionAction.setCurrentIndex(numberOfDimensions >= 2 ? 1 : 0);
}

void PositionAction::setDimensions(const std::vector<QString>& dimensionNames)
{
    setDimensions(static_cast<std::uint32_t>(dimensionNames.size()), dimensionNames);
}

std::int32_t PositionAction::getXDimension() const
{
    return _xDimensionAction.getCurrentIndex();
}

std::int32_t PositionAction::getYDimension() const
{
    return _yDimensionAction.getCurrentIndex();
}

PositionAction::Widget::Widget(QWidget* parent, PositionAction* positionAction, const Widget::State& state) :
    WidgetAction::Widget(parent, positionAction, state)
{
    auto xDimensionLabel    = new QLabel("X-dimension:");
    auto yDimensionLabel    = new QLabel("Y-dimension:");
    auto xDimensionWidget   = dynamic_cast<OptionAction::Widget*>(positionAction->_xDimensionAction.createWidget(this));
    auto yDimensionWidget   = dynamic_cast<OptionAction::Widget*>(positionAction->_yDimensionAction.createWidget(this));

    xDimensionLabel->setToolTip(positionAction->_xDimensionAction.toolTip());
    yDimensionLabel->setToolTip(positionAction->_yDimensionAction.toolTip());
    
    xDimensionWidget->getComboBox()->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    yDimensionWidget->getComboBox()->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    switch (state)
    {
        case Widget::State::Standard:
        {
            auto layout = new QHBoxLayout();

            layout->setMargin(0);
            layout->addWidget(xDimensionLabel);
            layout->addWidget(xDimensionWidget);
            layout->addWidget(yDimensionLabel);
            layout->addWidget(yDimensionWidget);

            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            auto layout = new QGridLayout();

            layout->addWidget(xDimensionLabel, 0, 0);
            layout->addWidget(xDimensionWidget, 0, 1);
            layout->addWidget(yDimensionLabel, 1, 0);
            layout->addWidget(yDimensionWidget, 1, 1);

            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }
}
