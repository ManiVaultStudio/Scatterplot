#pragma once

#include "PluginAction.h"

class ColorDimensionAction : public PluginAction
{
protected: // Widget

    class Widget : public PluginAction::Widget {
    public:
        Widget(QWidget* parent, ColorDimensionAction* colorDimensionAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    ColorDimensionAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    void setDimensions(const std::uint32_t& numberOfDimensions, const std::vector<QString>& dimensionNames = std::vector<QString>());
    void setDimensions(const std::vector<QString>& dimensionNames);

protected:
    hdps::gui::OptionAction _colorDimensionAction;

    friend class Widget;
};