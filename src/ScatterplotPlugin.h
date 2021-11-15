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
    void setColorDimension(const std::int32_t& dimensionIndex);

public: // Selection
    bool canSelect() const;
    bool canSelectAll() const;
    bool canClearSelection() const;
    bool canInvertSelection() const;
    void selectAll();
    void clearSelection();
    void invertSelection();

protected: // Data loading

    /**
     * Load point data
     * @param dataSetId Globally unique identifier of the points dataset
     */
    void loadPoints(const QString& dataSetId);

    /**
     * Load color data
     * @param dataSetId Globally unique identifier of the colors/clusters dataset
     */
    void loadColors(const QString& dataSetId);

public: // Miscellaneous

    /** Get current points dataset */
    DatasetRef<Points>& getPointsDataset();

    /** Get current color dataset */
    DatasetRef<DataSet>& getColorsDataset();

    /** Get cluster dataset names for the loaded points dataset */
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
    hdps::CoreInterface* getCore();

    SettingsAction& getSettingsAction() { return _settingsAction; }
private:
    void updateData();
    void calculatePositions(const Points& points);
    void calculateScalars(std::vector<float>& scalars, const Points& points, int colorIndex);
    void updateSelection();

private:
    DatasetRef<Points>              _points;        /** Currently loaded points dataset */
    DatasetRef<DataSet>             _colors;        /** Currently loaded color dataset (if any) */
    std::vector<hdps::Vector2f>     _positions;     /** Point positions */
    unsigned int                    _numPoints;     /** Number of point positions */
    
    
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
