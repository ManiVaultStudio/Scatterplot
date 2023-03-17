#include "SelectionAction.h"
#include "Application.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

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
    _displayModeAction(this, "Display mode", { "Outline", "Override" }),
    _outlineOverrideColorAction(this, "Custom color", true, true),
    _outlineScaleAction(this, "Scale", 100.0f, 500.0f, 200.0f, 200.0f, 1),
    _outlineOpacityAction(this, "Opacity", 0.0f, 100.0f, 100.0f, 100.0f, 1),
    _outlineHaloEnabledAction(this, "Halo")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("mouse-pointer"));

    if (_scatterplotPlugin.getFactory()->getNumberOfInstances() == 0) {
        //_overlayColorAction.setConnectionPermissionsFlag(ConnectionPermissionFlag::All);
        getOverlayColorAction().publish("GlobalSelectionColor");
    }
    else {
        getOverlayColorAction().connectToPublicActionByName("GlobalSelectionColor");
    }

    _displayModeAction.setToolTip("The way in which selection is visualized");

    _outlineScaleAction.setSuffix("%");
    _outlineOpacityAction.setSuffix("%");

    _displayModeAction.setCurrentIndex(static_cast<std::int32_t>(_scatterplotPlugin.getScatterplotWidget().getSelectionDisplayMode()));
    _outlineScaleAction.setValue(100.0f * _scatterplotPlugin.getScatterplotWidget().getSelectionOutlineScale());
    _outlineOpacityAction.setValue(100.0f * _scatterplotPlugin.getScatterplotWidget().getSelectionOutlineOpacity());

    _outlineHaloEnabledAction.setChecked(_scatterplotPlugin.getScatterplotWidget().getSelectionOutlineHaloEnabled());
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

    connect(&_outlineHaloEnabledAction, &ToggleAction::toggled, [this](bool toggled) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineHaloEnabled(toggled);
    });

    connect(&getOverlayColorAction(), &ColorAction::colorChanged, [this](const QColor& color) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionOutlineColor(color);
    });

    getOverlayColorAction().setText("Color");

    const auto updateActionsReadOnly = [this]() -> void {
        const auto isOutline = _scatterplotPlugin.getScatterplotWidget().getSelectionDisplayMode() == PointSelectionDisplayMode::Outline;

        _outlineScaleAction.setEnabled(isOutline);
        _outlineOpacityAction.setEnabled(isOutline);
        _outlineHaloEnabledAction.setEnabled(isOutline);
    };

    connect(&_displayModeAction, &OptionAction::currentIndexChanged, [this, updateActionsReadOnly](const std::int32_t& currentIndex) {
        _scatterplotPlugin.getScatterplotWidget().setSelectionDisplayMode(static_cast<PointSelectionDisplayMode>(currentIndex));
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
        
        layout->addWidget(selectionAction->getOverlayColorAction().createLabelWidget(this), 5, 0);
        layout->addWidget(selectionAction->getOverlayColorAction().createWidget(this), 5, 1);

        layout->addWidget(selectionAction->getDisplayModeAction().createLabelWidget(this), 6, 0);
        layout->addWidget(selectionAction->getDisplayModeAction().createWidget(this), 6, 1);

        //layout->addWidget(selectionAction->getOutlineOverrideColorAction().createWidget(this), 7, 1);
        layout->addWidget(selectionAction->getOutlineScaleAction().createLabelWidget(this), 8, 0);
        layout->addWidget(selectionAction->getOutlineScaleAction().createWidget(this), 8, 1);
        layout->addWidget(selectionAction->getOutlineOpacityAction().createLabelWidget(this), 9, 0);
        layout->addWidget(selectionAction->getOutlineOpacityAction().createWidget(this), 9, 1);

        layout->addWidget(selectionAction->getOutlineHaloEnabledAction().createWidget(this), 10, 1);

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
        layout->addWidget(selectionAction->getDisplayModeAction().createWidget(this));
        layout->addWidget(selectionAction->getOutlineScaleAction().createWidget(this));
        layout->addWidget(selectionAction->getOutlineOpacityAction().createWidget(this));
        layout->addWidget(selectionAction->getOverlayColorAction().createWidget(this));

        setLayout(layout);
    }
}
