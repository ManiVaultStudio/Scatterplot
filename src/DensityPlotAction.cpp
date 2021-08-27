#include "DensityPlotAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

DensityPlotAction::DensityPlotAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Density"),
    _sigmaAction(this, "Sigma", 1.0, 50.0, DEFAULT_SIGMA, DEFAULT_SIGMA)
{
    setToolTip("Density plot settings");

    _scatterplotPlugin->addAction(&_sigmaAction);

    const auto updateSigma = [this]() -> void {
        getScatterplotWidget()->setSigma(0.01 * _sigmaAction.getValue());
    };

    connect(&_sigmaAction, &DecimalAction::valueChanged, this, [this, updateSigma](const double& value) {
        updateSigma();
    });

    const auto updateSigmaAction = [this]() {
        _sigmaAction.setUpdateDuringDrag(_scatterplotPlugin->getNumberOfPoints() < 100000);
    };

    connect(scatterplotPlugin, &ScatterplotPlugin::currentPointsChanged, this, [this, updateSigmaAction](const QString& datasetName) {
        updateSigmaAction();
    });

    updateSigmaAction();

    updateSigma();
}

QMenu* DensityPlotAction::getContextMenu()
{
    auto menu = new QMenu("Plot settings");

    const auto renderMode = getScatterplotWidget()->getRenderMode();

    const auto addActionToMenu = [menu](QAction* action) {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    addActionToMenu(&_sigmaAction);

    return menu;
}

DensityPlotAction::Widget::Widget(QWidget* parent, DensityPlotAction* densityPlotAction, const Widget::State& state) :
    WidgetActionWidget(parent, densityPlotAction, state)
{
    switch (state)
    {
        case Widget::State::Standard:
        case Widget::State::Popup:
        {
            auto layout = new QHBoxLayout();

            layout->setMargin(0);
            layout->addWidget(densityPlotAction->_sigmaAction.createLabelWidget(this));
            layout->addWidget(densityPlotAction->_sigmaAction.createWidget(this));

            setLayout(layout);
            break;
        }

        default:
            break;
    }
}