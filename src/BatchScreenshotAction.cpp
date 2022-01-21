#include "BatchScreenshotAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <Application.h>

using namespace hdps::gui;

BatchScreenshotAction::BatchScreenshotAction(ScatterplotPlugin& scatterplotPlugin) :
    PluginAction(&scatterplotPlugin, "Batch screenshot"),
    _batchAction(this, "Batch export"),
    _dimensionsPreviewAction(this, "Dimensions preview"),
    _dimensionsPickerAction(this)
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("image"));

    _dimensionsPreviewAction.setEnabled(false);

    _batchAction.setToolTip("Batch export one or more dimensions");
    _dimensionsPreviewAction.setToolTip("Batch dimensions preview");
    _dimensionsPickerAction.setToolTip("Pick dimensions for batch export");

    // Update the dimensions picker when the position dataset changes
    const auto positionDatasetChanged = [this, &scatterplotPlugin]() {
        if (!scatterplotPlugin.getPositionDataset().isValid())
            return;

        _dimensionsPickerAction.setPointsDataset(scatterplotPlugin.getPositionDataset());
    };

    // Update read-only status when the position dataset changes
    connect(&scatterplotPlugin.getPositionDataset(), &Dataset<Points>::changed, this, positionDatasetChanged);

    // Update the read-only status of the batch screenshot action
    const auto updateBatchScreenshotAction = [this]() -> void {
        _dimensionsPickerAction.setEnabled(_batchAction.isChecked());
    };

    // Handle enable/disable of the batch action
    connect(&_batchAction, &ToggleAction::toggled, this, updateBatchScreenshotAction);

    // Perform initialization of action(s)
    updateBatchScreenshotAction();
}

BatchScreenshotAction::Widget::Widget(QWidget* parent, BatchScreenshotAction* batchScreenshotAction, const std::int32_t& widgetFlags) :
    WidgetActionWidget(parent, batchScreenshotAction, widgetFlags)
{
    auto layout = new QHBoxLayout();

    layout->addWidget(batchScreenshotAction->getBatchAction().createWidget(this));
    layout->addWidget(batchScreenshotAction->getDimensionsPreviewAction().createWidget(this));
    layout->addWidget(batchScreenshotAction->getDimensionsPickerAction().createCollapsedWidget(this));

    if (widgetFlags & PopupLayout) {
        setPopupLayout(layout);
    }
    else {
        layout->setMargin(0);
        setLayout(layout);
    }
}
