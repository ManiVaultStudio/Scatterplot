#include "ColoringAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include "PointData.h"

using namespace hdps::gui;

ColoringAction::ColoringAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _colorByAction(this, "Color by", { "Constant", "Data" }, "Constant", "Constant"),
    _colorByConstantTriggerAction(this, "Color by constant color"),
    _colorByDataTriggerAction(this, "Color by data"),
    _colorByActionGroup(this),
    _colorByConstantAction(scatterplotPlugin),
    _colorByDataAction(scatterplotPlugin),
    _colorMapAction(this, "Color map")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("palette"));

    scatterplotPlugin->addAction(&_colorByAction);
    scatterplotPlugin->addAction(&_colorByDataAction);
    scatterplotPlugin->addAction(&_colorByConstantTriggerAction);
    scatterplotPlugin->addAction(&_colorByDataTriggerAction);
    scatterplotPlugin->addAction(&_colorByDataTriggerAction);

    _colorByAction.setToolTip("Color by");
    _colorByConstantTriggerAction.setToolTip("Color data points with a constant color");
    _colorByConstantAction.setToolTip("Color by constant");
    _colorByDataTriggerAction.setToolTip("Color by data");
    _colorByDataAction.setToolTip("Color by data");

    _colorByConstantTriggerAction.setCheckable(true);
    _colorByDataTriggerAction.setCheckable(true);

    _colorByActionGroup.addAction(&_colorByConstantTriggerAction);
    _colorByActionGroup.addAction(&_colorByDataTriggerAction);

    const auto updateColoringMode = [this]() {
        getScatterplotWidget()->setColoringMode(static_cast<ScatterplotWidget::ColoringMode>(_colorByAction.getCurrentIndex()));
    };

    connect(&_colorByAction, &OptionAction::currentIndexChanged, this, [this, updateColoringMode](const std::uint32_t& currentIndex) {
        updateColoringMode();
    });

    connect(&_colorByConstantTriggerAction, &QAction::triggered, this, [this]() {
        getScatterplotWidget()->setColoringMode(ScatterplotWidget::ColoringMode::ConstantColor);
    });

    connect(&_colorByDataTriggerAction, &QAction::triggered, this, [this]() {
        getScatterplotWidget()->setColoringMode(ScatterplotWidget::ColoringMode::Data);
    });

    const auto updateColorMap = [this]() -> void {
        switch (getScatterplotWidget()->getRenderMode())
        {
            case ScatterplotWidget::RenderMode::SCATTERPLOT:
            {
                if (getScatterplotWidget()->getColoringMode() == ScatterplotWidget::ColoringMode::Data)
                    getScatterplotWidget()->setColorMap(_colorMapAction.getColorMapImage());

                break;
            }

            case ScatterplotWidget::RenderMode::LANDSCAPE:
            {
                getScatterplotWidget()->setColorMap(_colorMapAction.getColorMapImage());

                break;
            }

            default:
                break;
        }
    };

    connect(&_colorMapAction, &ColorMapAction::imageChanged, this, [this, updateColorMap](const QImage& image) {
        updateColorMap();
    });

    const auto updateActions = [this]() -> void {
        const auto coloringMode = getScatterplotWidget()->getColoringMode();
        const auto renderMode   = getScatterplotWidget()->getRenderMode();

        _colorByAction.setCurrentIndex(static_cast<std::int32_t>(coloringMode));

        _colorByConstantTriggerAction.setChecked(coloringMode == ScatterplotWidget::ColoringMode::ConstantColor);
        _colorByDataTriggerAction.setChecked(coloringMode == ScatterplotWidget::ColoringMode::Data);
        _colorMapAction.setEnabled(renderMode == ScatterplotWidget::LANDSCAPE || coloringMode == ScatterplotWidget::ColoringMode::Data);
    };

    const auto updateColorMapRange = [this]() {
        auto& rangeAction = _colorMapAction.getSettingsAction().getHorizontalAxisAction().getRangeAction();
        getScatterplotWidget()->setColorMapRange(rangeAction.getMinimum(), rangeAction.getMaximum());
    };

    connect(&_colorMapAction.getSettingsAction().getHorizontalAxisAction().getRangeAction(), &DecimalRangeAction::rangeChanged, this, [this, updateColorMapRange](const float& minimum, const float& maximum) {
        updateColorMapRange();
    });

    connect(&_colorByAction, &OptionAction::currentIndexChanged, this, [this, updateActions, updateColorMap](const std::uint32_t& currentIndex) {
        updateActions();
        updateColorMap();
    });

    connect(getScatterplotWidget(), &ScatterplotWidget::coloringModeChanged, this, [this, updateColorMap](const ScatterplotWidget::ColoringMode& coloringMode) {
        updateColorMap();
    });

    connect(getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, updateActions, updateColorMap](const ScatterplotWidget::RenderMode& renderMode) {
        updateActions();
        updateColorMap();
    });

    connect(&_scatterplotPlugin->getPositionDataset(), &DatasetRef<Points>::changed, this, [this, updateActions, updateColorMap](DataSet* dataset) {
        _colorByAction.reset();
        _colorByConstantAction.reset();
        _colorByDataAction.reset();

        updateActions();
        updateColorMap();
    });

    updateColoringMode();
    updateActions();
}

QMenu* ColoringAction::getContextMenu(QWidget* parent /*= nullptr*/)
{
    auto menu = new QMenu("Color", parent);

    const auto addActionToMenu = [menu](QAction* action) -> void {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    const auto renderMode = _scatterplotPlugin->getScatterplotWidget()->getRenderMode();

    menu->setEnabled(renderMode == ScatterplotWidget::RenderMode::SCATTERPLOT);

    menu->addAction(&_colorByConstantAction);
    menu->addAction(&_colorByDataAction);
    
    menu->addSeparator();

    switch (_scatterplotPlugin->getScatterplotWidget()->getColoringMode())
    {
        case ScatterplotWidget::ColoringMode::ConstantColor:
            menu->addMenu(_colorByConstantAction.getContextMenu());
            break;

        case ScatterplotWidget::ColoringMode::Data:
            menu->addMenu(_colorByDataAction.getContextMenu());
            break;

        default:
            break;
    }

    return menu;
}

ColoringAction::Widget::Widget(QWidget* parent, ColoringAction* coloringAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, coloringAction, widgetFlags)
{
    auto layout = new QHBoxLayout();

    auto stackedWidget = new StackedWidget();

    stackedWidget->addWidget(coloringAction->_colorByConstantAction.createWidget(this));
    stackedWidget->addWidget(coloringAction->_colorByDataAction.createWidget(this));

    const auto coloringModeChanged = [stackedWidget, coloringAction]() -> void {
        stackedWidget->setCurrentIndex(coloringAction->_colorByAction.getCurrentIndex());
    };

    connect(&coloringAction->_colorByAction, &OptionAction::currentIndexChanged, this, [this, coloringModeChanged](const std::uint32_t& currentIndex) {
        coloringModeChanged();
    });

    const auto renderModeChanged = [this, coloringAction]() {
        setEnabled(coloringAction->getScatterplotWidget()->getRenderMode() == ScatterplotWidget::SCATTERPLOT);
    };

    connect(coloringAction->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, renderModeChanged](const ScatterplotWidget::RenderMode& renderMode) {
        renderModeChanged();
    });

    renderModeChanged();
    coloringModeChanged();

    auto labelWidget    = coloringAction->_colorByAction.createLabelWidget(this);
    auto optionWidget   = coloringAction->_colorByAction.createWidget(this);

    // Adjust size to the contents of the color by combobox widget
    optionWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    if (widgetFlags & PopupLayout) {
        auto layout = new QGridLayout();

        layout->addWidget(labelWidget, 0, 0);
        layout->addWidget(optionWidget, 0, 1);
        layout->addWidget(stackedWidget, 0, 2);

        setPopupLayout(layout);
    }
    else {
        auto layout = new QHBoxLayout();

        layout->setMargin(0);
        layout->addWidget(labelWidget);
        layout->addWidget(optionWidget);
        layout->addWidget(stackedWidget);

        setLayout(layout);
    }
}
