#pragma once

#include <ViewPlugin.h>

#include "util/PixelSelectionTool.h"

#include "Common.h"

#include "SettingsAction.h"

using namespace hdps::plugin;
using namespace hdps::util;

class Points;

class ScatterplotWidget;

namespace hdps
{
    class CoreInterface;
    class Vector2f;

    namespace gui {
        class DropWidget;
    }
}

class ScatterplotPlugin : public ViewPlugin
{
    Q_OBJECT
    
public:
    ScatterplotPlugin(const PluginFactory* factory);
    ~ScatterplotPlugin() override;

    void init() override;

    /**
     * Load one (or more datasets in the view)
     * @param datasets Dataset(s) to load
     */
    void loadData(const Datasets& datasets) override;

    /** Get number of points in the position dataset */
    std::uint32_t getNumberOfPoints() const;

public:
    void createSubset(const bool& fromSourceData = false, const QString& name = "");

public: // Dimension picking
    void setXDimension(const std::int32_t& dimensionIndex);
    void setYDimension(const std::int32_t& dimensionIndex);

protected: // Data loading

    /** Invoked when the position points dataset changes */
    void positionDatasetChanged();

public: // Point colors

    /**
     * Load color from points dataset
     * @param points Smart pointer to points dataset
     * @param dimensionIndex Index of the dimension to load
     */
    void loadColors(const Dataset<Points>& points, const std::uint32_t& dimensionIndex);

    /**
     * Load color from clusters dataset
     * @param clusters Smart pointer to clusters dataset
     */
    void loadColors(const Dataset<Clusters>& clusters);

public: // Miscellaneous

    /** Get smart pointer to points dataset for point position */
    Dataset<Points>& getPositionDataset();

    /** Get smart pointer to source of the points dataset for point position (if any) */
    Dataset<Points>& getPositionSourceDataset();

    /** Use the pixel selection tool to select data points */
    void selectPoints();

protected:

    /** Updates the window title (displays the name of the view and the GUI name of the loaded points dataset) */
    void updateWindowTitle();

public:

    /** Get reference to the scatter plot widget */
    ScatterplotWidget& getScatterplotWidget();

    SettingsAction& getSettingsAction() { return _settingsAction; }

private:
    void updateData();
    void calculatePositions(const Points& points);
    void updateSelection();

private:
    Dataset<Points>                 _positionDataset;           /** Smart pointer to points dataset for point position */
    Dataset<Points>                 _positionSourceDataset;     /** Smart pointer to source of the points dataset for point position (if any) */
    std::vector<hdps::Vector2f>     _positions;                 /** Point positions */
    unsigned int                    _numPoints;                 /** Number of point positions */
    
    
protected:
    ScatterplotWidget*          _scatterPlotWidget;
    hdps::gui::DropWidget*      _dropWidget;
    SettingsAction              _settingsAction;
};

// =============================================================================
// Factory
// =============================================================================

class ScatterplotPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(hdps::plugin::ViewPluginFactory hdps::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "nl.tudelft.ScatterplotPlugin"
                      FILE  "ScatterplotPlugin.json")
    
public:
    ScatterplotPluginFactory(void) {}
    ~ScatterplotPluginFactory(void) override {}

    /** Returns the plugin icon */
    QIcon getIcon() const override;

    ViewPlugin* produce() override;

    /**
     * Get a list of producer actions given a sequence of input datasets
     * @param datasets Sequence of input datasets (order in which they were selected in the data hierarchy)
     * @return List of producer actions with which one (or more) plugins can be triggered
     */
    QList<QAction*> getProducers(const hdps::Datasets& datasets) const override;
};
