#pragma once

#include <actions/WidgetAction.h>

#include "PointPlotAction.h"
#include "DensityPlotAction.h"

using namespace hdps::gui;

class ScatterplotWidget;

class PlotAction : public WidgetAction
{
    Q_OBJECT

protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, PlotAction* plotAction, const std::int32_t& widgetFlags);
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
    Q_INVOKABLE PlotAction(QObject* parent, const QString& title);

    ScatterplotWidget* getScatterplotWidget();

    QMenu* getContextMenu();

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

    PointPlotAction& getPointPlotAction() { return _pointPlotAction; }
    DensityPlotAction& getDensityPlotAction() { return _densityPlotAction; }

protected:
    PointPlotAction     _pointPlotAction;
    DensityPlotAction   _densityPlotAction;

    friend class Widget;
};

Q_DECLARE_METATYPE(PlotAction)

inline const auto PointPlotActionMetaTypeId = qRegisterMetaType<PlotAction*>("PlotAction");