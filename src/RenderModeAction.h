#pragma once

#include <actions/OptionAction.h>
#include <actions/ToggleAction.h>

using namespace hdps::gui;

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

    QMenu* getContextMenu();

public: // Linking

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

protected:
    ToggleAction    _scatterPlotAction;
    ToggleAction    _densityPlotAction;
    ToggleAction    _contourPlotAction;
};

Q_DECLARE_METATYPE(RenderModeAction)

inline const auto renderModeActionMetaTypeId = qRegisterMetaType<RenderModeAction*>("RenderModeAction");