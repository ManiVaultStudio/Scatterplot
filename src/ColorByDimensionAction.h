#pragma once

#include "PluginAction.h"

#include "actions/DatasetPickerAction.h"

#include "PointsDimensionPickerAction.h"

using namespace hdps::gui;

class ColorByDimensionAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget
    {
    protected:
        Widget(QWidget* parent, ColorByDimensionAction* colorDataAction);

        friend class ColorByDimensionAction;
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this);
    };

public:
    ColorByDimensionAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    /** Update datasets in the pick dataset action */
    void updateDatasetPickerAction();

public: // Action getters

    DatasetPickerAction& getDatasetPickerAction() { return _datasetPickerAction; }
    PointsDimensionPickerAction& getPointsDimensionPickerAction() { return _pointsDimensionPickerAction; }

protected:
    DatasetPickerAction             _datasetPickerAction;           /** Dataset picker action */
    PointsDimensionPickerAction     _pointsDimensionPickerAction;   /** Dimension picker action */

    friend class Widget;
};