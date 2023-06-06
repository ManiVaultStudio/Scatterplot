#pragma once

#include <actions/GroupAction.h>
#include <actions/DatasetPickerAction.h>

using namespace hdps::gui;

class ScatterplotPlugin;

class DatasetsAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE DatasetsAction(QObject* parent, const QString& title);

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
    ScatterplotPlugin*      _scatterplotPlugin;                 /** Pointer to scatter plot plugin */
    DatasetPickerAction	    _positionDatasetPickerAction;       /** Dataset picker action for position dataset */
    DatasetPickerAction     _colorDatasetPickerAction;          /** Dataset picker action for color dataset */

    friend class Widget;
};

Q_DECLARE_METATYPE(DatasetsAction)

inline const auto datasetsActionMetaTypeId = qRegisterMetaType<DatasetsAction*>("DatasetsAction");