#pragma once

#include "PluginAction.h"

#include "ConstantColorAction.h"
#include "ColorDimensionAction.h"
#include "ColorDataAction.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>

using namespace hdps::gui;

class ColoringAction : public PluginAction
{
protected: // Widget
    class StackedWidget : public QStackedWidget {
    public:
        QSize sizeHint() const override { return currentWidget()->sizeHint(); }
        QSize minimumSizeHint() const override { return currentWidget()->minimumSizeHint(); }
    };

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, ColoringAction* coloringAction, const std::int32_t& widgetFlags);
    };

    QWidget* getWidget(QWidget* parent, const std::int32_t& widgetFlags) override {
        return new Widget(parent, this, widgetFlags);
    };

public:
    ColoringAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();
    
    OptionAction& getColorByAction() { return _colorByAction; }
    ConstantColorAction& getConstantColorAction() { return _constantColorAction; }
    ColorDimensionAction& getColorDimensionAction() { return _colorDimensionAction; }
    ColorDataAction& getColorDataAction() { return _colorDataAction; }
    ColorMapAction& getColorMapAction() { return _colorMapAction; }

    void setDimensions(const std::uint32_t& numberOfDimensions, const std::vector<QString>& dimensionNames = std::vector<QString>());
    void setDimensions(const std::vector<QString>& dimensionNames);

protected:
    OptionAction            _colorByAction;
    TriggerAction           _colorByConstantColorAction;
    TriggerAction           _colorByDimensionAction;
    TriggerAction           _colorByColorDataAction;
    QActionGroup            _colorByActionGroup;
    ConstantColorAction     _constantColorAction;
    ColorDimensionAction    _colorDimensionAction;
    ColorDataAction         _colorDataAction;
    ColorMapAction          _colorMapAction;

    friend class Widget;
};