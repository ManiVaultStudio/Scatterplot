#pragma once

#include "PluginAction.h"

#include "ColorByConstantAction.h"
#include "ColorByDataAction.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>

using namespace hdps::gui;

/**
 * Coloring action class
 *
 * Action class for configuring the coloring of points
 *
 * @author Thomas Kroes
 */
class ColoringAction : public PluginAction
{
protected: // Widget

    class StackedWidget : public QStackedWidget {
    public:
        QSize sizeHint() const override { return currentWidget()->sizeHint(); }
        QSize minimumSizeHint() const override { return currentWidget()->minimumSizeHint(); }
    };

    /** Widget class for coloring action */
    class Widget : public WidgetActionWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param coloringAction Pointer to coloring action
         */
        Widget(QWidget* parent, ColoringAction* coloringAction, const std::int32_t& widgetFlags);
    };

protected:

    /**
     * Get widget representation of the coloring action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:

    /**
     * Constructor
     * @param scatterplotPlugin Pointer to scatter plot plugin
     */
    ColoringAction(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

protected:

    /** Update the state of various actions */
    void updateActions();

public: // Action getters

    OptionAction& getColorByAction() { return _colorByAction; }
    TriggerAction& getColorByConstantTriggerAction() { return _colorByConstantTriggerAction; }
    TriggerAction& getColorByDataTriggerAction() { return _colorByDataTriggerAction; }
    ColorByConstantAction& getColorByConstantAction() { return _colorByConstantAction; }
    ColorByDataAction& getColorByDataAction() { return _colorByDataAction; }

protected:
    OptionAction            _colorByAction;                     /** Action for picking the coloring type */
    TriggerAction           _colorByConstantTriggerAction;      /** Color by constant color trigger action (for key shortcut) */
    TriggerAction           _colorByDataTriggerAction;          /** Color by data trigger action (for key shortcut) */
    QActionGroup            _colorByActionGroup;                /** Color by action group */
    ColorByConstantAction   _colorByConstantAction;             /** Color by constant action */
    ColorByDataAction       _colorByDataAction;                 /** Color by data action */

    friend class Widget;
};