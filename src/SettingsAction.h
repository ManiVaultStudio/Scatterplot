#pragma once

#include "PluginAction.h"

#include "RenderModeAction.h"
#include "PlotAction.h"
#include "PositionAction.h"
#include "ColoringAction.h"
#include "SubsetAction.h"
#include "SelectionAction.h"
#include "ManualClusteringAction.h"
#include "MiscellaneousAction.h"

class ScatterplotPlugin;

class SettingsAction : public PluginAction
{
public:
    class SpacerWidget : public QWidget {
    public:
        enum class Type {
            Divider,
            Spacer
        };

    public:
        SpacerWidget(const Type& type = Type::Divider);

        static Type getType(const hdps::gui::WidgetActionWidget::State& widgetTypeLeft, const hdps::gui::WidgetActionWidget::State& widgetTypeRight);
        static Type getType(const hdps::gui::WidgetActionStateWidget* stateWidgetLeft, const hdps::gui::WidgetActionStateWidget* stateWidgetRight);

        void setType(const Type& type);
        static std::int32_t getWidth(const Type& type);

    protected:
        Type            _type;
        QHBoxLayout*    _layout;
        QFrame*         _verticalLine;
    };

protected: // Widget

    class Widget : public hdps::gui::WidgetActionWidget {
    public:
        Widget(QWidget* parent, SettingsAction* settingsAction);

        bool eventFilter(QObject* object, QEvent* event);

    protected:
        void addStateWidget(WidgetAction* widgetAction, const std::int32_t& priority = 0);

    private:
        void updateLayout();

    protected:
        QHBoxLayout                                     _layout;
        QWidget                                         _toolBarWidget;
        QHBoxLayout                                     _toolBarLayout;
        QVector<hdps::gui::WidgetActionStateWidget*>    _stateWidgets;
        QVector<SpacerWidget*>                          _spacerWidgets;

        friend class SettingsAction;
    };

    QWidget* getWidget(QWidget* parent, const hdps::gui::WidgetActionWidget::State& state = hdps::gui::WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this);
    };

public:
    SettingsAction(ScatterplotPlugin* scatterplotPlugin);

    QMenu* getContextMenu();

    RenderModeAction& getRenderModeAction() { return _renderModeAction; }
    PlotAction& getPlotAction() { return _plotAction; }
    PositionAction& getPositionAction() { return _positionAction; }
    ColoringAction& getColoringAction() { return _coloringAction; }
    SubsetAction& getSubsetAction() { return _subsetAction; }
    SelectionAction& getSelectionAction() { return _selectionAction; }
    MiscellaneousAction& getMiscellaneousAction() { return _miscellaneousAction; }

protected:
    RenderModeAction        _renderModeAction;
    PlotAction              _plotAction;
    PositionAction          _positionAction;
    ColoringAction          _coloringAction;
    SubsetAction            _subsetAction;
    ManualClusteringAction           _clusterAction;
    SelectionAction         _selectionAction;
    MiscellaneousAction     _miscellaneousAction;
};