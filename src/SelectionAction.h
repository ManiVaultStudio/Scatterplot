#pragma once

#include <actions/GroupAction.h>
#include <actions/PixelSelectionAction.h>

class ScatterplotPlugin;

using namespace hdps::gui;

class SelectionAction : public GroupAction
{
    Q_OBJECT

public:
    
    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SelectionAction(QObject* parent, const QString& title);

    /**
     * Initialize the selection action with \p scatterplotPlugin
     * @param scatterplotPlugin Pointer to scatterplot plugin
     */
    void initialize(ScatterplotPlugin* scatterplotPlugin);

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
     * Load selection action from variant
     * @param Variant representation of the selection action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save selection action to variant
     * @return Variant representation of the selection action
     */
    QVariantMap toVariantMap() const override;

public: // Action getters
    
    PixelSelectionAction& getPixelSelectionAction() { return _pixelSelectionAction; }
    OptionAction& getDisplayModeAction() { return _displayModeAction; }
    ToggleAction& getOutlineOverrideColorAction() { return _outlineOverrideColorAction; }
    DecimalAction& getOutlineScaleAction() { return _outlineScaleAction; }
    DecimalAction& getOutlineOpacityAction() { return _outlineOpacityAction; }
    ToggleAction& getOutlineHaloEnabledAction() { return _outlineHaloEnabledAction; }

private:
    PixelSelectionAction    _pixelSelectionAction;          /** Pixel selection action */
    OptionAction            _displayModeAction;             /** Type of selection display (e.g. outline or override) */
    ToggleAction            _outlineOverrideColorAction;    /** Selection outline override color action */
    DecimalAction           _outlineScaleAction;            /** Selection outline scale action */
    DecimalAction           _outlineOpacityAction;          /** Selection outline opacity action */
    ToggleAction            _outlineHaloEnabledAction;      /** Selection outline halo enabled action */

    friend class hdps::AbstractActionsManager;
};

Q_DECLARE_METATYPE(SelectionAction)

inline const auto selectionActionMetaTypeId = qRegisterMetaType<SelectionAction*>("SelectionAction");