#include "SubsetAction.h"
#include "ScatterplotPlugin.h"
#include "PointData.h"

#include <QMenu>

using namespace hdps;
using namespace hdps::gui;

SubsetAction::SubsetAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Subset"),
    _subsetNameAction(this, "Subset name"),
    _createSubsetAction(this, "Create subset"),
    _sourceDataAction(this, "Source data")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("crop"));

    _subsetNameAction.setToolTip("Name of the subset");
    _createSubsetAction.setToolTip("Create subset from selected data points");

    const auto updateActions = [this]() -> void {
        setEnabled(_scatterplotPlugin->getNumberOfSelectedPoints() >= 1);
    };

    connect(_scatterplotPlugin, qOverload<>(&ScatterplotPlugin::selectionChanged), this, updateActions);

    connect(&_createSubsetAction, &QAction::triggered, this, [this]() {
        _scatterplotPlugin->createSubset(_sourceDataAction.getCurrentIndex() == 1, _subsetNameAction.getString());
    });

    const auto onCurrentDatasetChanged = [this]() -> void {
        const auto datasetName = _scatterplotPlugin->getCurrentDataset();

        QStringList sourceDataOptions;

        if (!datasetName.isEmpty()) {
            const auto sourceDatasetName = DataSet::getSourceData(_scatterplotPlugin->getCore()->requestData<Points>(datasetName)).getName();

            sourceDataOptions << QString("From: %1").arg(datasetName);

            if (sourceDatasetName != datasetName)
                sourceDataOptions << QString("From: %1 (source data)").arg(sourceDatasetName);
        }

        _sourceDataAction.setOptions(sourceDataOptions);
        _sourceDataAction.setEnabled(sourceDataOptions.count() >= 2);
    };

    connect(scatterplotPlugin, &ScatterplotPlugin::currentDatasetChanged, this, [this, onCurrentDatasetChanged](const QString& datasetName) {
        onCurrentDatasetChanged();
    });

    onCurrentDatasetChanged();
    updateActions();
}

QMenu* SubsetAction::getContextMenu()
{
    auto menu = new QMenu("Subset");

    menu->addAction(&_createSubsetAction);
    menu->addAction(&_sourceDataAction);

    return menu;
}

SubsetAction::Widget::Widget(QWidget* parent, SubsetAction* subsetAction, const Widget::State& state) :
    WidgetAction::Widget(parent, subsetAction, state)
{
    auto layout = new QHBoxLayout();

    layout->addWidget(subsetAction->_createSubsetAction.createWidget(this));
    layout->addWidget(subsetAction->_sourceDataAction.createWidget(this));

    switch (state)
    {
        case Widget::State::Standard:
            layout->setMargin(0);
            setLayout(layout);
            break;

        case Widget::State::Popup:
            setPopupLayout(layout);
            break;

        default:
            break;
    }
}