#pragma once

#include "PluginAction.h"

#include <QHBoxLayout>
#include <QLabel>

using namespace hdps::gui;

class PositionAction : public PluginAction
{
protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, PositionAction* positionAction, const WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    PositionAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    void setDimensions(const std::uint32_t& numberOfDimensions, const std::vector<QString>& dimensionNames = std::vector<QString>());
    void setDimensions(const std::vector<QString>& dimensionNames);

    std::int32_t getXDimension() const;
    std::int32_t getYDimension() const;

protected:
    OptionAction    _xDimensionAction;
    OptionAction    _yDimensionAction;

    friend class Widget;
};