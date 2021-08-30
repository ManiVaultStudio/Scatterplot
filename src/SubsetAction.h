#pragma once

#include "PluginAction.h"

using namespace hdps::gui;

class SubsetAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, SubsetAction* subsetAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    SubsetAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    StringAction     _subsetNameAction;
    TriggerAction    _createSubsetAction;
    OptionAction     _sourceDataAction;

    friend class Widget;
};