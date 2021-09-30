#pragma once

#include "PluginAction.h"

using namespace hdps::gui;

class ConstantColorAction : public PluginAction
{
protected:
    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, ConstantColorAction* colorByConstantAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    ConstantColorAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    ColorAction     _constantColorAction;
    TriggerAction   _resetAction;

    static const QColor DEFAULT_COLOR;

    friend class Widget;
};