#include "ColorDataAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"

#include <QMenu>
#include <QHBoxLayout>
#include <QLabel>

using namespace hdps::gui;

ColorDataAction::ColorDataAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _datasetNameAction(this, "Color dataset name")
{
    scatterplotPlugin->addAction(&_datasetNameAction);

    _datasetNameAction.setEnabled(false);
}

QMenu* ColorDataAction::getContextMenu()
{
    auto menu = new QMenu("Color data");

    menu->addAction(&_datasetNameAction);

    return menu;
}

ColorDataAction::Widget::Widget(QWidget* parent, ColorDataAction* colorDataAction, const Widget::State& state) :
    WidgetActionWidget(parent, colorDataAction, state)
{
    auto layout = new QVBoxLayout();

    layout->setMargin(0);
    layout->addWidget(colorDataAction->_datasetNameAction.createWidget(this));

    setLayout(layout);
}
