#pragma once

#include "PluginAction.h"

#include "ScalarSourceModel.h"

#include "PointsDimensionPickerAction.h"

using namespace hdps::gui;

/**
 * Scalar source action class
 *
 * Action class picking a scalar source (scalar by constant or dataset dimension)
 *
 * @author Thomas Kroes
 */
class ScalarSourceAction : public PluginAction
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
     * @param scatterplotPlugin Pointer to scatter plot plugin
     */
    ScalarSourceAction(ScatterplotPlugin* scatterplotPlugin);

    /** Get the scalar source model */
    ScalarSourceModel& getModel();

    /** Update scalar range */
    void updateScalarRange();

public: // Action getters

    OptionAction& getPickerAction() { return _pickerAction; }
    PointsDimensionPickerAction& getDimensionPickerAction() { return _dimensionPickerAction; }
    DecimalRangeAction& getRangeAction() { return _rangeAction; }

signals:

    /**
     * Signals that the scalar range changed
     * @param minimum Scalar range minimum
     * @param maximum Scalar range maximum
     */
    void scalarRangeChanged(const float& minimum, const float& maximum);

protected:
    ScalarSourceModel               _model;                     /** Scalar model */
    OptionAction                    _pickerAction;              /** Source picker action */
    PointsDimensionPickerAction     _dimensionPickerAction;     /** Point size dimension picker action */
    DecimalRangeAction              _rangeAction;               /** Range action */
};