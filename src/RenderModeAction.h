#pragma once

#include <actions/OptionAction.h>
#include <actions/ToggleAction.h>

using namespace hdps::gui;

class ScatterplotPlugin;

/**
 * Render mode action class
 *
 * Action class for configuring render mode settings
 *
 * @author Thomas Kroes
 */
class RenderModeAction : public OptionAction
{
    Q_OBJECT
    
    enum class RenderMode {
        ScatterPlot,
        DensityPlot,
        ContourPlot,
    };

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE RenderModeAction(QObject* parent, const QString& title);

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

protected: // Linking

    /**
     * Connect this action to a public action
     * @param publicAction Pointer to public action to connect to
     * @param recursive Whether to also connect descendant child actions
     */
    void connectToPublicAction(WidgetAction* publicAction, bool recursive) override;

    /**
     * Disconnect this action from its public action
     * @param recursive Whether to also disconnect descendant child actions
     */
    void disconnectFromPublicAction(bool recursive) override;

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

    ToggleAction& getScatterPlotAction() { return _scatterPlotAction; }
    ToggleAction& getDensityPlotAction() { return _densityPlotAction; }
    ToggleAction& getContourPlotAction() { return _contourPlotAction; }

private:
    ScatterplotPlugin*  _scatterplotPlugin;     /** Pointer to scatterplot plugin */
    ToggleAction        _scatterPlotAction;     /** Trigger action for activating the scatter plot render mode */
    ToggleAction        _densityPlotAction;     /** Trigger action for activating the density plot render mode */
    ToggleAction        _contourPlotAction;     /** Trigger action for activating the contour plot render mode */

    friend class hdps::AbstractActionsManager;
};

Q_DECLARE_METATYPE(RenderModeAction)

inline const auto renderModeActionMetaTypeId = qRegisterMetaType<RenderModeAction*>("RenderModeAction");