#pragma once

#include "PluginAction.h"
#include "PixelSelectionTool.h"

#include <QActionGroup>
#include <QDebug>

using namespace hdps::gui;

class SelectionAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, SelectionAction* selectionAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
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

public: // Action getters

    TriggerAction& getRectangleAction() { return _rectangleAction; }
    TriggerAction& getBrushAction() { return _brushAction; }
    TriggerAction& getLassoAction() { return _lassoAction; }
    TriggerAction& getPolygonAction() { return _polygonAction; }

protected:

    /**
     * Get the icon for the specified selection type
     * @param selectionType The type of selection e.g. brush rectangle etc.
     */
    QIcon getIcon(const PixelSelectionTool::Type& selectionType);

protected:
    OptionAction    _typeAction;
    TriggerAction   _rectangleAction;
    TriggerAction   _brushAction;
    TriggerAction   _lassoAction;
    TriggerAction   _polygonAction;
    QActionGroup    _typeActionGroup;
    DecimalAction   _brushRadiusAction;
    ToggleAction    _modifierAddAction;
    ToggleAction    _modifierRemoveAction;
    QActionGroup    _modifierActionGroup;
    TriggerAction   _clearSelectionAction;
    TriggerAction   _selectAllAction;
    TriggerAction   _invertSelectionAction;
    ToggleAction    _notifyDuringSelectionAction;

    friend class Widget;
};