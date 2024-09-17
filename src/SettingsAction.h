#pragma once

#include <actions/GroupAction.h>

#include "ClusteringAction.h"
#include "ColoringAction.h"
#include "DatasetsAction.h"
#include "ExportAction.h"
#include "MiscellaneousAction.h"
#include "PlotAction.h"
#include "PositionAction.h"
#include "RenderModeAction.h"
#include "SelectionAction.h"
#include "SubsetAction.h"

using namespace mv::gui;

class ScatterplotPlugin;

/**
 * Settings action class
 *
 * Action class for configuring settings
 *
 * @author Thomas Kroes
 */
class SettingsAction : public GroupAction
{
public:
    
    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SettingsAction(QObject* parent, const QString& title);

    /**
     * Get action context menu
     * @return Pointer to menu
     */
    QMenu* getContextMenu();

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

public: // Action getters
    
    RenderModeAction& getRenderModeAction() { return _renderModeAction; }
    PositionAction& getPositionAction() { return _positionAction; }
    PlotAction& getPlotAction() { return _plotAction; }
    ColoringAction& getColoringAction() { return _coloringAction; }
    SubsetAction& getSubsetAction() { return _subsetAction; }
    ClusteringAction& getClusteringAction() { return _clusteringAction; }
    SelectionAction& getSelectionAction() { return _selectionAction; }
    ExportAction& getExportAction() { return _exportAction; }
    MiscellaneousAction& getMiscellaneousAction() { return _miscellaneousAction; }
    DatasetsAction& getDatasetsAction() { return _datasetsAction; }

protected:
    ScatterplotPlugin*          _scatterplotPlugin;         /** Pointer to scatter plot plugin */
    RenderModeAction            _renderModeAction;          /** Action for configuring render mode */
    PositionAction              _positionAction;            /** Action for configuring point positions */
    PlotAction                  _plotAction;                /** Action for configuring plot settings */
    ColoringAction              _coloringAction;            /** Action for configuring point coloring */
    SubsetAction                _subsetAction;              /** Action for creating subset(s) */
    ClusteringAction            _clusteringAction;          /** Action for creating clusters */
    SelectionAction             _selectionAction;           /** Action for selecting points */
    ExportAction                _exportAction;              /** Action for exporting */
    MiscellaneousAction         _miscellaneousAction;       /** Action for miscellaneous settings */
    DatasetsAction              _datasetsAction;            /** Action for picking dataset(s) */
};