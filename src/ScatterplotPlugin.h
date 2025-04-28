#pragma once

#include <ViewPlugin.h>

#include <actions/HorizontalToolbarAction.h>
#include <graphics/Vector2f.h>

#include "SettingsAction.h"

#include <QTimer>

using namespace mv::plugin;
using namespace mv::util;
using namespace mv::gui;

class Points;
class ScatterplotWidget;

namespace mv
{
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

    /** Use the sampler pixel selection tool to sample data points */
    void samplePoints();

public:

    /** Get reference to the scatter plot widget */
    ScatterplotWidget& getScatterplotWidget();

    SettingsAction& getSettingsAction() { return _settingsAction; }

private:
    void updateData();
    void updateSelection();

public: // Serialization

    /**
     * Load plugin from variant map
     * @param variantMap Variant map representation of the plugin
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save plugin to variant map
     * @return Variant map representation of the plugin
     */
    QVariantMap toVariantMap() const override;

private:
    mv::gui::DropWidget*            _dropWidget;                /** Widget for dropping datasets */
    ScatterplotWidget*              _scatterPlotWidget;         /** The visualization widget */
    Dataset<Points>                 _positionDataset;           /** Smart pointer to points dataset for point position */
    Dataset<Points>                 _positionSourceDataset;     /** Smart pointer to source of the points dataset for point position (if any) */
    std::vector<mv::Vector2f>       _positions;                 /** Point positions */
    unsigned int                    _numPoints;                 /** Number of point positions */
    SettingsAction                  _settingsAction;            /** Group action for all settings */
    HorizontalToolbarAction         _primaryToolbarAction;      /** Horizontal toolbar for primary content */
    QRectF                          _selectionBoundaries;       /** Boundaries of the selection */

    static const std::int32_t LAZY_UPDATE_INTERVAL = 2;

};

// =============================================================================
// Factory
// =============================================================================

class ScatterplotPluginFactory : public ViewPluginFactory
{
    Q_INTERFACES(mv::plugin::ViewPluginFactory mv::plugin::PluginFactory)
    Q_OBJECT
    Q_PLUGIN_METADATA(IID   "studio.manivault.ScatterplotPlugin"
                      FILE  "ScatterplotPlugin.json")

public:
    ScatterplotPluginFactory();

    ViewPlugin* produce() override;

    /**
     * Get plugin trigger actions given \p datasets
     * @param datasets Vector of input datasets
     * @return Vector of plugin trigger actions
     */
    PluginTriggerActions getPluginTriggerActions(const mv::Datasets& datasets) const override;

    /**
     * Get the URL of the GitHub repository
     * @return URL of the GitHub repository (or readme markdown URL if set)
     */
    QUrl getRepositoryUrl() const override;
};