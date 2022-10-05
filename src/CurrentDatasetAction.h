#pragma once

#include "PluginAction.h"

#include "actions/DatasetPickerAction.h"

using namespace hdps::gui;

class CurrentDatasetAction : public PluginAction
{
protected:

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, CurrentDatasetAction* currentDatasetAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:
    CurrentDatasetAction(ScatterplotPlugin* scatterplotPlugin);

protected:
    DatasetPickerAction	_datasetPickerAction;

    friend class Widget;
};