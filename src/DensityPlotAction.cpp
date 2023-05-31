#include "DensityPlotAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

DensityPlotAction::DensityPlotAction(QObject* parent) :
    WidgetAction(parent, "Density"),
    _sigmaAction(this, "Sigma", 0.01f, 0.5f, DEFAULT_SIGMA, DEFAULT_SIGMA, 3),
    _continuousUpdatesAction(this, "Live Updates", DEFAULT_CONTINUOUS_UPDATES, DEFAULT_CONTINUOUS_UPDATES)
{
    setToolTip("Density plot settings");

    const auto computeDensity = [this]() -> void {
        getScatterplotPlugin()->getScatterplotWidget().setSigma(_sigmaAction.getValue());

        const auto maxDensity = getScatterplotPlugin()->getScatterplotWidget().getDensityRenderer().getMaxDensity();

        if (maxDensity > 0)
            getScatterplotPlugin()->getSettingsAction().getColoringAction().getColorMapAction().getRangeAction(ColorMapAction::Axis::X).setRange({ 0.0f, maxDensity });
    };

    connect(&_sigmaAction, &DecimalAction::valueChanged, this, [this, computeDensity](const double& value) {
        computeDensity();
    });

    const auto updateSigmaAction = [this]() {
        _sigmaAction.setUpdateDuringDrag(_continuousUpdatesAction.isChecked());
    };

    connect(&_continuousUpdatesAction, &ToggleAction::toggled, updateSigmaAction);

    connect(&getScatterplotPlugin()->getPositionDataset(), &Dataset<Points>::changed, this, [this, updateSigmaAction, computeDensity](DatasetImpl* dataset) {
        updateSigmaAction();
        computeDensity();
    });

    connect(&getScatterplotPlugin()->getScatterplotWidget(), &ScatterplotWidget::renderModeChanged, this, [this, computeDensity](const ScatterplotWidget::RenderMode& renderMode) {
        computeDensity();
    });

    updateSigmaAction();
    computeDensity();
}

ScatterplotPlugin* DensityPlotAction::getScatterplotPlugin()
{
    return dynamic_cast<ScatterplotPlugin*>(parent()->parent());
}

QMenu* DensityPlotAction::getContextMenu()
{
    auto menu = new QMenu("Plot settings");

    const auto renderMode = getScatterplotPlugin()->getScatterplotWidget().getRenderMode();

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_sigmaAction);
    addActionToMenu(&_continuousUpdatesAction);

    return menu;
}

void DensityPlotAction::fromVariantMap(const QVariantMap& variantMap)
{
    WidgetAction::fromVariantMap(variantMap);

    _sigmaAction.fromParentVariantMap(variantMap);
    _continuousUpdatesAction.fromParentVariantMap(variantMap);
}

QVariantMap DensityPlotAction::toVariantMap() const
{
    QVariantMap variantMap = WidgetAction::toVariantMap();

    _sigmaAction.insertIntoVariantMap(variantMap);
    _continuousUpdatesAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

DensityPlotAction::Widget::Widget(QWidget* parent, DensityPlotAction* densityPlotAction) :
    WidgetActionWidget(parent, densityPlotAction)
{
    auto layout = new QHBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->addWidget(densityPlotAction->_sigmaAction.createLabelWidget(this));
    layout->addWidget(densityPlotAction->_sigmaAction.createWidget(this));
    layout->addWidget(densityPlotAction->_continuousUpdatesAction.createWidget(this));

    setLayout(layout);
}