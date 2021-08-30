#pragma once

#include "PluginAction.h"

#include <QActionGroup>

using namespace hdps::gui;

class QMenu;

class MiscellaneousAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, MiscellaneousAction* miscellaneousAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    MiscellaneousAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    ColorAction  _backgroundColorAction;

    static const QColor DEFAULT_BACKGROUND_COLOR;

    friend class Widget;
};