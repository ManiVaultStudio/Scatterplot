#pragma once

#include <actions/WidgetAction.h>
#include <actions/DecimalRangeAction.h>

#include "ScalarSourceModel.h"

#include <PointData/DimensionPickerAction.h>

using namespace hdps::gui;

/**
 * Scalar source action class
 *
 * Action class picking a scalar source (scalar by constant or dataset dimension)
 *
 * @author Thomas Kroes
 */
class ScalarSourceAction : public WidgetAction
{
    Q_OBJECT

protected: // Widget

    /** Widget class for scalar source action */
    class Widget : public WidgetActionWidget {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param scalarSourceAction Pointer to scalar source action
         */
        Widget(QWidget* parent, ScalarSourceAction* scalarSourceAction);
    };

protected:

    /**
     * Get widget representation of the scalar source action
     * @param parent Pointer to parent widget
     * @param widgetFlags Widget flags for the configuration of the widget (type)
     */
    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this);
    };

public:

    /**
     * Constructor
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE ScalarSourceAction(QObject* parent, const QString& title);

    /** Get the scalar source model */
    ScalarSourceModel& getModel();

    /** Update scalar range */
    void updateScalarRange();

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

public: // Action getters

    OptionAction& getPickerAction() { return _pickerAction; }
    DimensionPickerAction& getDimensionPickerAction() { return _dimensionPickerAction; }
    DecimalAction& getOffsetAction() { return _offsetAction; }
    DecimalRangeAction& getRangeAction() { return _rangeAction; }

    const OptionAction& getPickerAction() const { return _pickerAction; }
    const DimensionPickerAction& getDimensionPickerAction() const { return _dimensionPickerAction; }
    const DecimalAction& getOffsetAction() const { return _offsetAction; }
    const DecimalRangeAction& getRangeAction() const { return _rangeAction; }

signals:

    /**
     * Signals that the scalar range changed
     * @param minimum Scalar range minimum
     * @param maximum Scalar range maximum
     */
    void scalarRangeChanged(const float& minimum, const float& maximum);

protected:
    ScalarSourceModel       _model;                     /** Scalar model */
    OptionAction            _pickerAction;              /** Source picker action */
    DimensionPickerAction   _dimensionPickerAction;     /** Dimension picker action */
    DecimalAction           _offsetAction;              /** Scalar source offset action */
    DecimalRangeAction      _rangeAction;               /** Range action */
};

Q_DECLARE_METATYPE(ScalarSourceAction)

inline const auto scalarSourceActionMetaTypeId = qRegisterMetaType<ScalarSourceAction*>("ScalarSourceAction");