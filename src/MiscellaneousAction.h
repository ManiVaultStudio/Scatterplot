#pragma once

#include "PluginAction.h"

#include <QActionGroup>
#include <QHBoxLayout>

class QMenu;

class MiscellaneousAction : public PluginAction
{
protected: // Widget

    class Widget : public PluginAction::Widget {
    public:
        Widget(QWidget* parent, MiscellaneousAction* miscellaneousAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    MiscellaneousAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

protected:
    hdps::gui::ColorAction  _backgroundColorAction;

    static const QColor DEFAULT_BACKGROUND_COLOR;

    friend class Widget;
};