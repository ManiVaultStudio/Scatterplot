#include "ConstantColorAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <QHBoxLayout>
#include <QMenu>

using namespace hdps::gui;

const QColor ConstantColorAction::DEFAULT_COLOR = qRgb(93, 93, 225);

ConstantColorAction::ConstantColorAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _constantColorAction(this, "Constant color", DEFAULT_COLOR, DEFAULT_COLOR),
    _resetAction(this, "Reset")
{
    _scatterplotPlugin->addAction(&_constantColorAction);

    _constantColorAction.setToolTip("Constant color");
    _resetAction.setToolTip("Reset color settings");

    const auto updateConstantColor = [this]() -> void {
        if (_scatterplotPlugin->getNumberOfPoints() == 0)
            return;

        QPixmap colorPixmap(1, 1);

        colorPixmap.fill(_constantColorAction.getColor());
        
        getScatterplotWidget()->setColorMap(colorPixmap.toImage());
    };

    const auto updateResetAction = [this]() -> void {
        _resetAction.setEnabled(_constantColorAction.getColor() != DEFAULT_COLOR);
    };

    connect(&_constantColorAction, &ColorAction::colorChanged, this, [this, updateConstantColor, updateResetAction](const QColor& color) {
        updateConstantColor();
        updateResetAction();
    });

    connect(&_resetAction, &QAction::triggered, this, [this, updateResetAction](const QColor& color) {
        _constantColorAction.setColor(DEFAULT_COLOR);
    });

    connect(getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, updateConstantColor](const ScatterplotWidget::RenderMode& renderMode) {
        if (renderMode == ScatterplotWidget::RenderMode::SCATTERPLOT)
            updateConstantColor();
    });

    connect(getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, [this, updateConstantColor](const ScatterplotWidget::ColoringMode& coloringMode) {
        if (coloringMode == ScatterplotWidget::ColoringMode::ConstantColor)
            updateConstantColor();
    });

    connect(&_scatterplotPlugin->getPointsDataset(), &DatasetRef<Points>::datasetNameChanged, this, [this, updateConstantColor](const QString& oldDatasetName, const QString& newDatasetName) {
        if (getScatterplotWidget()->getColoringMode() == ScatterplotWidget::ColoringMode::ConstantColor)
            updateConstantColor();
    });

    updateResetAction();
    updateConstantColor();
}

QMenu* ConstantColorAction::getContextMenu()
{
    auto menu = new QMenu("Constant color");

    menu->addAction(&_constantColorAction);
    menu->addAction(&_resetAction);

    return menu;
}

ConstantColorAction::Widget::Widget(QWidget* parent, ConstantColorAction* colorByConstantAction) :
    WidgetActionWidget(parent, colorByConstantAction)
{
    auto layout = new QHBoxLayout();

    layout->setMargin(0);
    layout->addWidget(colorByConstantAction->_constantColorAction.createWidget(this));

    setLayout(layout);
}
