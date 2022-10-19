#pragma once

#include "PluginAction.h"

#include "actions/DatasetPickerAction.h"

using namespace hdps::gui;

class LoadedDatasetsAction : public PluginAction
{
protected:

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, LoadedDatasetsAction* currentDatasetAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:
    LoadedDatasetsAction(ScatterplotPlugin* scatterplotPlugin);

protected:
    DatasetPickerAction	    _positionDatasetPickerAction;
    DatasetPickerAction     _colorDatasetPickerAction;

    friend class Widget;
};