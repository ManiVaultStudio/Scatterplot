#pragma once

#include "PluginAction.h"

#include "ConstantColorAction.h"
#include "ColorDimensionAction.h"
#include "ColorDataAction.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>

class ColoringAction : public PluginAction
{
protected: // Widget
    class StackedWidget : public QStackedWidget {
    public:
        QSize sizeHint() const override { return currentWidget()->sizeHint(); }
        QSize minimumSizeHint() const override { return currentWidget()->minimumSizeHint(); }
    };

    class Widget : public PluginAction::Widget {
    public:
        Widget(QWidget* parent, ColoringAction* coloringAction, const Widget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const Widget::State& state = Widget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:
    ColoringAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();
    
    ConstantColorAction& getConstantColorAction() { return _constantColorAction; }
    ColorDimensionAction& getColorDimensionAction() { return _colorDimensionAction; }
    ColorDataAction& getColorDataAction() { return _colorDataAction; }

    void setDimensions(const std::uint32_t& numberOfDimensions, const std::vector<QString>& dimensionNames = std::vector<QString>());
    void setDimensions(const std::vector<QString>& dimensionNames);

protected:
    hdps::gui::OptionAction     _colorByAction;
    hdps::gui::TriggerAction    _colorByConstantColorAction;
    hdps::gui::TriggerAction    _colorByDimensionAction;
    hdps::gui::TriggerAction    _colorByColorDataAction;
    QActionGroup                _colorByActionGroup;
    ConstantColorAction         _constantColorAction;
    ColorDimensionAction        _colorDimensionAction;
    ColorDataAction             _colorDataAction;

    friend class Widget;
};