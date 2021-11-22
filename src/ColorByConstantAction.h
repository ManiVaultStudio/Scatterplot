#pragma once

#include "PluginAction.h"

using namespace hdps::gui;

/**
 * Color by constant action class
 *
 * Action class for coloring points by constant color
 *
 * @author Thomas Kroes
 */
class ColorByConstantAction : public PluginAction
{
protected: // Widget

    /** Widget class for color by constant color action */
    class Widget : public WidgetActionWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param colorByConstantAction Pointer to color by constant action
         */
        Widget(QWidget* parent, ColorByConstantAction* colorByConstantAction);
    };

protected:

    /**
     * Get widget representation of the coloring action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this);
    };

public:

    /**
     * Constructor
     * @param scatterplotPlugin Pointer to scatter plot plugin
     */
    ColorByConstantAction(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

protected:

    /** Update the scatter plot widget color map */
    void updateScatterplotWidgetColorMap();

protected:
    ColorAction     _constantColorAction;       /** Constant color action */
    TriggerAction   _resetAction;               /** Reset color to default action */

    static const QColor DEFAULT_COLOR;

    friend class Widget;
};