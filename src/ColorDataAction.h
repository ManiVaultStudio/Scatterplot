#pragma once

#include "PluginAction.h"

class ColorDataAction : public PluginAction
{

protected: // Widget

    class Widget : public hdps::gui::WidgetActionWidget
    {
    protected:
        Widget(QWidget* parent, ColorDataAction* colorDataAction, const Widget::State& state);

        friend class ColorDataAction;
    };

    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    ColorDataAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    hdps::gui::StringAction& getDatasetNameAction() { return _datasetNameAction; }

protected:
    hdps::gui::StringAction     _datasetNameAction;

    friend class Widget;
};