#pragma once

#include <ViewPlugin.h>

#include "util/DatasetRef.h"
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

    std::uint32_t getNumberOfPoints() const;
    std::uint32_t getNumberOfSelectedPoints() const;

public:
    void createSubset(const bool& fromSourceData = false, const QString& name = "");

public: // Dimension picking
    void setXDimension(const std::int32_t& dimensionIndex);
    void setYDimension(const std::int32_t& dimensionIndex);

public: // Selection
    bool canSelect() const;
    bool canSelectAll() const;
    bool canClearSelection() const;
    bool canInvertSelection() const;
    void selectAll();
    void clearSelection();
    void invertSelection();

protected: // Data loading

    /** Invoked when the position points dataset changes */
    void positionDatasetChanged();

public: // Point colors

    /** Load color from points dataset */
    void loadColors(const DatasetRef<Points>& points, const std::uint32_t& dimensionIndex);

    /** Load color from clusters dataset */
    void loadColors(const DatasetRef<Clusters>& clusters);

public: // Miscellaneous

    /** Get points dataset for point position */
    const DatasetRef<Points>& getPositionDataset();

    /** Get source of the points dataset for point position (if any) */
    const DatasetRef<Points>& getPositionSourceDataset();

    /** Get points dataset for point color */
    const DatasetRef<Points>& getColorsDataset();

    /** _clustersDataset */
    const DatasetRef<Clusters>& getClustersDataset();

    /** Get clusters dataset for point color */
    QStringList getClusterDatasetNames();

    /** Use the pixel selection tool to select data points */
    void selectPoints();

protected:

    /** Updates the window title (displays the name of the view and the GUI name of the loaded points dataset) */
    void updateWindowTitle();

signals:
    void selectionChanged();

public:
    ScatterplotWidget* getScatterplotWidget();

    SettingsAction& getSettingsAction() { return _settingsAction; }

private:
    void updateData();
    void calculatePositions(const Points& points);
    void updateSelection();

private:
    DatasetRef<Points>              _positionDataset;           /** Points dataset for point position */
    DatasetRef<Points>              _positionSourceDataset;     /** Source of the points dataset for point position (if any) */
    DatasetRef<Points>              _colorsDataset;             /** Points dataset for point color */
    DatasetRef<Clusters>            _clustersDataset;           /** Clusters dataset for point color */
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

    hdps::DataTypes supportedDataTypes() const override;
};
