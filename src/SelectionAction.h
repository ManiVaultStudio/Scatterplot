#pragma once

#include "PluginAction.h"

#include <QActionGroup>
#include <QDebug>

class SelectionAction : public PluginAction
{
protected: // Widget

    class Widget : public hdps::gui::WidgetActionWidget {
    public:
        Widget(QWidget* parent, SelectionAction* selectionAction, const hdps::gui::WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    SelectionAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

public: // Event handling

    /**
     * Listens to the events of target \p object
     * @param object Target object to watch for events
     * @param event Event that occurred
     */
    bool eventFilter(QObject* object, QEvent* event) override;

protected:
    hdps::gui::OptionAction     _typeAction;
    hdps::gui::TriggerAction    _rectangleAction;
    hdps::gui::TriggerAction    _brushAction;
    hdps::gui::TriggerAction    _lassoAction;
    hdps::gui::TriggerAction    _polygonAction;
    QActionGroup                _typeActionGroup;
    hdps::gui::DecimalAction    _brushRadiusAction;
    hdps::gui::ToggleAction     _modifierAddAction;
    hdps::gui::ToggleAction     _modifierRemoveAction;
    QActionGroup                _modifierActionGroup;
    hdps::gui::TriggerAction    _clearSelectionAction;
    hdps::gui::TriggerAction    _selectAllAction;
    hdps::gui::TriggerAction    _invertSelectionAction;
    hdps::gui::ToggleAction     _notifyDuringSelectionAction;

    friend class Widget;
};