#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"
#include "DataHierarchyItem.h"
#include "Application.h"

#include "util/PixelSelectionTool.h"

#include "PointData.h"
#include "ClusterData.h"
#include "ColorData.h"

#include "graphics/Vector2f.h"
#include "graphics/Vector3f.h"
#include "widgets/DropWidget.h"

#include <QtCore>
#include <QApplication>
#include <QDebug>
#include <QMenu>
#include <QAction>
#include <QMetaType>

#include <algorithm>
#include <functional>
#include <limits>
#include <set>
#include <vector>

Q_PLUGIN_METADATA(IID "nl.tudelft.ScatterplotPlugin")

using namespace hdps;
using namespace hdps::util;

ScatterplotPlugin::ScatterplotPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _positionDataset(),
    _positionSourceDataset(),
    _positions(),
    _numPoints(0),
    _scatterPlotWidget(new ScatterplotWidget()),
    _dropWidget(nullptr),
    _settingsAction(this)
{
    _dropWidget = new DropWidget(_scatterPlotWidget);

    setDockingLocation(DockableWidget::DockingLocation::Right);
    setFocusPolicy(Qt::ClickFocus);

    connect(_scatterPlotWidget, &ScatterplotWidget::customContextMenuRequested, this, [this](const QPoint& point) {
        if (!_positionDataset.isValid())
            return;

        auto contextMenu = _settingsAction.getContextMenu();
        
        contextMenu->addSeparator();

        _positionDataset->populateContextMenu(contextMenu);

        contextMenu->exec(mapToGlobal(point));
    });

    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(this, "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        DropWidget::DropRegions dropRegions;

        const auto mimeText = mimeData->text();
        const auto tokens   = mimeText.split("\n");

        if (tokens.count() == 1)
            return dropRegions;

        const auto datasetGuiName       = tokens[0];
        const auto datasetId            = tokens[1];
        const auto dataType             = DataType(tokens[2]);
        const auto dataTypes            = DataTypes({ PointType , ColorType, ClusterType });

        // Check if the data type can be dropped
        if (!dataTypes.contains(dataType))
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", false);

        // Points dataset is about to be dropped
        if (dataType == PointType) {

            // Get points dataset from the core
            auto candidateDataset = _core->requestDataset<Points>(datasetId);

            // Establish drop region description
            const auto description = QString("Visualize %1 as points or density/contour map").arg(datasetGuiName);

            if (!_positionDataset.isValid()) {

                // Load as point positions when no dataset is currently loaded
                dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDataset]() {
                    _positionDataset = candidateDataset;
                });
            }
            else {
                if (candidateDataset == _positionDataset) {

                    // Present the user with a warning that the dataset is already loaded
                    dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", false);
                }
                else {
                    if (_positionDataset->getNumPoints() != candidateDataset->getNumPoints()) {

                        // The number of points is not equal, so replace the old points dataset with the dropped one
                        dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDataset]() {
                            _positionDataset = candidateDataset;
                        });
                    }
                    else {

                        // The number of points is equal, so offer the option to replace the existing points dataset
                        dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDataset]() {
                            _positionDataset = candidateDataset;
                        });

                        // The number of points is equal, so offer the option to use the points dataset as source for points colors
                        dropRegions << new DropWidget::DropRegion(this, "Color", QString("Color %1 by %2").arg(_positionDataset->getGuiName(), candidateDataset->getGuiName()), true, [this, candidateDataset]() {
                            _settingsAction.getColoringAction().getColorByDataAction().addColorDataset(candidateDataset);
                        });
                    }
                }
            }
        }

        // Cluster dataset is about to be dropped
        if (dataType == ClusterType) {

            // Get clusters dataset from the core
            auto candidateDataset  = _core->requestDataset<Clusters>(datasetId);
            
            // Establish drop region description
            const auto description  = QString("Color points by %1").arg(candidateDataset->getGuiName());

            // Only allow user to color by clusters when there is a positions dataset loaded
            if (_positionDataset.isValid()) {

                if (_settingsAction.getColoringAction().getColorByDataAction().hasColorDataset(candidateDataset)) {

                    // The clusters dataset is already loaded
                    dropRegions << new DropWidget::DropRegion(this, "Color", "Cluster set is already in use", false, [this]() {});
                }
                else {

                    // Use the clusters set for points color
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, true, [this, candidateDataset]() {
                        _settingsAction.getColoringAction().getColorByDataAction().addColorDataset(candidateDataset);
                    });
                }
            }
            else {

                // Only allow user to color by clusters when there is a positions dataset loaded
                dropRegions << new DropWidget::DropRegion(this, "No points data loaded", "Clusters can only be visualized in concert with points data", false);
            }
        }

        return dropRegions;
    });
}

ScatterplotPlugin::~ScatterplotPlugin()
{
}

void ScatterplotPlugin::init()
{
    auto layout = new QVBoxLayout();

    layout->setMargin(0);
    layout->setSpacing(0);
    layout->addWidget(_settingsAction.createWidget(this));
    layout->addWidget(_scatterPlotWidget, 1);

    setLayout(layout);

    connect(_scatterPlotWidget, &ScatterplotWidget::initialized, this, &ScatterplotPlugin::updateData);

    // Register for points dataset events
    registerDataEventByType(PointType, [this](DataEvent* dataEvent) {
        if (!_positionDataset.isValid())
            return;

        switch (dataEvent->getType())
        {
            case EventType::DataChanged:
            {
                if (dataEvent->getDataset() != _positionDataset)
                    return;

                updateData();

                break;
            }

            case EventType::DataSelectionChanged:
            {
                if (dataEvent->getDataset() != _positionDataset)
                    return;

                updateSelection();

                break;
            }

            default:
                break;
        }
    });

    // Update the selection when the pixel selection tool selected area changed
    connect(&_scatterPlotWidget->getPixelSelectionTool(), &PixelSelectionTool::areaChanged, [this]() {
        if (_scatterPlotWidget->getPixelSelectionTool().isNotifyDuringSelection())
            selectPoints();
    });

    connect(&_scatterPlotWidget->getPixelSelectionTool(), &PixelSelectionTool::ended, [this]() {
        if (_scatterPlotWidget->getPixelSelectionTool().isNotifyDuringSelection())
            return;

        selectPoints();
    });

    // Load points when the pointer to the position dataset changes
    connect(&_positionDataset, &Dataset<Points>::changed, this, &ScatterplotPlugin::positionDatasetChanged);

    // Update the window title when the GUI name of the position dataset changes
    connect(&_positionDataset, &Dataset<Points>::dataGuiNameChanged, this, &ScatterplotPlugin::updateWindowTitle);

    // Do an initial update of the window title
    updateWindowTitle();
}

void ScatterplotPlugin::createSubset(const bool& fromSourceData /*= false*/, const QString& name /*= ""*/)
{
    auto subsetPoints  = _positionDataset->getSourceDataset<Points>();

    // Create the subset
    auto& subset = subsetPoints->createSubset(_positionDataset->getGuiName(), _positionDataset);

    // Notify others that the subset was added
    _core->notifyDataAdded(subset);
    
    // And select the subset
    subset->getDataHierarchyItem().select();
}

void ScatterplotPlugin::selectPoints()
{
    if (!_positionDataset.isValid() || !_scatterPlotWidget->getPixelSelectionTool().isActive())
        return;

    auto selectionAreaImage = _scatterPlotWidget->getPixelSelectionTool().getAreaPixmap().toImage();

    auto selectionSet = _positionDataset->getSelection<Points>();

    std::vector<std::uint32_t> targetIndices;

    targetIndices.reserve(_positionDataset->getNumPoints());
    std::vector<unsigned int> localGlobalIndices;
    
    _positionDataset->getGlobalIndices(localGlobalIndices);

    const auto dataBounds   = _scatterPlotWidget->getBounds();
    const auto width        = selectionAreaImage.width();
    const auto height       = selectionAreaImage.height();
    const auto size         = width < height ? width : height;
    const auto set          = _positionDataset->getSourceDataset<Points>();
    const auto setIndices   = set->indices;

    std::vector<unsigned int> localIndices;
    for (unsigned int i = 0; i < _positions.size(); i++) {
        const auto uvNormalized     = QPointF((_positions[i].x - dataBounds.getLeft()) / dataBounds.getWidth(), (dataBounds.getTop() - _positions[i].y) / dataBounds.getHeight());
        const auto uvOffset         = QPoint((selectionAreaImage.width() - size) / 2.0f, (selectionAreaImage.height() - size) / 2.0f);
        const auto uv               = uvOffset + QPoint(uvNormalized.x() * size, uvNormalized.y() * size);

        if (selectionAreaImage.pixelColor(uv).alpha() > 0) {
            int globalIndex = localGlobalIndices[i];
            targetIndices.push_back(globalIndex);
            localIndices.push_back(i); // FIXME Find more performant way to add this
        }
    }
    
    auto& selectionSetIndices = selectionSet->indices;

    switch (_scatterPlotWidget->getPixelSelectionTool().getModifier())
    {
        case PixelSelectionModifierType::Replace:
            break;

        case PixelSelectionModifierType::Add:
        case PixelSelectionModifierType::Remove:
        {
            QSet<std::uint32_t> set(selectionSetIndices.begin(), selectionSetIndices.end());

            switch (_scatterPlotWidget->getPixelSelectionTool().getModifier())
            {
                case PixelSelectionModifierType::Add:
                {
                    for (const auto& targetIndex : targetIndices)
                        set.insert(targetIndex);

                    break;
                }

                case PixelSelectionModifierType::Remove:
                {
                    for (const auto& targetIndex : targetIndices)
                        set.remove(targetIndex);

                    break;
                }

                default:
                    break;
            }

            targetIndices = std::vector<std::uint32_t>(set.begin(), set.end());

            break;
        }

        default:
            break;
    }

    _positionDataset->setSelection(targetIndices);

    // Notify others that the selection changed
    _core->notifyDataSelectionChanged(_positionDataset);
}

void ScatterplotPlugin::updateWindowTitle()
{
    if (!_positionDataset.isValid())
        setWindowTitle(getGuiName());
    else
        setWindowTitle(QString("%1: %2").arg(getGuiName(), _positionDataset->getGuiName()));
}

Dataset<Points>& ScatterplotPlugin::getPositionDataset()
{
    return _positionDataset;
}

Dataset<Points>& ScatterplotPlugin::getPositionSourceDataset()
{
    return _positionSourceDataset;
}

QStringList ScatterplotPlugin::getClusterDatasetNames()
{
    QStringList clusterDatasetNames;

    /* TODO
    if (!_points.isValid())
        return clusterDatasetNames;

    for (auto child : _points->getDataHierarchyItem().getChildren())
        if (child->getDataType() == ClusterType)
            clusterDatasetNames << child->getDatasetName();
    */

    return clusterDatasetNames;
}

void ScatterplotPlugin::positionDatasetChanged()
{
    // Only proceed if we have a valid position dataset
    if (!_positionDataset.isValid())
        return;

    // Reset dataset references
    _positionSourceDataset.reset();

    // Set position source dataset reference when the position dataset is derived
    if (_positionDataset->isDerivedData())
        _positionSourceDataset = _positionDataset->getSourceDataset<Points>();

    // Enable pixel selection if the point positions dataset is valid
    _scatterPlotWidget->getPixelSelectionTool().setEnabled(_positionDataset.isValid());
    
    // Do not show the drop indicator if there is a valid point positions dataset
    _dropWidget->setShowDropIndicator(!_positionDataset.isValid());

    // Update positions data
    updateData();

    // Update the window title to reflect the position dataset change
    updateWindowTitle();
}

void ScatterplotPlugin::loadColors(const Dataset<Points>& points, const std::uint32_t& dimensionIndex)
{
    // Only proceed with valid points dataset
    if (!points.isValid())
        return;

    // Generate point scalars for color mapping
    std::vector<float> scalars;

    if (_positionDataset->getNumPoints() != _numPoints)
    {
        qWarning("Number of points used for coloring does not match number of points in data, aborting attempt to color plot");
        return;
    }

    // Populate point scalars
    if (dimensionIndex >= 0)
        points->extractDataForDimension(scalars, dimensionIndex);

    // Assign scalars and scalar effect
    _scatterPlotWidget->setScalars(scalars);
    _scatterPlotWidget->setScalarEffect(PointEffect::Color);

    // Render
    update();
}

void ScatterplotPlugin::loadColors(const Dataset<Clusters>& clusters)
{
    // Only proceed with valid clusters dataset
    if (!clusters.isValid())
        return;

    // Generate colors buffer
    std::vector<Vector3f> colors(_positions.size());

    // Loop over all clusters
    for (const auto& cluster : clusters->getClusters())
    {
        // Get cluster color
        const auto clusterColor = cluster.getColor();

        // Loop over all indices in the cluster
        for (const auto& index : cluster.getIndices())
        {
            if (index < 0 || index > colors.size())
            {
                qWarning("Cluster index is out of range of data, aborting attempt to color plot");
                return;
            }

            // Assign point color
            colors[index] = Vector3f(clusterColor.redF(), clusterColor.greenF(), clusterColor.blueF());
        }
    }

    // Assign scalars and scalar effect
    _scatterPlotWidget->setColors(colors);

    // Render
    update();
}

ScatterplotWidget* ScatterplotPlugin::getScatterplotWidget()
{
    return _scatterPlotWidget;
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

        // Determine number of points depending on if its a full dataset or a subset
        _numPoints = _positionDataset->getNumPoints();

        // Extract 2-dimensional points from the data set based on the selected dimensions
        calculatePositions(*_positionDataset);

        // Pass the 2D points to the scatter plot widget
        _scatterPlotWidget->setData(&_positions);

        updateSelection();
    }
    else {
        _positions.clear();
        _scatterPlotWidget->setData(&_positions);
    }
}

void ScatterplotPlugin::calculatePositions(const Points& points)
{
    points.extractDataForDimensions(_positions, _settingsAction.getPositionAction().getDimensionX(), _settingsAction.getPositionAction().getDimensionY());
}

void ScatterplotPlugin::updateSelection()
{
    if (!_positionDataset.isValid())
        return;

    auto& selection = _positionDataset->getSelection<Points>();

    std::vector<bool> selected;
    std::vector<char> highlights;

    _positionDataset->selectedLocalIndices(selection->indices, selected);

    highlights.resize(_positionDataset->getNumPoints(), 0);

    for (int i = 0; i < selected.size(); i++)
        highlights[i] = selected[i] ? 1 : 0;

    _scatterPlotWidget->setHighlights(highlights, static_cast<std::int32_t>(selection->indices.size()));

    emit selectionChanged();
}

std::uint32_t ScatterplotPlugin::getNumberOfPoints() const
{
    if (!_positionDataset.isValid())
        return 0;

    return _positionDataset->getNumPoints();
}

std::uint32_t ScatterplotPlugin::getNumberOfSelectedPoints() const
{
    if (!_positionDataset.isValid())
        return 0;

    return static_cast<std::uint32_t>(_positionDataset->getSelection<Points>()->indices.size());
}

void ScatterplotPlugin::setXDimension(const std::int32_t& dimensionIndex)
{
    updateData();
}

void ScatterplotPlugin::setYDimension(const std::int32_t& dimensionIndex)
{
    updateData();
}

bool ScatterplotPlugin::canSelect() const
{
    return _positionDataset.isValid() && getNumberOfPoints() >= 0;
}

bool ScatterplotPlugin::canSelectAll() const
{
    return getNumberOfPoints() == -1 ? false : getNumberOfSelectedPoints() != getNumberOfPoints();
}

bool ScatterplotPlugin::canClearSelection() const
{
    return getNumberOfPoints() == -1 ? false : getNumberOfSelectedPoints() >= 1;
}

bool ScatterplotPlugin::canInvertSelection() const
{
    return getNumberOfPoints() >= 0;
}

void ScatterplotPlugin::selectAll()
{
    if (!_positionDataset.isValid())
        return;

    _positionDataset->selectAll();
}

void ScatterplotPlugin::clearSelection()
{
    if (!_positionDataset.isValid())
        return;

    _positionDataset->selectNone();
}

void ScatterplotPlugin::invertSelection()
{
    if (!_positionDataset.isValid())
        return;

    _positionDataset->selectInvert();
}

QIcon ScatterplotPluginFactory::getIcon() const
{
    return Application::getIconFont("FontAwesome").getIcon("braille");
}

ViewPlugin* ScatterplotPluginFactory::produce()
{
    return new ScatterplotPlugin(this);
}

hdps::DataTypes ScatterplotPluginFactory::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}
