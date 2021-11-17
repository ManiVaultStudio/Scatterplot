#pragma once

#include "PluginAction.h"

#include "actions/DatasetPickerAction.h"

#include "PointsDimensionPickerAction.h"

using namespace hdps::gui;

/**
 * Color by data action class
 *
 * Action class for configuring color data
 *
 * @author Thomas Kroes
 */
class ColorByDataAction : public PluginAction
{
protected: // Widget

    /** Widget class for color by data action */
    class Widget : public WidgetActionWidget
    {
    protected:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param colorByDataAction Pointer to color by data action
         */
        Widget(QWidget* parent, ColorByDataAction* colorByDataAction);

        friend class ColorByDataAction;
    };

protected:

    /**
     * Get widget representation of the color by data action
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
    ColorByDataAction(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

    /** Update datasets in the pick dataset action */
    void updateDatasetPickerAction();

public: // Action getters

    DatasetPickerAction& getDatasetPickerAction() { return _datasetPickerAction; }
    PointsDimensionPickerAction& getPointsDimensionPickerAction() { return _pointsDimensionPickerAction; }

protected:
    DatasetPickerAction             _datasetPickerAction;           /** Dataset picker action */
    PointsDimensionPickerAction     _pointsDimensionPickerAction;   /** Dimension picker action */

    friend class Widget;
};