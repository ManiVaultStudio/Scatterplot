#include "ColoringAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

ColoringAction::ColoringAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _colorByAction(this, "Color by"),
    _colorByConstantColorAction(this, "Color by constant color"),
    _colorByDimensionAction(this, "Color by dimension"),
    _colorByColorDataAction(this, "Color by color data"),
    _colorByActionGroup(this),
    _constantColorAction(scatterplotPlugin),
    _colorDimensionAction(scatterplotPlugin),
    _colorDataAction(scatterplotPlugin),
    _colorMapAction(this, "Color map")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("palette"));

    scatterplotPlugin->addAction(&_colorByAction);
    scatterplotPlugin->addAction(&_colorByDimensionAction);
    scatterplotPlugin->addAction(&_colorByColorDataAction);
    scatterplotPlugin->addAction(&_colorDimensionAction);
    scatterplotPlugin->addAction(&_colorDataAction);

    _colorByAction.setOptions(QStringList() << "Constant color" << "Color dimension" << "Color data");

    _colorByAction.setToolTip("Color by");
    _colorByConstantColorAction.setToolTip("Color data points with a constant color");
    _colorByDimensionAction.setToolTip("Color data points by chosen dimension");
    _colorByColorDataAction.setToolTip("Color data points with a color data set");
    _constantColorAction.setToolTip("Constant color");
    _colorDimensionAction.setToolTip("Color dimension");
    _colorDataAction.setToolTip("Color data");

    _colorByConstantColorAction.setCheckable(true);
    _colorByDimensionAction.setCheckable(true);
    _colorByColorDataAction.setCheckable(true);

    _colorDataAction.setEnabled(false);

    _colorByActionGroup.addAction(&_colorByConstantColorAction);
    _colorByActionGroup.addAction(&_colorByDimensionAction);
    _colorByActionGroup.addAction(&_colorByColorDataAction);

    _colorMapAction.setWidgetFlags(ColorMapAction::All);

    const auto updateScalarRangeActions = [this]() {
        const auto colorMapRange    = getScatterplotWidget()->getColorMapRange();
        const auto colorMapRangeMin = colorMapRange.x;
        const auto colorMapRangeMax = colorMapRange.y;

        _colorMapAction.getSettingsAction().getHorizontalAxisAction().getRangeAction().initialize(colorMapRangeMin, colorMapRangeMax, colorMapRangeMin, colorMapRangeMax, colorMapRangeMin, colorMapRangeMax);
    };

    const auto updateColoringMode = [this]() {
        getScatterplotWidget()->setColoringMode(static_cast<ScatterplotWidget::ColoringMode>(_colorByAction.getCurrentIndex()));
    };

    connect(&_colorByAction, &OptionAction::currentIndexChanged, this, [this, updateColoringMode, updateScalarRangeActions](const std::uint32_t& currentIndex) {
        updateColoringMode();
        updateScalarRangeActions();
    });

    connect(&_colorByConstantColorAction, &QAction::triggered, this, [this]() {
        getScatterplotWidget()->setColoringMode(ScatterplotWidget::ColoringMode::ConstantColor);
    });

    connect(&_colorByDimensionAction, &QAction::triggered, this, [this]() {
        getScatterplotWidget()->setColoringMode(ScatterplotWidget::ColoringMode::ColorDimension);
    });

    connect(&_colorByColorDataAction, &QAction::triggered, this, [this]() {
        getScatterplotWidget()->setColoringMode(ScatterplotWidget::ColoringMode::ColorData);
    });

    const auto updateColorMap = [this]() -> void {
        switch (getScatterplotWidget()->getRenderMode())
        {
            case ScatterplotWidget::RenderMode::SCATTERPLOT:
            {
                if (getScatterplotWidget()->getColoringMode() == ScatterplotWidget::ColoringMode::ColorDimension)
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

        _colorByConstantColorAction.setChecked(coloringMode == ScatterplotWidget::ColoringMode::ConstantColor);
        _colorByDimensionAction.setChecked(coloringMode == ScatterplotWidget::ColoringMode::ColorDimension);
        _colorByColorDataAction.setChecked(coloringMode == ScatterplotWidget::ColoringMode::ColorData);
        _colorMapAction.setEnabled(renderMode == ScatterplotWidget::LANDSCAPE || coloringMode == ScatterplotWidget::ColoringMode::ColorDimension);
    };

    connect(&_colorDimensionAction.getCurrentDimensionAction(), &OptionAction::currentIndexChanged, this, [this, updateScalarRangeActions](const std::int32_t& currentIndex) {
        updateScalarRangeActions();
    });

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

    connect(getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, updateActions, updateScalarRangeActions, updateColorMap](const ScatterplotWidget::RenderMode& renderMode) {
        updateActions();
        updateScalarRangeActions();
        updateColorMap();
    });

    connect(&_scatterplotPlugin->getPointsDataset(), &DatasetRef<Points>::datasetNameChanged, this, [this, updateActions, updateColorMap](const QString& oldDatasetName, const QString& newDatasetName) {
        _colorByAction.reset();
        _colorByConstantColorAction.reset();
        _colorByColorDataAction.reset();

        updateActions();
        updateColorMap();
    });

    updateColoringMode();
    updateActions();
    updateScalarRangeActions();
}

QMenu* ColoringAction::getContextMenu()
{
    auto menu = new QMenu("Color");

    const auto addActionToMenu = [menu](QAction* action) -> void {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    const auto renderMode = _scatterplotPlugin->getScatterplotWidget()->getRenderMode();

    menu->setEnabled(renderMode == ScatterplotWidget::RenderMode::SCATTERPLOT);

    menu->addAction(&_colorByConstantColorAction);
    menu->addAction(&_colorByDimensionAction);
    menu->addAction(&_colorByColorDataAction);
    
    menu->addSeparator();

    switch (_scatterplotPlugin->getScatterplotWidget()->getColoringMode())
    {
        case ScatterplotWidget::ColoringMode::ConstantColor:
            menu->addMenu(_constantColorAction.getContextMenu());
            break;

        case ScatterplotWidget::ColoringMode::ColorDimension:
            menu->addMenu(_colorDimensionAction.getContextMenu());
            break;

        case ScatterplotWidget::ColoringMode::ColorData:
            menu->addMenu(_colorDataAction.getContextMenu());
            break;

        default:
            break;
    }

    return menu;
}

void ColoringAction::setDimensions(const std::uint32_t& numberOfDimensions, const std::vector<QString>& dimensionNames /*= std::vector<QString>()*/)
{
    _colorDimensionAction.setDimensions(numberOfDimensions, dimensionNames);
}

void ColoringAction::setDimensions(const std::vector<QString>& dimensionNames)
{
    setDimensions(static_cast<std::uint32_t>(dimensionNames.size()), dimensionNames);
}

ColoringAction::Widget::Widget(QWidget* parent, ColoringAction* coloringAction, const Widget::State& state) :
    WidgetActionWidget(parent, coloringAction, state)
{
    auto layout = new QHBoxLayout();

    auto stackedWidget = new StackedWidget();

    stackedWidget->addWidget(coloringAction->_constantColorAction.createWidget(this));
    stackedWidget->addWidget(coloringAction->_colorDimensionAction.createWidget(this));
    stackedWidget->addWidget(coloringAction->_colorDataAction.createWidget(this));

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

    switch (state)
    {
        case Widget::State::Standard:
        {
            auto layout = new QHBoxLayout();

            layout->setMargin(0);
            layout->addWidget(labelWidget);
            layout->addWidget(optionWidget);
            layout->addWidget(stackedWidget);

            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            auto layout = new QGridLayout();

            layout->addWidget(labelWidget, 0, 0);
            layout->addWidget(optionWidget, 0, 1);
            layout->addWidget(stackedWidget, 0, 2);

            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }
}
