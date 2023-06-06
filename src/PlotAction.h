#pragma once

#include <actions/GroupAction.h>

#include "PointPlotAction.h"
#include "DensityPlotAction.h"

class ScatterplotPlugin;

using namespace hdps::gui;

/**
 * Plot action class
 *
 * Action class for configuring plot settings
 *
 * @author Thomas Kroes
 */
class PlotAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE PlotAction(QObject* parent, const QString& title);

    /**
     * Initialize the selection action with \p scatterplotPlugin
     * @param scatterplotPlugin Pointer to scatterplot plugin
     */
    void initialize(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Get action context menu
     * @return Pointer to menu
     */
    QMenu* getContextMenu();

public: // Serialization

    /**
     * Load widget action from variant map
     * @param Variant map representation of the widget action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save widget action to variant map
     * @return Variant map representation of the widget action
     */
    QVariantMap toVariantMap() const override;

public: // Action getters

    PointPlotAction& getPointPlotAction() { return _pointPlotAction; }
    DensityPlotAction& getDensityPlotAction() { return _densityPlotAction; }

protected:
    ScatterplotPlugin*  _scatterplotPlugin;     /** Pointer to scatterplot plugin */
    PointPlotAction     _pointPlotAction;       /** Point plot action */
    DensityPlotAction   _densityPlotAction;     /** Density plot action */

    friend class Widget;
};

Q_DECLARE_METATYPE(PlotAction)

inline const auto PointPlotActionMetaTypeId = qRegisterMetaType<PlotAction*>("PlotAction");