#pragma once

#include <actions/WidgetAction.h>
#include <actions/DatasetPickerAction.h>

using namespace hdps::gui;

class LoadedDatasetsAction : public WidgetAction
{
    Q_OBJECT

protected:

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, LoadedDatasetsAction* currentDatasetAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE LoadedDatasetsAction(QObject* parent, const QString& title);

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
    DatasetPickerAction	    _positionDatasetPickerAction;
    DatasetPickerAction     _colorDatasetPickerAction;

    friend class Widget;
};

Q_DECLARE_METATYPE(LoadedDatasetsAction)

inline const auto loadedDatasetsActionMetaTypeId = qRegisterMetaType<LoadedDatasetsAction*>("LoadedDatasetsAction");