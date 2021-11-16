#pragma once

#include "PluginAction.h"

#include "ColorByConstantAction.h"
#include "ColorByDimensionAction.h"
#include "ColorByClustersAction.h"

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
    ColorByConstantAction& getConstantColorAction() { return _colorByConstantAction; }
    ColorByDimensionAction& getColorDimensionAction() { return _colorByDimensionAction; }
    ColorByClustersAction& getColorDataAction() { return _colorByClustersAction; }
    ColorMapAction& getColorMapAction() { return _colorMapAction; }

protected:
    OptionAction                _colorByAction;                         /** Action for picking the coloring type */
    TriggerAction               _colorByConstantColorTriggerAction;     /** Color by constant color trigger action (for key shortcut) */
    TriggerAction               _colorByDimensionTriggerAction;         /** Color by dimension trigger action (for key shortcut) */
    TriggerAction               _colorByClustersTriggerAction;          /** Color by cluster data trigger action (for key shortcut) */
    QActionGroup                _colorByActionGroup;                    /** Color by action group */
    ColorByConstantAction       _colorByConstantAction;                 /** Color by constant action */
    ColorByDimensionAction      _colorByDimensionAction;                /** Color by dimension action */
    ColorByClustersAction       _colorByClustersAction;                 /** Color by clusters action */
    ColorMapAction              _colorMapAction;                        /** Color map action */

    friend class Widget;
};