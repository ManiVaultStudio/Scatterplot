#pragma once

#include <actions/WidgetAction.h>

#include <PointData/DimensionPickerAction.h>

#include <QHBoxLayout>
#include <QLabel>

using namespace hdps::gui;

/**
 * Position action class
 *
 * Action class for picking data dimensions for the point positions
 *
 * @author Thomas Kroes
 */
class PositionAction : public WidgetAction
{
    Q_OBJECT

protected: // Widget

    /** Widget class for position action */
    class Widget : public WidgetActionWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param positionAction Pointer to position action
         */
        Widget(QWidget* parent, PositionAction* positionAction, const std::int32_t& widgetFlags);
    };

protected:

    /**
     * Get widget representation of the position action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:

    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE PositionAction(QObject* parent, const QString& title);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

    /** Get current x-dimension */
    std::int32_t getDimensionX() const;

    /** Get current y-dimension */
    std::int32_t getDimensionY() const;

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
    DimensionPickerAction    _xDimensionPickerAction;   /** X-dimension picker action */
    DimensionPickerAction    _yDimensionPickerAction;   /** Y-dimension picker action */

    friend class Widget;
};

Q_DECLARE_METATYPE(PositionAction)

inline const auto positionActionMetaTypeId = qRegisterMetaType<PositionAction*>("PositionAction");