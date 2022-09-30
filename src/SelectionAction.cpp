#include "SelectionAction.h"
#include "Application.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include "renderers/PointRenderer.h"

#include "util/PixelSelectionTool.h"

#include <QHBoxLayout>
#include <QPushButton>

using namespace hdps::gui;

const auto allowedPixelSelectionTypes = PixelSelectionTypes({
    PixelSelectionType::Rectangle,
    PixelSelectionType::Brush,
    PixelSelectionType::Lasso,
    PixelSelectionType::Polygon
});

SelectionAction::SelectionAction(ScatterplotPlugin& scatterplotPlugin) :
    PixelSelectionAction(&scatterplotPlugin, &scatterplotPlugin.getScatterplotWidget(), scatterplotPlugin.getScatterplotWidget().getPixelSelectionTool(), allowedPixelSelectionTypes),
    _scatterplotPlugin(scatterplotPlugin),
    _outlineEnabledAction(this, "Show selection", true, true),
    _outlineOverrideColorAction(this, "Custom color", true, true),
    _outlineScaleAction(this, "Scale", 100.0f, 500.0f, 200.0f, 200.0f, 1),
    _outlineOpacityAction(this, "Opacity", 0.0f, 100.0f, 100.0f, 100.0f, 1),
    _haloEnabledAction(this, "Halo")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("mouse-pointer"));
    
    _outlineScaleAction.setSuffix("%");
    _outlineOpacityAction.setSuffix("%");

    _outlineEnabledAction.setChecked(_scatterplotPlugin.getScatterplotWidget().getSelectionOutlineEnabled());
    _outlineScaleAction.setValue(100.0f * _scatterplotPlugin.getScatterplotWidget().getSelectionOutlineScale());
    _outlineOpacityAction.setValue(100.0f * _scatterplotPlugin.getScatterplotWidget().getSelectionOutlineOpacity());

    _haloEnabledAction.setChecked(_scatterplotPlugin.getScatterplotWidget().getSelectionOutlineHaloEnabled());
    _outlineOverrideColorAction.setChecked(_scatterplotPlugin.getScatterplotWidget().getSelectionOutlineOverrideColor());

    connect(&getSelectAllAction(), &QAction::triggered, [this]() {
        if (_scatterplotPlugin.getPositionDataset().isValid())
            _scatterplotPlugin.getPositionDataset()->selectAll();
    });

    connect(&getClearSelectionAction(), &QAction::triggered, [this]() {
        if (_scatterplotPlugin.getPositionDataset().isValid())
            _scatterplotPlugin.getPositionDataset()->selectNone();
    });

    connect(&getInvertSelectionAction(), &QAction::triggered, [this]() {
        if (_scatterplotPlugin.getPositionDataset().isValid())
            _scatterplotPlugin.getPositionDataset()->selectInvert();
    });

    connect(&_outlineScaleAction, &DecimalAction::valueChanged, [this](float value) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineScale(0.01f * value);
    });

    connect(&_outlineOpacityAction, &DecimalAction::valueChanged, [this](float value) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineOpacity(0.01f * value);
    });

    connect(&_haloEnabledAction, &ToggleAction::toggled, [this](bool toggled) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineHaloEnabled(toggled);
    });

    connect(&getOverlayColorAction(), &ColorAction::colorChanged, [this](const QColor& color) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineColor(color);
    });

    const auto updateActionsReadOnly = [this]() -> void {
        _outlineOverrideColorAction.setEnabled(_outlineEnabledAction.isChecked());
        getOverlayColorAction().setEnabled(_outlineEnabledAction.isChecked() && _outlineOverrideColorAction.isChecked());
        _outlineScaleAction.setEnabled(_outlineEnabledAction.isChecked());
        _outlineOpacityAction.setEnabled(_outlineEnabledAction.isChecked());
        _haloEnabledAction.setEnabled(_outlineEnabledAction.isChecked());
    };

    connect(&_outlineEnabledAction, &ToggleAction::toggled, [this, updateActionsReadOnly](bool toggled) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineEnabled(toggled);
        updateActionsReadOnly();
    });

    connect(&_outlineOverrideColorAction, &ToggleAction::toggled, [this, updateActionsReadOnly](bool toggled) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineOverrideColor(toggled);
        updateActionsReadOnly();
    });

    updateActionsReadOnly();
}

SelectionAction::Widget::Widget(QWidget* parent, SelectionAction* selectionAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, selectionAction, widgetFlags)
{
    if (widgetFlags & PopupLayout) {
        const auto getTypeWidget = [&, this]() -> QWidget* {
            auto layout = new QHBoxLayout();

            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(selectionAction->getTypeAction().createWidget(this));
            layout->addWidget(selectionAction->getModifierAddAction().createWidget(this, ToggleAction::PushButtonIcon));
            layout->addWidget(selectionAction->getModifierSubtractAction().createWidget(this, ToggleAction::PushButtonIcon));
            layout->itemAt(0)->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            auto widget = new QWidget();

            widget->setLayout(layout);

            return widget;
        };

        const auto getSelectWidget = [&, this]() -> QWidget* {
            auto layout = new QHBoxLayout();

            layout->setContentsMargins(0, 0, 0, 0);
            layout->addWidget(selectionAction->getClearSelectionAction().createWidget(this));
            layout->addWidget(selectionAction->getSelectAllAction().createWidget(this));
            layout->addWidget(selectionAction->getInvertSelectionAction().createWidget(this));
            layout->addStretch(1);

            auto widget = new QWidget();

            widget->setLayout(layout);

            return widget;
        };

        auto layout = new QGridLayout();

        layout->addWidget(selectionAction->getTypeAction().createLabelWidget(this), 0, 0);
        layout->addWidget(getTypeWidget(), 0, 1);
        layout->addWidget(selectionAction->_brushRadiusAction.createLabelWidget(this), 1, 0);
        layout->addWidget(selectionAction->getBrushRadiusAction().createWidget(this), 1, 1);
        layout->addWidget(getSelectWidget(), 2, 1);
        layout->addWidget(selectionAction->getNotifyDuringSelectionAction().createWidget(this), 3, 1);
        
        layout->addWidget(selectionAction->getOutlineEnabledAction().createWidget(this), 4, 1);
        layout->addWidget(selectionAction->getOverlayColorAction().createLabelWidget(this), 5, 0);
        layout->addWidget(selectionAction->getOverlayColorAction().createWidget(this), 5, 1);
        layout->addWidget(selectionAction->getOutlineOverrideColorAction().createWidget(this), 6, 1);
        layout->addWidget(selectionAction->getOutlineScaleAction().createLabelWidget(this), 7, 0);
        layout->addWidget(selectionAction->getOutlineScaleAction().createWidget(this), 7, 1);
        layout->addWidget(selectionAction->getOutlineOpacityAction().createLabelWidget(this), 8, 0);
        layout->addWidget(selectionAction->getOutlineOpacityAction().createWidget(this), 8, 1);

        layout->addWidget(selectionAction->getHaloEnabledAction().createWidget(this), 9, 1);

        layout->itemAtPosition(1, 1)->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

        setPopupLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        layout->setContentsMargins(0, 0, 0, 0);

        layout->addWidget(selectionAction->getTypeAction().createWidget(this));
        layout->addWidget(selectionAction->getBrushRadiusAction().createWidget(this));
        layout->addWidget(selectionAction->getModifierAddAction().createWidget(this, ToggleAction::PushButtonIcon));
        layout->addWidget(selectionAction->getModifierSubtractAction().createWidget(this, ToggleAction::PushButtonIcon));
        layout->addWidget(selectionAction->getClearSelectionAction().createWidget(this));
        layout->addWidget(selectionAction->getSelectAllAction().createWidget(this));
        layout->addWidget(selectionAction->getInvertSelectionAction().createWidget(this));
        layout->addWidget(selectionAction->getNotifyDuringSelectionAction().createWidget(this));
        layout->addWidget(selectionAction->getOutlineEnabledAction().createWidget(this));
        layout->addWidget(selectionAction->getOutlineScaleAction().createWidget(this));
        layout->addWidget(selectionAction->getOutlineOpacityAction().createWidget(this));
        layout->addWidget(selectionAction->getOverlayColorAction().createWidget(this));

        setLayout(layout);
    }
}
