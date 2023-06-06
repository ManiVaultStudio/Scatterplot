#pragma once

#include <actions/GroupAction.h>
#include <actions/DecimalAction.h>
#include <actions/ToggleAction.h>

using namespace hdps::gui;

class ScatterplotPlugin;

/**
 * Density plot action class
 *
 * Action class for configuring density plot settings
 *
 * @author Thomas Kroes
 */
class DensityPlotAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE DensityPlotAction(QObject* parent, const QString& title);

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

    /**
     * Override to show/hide child actions
     * @param visible Whether the action is visible or not
     */
    void setVisible(bool visible);

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

    DecimalAction& getSigmaAction() { return _sigmaAction; }
    ToggleAction& getContinuousUpdatesAction() { return _continuousUpdatesAction; }

protected:
    ScatterplotPlugin*  _scatterplotPlugin;         /** Pointer to scatterplot plugin */
    DecimalAction       _sigmaAction;               /** Density sigma action */
    ToggleAction        _continuousUpdatesAction;   /** Live updates action */

    static constexpr double DEFAULT_SIGMA = 0.15f;
    static constexpr bool DEFAULT_CONTINUOUS_UPDATES = true;

    friend class Widget;
    friend class PlotAction;
};