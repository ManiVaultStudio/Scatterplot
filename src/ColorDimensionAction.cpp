#include "ColorDimensionAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

#include <QHBoxLayout>
#include <QComboBox>

ColorDimensionAction::ColorDimensionAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _currentDimensionAction(this, "Color dimension")
{
    scatterplotPlugin->addAction(&_currentDimensionAction);

    const auto updateColorDimension = [this]() -> void {
        if (!_currentDimensionAction.hasSelection())
            return;

        if (getScatterplotWidget()->getColoringMode() == ScatterplotWidget::ColoringMode::ColorDimension)
            _scatterplotPlugin->setColorDimension(_currentDimensionAction.getCurrentIndex());
    };

    connect(&_currentDimensionAction, &OptionAction::currentIndexChanged, this, [this, updateColorDimension](const std::uint32_t& currentIndex) {
        updateColorDimension();
    });

    connect(getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, [this, updateColorDimension](const ScatterplotWidget::ColoringMode& coloringMode) {
        if (coloringMode == ScatterplotWidget::ColoringMode::ColorDimension)
            updateColorDimension();
    });

    connect(getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, updateColorDimension](const ScatterplotWidget::RenderMode& renderMode) {
        updateColorDimension();
    });

    updateColorDimension();
}

QMenu* ColorDimensionAction::getContextMenu()
{
    auto menu = new QMenu("Color dimension");

    menu->addAction(&_currentDimensionAction);

    return menu;
}

void ColorDimensionAction::setDimensions(const std::uint32_t& numberOfDimensions, const std::vector<QString>& dimensionNames /*= std::vector<QString>()*/)
{
    _currentDimensionAction.setOptions(common::getDimensionNamesStringList(numberOfDimensions, dimensionNames));
}

void ColorDimensionAction::setDimensions(const std::vector<QString>& dimensionNames)
{
    setDimensions(static_cast<std::uint32_t>(dimensionNames.size()), dimensionNames);
}

ColorDimensionAction::Widget::Widget(QWidget* parent, ColorDimensionAction* colorDimensionAction) :
    WidgetActionWidget(parent, colorDimensionAction)
{
    auto layout = new QHBoxLayout();

    auto colorDimensionWidget = colorDimensionAction->_currentDimensionAction.createWidget(this);
    
    colorDimensionWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    layout->setMargin(0);
    layout->addWidget(colorDimensionWidget);

    setLayout(layout);
}