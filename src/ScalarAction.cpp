#include "ScalarAction.h"
#include "ScalarSourceModel.h"

#include "ScatterplotPlugin.h"
#include "Application.h"

#include <QHBoxLayout>

using namespace hdps::gui;

ScalarAction::ScalarAction(ScatterplotPlugin* scatterplotPlugin, const QString& title, const float& minimum, const float& maximum, const float& value, const float& defaultValue) :
    PluginAction(scatterplotPlugin, "Coloring"),
    _magnitudeAction(this, title, minimum, maximum, value, defaultValue),
    _sourceAction(scatterplotPlugin)
{
    setText(title);

    _scatterplotPlugin->addAction(&_sourceAction);

    // Pass-through scalar range updates
    connect(&_sourceAction, &ScalarSourceAction::scalarRangeChanged, this, [this](const float& minimum, const float& maximum) {
        emit scalarRangeChanged(minimum, maximum);
    });
}

void ScalarAction::addDataset(const Dataset<DatasetImpl>& dataset)
{
    // Get reference to the point size source model
    auto& sourceModel = _sourceAction.getModel();

    // Avoid duplicates
    if (sourceModel.rowIndex(dataset) != 0)
        return;

    // Add dataset to the list of candidate datasets
    sourceModel.addDataset(dataset);

    // Connect to the data changed signal so that we can update the scatter plot point size appropriately
    connect(&dataset, &Dataset<DatasetImpl>::dataChanged, this, [this, dataset]() {

        // Get smart pointer to current dataset
        const auto currentDataset = getCurrentDataset();

        // Only proceed if we have a valid point size dataset
        if (!currentDataset.isValid())
            return;

        // Update scatter plot widget point size if the dataset matches
        if (currentDataset == dataset)
            emit sourceDataChanged(dataset);
    });

    // Connect to the data changed signal so that we can update the scatter plot point size appropriately
    connect(&_magnitudeAction, &DecimalAction::valueChanged, this, [this, dataset](const float& value) {
        emit magnitudeChanged(value);
    });
}

void ScalarAction::removeAllDatasets()
{
    _sourceAction.getModel().removeAllDatasets();
}

Dataset<DatasetImpl> ScalarAction::getCurrentDataset()
{
    // Get reference to the scalar source model
    auto& scalarSourceModel = _sourceAction.getModel();

    // Get current scalar source index
    const auto currentSourceIndex = _sourceAction.getPickerAction().getCurrentIndex();

    // Only proceed if we have a valid point size dataset row index
    if (currentSourceIndex < 1)
        return Dataset<DatasetImpl>();

    return scalarSourceModel.getDataset(currentSourceIndex);
}

ScalarAction::Widget::Widget(QWidget* parent, ScalarAction* scalarAction) :
    WidgetActionWidget(parent, scalarAction)
{
    auto layout = new QHBoxLayout();

    layout->setMargin(0);

    auto magnitudeLabelWidget   = scalarAction->getMagnitudeAction().createLabelWidget(this);
    auto magnitudeWidget        = scalarAction->getMagnitudeAction().createWidget(this);
    auto sourceWidget           = scalarAction->getSourceAction().createCollapsedWidget(this);

    layout->addWidget(magnitudeLabelWidget);
    layout->addWidget(magnitudeWidget);
    layout->addWidget(sourceWidget);

    // Adjust size of the combo boxes to the contents
    //pointSizeByWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    setLayout(layout);
}
