#include "PluginAction.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

PluginAction::PluginAction(QObject* parent, ScatterplotPlugin* scatterplotPlugin, const QString& title) :
    WidgetAction(parent),
    _scatterplotPlugin(scatterplotPlugin)
{
    _scatterplotPlugin->getWidget().addAction(this);

    setText(title);
    setToolTip(title);
}

ScatterplotWidget& PluginAction::getScatterplotWidget()
{
    Q_ASSERT(_scatterplotPlugin != nullptr);

    return _scatterplotPlugin->getScatterplotWidget();
}