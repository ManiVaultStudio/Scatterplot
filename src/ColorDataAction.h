#pragma once

#include "PluginAction.h"

using namespace hdps::gui;

class ColorDataAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget
    {
    protected:
        Widget(QWidget* parent, ColorDataAction* colorDataAction);

        friend class ColorDataAction;
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this);
    };

public:
    ColorDataAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    StringAction& getDatasetNameAction() { return _datasetNameAction; }

protected:
    StringAction     _datasetNameAction;

    friend class Widget;
};