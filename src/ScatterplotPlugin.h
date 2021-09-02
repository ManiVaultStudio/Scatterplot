#pragma once

#include <ViewPlugin.h>

#include "util/DatasetRef.h"

#include "Common.h"

#include "SettingsAction.h"

using namespace hdps::plugin;
using namespace hdps::util;

class Points;

class PixelSelectionTool;
class ScatterplotWidget;

namespace hdps
{
    class CoreInterface;
    class Vector2f;
    class DataHierarchyItems;

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
    
    /** Returns the icon of this plugin */
    QIcon getIcon() const override;

    void init() override;

    void onDataEvent(hdps::DataEvent* dataEvent);

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

    PixelSelectionTool* getSelectionTool();

public: // Data loading

    /**
     * Load point data
     * @param dataSetName Name of the point dataset
     */
    void loadPointData(const QString& dataSetName);

    /**
     * Load color data
     * @param dataSetName Name of the color/cluster dataset
     */
    void loadColorData(const QString& dataSetName);

    void selectPoints();

public: // Miscellaneous

    /** Get current points dataset */
    DatasetRef<Points>& getPointsDataset();

    /** Returns whether a points dataset is loaded or not */
    bool arePointsLoaded() const;

    /** Get current color dataset */
    DatasetRef<DataSet>& getColorDataset();

    /** Returns whether a color dataset is loaded or not */
    bool areColorsLoaded() const;

    /** Get cluster dataset names for the loaded points dataset */
    QStringList getClusterDatasetNames();

signals:
    void currentPointsChanged(const QString& datasetName);
    void currentColorsChanged(const QString& datasetName);
    void selectionChanged();

public:
    ScatterplotWidget* getScatterplotWidget();
    hdps::CoreInterface* getCore();

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
    PixelSelectionTool*         _pixelSelectionTool;
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
    
    ViewPlugin* produce() override;

    hdps::DataTypes supportedDataTypes() const override;
};
