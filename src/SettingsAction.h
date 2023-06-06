#pragma once

#include <actions/HorizontalToolbarAction.h>

#include "DatasetsAction.h"
#include "RenderModeAction.h"
#include "PlotAction.h"
#include "PositionAction.h"
#include "ColoringAction.h"
#include "SubsetAction.h"
#include "SelectionAction.h"
#include "ManualClusteringAction.h"
#include "MiscellaneousAction.h"

using namespace hdps::gui;

class ScatterplotPlugin;

/**
 * Settings action class
 *
 * Action class for configuring settings
 *
 * @author Thomas Kroes
 */
class SettingsAction : public HorizontalToolbarAction
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
     * @param Variant map representation of the plugin
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
    ColoringAction& getColoringAction() { return _coloringAction; }
    SubsetAction& getSubsetAction() { return _subsetAction; }
    SelectionAction& getSelectionAction() { return _selectionAction; }
    PlotAction& getPlotAction() { return _plotAction; }
    TriggerAction& getExportAction() { return _exportAction; }
    MiscellaneousAction& getMiscellaneousAction() { return _miscellaneousAction; }

protected:
    ScatterplotPlugin*          _scatterplotPlugin;         /** Pointer to scatter plot plugin */
    RenderModeAction            _renderModeAction;          /** Action for configuring render mode */
    PositionAction              _positionAction;            /** Action for configuring point positions */
    ColoringAction              _coloringAction;            /** Action for configuring point coloring */
    DatasetsAction              _datasetsAction;            /** Action for picking dataset(s) */
    SubsetAction                _subsetAction;              /** Action for creating subset(s) */
    ManualClusteringAction      _manualClusteringAction;    /** Action for creating clusters */
    SelectionAction             _selectionAction;           /** Action for selecting points */
    PlotAction                  _plotAction;                /** Action for configuring plot settings */
    TriggerAction               _exportAction;              /** Action for creating screenshot(s) */
    MiscellaneousAction         _miscellaneousAction;       /** Action for miscellaneous settings */
};