#pragma once

#include "PluginAction.h"

class ConstantColorAction : public PluginAction
{
protected:
    class Widget : public hdps::gui::WidgetActionWidget {
    public:
        Widget(QWidget* parent, ConstantColorAction* colorByConstantAction, const hdps::gui::WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    ConstantColorAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    hdps::gui::ColorAction      _constantColorAction;
    hdps::gui::TriggerAction    _resetAction;

    static const QColor DEFAULT_COLOR;

    friend class Widget;
};