#pragma once

#include "PluginAction.h"

#include <QActionGroup>
#include <QHBoxLayout>

class QMenu;

class MiscellaneousAction : public PluginAction
{
protected: // Widget

    class Widget : public hdps::gui::WidgetActionWidget {
    public:
        Widget(QWidget* parent, MiscellaneousAction* miscellaneousAction, const hdps::gui::WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
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