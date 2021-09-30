#pragma once

#include "PluginAction.h"

using namespace hdps::gui;

class ColorDimensionAction : public PluginAction
{
protected: // Widget

    class Widget : public ::WidgetActionWidget {
    public:
        Widget(QWidget* parent, ColorDimensionAction* colorDimensionAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    ColorDimensionAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    void setDimensions(const std::uint32_t& numberOfDimensions, const std::vector<QString>& dimensionNames = std::vector<QString>());
    void setDimensions(const std::vector<QString>& dimensionNames);

    OptionAction& getCurrentDimensionAction() { return _currentDimensionAction; }

protected:
    OptionAction _currentDimensionAction;

    friend class Widget;
};