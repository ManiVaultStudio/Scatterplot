#pragma once

#include "PluginAction.h"

#include "actions/DatasetPickerAction.h"

#include "PointsDimensionPickerAction.h"

using namespace hdps::gui;

class ColoringAction;

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
     * @param coloringAction Reference to parent coloring action
     */
    ColorByDataAction(ScatterplotPlugin* scatterplotPlugin, ColoringAction& coloringAction);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

    /**
     * Update datasets in the pick dataset action
     * @param datasetToSelect Dataset to select (optional)
     */
    void updateDatasetPickerAction(const Dataset<hdps::DatasetImpl>& datasetToSelect);

    /**
     * Add color dataset
     * @param colorDataset Smart pointer to color dataset
     */
    void addColorDataset(const Dataset<DatasetImpl>& colorDataset);

    /** Determines whether a given color dataset is already loaded */
    bool hasColorDataset(const Dataset<DatasetImpl>& colorDataset) const;

    /** Get smart pointer to current color dataset (if any) */
    Dataset<DatasetImpl> getCurrentColorDataset() const;

protected:

    /** Update point colors in the scatter plot rendering widget */
    void updateColors();

    /** Updates the scalar range in the color map */
    void updateColorMapActionScalarRange();

    /** Update the scatter plot widget color map */
    void updateScatterplotWidgetColorMap();

    /** Update the color map range in the scatter plot widget */
    void updateScatterPlotWidgetColorMapRange();

    /** Determine whether the color map should be enabled */
    bool shouldEnableColorMap() const;

    /** Enables/disables the color map */
    void updateColorMapActionReadOnly();

public: // Action getters

    DatasetPickerAction& getDatasetPickerAction() { return _datasetPickerAction; }
    PointsDimensionPickerAction& getPointsDimensionPickerAction() { return _pointsDimensionPickerAction; }
    ColorMapAction& getColorMapAction() { return _colorMapAction; }

protected:
    ColoringAction&                 _coloringAction;                /** Reference to parent coloring action */
    DatasetPickerAction             _datasetPickerAction;           /** Dataset picker action */
    PointsDimensionPickerAction     _pointsDimensionPickerAction;   /** Dimension picker action */
    ColorMapAction                  _colorMapAction;                /** Color map action */
    QVector<Dataset<DatasetImpl>>   _colorDatasets;                 /** Smart pointer to color datasets */

    friend class Widget;
};