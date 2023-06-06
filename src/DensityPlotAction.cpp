#include "DensityPlotAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

DensityPlotAction::DensityPlotAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _sigmaAction(this, "Sigma", 0.01f, 0.5f, DEFAULT_SIGMA, DEFAULT_SIGMA, 3),
    _continuousUpdatesAction(this, "Live Updates", DEFAULT_CONTINUOUS_UPDATES, DEFAULT_CONTINUOUS_UPDATES)
{
    setToolTip("Density plot settings");
    setDefaultWidgetFlags(GroupAction::Horizontal);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::NoLabelInGroup);

    addAction(&_sigmaAction);
    addAction(&_continuousUpdatesAction);
}

void DensityPlotAction::initialize(ScatterplotPlugin* scatterplotPlugin)
{
    Q_ASSERT(scatterplotPlugin != nullptr);

    if (scatterplotPlugin == nullptr)
        return;

    _scatterplotPlugin = scatterplotPlugin;

    const auto computeDensity = [this]() -> void {
        if (static_cast<std::int32_t>(_scatterplotPlugin->getSettingsAction().getRenderModeAction().getCurrentIndex()) != ScatterplotWidget::RenderMode::DENSITY)
            return;

        _scatterplotPlugin->getScatterplotWidget().setSigma(_sigmaAction.getValue());

        const auto maxDensity = _scatterplotPlugin->getScatterplotWidget().getDensityRenderer().getMaxDensity();

        if (maxDensity > 0)
            _scatterplotPlugin->getSettingsAction().getColoringAction().getColorMapAction().getRangeAction(ColorMapAction::Axis::X).setRange({ 0.0f, maxDensity });
    };

    connect(&_sigmaAction, &DecimalAction::valueChanged, this, computeDensity);

    const auto updateSigmaAction = [this]() {
        _sigmaAction.setUpdateDuringDrag(_continuousUpdatesAction.isChecked());
    };

    connect(&_continuousUpdatesAction, &ToggleAction::toggled, updateSigmaAction);

    connect(&_scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, [this, updateSigmaAction, computeDensity](DatasetImpl* dataset) {
        updateSigmaAction();
        computeDensity();
    });

    connect(&_scatterplotPlugin->getSettingsAction().getRenderModeAction(), &OptionAction::currentIndexChanged, this, computeDensity);

    updateSigmaAction();
    computeDensity();
}

QMenu* DensityPlotAction::getContextMenu()
{
    if (_scatterplotPlugin == nullptr)
        return nullptr;

    auto menu = new QMenu("Plot settings");

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_sigmaAction);
    addActionToMenu(&_continuousUpdatesAction);

    return menu;
}

void DensityPlotAction::setVisible(bool visible)
{
    _sigmaAction.setVisible(visible);
    _continuousUpdatesAction.setVisible(visible);
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
