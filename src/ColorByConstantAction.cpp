#include "ColorByConstantAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <QHBoxLayout>
#include <QMenu>

using namespace hdps::gui;

const QColor ColorByConstantAction::DEFAULT_COLOR = qRgb(93, 93, 225);

ColorByConstantAction::ColorByConstantAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _constantColorAction(this, "Constant color", DEFAULT_COLOR, DEFAULT_COLOR),
    _resetAction(this, "Reset")
{
    _scatterplotPlugin->addAction(&_constantColorAction);

    _constantColorAction.setToolTip("Constant color");
    _resetAction.setToolTip("Reset color settings");

    // Updates the state of the reset action
    const auto updateResetAction = [this]() -> void {
        _resetAction.setEnabled(_constantColorAction.getColor() != DEFAULT_COLOR);
    };

    // Update the scatter plot widget color map and reset action when the color changes
    connect(&_constantColorAction, &ColorAction::colorChanged, this, [this, updateResetAction](const QColor& color) {
        updateScatterplotWidgetColorMap();
        updateResetAction();
    });

    // Reset the constant color when the reset action is triggered
    connect(&_resetAction, &QAction::triggered, this, [this, updateResetAction](const QColor& color) {
        _constantColorAction.setColor(DEFAULT_COLOR);
    });

    // Update scatter plot widget color map when the rendering or coloring mode changed
    connect(getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, &ColorByConstantAction::updateScatterplotWidgetColorMap);
    connect(getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, &ColorByConstantAction::updateScatterplotWidgetColorMap);

    updateResetAction();
    updateScatterplotWidgetColorMap();
}

QMenu* ColorByConstantAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("Constant color", parent);

    menu->addAction(&_constantColorAction);
    menu->addAction(&_resetAction);

    return menu;
}

void ColorByConstantAction::updateScatterplotWidgetColorMap()
{
    // Only update color map in scatter plot rendering mode
    if (_scatterplotPlugin->getScatterplotWidget()->getRenderMode() != ScatterplotWidget::SCATTERPLOT)
        return;

    // Only update color map in constant coloring mode
    if (_scatterplotPlugin->getScatterplotWidget()->getColoringMode() != ScatterplotWidget::ColoringMode::Constant)
        return;

    // Only update color map when we have a valid position dataset
    if (!_scatterplotPlugin->getPositionDataset().isValid())
        return;

    // Create 1x1 pixmap for the (constant) color map
    QPixmap colorPixmap(1, 1);

    // Fill it with the constant color
    colorPixmap.fill(_constantColorAction.getColor());

    // And update the scatter plot widget color map
    getScatterplotWidget()->setColorMap(colorPixmap.toImage());
}

ColorByConstantAction::Widget::Widget(QWidget* parent, ColorByConstantAction* colorByConstantAction) :
    WidgetActionWidget(parent, colorByConstantAction)
{
    setFixedWidth(100);

    auto layout = new QHBoxLayout();

    layout->setMargin(0);
    layout->addWidget(colorByConstantAction->_constantColorAction.createWidget(this));

    setLayout(layout);
}
