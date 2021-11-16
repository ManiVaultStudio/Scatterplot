#pragma once

#include "PluginAction.h"

using namespace hdps::gui;

class ColorByConstantAction : public PluginAction
{
protected:
    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, ColorByConstantAction* colorByConstantAction);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this);
    };

public:
    ColorByConstantAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    ColorAction     _constantColorAction;
    TriggerAction   _resetAction;

    static const QColor DEFAULT_COLOR;

    friend class Widget;
};