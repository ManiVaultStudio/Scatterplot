#pragma once

#include "actions/Actions.h"

class ScatterplotPlugin;
class ScatterplotWidget;

class PluginAction : public hdps::gui::WidgetAction
{
public:
    PluginAction(QObject* parent, ScatterplotPlugin* scatterplotPlugin, const QString& title);

    ScatterplotWidget& getScatterplotWidget();

protected:
    ScatterplotPlugin*  _scatterplotPlugin;
};