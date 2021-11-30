#include "ScalarSourceAction.h"
#include "Application.h"

#include "ScatterplotPlugin.h"
#include "PointData.h"

#include <QGridLayout>
#include <QHBoxLayout>

using namespace hdps::gui;

ScalarSourceAction::ScalarSourceAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Scalar source"),
    _model(this),
    _pickerAction(this, "Scalar options"),
    _dimensionPickerAction(this, "Data dimension"),
    _rangeAction(this, "Scalar range")
{
    _scatterplotPlugin->addAction(&_pickerAction);
    _scatterplotPlugin->addAction(&_dimensionPickerAction);

    // Configure scalar option picker
    _pickerAction.setCustomModel(&_model);
    _pickerAction.setToolTip("Pick scalar option");

    // Invoked when the scalar source changed
    const auto scalarSourceChanged = [this]() -> void {

        // Current source index
        const auto sourceIndex = _pickerAction.getCurrentIndex();

        // Set dimension picker visibility
        _dimensionPickerAction.setVisible(sourceIndex >= 1);

        // Assign the icon
        setIcon(_model.data(_model.index(sourceIndex, 0), Qt::DecorationRole).value<QIcon>());

        // Update dimension picker
        _dimensionPickerAction.setPointsDataset(Dataset<Points>(_model.getDataset(_pickerAction.getCurrentIndex())));

        // Update the scalar range if possible
        updateScalarRange();
    };

    // Handle when the source index changes
    connect(&_pickerAction, &OptionAction::currentIndexChanged, this, scalarSourceChanged);

    // Update scalar range when dimension is picked
    connect(&_dimensionPickerAction, &PointsDimensionPickerAction::currentDimensionIndexChanged, this, &ScalarSourceAction::updateScalarRange);

    // Notify others that the range changed when the user changes the range minimum
    connect(&_rangeAction.getRangeMinAction(), &DecimalAction::valueChanged, this, [this]() {
        emit scalarRangeChanged(_rangeAction.getRangeMinAction().getValue(), _rangeAction.getRangeMaxAction().getValue());
    });

    // Notify others that the range changed when the user changes the range maximum
    connect(&_rangeAction.getRangeMaxAction(), &DecimalAction::valueChanged, this, [this]() {
        emit scalarRangeChanged(_rangeAction.getRangeMinAction().getValue(), _rangeAction.getRangeMaxAction().getValue());
    });

    // Force initial update
    scalarSourceChanged();
}

ScalarSourceModel& ScalarSourceAction::getModel()
{
    return _model;
}

void ScalarSourceAction::updateScalarRange()
{
    // Get smart pointer to points dataset
    auto points = Dataset<Points>(_model.getDataset(_pickerAction.getCurrentIndex()));

    float minimum = 0.0f;
    float maximum = 1.0f;

    // Establish whether there is valid scalar range
    const auto hasScalarRange = points.isValid() && _pickerAction.getCurrentIndex() >= 1;

    if (hasScalarRange) {

        // Initialize minimum and maximum with extremes
        minimum = std::numeric_limits<float>::max();
        maximum = std::numeric_limits<float>::lowest();

        points->visitData([this, &minimum, &maximum, points](auto pointData) {

            // Get current dimension index
            const auto currentDimensionIndex = _dimensionPickerAction.getCurrentDimensionIndex();

            // Loop over all points to find minimum and maximum
            for (std::uint32_t pointIndex = 0; pointIndex < points->getNumPoints(); pointIndex++) {

                // Get point value for dimension
                auto pointValue = static_cast<float>(pointData[pointIndex][currentDimensionIndex]);
                
                // Adjust minimum and maximum
                minimum = std::min(minimum, pointValue);
                maximum = std::max(maximum, pointValue);
            }
        });
    }

    // Update range action
    _rangeAction.getRangeMinAction().initialize(minimum, maximum, minimum, 1);
    _rangeAction.getRangeMaxAction().initialize(minimum, maximum, maximum, 1);

    // Enable/disable range action
    _rangeAction.getRangeMinAction().setEnabled(hasScalarRange);
    _rangeAction.getRangeMaxAction().setEnabled(hasScalarRange);

    // Notify others the range ranged
    emit scalarRangeChanged(minimum, maximum);
}

ScalarSourceAction::Widget::Widget(QWidget* parent, ScalarSourceAction* scalarSourceAction) :
    WidgetActionWidget(parent, scalarSourceAction)
{
    auto layout = new QGridLayout();

    // Create action widgets
    auto pickerWidget           = scalarSourceAction->getPickerAction().createWidget(this);
    auto dimensionPickerWidget  = scalarSourceAction->getDimensionPickerAction().createWidget(this);
    auto rangeMinLabelWidget    = scalarSourceAction->getRangeAction().getRangeMinAction().createLabelWidget(this);
    auto rangeMinWidget         = scalarSourceAction->getRangeAction().getRangeMinAction().createWidget(this);
    auto rangeMaxLabelWidget    = scalarSourceAction->getRangeAction().getRangeMaxAction().createLabelWidget(this);
    auto rangeMaxWidget         = scalarSourceAction->getRangeAction().getRangeMaxAction().createWidget(this);

    // Adjust size of the combo boxes to the contents
    pickerWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);
    dimensionPickerWidget->findChild<QComboBox*>("ComboBox")->setSizeAdjustPolicy(QComboBox::AdjustToContents);

    auto selectionLayout = new QHBoxLayout();

    selectionLayout->addWidget(pickerWidget);
    selectionLayout->addWidget(dimensionPickerWidget);

    layout->addLayout(selectionLayout, 0, 0, 1, 2);

    layout->addWidget(rangeMinLabelWidget, 1, 0);
    layout->addWidget(rangeMinWidget, 1, 1);
    layout->addWidget(rangeMaxLabelWidget, 2, 0);
    layout->addWidget(rangeMaxWidget, 2, 1);

    setPopupLayout(layout);
}
