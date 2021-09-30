#pragma once

#include "PluginAction.h"

using namespace hdps::gui;

class ColorDataAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget
    {
    protected:
        Widget(QWidget* parent, ColorDataAction* colorDataAction, const Widget::State& state);

        friend class ColorDataAction;
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    ColorDataAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    StringAction& getDatasetNameAction() { return _datasetNameAction; }

protected:
    StringAction     _datasetNameAction;

    friend class Widget;
};