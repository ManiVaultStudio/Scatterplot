#include "ScatterplotPlugin.h"

#include "MappingUtils.h"
#include "ScatterplotWidget.h"

#include <Application.h>
#include <DataHierarchyItem.h>

#include <util/PixelSelectionTool.h>
#include <util/Timer.h>

#include <ClusterData/ClusterData.h>
#include <ColorData/ColorData.h>
#include <PointData/PointData.h>

#include <graphics/Vector3f.h>

#include <widgets/DropWidget.h>
#include <widgets/ViewPluginLearningCenterOverlayWidget.h>

#include <actions/PluginTriggerAction.h>
#include <actions/ViewPluginSamplerAction.h>

#include <DatasetsMimeData.h>

#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QMetaType>
#include <QtCore>

#include <algorithm>
#include <cassert>
#include <exception>
#include <map>
#include <stdexcept>
#include <vector>

#define VIEW_SAMPLING_HTML
//#define VIEW_SAMPLING_WIDGET

Q_PLUGIN_METADATA(IID "studio.manivault.ScatterplotPlugin")

using namespace mv;
using namespace mv::util;

ScatterplotPlugin::ScatterplotPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _dropWidget(nullptr),
    _scatterPlotWidget(new ScatterplotWidget(this)),
    _numPoints(0),
    _settingsAction(this, "Settings"),
    _primaryToolbarAction(this, "Primary Toolbar")
{
    setObjectName("Scatterplot");

    auto& shortcuts = getShortcuts();

    shortcuts.add({ QKeySequence(Qt::Key_R), "Selection", "Rectangle (default)" });
    shortcuts.add({ QKeySequence(Qt::Key_L), "Selection", "Lasso" });
    shortcuts.add({ QKeySequence(Qt::Key_P), "Selection", "Polygon" });
    shortcuts.add({ QKeySequence(Qt::Key_B), "Selection", "Circular brush (mouse wheel adjusts the radius)" });
    shortcuts.add({ QKeySequence(Qt::SHIFT), "Selection", "Add to selection" });
    shortcuts.add({ QKeySequence(Qt::CTRL), "Selection", "Remove from selection" });
    shortcuts.add({ QKeySequence(Qt::Key_A), "Selection", "Select all" });
    shortcuts.add({ QKeySequence(Qt::Key_E), "Selection", "Clear selection" });
    shortcuts.add({ QKeySequence(Qt::Key_I), "Selection", "Invert selection" });

    shortcuts.add({ QKeySequence(Qt::Key_S), "Render", "Scatter mode (default)" });
    shortcuts.add({ QKeySequence(Qt::Key_D), "Render", "Density mode" });
    shortcuts.add({ QKeySequence(Qt::Key_C), "Render", "Contour mode" });

    shortcuts.add({ QKeySequence(Qt::Key_Plus), "Navigation", "Zoom in by 10%" });
    shortcuts.add({ QKeySequence(Qt::Key_Minus), "Navigation", "Zoom out by 10%" });
    shortcuts.add({ QKeySequence(Qt::ALT), "Navigation", "Pan (LMB down)" });
    shortcuts.add({ QKeySequence(Qt::ALT), "Navigation", "Zoom (mouse wheel)" });
    shortcuts.add({ QKeySequence(Qt::Key_O), "Navigation", "Original view" });
    shortcuts.add({ QKeySequence(Qt::Key_H), "Navigation", "Zoom to selection" });
    shortcuts.add({ QKeySequence(Qt::Key_F), "Navigation", "Zoom to window" });
    
    _dropWidget = new DropWidget(_scatterPlotWidget);

    getWidget().setFocusPolicy(Qt::ClickFocus);

    _primaryToolbarAction.addAction(&_settingsAction.getDatasetsAction());
    _primaryToolbarAction.addAction(&_settingsAction.getRenderModeAction(), 3, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getPositionAction(), 1, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getPlotAction(), 2, GroupAction::Horizontal);
    _primaryToolbarAction.addAction(&_settingsAction.getColoringAction());
    _primaryToolbarAction.addAction(&_settingsAction.getSubsetAction());
    _primaryToolbarAction.addAction(&_settingsAction.getClusteringAction());
    _primaryToolbarAction.addAction(&_settingsAction.getSelectionAction());
    _primaryToolbarAction.addAction(&getSamplerAction());

    auto focusSelectionAction = new ToggleAction(this, "Focus selection");

    focusSelectionAction->setIconByName("mouse-pointer");

    connect(focusSelectionAction, &ToggleAction::toggled, this, [this](bool toggled) -> void {
        _settingsAction.getPlotAction().getPointPlotAction().getFocusSelection().setChecked(toggled);
    });

    connect(&_settingsAction.getPlotAction().getPointPlotAction().getFocusSelection(), &ToggleAction::toggled, this, [this, focusSelectionAction](bool toggled) -> void {
        focusSelectionAction->setChecked(toggled);
    });

    const auto updateReadOnly = [this, focusSelectionAction]() {
        focusSelectionAction->setEnabled(getScatterplotWidget().getRenderMode() == ScatterplotWidget::SCATTERPLOT && _positionDataset.isValid());
    };

    updateReadOnly();

    connect(_scatterPlotWidget, &ScatterplotWidget::renderModeChanged, this, updateReadOnly);
    connect(&_positionDataset, &Dataset<Points>::changed, this, updateReadOnly);

    //_secondaryToolbarAction.addAction(&_settingsAction.getMiscellaneousAction());

    connect(_scatterPlotWidget, &ScatterplotWidget::customContextMenuRequested, this, [this](const QPoint& point) {
        if (!_positionDataset.isValid())
            return;

        auto contextMenu = _settingsAction.getContextMenu();

        contextMenu->addSeparator();

        _positionDataset->populateContextMenu(contextMenu);

        contextMenu->exec(getWidget().mapToGlobal(point));
    });

    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(&getWidget(), "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        DropWidget::DropRegions dropRegions;

        const auto datasetsMimeData = dynamic_cast<const DatasetsMimeData*>(mimeData);

        if (datasetsMimeData == nullptr)
            return dropRegions;

        if (datasetsMimeData->getDatasets().count() > 1)
            return dropRegions;

        const auto dataset          = datasetsMimeData->getDatasets().first();
        const auto datasetGuiName   = dataset->text();
        const auto datasetId        = dataset->getId();
        const auto dataType         = dataset->getDataType();
        const auto dataTypes        = DataTypes({ PointType , ColorType, ClusterType });

        // Check if the data type can be dropped
        if (!dataTypes.contains(dataType))
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", "exclamation-circle", false);

        // Points dataset is about to be dropped
        if (dataType == PointType) {

            // Get points dataset from the core
            auto candidateDataset = mv::data().getDataset<Points>(datasetId);

            // Establish drop region description
            const auto description = QString("Visualize %1 as points or density/contour map").arg(datasetGuiName);

            if (!_positionDataset.isValid()) {

                // Load as point positions when no dataset is currently loaded
                dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                    _positionDataset = candidateDataset;
                    });
            }
            else {
                if (_positionDataset != candidateDataset && candidateDataset->getNumDimensions() >= 2) {

                    // The number of points is equal, so offer the option to replace the existing points dataset
                    dropRegions << new DropWidget::DropRegion(this, "Point position", description, "map-marker-alt", true, [this, candidateDataset]() {
                        _positionDataset = candidateDataset;
                        });
                }

                // Accept for recoloring:
                //  1. data with the same number of points
                //  2. data which is derived from a parent that has the same number of points (e.g. for HSNE embeddings), where we can use global indices for mapping
                //  3. data which has a fully-covering selection mapping, that we can use for setting colors. Mapping in order of preference:
                //      a) from color (or it's parent) to position
                //      b) from color to position (or it's parent)
                //      c) from source of position to color

                // [1. Same number of points]
                const auto numPointsCandidate   = candidateDataset->getNumPoints();
                const auto numPointsPosition    = _positionDataset->getNumPoints();
                const bool hasSameNumPoints     = numPointsPosition == numPointsCandidate;

                // [2. Derived from a parent]
                const bool hasSameNumPointsAsFull = fullSourceHasSameNumPoints(_positionDataset, candidateDataset);

                // [3. Full selection mapping]
                const bool hasSelectionMapping  = checkSelectionMapping(candidateDataset, _positionDataset);

                if (hasSameNumPoints || hasSameNumPointsAsFull || hasSelectionMapping) {
                    // Offer the option to use the points dataset as source for points colors
                    dropRegions << new DropWidget::DropRegion(this, "Point color", QString("Colorize %1 points with %2").arg(_positionDataset->text(), candidateDataset->text()), "palette", true, [this, candidateDataset]() {
                        _settingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);   // calls addColorDataset internally
                        });

                }

                // Accept for resizing and opacity: Only data with the same number of points
                if (hasSameNumPoints) {
                    // Offer the option to use the points dataset as source for points size
                    dropRegions << new DropWidget::DropRegion(this, "Point size", QString("Size %1 points with %2").arg(_positionDataset->text(), candidateDataset->text()), "ruler-horizontal", true, [this, candidateDataset]() {
                        _settingsAction.getPlotAction().getPointPlotAction().setCurrentPointSizeDataset(candidateDataset);
                        });

                    // Offer the option to use the points dataset as source for points opacity
                    dropRegions << new DropWidget::DropRegion(this, "Point opacity", QString("Set %1 points opacity with %2").arg(_positionDataset->text(), candidateDataset->text()), "brush", true, [this, candidateDataset]() {
                        _settingsAction.getPlotAction().getPointPlotAction().setCurrentPointOpacityDataset(candidateDataset);
                        });
                }
            }
        }

        // Cluster dataset is about to be dropped
        if (dataType == ClusterType) {

            // Get clusters dataset from the core
            auto candidateDataset = mv::data().getDataset<Clusters>(datasetId);

            // Establish drop region description
            const auto description = QString("Color points by %1").arg(candidateDataset->text());

            // Only allow user to color by clusters when there is a positions dataset loaded
            if (_positionDataset.isValid()) {

                if (_settingsAction.getColoringAction().hasColorDataset(candidateDataset)) {

                    // The clusters dataset is already loaded
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, "palette", true, [this, candidateDataset]() {
                        _settingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
                        });
                }
                else {

                    // Use the clusters set for points color
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, "palette", true, [this, candidateDataset]() {
                        _settingsAction.getColoringAction().addColorDataset(candidateDataset);
                        _settingsAction.getColoringAction().setCurrentColorDataset(candidateDataset);
                        });
                }
            }
            else {

                // Only allow user to color by clusters when there is a positions dataset loaded
                dropRegions << new DropWidget::DropRegion(this, "No points data loaded", "Clusters can only be visualized in concert with points data", "exclamation-circle", false);
            }
        }

        return dropRegions;
    });

    auto& selectionAction = _settingsAction.getSelectionAction();

    getSamplerAction().initialize(this, &selectionAction.getPixelSelectionAction(), &selectionAction.getSamplerPixelSelectionAction());
    getSamplerAction().getEnabledAction().setChecked(false);

    getLearningCenterAction().addVideos(QStringList({ "Practitioner", "Developer" }));

    setOverlayActionsTargetWidget(_scatterPlotWidget);

    
}

ScatterplotPlugin::~ScatterplotPlugin()
{
}

void ScatterplotPlugin::init()
{
    getWidget().setMouseTracking(true);

    auto layout = new QVBoxLayout();

    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_primaryToolbarAction.createWidget(&getWidget()));
    layout->addWidget(_scatterPlotWidget, 100);

    if (auto navigationWidget = _scatterPlotWidget->getPointRendererNavigator().getNavigationAction().createWidget(&getWidget())) {
        layout->addWidget(navigationWidget);
        layout->setAlignment(navigationWidget, Qt::AlignCenter);
    }

    getWidget().setLayout(layout);

    connect(_scatterPlotWidget, &ScatterplotWidget::initialized, this, &ScatterplotPlugin::updateData);

    connect(&_scatterPlotWidget->getPixelSelectionTool(), &PixelSelectionTool::areaChanged, [this]() {
        if (_scatterPlotWidget->getPixelSelectionTool().isNotifyDuringSelection()) {
            selectPoints();
        }
    });

    connect(&_scatterPlotWidget->getPixelSelectionTool(), &PixelSelectionTool::ended, [this]() {
        if (_scatterPlotWidget->getPixelSelectionTool().isNotifyDuringSelection())
            return;

        selectPoints();
    });

    connect(&getSamplerAction(), &ViewPluginSamplerAction::sampleContextRequested, this, &ScatterplotPlugin::samplePoints);

    connect(&_positionDataset, &Dataset<Points>::changed, this, &ScatterplotPlugin::positionDatasetChanged);
    connect(&_positionDataset, &Dataset<Points>::dataChanged, this, &ScatterplotPlugin::updateData);
    connect(&_positionDataset, &Dataset<Points>::dataSelectionChanged, this, &ScatterplotPlugin::updateSelection);

    _scatterPlotWidget->installEventFilter(this);

    getLearningCenterAction().getViewPluginOverlayWidget()->setTargetWidget(_scatterPlotWidget);

    connect(&getScatterplotWidget().getPointRendererNavigator().getNavigationAction().getZoomSelectionAction(), &TriggerAction::triggered, this, [this]() -> void {
        if (_selectionBoundaries.isValid())
            _scatterPlotWidget->getPointRendererNavigator().setZoomRectangleWorld(_selectionBoundaries);
	});

#ifdef VIEW_SAMPLING_HTML
    getSamplerAction().setHtmlViewGeneratorFunction([this](const ViewPluginSamplerAction::SampleContext& toolTipContext) -> QString {
        QStringList localPointIndices, globalPointIndices;

        for (const auto& localPointIndex : toolTipContext["LocalPointIndices"].toList())
            localPointIndices << QString::number(localPointIndex.toInt());

        for (const auto& globalPointIndex : toolTipContext["GlobalPointIndices"].toList())
            globalPointIndices << QString::number(globalPointIndex.toInt());

        if (localPointIndices.isEmpty())
            return {};

        return  QString("<table> \
                    <tr> \
                        <td><b>Point ID's: </b></td> \
                        <td>%1</td> \
                    </tr> \
                   </table>").arg(globalPointIndices.join(", "));
        });
#endif

#ifdef VIEW_SAMPLING_WIDGET
    auto pointIndicesTableWidget = new QTableWidget(5, 2);

    pointIndicesTableWidget->setHorizontalHeaderLabels({ "Local point index", "Global point index" });

    getSamplerAction().setWidgetViewGeneratorFunction([this, pointIndicesTableWidget](const ViewPluginSamplerAction::SampleContext& sampleContext) -> QWidget* {
        const auto localPointIndices = sampleContext["LocalPointIndices"].toList();
        const auto globalPointIndices = sampleContext["GlobalPointIndices"].toList();

        pointIndicesTableWidget->setRowCount(localPointIndices.count());

        for (int i = 0; i < localPointIndices.count(); ++i) {
            pointIndicesTableWidget->setItem(i, 0, new QTableWidgetItem(QString::number(localPointIndices[i].toInt())));
            pointIndicesTableWidget->setItem(i, 1, new QTableWidgetItem(QString::number(globalPointIndices[i].toInt())));
        }

        pointIndicesTableWidget->resizeColumnsToContents();

        return pointIndicesTableWidget;
        });
#endif

    updateHeadsUpDisplay();

    connect(&_positionDataset, &Dataset<>::changed, this, &ScatterplotPlugin::updateHeadsUpDisplay);
    connect(&_positionDataset, &Dataset<>::guiNameChanged, this, &ScatterplotPlugin::updateHeadsUpDisplay);
    connect(&_settingsAction.getColoringAction(), &ColoringAction::currentColorDatasetChanged, this, &ScatterplotPlugin::updateHeadsUpDisplay);
    connect(&_settingsAction.getColoringAction().getColorByAction(), &OptionAction::currentIndexChanged, this, &ScatterplotPlugin::updateHeadsUpDisplay);
}

void ScatterplotPlugin::loadData(const Datasets& datasets)
{
    // Exit if there is nothing to load
    if (datasets.isEmpty())
        return;

    // Load the first dataset
    _positionDataset = datasets.first();

    // And set the coloring mode to constant
    _settingsAction.getColoringAction().getColorByAction().setCurrentIndex(0);
}

void ScatterplotPlugin::createSubset(const bool& fromSourceData /*= false*/, const QString& name /*= ""*/)
{
    // Create the subset
    mv::Dataset<DatasetImpl> subset;

    if (fromSourceData)
        // Make subset from the source data, this is not the displayed data, so no restrictions here
        subset = _positionSourceDataset->createSubsetFromSelection(name, _positionSourceDataset);
    else
        // Avoid making a bigger subset than the current data by restricting the selection to the current data
        subset = _positionDataset->createSubsetFromVisibleSelection(name, _positionDataset);

    subset->getDataHierarchyItem().select();
}

void ScatterplotPlugin::selectPoints()
{
    if (getSettingsAction().getSelectionAction().getFreezeSelectionAction().isChecked())
        return;

    auto& pixelSelectionTool = _scatterPlotWidget->getPixelSelectionTool();

    // Only proceed with a valid points position dataset and when the pixel selection tool is active
    if (!_positionDataset.isValid() || !pixelSelectionTool.isActive() || _scatterPlotWidget->_pointRenderer.getNavigator().isNavigating() || !pixelSelectionTool.isEnabled())
        return;

    auto selectionAreaImage = pixelSelectionTool.getAreaPixmap().toImage();
    auto selectionSet       = _positionDataset->getSelection<Points>();

    std::vector<std::uint32_t> targetSelectionIndices;

    targetSelectionIndices.reserve(_positionDataset->getNumPoints());

    std::vector<std::uint32_t> localGlobalIndices;

    _positionDataset->getGlobalIndices(localGlobalIndices);

    auto& pointRenderer = _scatterPlotWidget->_pointRenderer;
    auto& navigator     = pointRenderer.getNavigator();

    const auto zoomRectangleWorld   = navigator.getZoomRectangleWorld();
    const auto screenRectangle      = QRect(QPoint(), pointRenderer.getRenderSize());

    float boundaries[4]{
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::lowest(),
        std::numeric_limits<float>::max(),
        std::numeric_limits<float>::lowest()
    };

    // Go over all points in the dataset to see if they are selected
    for (std::uint32_t localPointIndex = 0; localPointIndex < _positions.size(); localPointIndex++) {
	    const auto& point = _positions[localPointIndex];

    	// Compute the offset of the point in the world space
    	const auto pointOffsetWorld = QPointF(point.x - zoomRectangleWorld.left(), point.y - zoomRectangleWorld.top());

    	// Normalize it 
    	const auto pointOffsetWorldNormalized = QPointF(pointOffsetWorld.x() / zoomRectangleWorld.width(), pointOffsetWorld.y() / zoomRectangleWorld.height());

    	// Convert it to screen space
    	const auto pointOffsetScreen = QPoint(pointOffsetWorldNormalized.x() * screenRectangle.width(), screenRectangle.height() - pointOffsetWorldNormalized.y() * screenRectangle.height());

    	// Continue to next point if the point is outside the screen
    	if (!screenRectangle.contains(pointOffsetScreen))
    		continue;

    	// If the corresponding pixel is not transparent, add the point to the selection
    	if (selectionAreaImage.pixelColor(pointOffsetScreen).alpha() > 0) {
    		targetSelectionIndices.push_back(localGlobalIndices[localPointIndex]);

            boundaries[0] = std::min(boundaries[0], point.x);
            boundaries[1] = std::max(boundaries[1], point.x);
            boundaries[2] = std::min(boundaries[2], point.y);
            boundaries[3] = std::max(boundaries[3], point.y);
	    }
    }

    _selectionBoundaries = QRectF(boundaries[0], boundaries[2], boundaries[1] - boundaries[0], boundaries[3] - boundaries[2]);

    switch (const auto selectionModifier = pixelSelectionTool.isAborted() ? PixelSelectionModifierType::Subtract : pixelSelectionTool.getModifier())
    {
        case PixelSelectionModifierType::Replace:
            break;

        case PixelSelectionModifierType::Add:
        case PixelSelectionModifierType::Subtract:
        {
            // Get reference to the indices of the selection set
            auto& selectionSetIndices = selectionSet->indices;

            // Create a set from the selection set indices
            QSet<std::uint32_t> set(selectionSetIndices.begin(), selectionSetIndices.end());

            switch (selectionModifier)
            {
                // Add points to the current selection
                case PixelSelectionModifierType::Add:
                {
                    // Add indices to the set 
                    for (const auto& targetIndex : targetSelectionIndices)
                        set.insert(targetIndex);

                    break;
                }

                // Remove points from the current selection
                case PixelSelectionModifierType::Subtract:
                {
                    // Remove indices from the set 
                    for (const auto& targetIndex : targetSelectionIndices)
                        set.remove(targetIndex);

                    break;
                }

                case PixelSelectionModifierType::Replace:
                    break;
            }

            targetSelectionIndices = std::vector<std::uint32_t>(set.begin(), set.end());

            break;
        }
    }

    auto& navigationAction = _scatterPlotWidget->getPointRendererNavigator().getNavigationAction();

    navigationAction.getZoomSelectionAction().setEnabled(!targetSelectionIndices.empty() && !navigationAction.getFreezeNavigation().isChecked());

    _positionDataset->setSelectionIndices(targetSelectionIndices);

    events().notifyDatasetDataSelectionChanged(_positionDataset->getSourceDataset<Points>());
}

void ScatterplotPlugin::samplePoints()
{
    auto& samplerPixelSelectionTool = _scatterPlotWidget->getSamplerPixelSelectionTool();

    if (!_positionDataset.isValid() || _scatterPlotWidget->_pointRenderer.getNavigator().isNavigating() || !samplerPixelSelectionTool.isActive())
        return;

    auto selectionAreaImage = samplerPixelSelectionTool.getAreaPixmap().toImage();

    std::vector<std::uint32_t> targetSelectionIndices;

    targetSelectionIndices.reserve(_positionDataset->getNumPoints());

    std::vector<std::uint32_t> localGlobalIndices;

    _positionDataset->getGlobalIndices(localGlobalIndices);
    
    std::vector<char> focusHighlights(_positions.size());

    std::vector<std::pair<float, std::uint32_t>> sampledPoints;

    auto& pointRenderer = _scatterPlotWidget->_pointRenderer;
    auto& navigator     = pointRenderer.getNavigator();

    const auto zoomRectangleWorld   = navigator.getZoomRectangleWorld();
    const auto screenRectangle      = QRect(QPoint(), pointRenderer.getRenderSize());
    const auto mousePositionWorld   = pointRenderer.getScreenPointToWorldPosition(pointRenderer.getNavigator().getViewMatrix(), _scatterPlotWidget->mapFromGlobal(QCursor::pos()));

    // Go over all points in the dataset to see if they should be sampled
    for (std::uint32_t localPointIndex = 0; localPointIndex < _positions.size(); localPointIndex++) {

        // Compute the offset of the point in the world space
        const auto pointOffsetWorld = QPointF(_positions[localPointIndex].x - zoomRectangleWorld.left(), _positions[localPointIndex].y - zoomRectangleWorld.top());

        // Normalize it 
        const auto pointOffsetWorldNormalized = QPointF(pointOffsetWorld.x() / zoomRectangleWorld.width(), pointOffsetWorld.y() / zoomRectangleWorld.height());

        // Convert it to screen space
        const auto pointOffsetScreen = QPoint(pointOffsetWorldNormalized.x() * screenRectangle.width(), screenRectangle.height() - pointOffsetWorldNormalized.y() * screenRectangle.height());

        // Continue to next point if the point is outside the screen
        if (!screenRectangle.contains(pointOffsetScreen))
            continue;

        // If the corresponding pixel is not transparent, add the point to the selection
        if (selectionAreaImage.pixelColor(pointOffsetScreen).alpha() > 0) {
            const auto sample = std::pair((QVector2D(_positions[localPointIndex].x, _positions[localPointIndex].y) - mousePositionWorld.toVector2D()).length(), localPointIndex);

            sampledPoints.emplace_back(sample);
        }
    }
    
    QVariantList localPointIndices, globalPointIndices, distances;

    localPointIndices.reserve(static_cast<std::int32_t>(sampledPoints.size()));
    globalPointIndices.reserve(static_cast<std::int32_t>(sampledPoints.size()));
    distances.reserve(static_cast<std::int32_t>(sampledPoints.size()));

    std::int32_t numberOfPoints = 0;

    std::sort(sampledPoints.begin(), sampledPoints.end(), [](const auto& sampleA, const auto& sampleB) -> bool {
        return sampleB.first > sampleA.first;
    });

    for (const auto& sampledPoint : sampledPoints) {
        if (getSamplerAction().getRestrictNumberOfElementsAction().isChecked() && numberOfPoints >= getSamplerAction().getMaximumNumberOfElementsAction().getValue())
            break;

        const auto& distance            = sampledPoint.first;
        const auto& localPointIndex     = sampledPoint.second;
        const auto& globalPointIndex    = localGlobalIndices[localPointIndex];

        distances << distance;
        localPointIndices << localPointIndex;
        globalPointIndices << globalPointIndex;

        focusHighlights[localPointIndex] = true;

        numberOfPoints++;
    }

    if (getSamplerAction().getHighlightFocusedElementsAction().isChecked())
        const_cast<PointRenderer&>(_scatterPlotWidget->getPointRenderer()).setFocusHighlights(focusHighlights, static_cast<std::int32_t>(focusHighlights.size()));

    _scatterPlotWidget->update();

    auto& coloringAction = _settingsAction.getColoringAction();

    getSamplerAction().setSampleContext({
        { "PositionDatasetID", _positionDataset.getDatasetId() },
        { "ColorDatasetID", _settingsAction.getColoringAction().getCurrentColorDataset().getDatasetId() },
        { "LocalPointIndices", localPointIndices },
        { "GlobalPointIndices", globalPointIndices },
        { "Distances", distances },
        { "ColorBy", coloringAction.getColorByAction().getCurrentText() },
        { "ConstantColor", coloringAction.getConstantColorAction().getColor() },
        { "ColorMap1D", coloringAction.getColorMap1DAction().getColorMapImage() },
        { "ColorMap2D", coloringAction.getColorMap2DAction().getColorMapImage() },
        { "ColorDimensionIndex", coloringAction.getDimensionAction().getCurrentDimensionAction().getCurrentIndex() },
        { "RenderMode", _settingsAction.getRenderModeAction().getCurrentText() }
    });
}

Dataset<Points>& ScatterplotPlugin::getPositionDataset()
{
    return _positionDataset;
}

Dataset<Points>& ScatterplotPlugin::getPositionSourceDataset()
{
    return _positionSourceDataset;
}

void ScatterplotPlugin::positionDatasetChanged()
{
    _dropWidget->setShowDropIndicator(!_positionDataset.isValid());
    _scatterPlotWidget->getPixelSelectionTool().setEnabled(_positionDataset.isValid());

    if (!_positionDataset.isValid())
        return;
     
    // Reset dataset references
    //_positionSourceDataset.reset();

    // Set position source dataset reference when the position dataset is derived
    //if (_positionDataset->isDerivedData())
    _positionSourceDataset = _positionDataset->getSourceDataset<Points>();

    _numPoints = _positionDataset->getNumPoints();

    _scatterPlotWidget->getPointRendererNavigator().resetView(true);
    _scatterPlotWidget->getDensityRendererNavigator().resetView(true);

    updateData();
}

void ScatterplotPlugin::loadColors(const Dataset<Points>& pointsColor, const std::uint32_t& dimensionIndex)
{
    // Only proceed with valid points dataset
    if (!pointsColor.isValid())
        return;

    const auto numColorPoints = pointsColor->getNumPoints();

    // Generate point colorScalars for color mapping
    std::vector<float> colorScalars = {};
    pointsColor->extractDataForDimension(colorScalars, dimensionIndex);

    // If number of points do not match, use a mapping
    // prefer global IDs (for derived data) over selection mapping
    // prefer color to position over position to color over source of position to color
    if (numColorPoints != _numPoints) {

        std::vector<float> mappedColorScalars(_numPoints, std::numeric_limits<float>::lowest());

        try {
            const bool hasSameNumPointsAsFull = fullSourceHasSameNumPoints(_positionDataset, pointsColor);

            if (hasSameNumPointsAsFull) {
                std::vector<std::uint32_t> globalIndices = {};
                _positionDataset->getGlobalIndices(globalIndices);

                for (std::int32_t localIndex = 0; localIndex < globalIndices.size(); localIndex++) {
                    mappedColorScalars[localIndex] = colorScalars[globalIndices[localIndex]];
                }

            }
            else if ( // mapping from color data set to position data set
                const auto [selectionMapping, numPointsTarget] = getSelectionMappingColorsToPositions(pointsColor, _positionDataset);
                /* check if valid */ 
                selectionMapping != nullptr && 
                numPointsTarget == _numPoints &&
                checkSurjectiveMapping(*selectionMapping, numPointsTarget)
                )
            {
                // Map values like selection
                const mv::SelectionMap::Map& mapColorsToPositions = selectionMapping->getMapping().getMap();

                for (const auto& [fromColorID, vecOfPositionIDs] : mapColorsToPositions) {
                    for (std::uint32_t toPositionID : vecOfPositionIDs) {
                        mappedColorScalars[toPositionID] = colorScalars[fromColorID];
                    }
                }

            }
            else if ( // mapping from position data set to color data set 
                const auto [selectionMapping, numPointsTarget] = getSelectionMappingPositionsToColors(_positionDataset, pointsColor);
                /* check if valid */ 
                selectionMapping != nullptr &&
                numPointsTarget == numColorPoints &&
                checkSurjectiveMapping(*selectionMapping, numPointsTarget)
                )
            {
                // Map values like selection (in reverse, use first value that occurs)
                const mv::SelectionMap::Map& mapPositionsToColors = selectionMapping->getMapping().getMap();

                for (const auto& [fromPositionID, vecOfColorIDs] : mapPositionsToColors) {
                    if (mappedColorScalars[fromPositionID] != std::numeric_limits<float>::lowest())
                        continue;
                    for (std::uint32_t toColorID : vecOfColorIDs) {
                        mappedColorScalars[fromPositionID] = colorScalars[toColorID];
                    }
                }

            }
            else if ( // mapping from source of position data set to color data set 
                const auto [selectionMapping, numPointsTarget] = getSelectionMappingPositionSourceToColors(_positionDataset, pointsColor);
                /* check if valid */ 
                selectionMapping != nullptr && 
                numPointsTarget == numColorPoints &&
                checkSurjectiveMapping(*selectionMapping, numPointsTarget)
                )
            {
                // the selection map is from full source data of positions data to pointsColor
                // we need to use both the global indices of the positions (i.e. in the source) and the linked data mapping
                const mv::SelectionMap::Map& mapGlobalToColors = selectionMapping->getMapping().getMap();
                std::vector<std::uint32_t> globalIndices = {};
                _positionDataset->getGlobalIndices(globalIndices);

                for (std::int32_t localIndex = 0; localIndex < globalIndices.size(); localIndex++) {

                    if (mappedColorScalars[localIndex] != std::numeric_limits<float>::lowest())
                        continue;

                    const auto& indxColors = mapGlobalToColors.at(globalIndices[localIndex]);   // from full source (parent) to colorDataset

                    for (const auto& indColors : indxColors) {
                        mappedColorScalars[localIndex] = colorScalars[indColors];
                    }
                }

            }
            else {
                throw std::runtime_error("Coloring data set does not match position data set in a known way, aborting attempt to color plot");
            }

        }
        catch (const std::exception& e) {
            qDebug() << "ScatterplotPlugin::loadColors: mapping failed -> " << e.what();
            _settingsAction.getColoringAction().getColorByAction().setCurrentIndex(0);  // reset to color by constant
            return;
        }
        catch (...) {
            qDebug() << "ScatterplotPlugin::loadColors: mapping failed for an unknown reason.";
            _settingsAction.getColoringAction().getColorByAction().setCurrentIndex(0);  // reset to color by constant
            return;
        }

        std::swap(mappedColorScalars, colorScalars);
    }

    assert(colorScalars.size() == _numPoints);

    // Assign colorScalars and scalar effect
    _scatterPlotWidget->setScalars(colorScalars);
    _scatterPlotWidget->setScalarEffect(PointEffect::Color);

    _settingsAction.getColoringAction().updateColorMapActionScalarRange();

    // Render
    getWidget().update();
}

void ScatterplotPlugin::loadColors(const Dataset<Clusters>& clusters)
{
    // Only proceed with valid clusters and position dataset
    if (!clusters.isValid() || !_positionDataset.isValid())
        return;

    // Mapping from local to global indices
    std::vector<std::uint32_t> globalIndices;

    // Get global indices from the position dataset
    int totalNumPoints = 0;
    if (_positionDataset->isDerivedData())
        totalNumPoints = _positionSourceDataset->getFullDataset<Points>()->getNumPoints();
    else
        totalNumPoints = _positionDataset->getFullDataset<Points>()->getNumPoints();

    _positionDataset->getGlobalIndices(globalIndices);

    // Generate color buffer for global and local colors
    std::vector<Vector3f> globalColors(totalNumPoints);
    std::vector<Vector3f> localColors(_positions.size());

    const auto& clusterVec = clusters->getClusters();

    if (totalNumPoints == _positions.size() && clusterVec.size() == totalNumPoints)
    {
        for (size_t i = 0; i < static_cast<size_t>(clusterVec.size()); i++)
        {
            const auto& cluster = clusterVec[i];
            const auto color    = cluster.getColor();

            localColors[cluster.getIndices()[0]] = Vector3f(color.redF(), color.greenF(), color.blueF());
        }

    }
    else
    {
        // Loop over all clusters and populate global colors
        for (const auto& cluster : clusterVec)
        {
            const auto color = cluster.getColor();
            for (const auto& index : cluster.getIndices())
                globalColors[index] = Vector3f(color.redF(), color.greenF(), color.blueF());

        }

        // Loop over all global indices and find the corresponding local color
        std::int32_t localColorIndex = 0;
        for (const auto& globalIndex : globalIndices)
            localColors[localColorIndex++] = globalColors[globalIndex];
    }

    // Apply colors to scatter plot widget without modification
    _scatterPlotWidget->setColors(localColors);

    // Render
    getWidget().update();
}

ScatterplotWidget& ScatterplotPlugin::getScatterplotWidget()
{
    return *_scatterPlotWidget;
}

void ScatterplotPlugin::updateData()
{
    // Check if the scatter plot is initialized, if not, don't do anything
    if (!_scatterPlotWidget->isInitialized())
        return;

    // If no dataset has been selected, don't do anything
    if (_positionDataset.isValid()) {

        // Get the selected dimensions to use as X and Y dimension in the plot
        const auto xDim = _settingsAction.getPositionAction().getDimensionX();
        const auto yDim = _settingsAction.getPositionAction().getDimensionY();

        // If one of the dimensions was not set, do not draw anything
        if (xDim < 0 || yDim < 0)
            return;

        // Ensure that if positionDataset has now more points, the additional points are plotted
        if (_numPoints != _positionDataset->getNumPoints())
        {
            _settingsAction.getPlotAction().getPointPlotAction().updateScatterPlotWidgetPointSizeScalars();
            _settingsAction.getPlotAction().getPointPlotAction().updateScatterPlotWidgetPointOpacityScalars();
        }

        // Determine number of points depending on if its a full dataset or a subset
        _numPoints = _positionDataset->getNumPoints();

        // Extract 2-dimensional points from the data set based on the selected dimensions
        _positionDataset->extractDataForDimensions(_positions, xDim, yDim);

        // Pass the 2D points to the scatter plot widget
        _scatterPlotWidget->setData(&_positions);

        updateSelection();
    }
    else {
        _numPoints = 0;
        _positions.clear();
        _scatterPlotWidget->setData(&_positions);
    }
}

void ScatterplotPlugin::updateSelection()
{
    if (!_positionDataset.isValid())
        return;

    //Timer timer(__FUNCTION__);

    auto selection = _positionDataset->getSelection<Points>();

    std::vector<bool> selected;
    std::vector<char> highlights;

    _positionDataset->selectedLocalIndices(selection->indices, selected);

    highlights.resize(_positionDataset->getNumPoints(), 0);

    for (std::size_t i = 0; i < selected.size(); i++)
        highlights[i] = selected[i] ? 1 : 0;

    _scatterPlotWidget->setHighlights(highlights, static_cast<std::int32_t>(selection->indices.size()));

    if (getSamplerAction().getSamplingMode() == ViewPluginSamplerAction::SamplingMode::Selection) {
        std::vector<std::uint32_t> localGlobalIndices;

        _positionDataset->getGlobalIndices(localGlobalIndices);

        std::vector<std::uint32_t> sampledPoints;

        sampledPoints.reserve(_positions.size());

        for (auto selectionIndex : selection->indices)
            sampledPoints.push_back(selectionIndex);

        std::int32_t numberOfPoints = 0;

        QVariantList localPointIndices, globalPointIndices;

        const auto numberOfSelectedPoints = selection->indices.size();

        localPointIndices.reserve(static_cast<std::int32_t>(numberOfSelectedPoints));
        globalPointIndices.reserve(static_cast<std::int32_t>(numberOfSelectedPoints));

        for (const auto& sampledPoint : sampledPoints) {
            if (getSamplerAction().getRestrictNumberOfElementsAction().isChecked() && numberOfPoints >= getSamplerAction().getMaximumNumberOfElementsAction().getValue())
                break;

            const auto& localPointIndex = sampledPoint;
            const auto& globalPointIndex = localGlobalIndices[localPointIndex];

            localPointIndices << localPointIndex;
            globalPointIndices << globalPointIndex;

            numberOfPoints++;
        }

        _scatterPlotWidget->update();

        auto& coloringAction = _settingsAction.getColoringAction();

        getSamplerAction().setSampleContext({
            { "PositionDatasetID", _positionDataset.getDatasetId() },
            { "ColorDatasetID", _settingsAction.getColoringAction().getCurrentColorDataset().getDatasetId() },
            { "LocalPointIndices", localPointIndices },
            { "GlobalPointIndices", globalPointIndices },
            { "Distances", QVariantList()},
            { "ColorBy", coloringAction.getColorByAction().getCurrentText() },
            { "ConstantColor", coloringAction.getConstantColorAction().getColor() },
            { "ColorMap1D", coloringAction.getColorMap1DAction().getColorMapImage() },
            { "ColorMap2D", coloringAction.getColorMap2DAction().getColorMapImage() },
            { "ColorDimensionIndex", coloringAction.getDimensionAction().getCurrentDimensionAction().getCurrentIndex() },
            { "RenderMode", _settingsAction.getRenderModeAction().getCurrentText() }
		});
    }
}

void ScatterplotPlugin::updateHeadsUpDisplay()
{
    getHeadsUpDisplayAction().removeAllHeadsUpDisplayItems();

    if (_positionDataset.isValid()) {
        auto datasetsItem = getHeadsUpDisplayAction().addHeadsUpDisplayItem("Datasets", "", "");

        getHeadsUpDisplayAction().addHeadsUpDisplayItem("Position by:", _positionDataset->getGuiName(), "", datasetsItem);

        if (_settingsAction.getColoringAction().getCurrentColorDataset().isValid())
            getHeadsUpDisplayAction().addHeadsUpDisplayItem("Color by:", _settingsAction.getColoringAction().getCurrentColorDataset()->getGuiName(), "", datasetsItem);
    } else {
        getHeadsUpDisplayAction().addHeadsUpDisplayItem("No datasets loaded", "", "");
    }
}

void ScatterplotPlugin::fromVariantMap(const QVariantMap& variantMap)
{
    ViewPlugin::fromVariantMap(variantMap);

    variantMapMustContain(variantMap, "Settings");

    auto& pointRenderer = const_cast<PointRenderer&>(_scatterPlotWidget->getPointRenderer());

    pointRenderer.getNavigator().resetView(true);

    _primaryToolbarAction.fromParentVariantMap(variantMap);
    _settingsAction.fromParentVariantMap(variantMap);

    

    if (pointRenderer.getNavigator().getNavigationAction().getSerializationCountFrom() == 0) {
        qDebug() << "Resetting view";
        

        _scatterPlotWidget->update();
    }
}

QVariantMap ScatterplotPlugin::toVariantMap() const
{
    QVariantMap variantMap = ViewPlugin::toVariantMap();

    _primaryToolbarAction.insertIntoVariantMap(variantMap);
    _settingsAction.insertIntoVariantMap(variantMap);

    return variantMap;
}

std::uint32_t ScatterplotPlugin::getNumberOfPoints() const
{
    if (!_positionDataset.isValid())
        return 0;

    return _positionDataset->getNumPoints();
}

void ScatterplotPlugin::setXDimension(const std::int32_t& dimensionIndex)
{
    updateData();
}

void ScatterplotPlugin::setYDimension(const std::int32_t& dimensionIndex)
{
    updateData();
}

ScatterplotPluginFactory::ScatterplotPluginFactory()
{
    setIconByName("braille");

    getPluginMetadata().setDescription("Scatterplot view");
    getPluginMetadata().setSummary("High-performance scatterplot plugin for ManiVault Studio, capable of handling millions of data points.");
    getPluginMetadata().setCopyrightHolder({ "BioVault (Biomedical Visual Analytics Unit LUMC - TU Delft)" });
    getPluginMetadata().setAuthors({
        { "J. Thijssen", { "Software architect" }, { "LUMC", "TU Delft" } },
        { "T. Kroes", { "Lead software architect" }, { "LUMC" } },
        { "A. Vieth", { "Plugin developer", "Maintainer" }, { "LUMC", "TU Delft" } }
	});
    getPluginMetadata().setOrganizations({
        { "LUMC", "Leiden University Medical Center", "https://www.lumc.nl/en/" },
        { "TU Delft", "Delft university of technology", "https://www.tudelft.nl/" }
	});
    getPluginMetadata().setLicenseText("This plugin is distributed under the [LGPL v3.0](https://www.gnu.org/licenses/lgpl-3.0.en.html) license.");
}

ViewPlugin* ScatterplotPluginFactory::produce()
{
    return new ScatterplotPlugin(this);
}

PluginTriggerActions ScatterplotPluginFactory::getPluginTriggerActions(const mv::Datasets& datasets) const
{
    PluginTriggerActions pluginTriggerActions;

    const auto getInstance = [this]() -> ScatterplotPlugin* {
        return dynamic_cast<ScatterplotPlugin*>(Application::core()->getPluginManager().requestViewPlugin(getKind()));
    };

    const auto numberOfDatasets = datasets.count();

    if (PluginFactory::areAllDatasetsOfTheSameType(datasets, PointType)) {
        if (numberOfDatasets >= 1) {
            auto pluginTriggerAction = new PluginTriggerAction(const_cast<ScatterplotPluginFactory*>(this), this, "Scatterplot", "View selected datasets side-by-side in separate scatter plot viewers", StyledIcon("braille"), [this, getInstance, datasets](PluginTriggerAction& pluginTriggerAction) -> void {
                for (const auto& dataset : datasets)
                    getInstance()->loadData(Datasets({ dataset }));
            });

            pluginTriggerActions << pluginTriggerAction;
        }
    }

    /*
    const auto numberOfPointsDatasets   = PluginFactory::getNumberOfDatasetsForType(datasets, PointType);
    const auto numberOfClusterDatasets  = PluginFactory::getNumberOfDatasetsForType(datasets, ClusterType);

    if (numberOfPointsDatasets == numberOfClusterDatasets) {
        QRegularExpression re("(Points, Clusters)");

        const auto reMatch = re.match(PluginFactory::getDatasetTypesAsStringList(datasets).join(","));

        if (reMatch.hasMatch() && reMatch.captured().count() == numberOfPointsDatasets) {
            auto pluginTriggerAction = createPluginTriggerAction("Scatterplot", "Load points dataset in separate viewer and apply cluster", datasets, "braille");

            connect(pluginTriggerAction, &QAction::triggered, [this, getInstance, datasets, numberOfPointsDatasets]() -> void {

                for (int i = 0; i < numberOfPointsDatasets; i++) {
                    getInstance()->loadData(Datasets({ datasets[i * 2] }));
                    getInstance()->loadColors(Dataset<Clusters>(datasets[i * 2 + 1]));
                }
            });

            pluginTriggerActions << pluginTriggerAction;
        }
    }
    */

    return pluginTriggerActions;
}

QUrl ScatterplotPluginFactory::getRepositoryUrl() const
{
    return { "https://github.com/ManiVaultStudio/Scatterplot" };
}
