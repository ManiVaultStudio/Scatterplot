#pragma once

#include "PluginAction.h"

#include "PointSizeByModel.h"

#include "PointsDimensionPickerAction.h"

#include <QLabel>

using namespace hdps::gui;

class PointPlotAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, PointPlotAction* pointPlotAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:
    PointPlotAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    /**
     * Add point size dataset
     * @param pointSizeDataset Smart pointer to point size dataset
     */
    void addPointSizeDataset(const Dataset<DatasetImpl>& pointSizeDataset);

    /** Get smart pointer to current point size dataset (if any) */
    Dataset<DatasetImpl> getCurrentPointSizeDataset() const;

protected:

    /** Update the size by action options */
    void updateSizeByActionOptions();

    /** Update the scatter plot widget point size */
    void updateScatterPlotWidgetPointSize();

public: // Action getters

    OptionAction& getSizeByAction() { return _sizeByAction; }
    DecimalAction& getSizeAction() { return _sizeAction; }
    PointsDimensionPickerAction& getPointSizeDimensionAction() { return _pointSizeDimensionAction; }
    DecimalAction& getOpacityAction() { return _opacityAction; }

protected:
    PointSizeByModel                _sizeByModel;                   /** Point size by model */
    OptionAction                    _sizeByAction;                  /** Point size by action */
    PointsDimensionPickerAction     _pointSizeDimensionAction;      /** Point size dimension picker action */
    DecimalAction                   _sizeAction;                    /** Point size action */
    DecimalAction                   _opacityAction;                 /** Point opacity action */
    Datasets                        _pointSizeDatasets;             /** Vector of datasets that can serve as input for point size */

    static constexpr double DEFAULT_POINT_SIZE      = 10.0;     /** Default point size */
    static constexpr double DEFAULT_POINT_OPACITY   = 50.0;     /** Default point opacity */

    friend class PlotAction;
};