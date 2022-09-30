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
    _outlineEnabledAction(this, "Show outline", true, true),
    _outlineScaleAction(this, "Outline scale", 0.0f, 500.0f, 120.0f, 120.0f, 2),
    _haloEnabledAction(this, "Show halo"),
    _haloScaleAction(this, "Halo scale", 100.0f, 1000.0f, 200.0f, 200.0f, 2)
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("mouse-pointer"));
    
    _outlineScaleAction.setSuffix("%");
    _haloScaleAction.setSuffix("%");

    _outlineEnabledAction.setChecked(_scatterplotPlugin.getScatterplotWidget().getSelectionOutlineEnabled());
    _outlineScaleAction.setValue(100.0f * _scatterplotPlugin.getScatterplotWidget().getSelectionOutlineScale());

    _haloEnabledAction.setChecked(_scatterplotPlugin.getScatterplotWidget().getSelectionHaloEnabled());
    _haloScaleAction.setValue(100.0f * _scatterplotPlugin.getScatterplotWidget().getSelectionHaloScale());

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

    connect(&_outlineEnabledAction, &ToggleAction::toggled, [this](bool toggled) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineEnabled(toggled);
    });

    connect(&_outlineScaleAction, &DecimalAction::valueChanged, [this](float value) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineScale(0.01f * value);
    });

    connect(&_haloEnabledAction, &ToggleAction::toggled, [this](bool toggled) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionHaloEnabled(toggled);
    });

    connect(&_haloScaleAction, &DecimalAction::valueChanged, [this](float value) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionHaloScale(0.01f * value);
    });

    getOverlayColorAction().setText("Outline color");

    connect(&getOverlayColorAction(), &ColorAction::colorChanged, [this](const QColor& color) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineColor(color);
    });
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
        
        layout->addWidget(selectionAction->getOverlayColorAction().createLabelWidget(this), 4, 0);
        layout->addWidget(selectionAction->getOverlayColorAction().createWidget(this), 4, 1);
        
        layout->addWidget(selectionAction->getOutlineEnabledAction().createWidget(this), 5, 1);
        layout->addWidget(selectionAction->getOutlineScaleAction().createLabelWidget(this), 6, 0);
        layout->addWidget(selectionAction->getOutlineScaleAction().createWidget(this), 6, 1);

        layout->addWidget(selectionAction->getHaloEnabledAction().createWidget(this), 7, 1);
        layout->addWidget(selectionAction->getHaloScaleAction().createLabelWidget(this), 8, 0);
        layout->addWidget(selectionAction->getHaloScaleAction().createWidget(this), 8, 1);

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
        layout->addWidget(selectionAction->_outlineScaleAction.createWidget(this));

        setLayout(layout);
    }
}
