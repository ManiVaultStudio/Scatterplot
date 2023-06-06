#pragma once

#include <actions/GroupAction.h>
#include <actions/StringAction.h>
#include <actions/TriggerAction.h>
#include <actions/OptionAction.h>

using namespace hdps::gui;

class ScatterplotPlugin;

/**
 * Subset action class
 *
 * Action class for creating a subset
 *
 * @author Thomas Kroes
 */
class SubsetAction : public GroupAction
{
    Q_OBJECT

public:
    
    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE SubsetAction(QObject* parent, const QString& title);

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

protected:
    ScatterplotPlugin*  _scatterplotPlugin;     /** Pointer to scatter plot plugin */
    StringAction        _subsetNameAction;      /** String action for configuring the subset name */
    OptionAction        _sourceDataAction;      /** Option action for picking the source dataset */
    TriggerAction       _createSubsetAction;    /** Triggers the subset creation process */
};

Q_DECLARE_METATYPE(SubsetAction)

inline const auto subsetActionMetaTypeId = qRegisterMetaType<SubsetAction*>("SubsetAction");