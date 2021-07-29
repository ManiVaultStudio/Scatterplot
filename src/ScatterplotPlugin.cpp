#include "ScatterplotPlugin.h"

#include "PixelSelectionTool.h"
#include "ScatterplotWidget.h"

#include "Application.h"

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

ScatterplotPlugin::ScatterplotPlugin() :
    ViewPlugin("Scatterplot View"),
    _currentDataSet(),
    _currentColorDataSet(),
    _points(),
    _numPoints(0),
    _pixelSelectionTool(new PixelSelectionTool(this, false)),
    _scatterPlotWidget(new ScatterplotWidget(*_pixelSelectionTool)),
    _dropWidget(nullptr),
    _settingsAction(this)
{
    _scatterPlotWidget->setMouseTracking(true);

    _dropWidget = new DropWidget(_scatterPlotWidget);

    setDockingLocation(DockableWidget::DockingLocation::Right);
    setFocusPolicy(Qt::ClickFocus);

    connect(_scatterPlotWidget, &ScatterplotWidget::customContextMenuRequested, this, [this](const QPoint& point) {
        if (_currentDataSet.isEmpty())
            return;

        auto contextMenu = _settingsAction.getContextMenu();
        
        DataSet& dataSet = _core->requestData(_currentDataSet);

        const auto analyses = dataSet.getProperty("Analyses", QVariantList()).toList();
        
        contextMenu->addSeparator();

        auto datasetContextMenu = dataSet.getContextMenu();

        datasetContextMenu->setTitle(_currentDataSet);

        contextMenu->addMenu(datasetContextMenu);

        contextMenu->exec(mapToGlobal(point));
    });

    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(this, "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        DropWidget::DropRegions dropRegions;

        const auto mimeText             = mimeData->text();
        const auto tokens               = mimeText.split("\n");
        const auto datasetName          = tokens[0];
        const auto dataType             = DataType(tokens[1]);
        const auto dataTypes            = DataTypes({ PointType , ColorType, ClusterType });
        const auto currentDatasetName   = getCurrentDataset();
        const auto candidateDataset     = getCore()->requestData<Points>(datasetName);
        const auto candidateDatasetName = candidateDataset.getName();

        if (!dataTypes.contains(dataType))
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", false);

        if (dataType == PointType) {
            const auto description = QString("Visualize %1 as points or density/contour map").arg(candidateDatasetName);

            if (currentDatasetName.isEmpty()) {
                dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDatasetName]() {
                    loadPointData(candidateDatasetName);
                });
            }
            else {
                if (candidateDatasetName == currentDatasetName) {
                    dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", false);
                }
                else {
                    const auto currentDataset = getCore()->requestData<Points>(currentDatasetName);

                    if (currentDataset.getNumPoints() != candidateDataset.getNumPoints()) {
                        dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDatasetName]() {
                            loadPointData(candidateDatasetName);
                        });
                    }
                    else {
                        dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDatasetName]() {
                            loadPointData(candidateDatasetName);
                        });

                        if (candidateDatasetName != _currentColorDataSet) {
                            dropRegions << new DropWidget::DropRegion(this, "Color", QString("Color %1 by %2").arg(currentDatasetName, candidateDatasetName), true, [this, candidateDatasetName]() {
                                loadColorData(candidateDatasetName);
                            });
                        }
                    }
                }
            }
        }

        return dropRegions;
    });
}

ScatterplotPlugin::~ScatterplotPlugin()
{
}

QIcon ScatterplotPlugin::getIcon() const
{
    return Application::getIconFont("FontAwesome").getIcon("braille");
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

    registerDataEventByType(PointType, std::bind(&ScatterplotPlugin::onDataEvent, this, std::placeholders::_1));

    QObject::connect(_pixelSelectionTool, &PixelSelectionTool::areaChanged, [this]() {
        if (!_pixelSelectionTool->isNotifyDuringSelection())
            return;

        selectPoints();
    });

    QObject::connect(_pixelSelectionTool, &PixelSelectionTool::ended, [this]() {
        if (_pixelSelectionTool->isNotifyDuringSelection())
            return;

        selectPoints();
    });

    updateWindowTitle();
}

void ScatterplotPlugin::onDataEvent(DataEvent* dataEvent)
{
    if (dataEvent->getType() == EventType::DataChanged)
    {
        if (dataEvent->dataSetName != _currentDataSet) {
            return;
        }

        updateData();
    }
    if (dataEvent->getType() == EventType::SelectionChanged)
    {
        if (_currentDataSet.isEmpty()) return;
        
        if (_currentDataSet == dataEvent->dataSetName)
            updateSelection();
    }
    if (dataEvent->getType() == EventType::DataRenamed)
    {
        DataRenamedEvent* renamedEvent = (DataRenamedEvent*) dataEvent;

        if (renamedEvent->oldName == _currentDataSet)
            loadPointData(renamedEvent->dataSetName);
    }
}

hdps::DataTypes ScatterplotPlugin::supportedDataTypes() const
{
    DataTypes supportedTypes;
    supportedTypes.append(PointType);
    return supportedTypes;
}

void ScatterplotPlugin::createSubset(const bool& fromSourceData /*= false*/, const QString& name /*= ""*/)
{
    auto& loadedPoints  = _core->requestData<Points>(_currentDataSet);
    auto& subsetPoints  = loadedPoints.isDerivedData() && fromSourceData ? DataSet::getSourceData(loadedPoints) : loadedPoints;

    subsetPoints.createSubset();
}

void ScatterplotPlugin::selectPoints()
{
    if (_currentDataSet.isEmpty() || !_pixelSelectionTool->isActive())
        return;

    auto selectionAreaImage = _pixelSelectionTool->getAreaPixmap().toImage();

    const Points& points = _core->requestData<Points>(_currentDataSet);
    Points& selectionSet = dynamic_cast<Points&>(points.getSelection());

    std::vector<std::uint32_t> targetIndices;

    targetIndices.reserve(points.getNumPoints());
    std::vector<unsigned int> localGlobalIndices;
    points.getGlobalIndices(localGlobalIndices);

    const auto dataBounds   = _scatterPlotWidget->getBounds();
    const auto width        = selectionAreaImage.width();
    const auto height       = selectionAreaImage.height();
    const auto size         = width < height ? width : height;
    const auto& set         = points.isDerivedData() ? DataSet::getSourceData(points) : points;
    const auto& setIndices  = set.indices;

    std::vector<unsigned int> localIndices;
    for (unsigned int i = 0; i < _points.size(); i++) {
        const auto uvNormalized     = QPointF((_points[i].x - dataBounds.getLeft()) / dataBounds.getWidth(), (dataBounds.getTop() - _points[i].y) / dataBounds.getHeight());
        const auto uvOffset         = QPoint((selectionAreaImage.width() - size) / 2.0f, (selectionAreaImage.height() - size) / 2.0f);
        const auto uv               = uvOffset + QPoint(uvNormalized.x() * size, uvNormalized.y() * size);

        if (selectionAreaImage.pixelColor(uv).alpha() > 0) {
            int globalIndex = localGlobalIndices[i];
            targetIndices.push_back(globalIndex);
            localIndices.push_back(i); // FIXME Find more performant way to add this
        }
    }
    
    auto& selectionSetIndices = selectionSet.indices;

    switch (_pixelSelectionTool->getModifier())
    {
        case PixelSelectionTool::Modifier::Replace:
            break;

        case PixelSelectionTool::Modifier::Add:
        case PixelSelectionTool::Modifier::Remove:
        {
            QSet<std::uint32_t> set(selectionSetIndices.begin(), selectionSetIndices.end());

            switch (_pixelSelectionTool->getModifier())
            {
                case PixelSelectionTool::Modifier::Add:
                {
                    for (const auto& targetIndex : targetIndices)
                        set.insert(targetIndex);

                    break;
                }

                case PixelSelectionTool::Modifier::Remove:
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

    selectionSetIndices = targetIndices; // Global selection indices
    
    // TEMP HSNE selection
    {
        // Transmute local indices by drill indices specifying relation to full hierarchy scale
        if (points.hasProperty("drill_indices"))
        {
            QList<uint32_t> drillIndices = points.getProperty("drill_indices").value<QList<uint32_t>>();

            for (int i = 0; i < localIndices.size(); i++)
                localIndices[i] = drillIndices[localIndices[i]];
        }

        // Check if shown dataset is an HSNE embedding with a hierarchy
        if (points.hasProperty("scale"))
        {
            int scale = points.getProperty("scale").value<int>();

            if (scale > 0)
            {
                // Store the additionally selected points in a separate array not to cloud the for loop
                std::vector<unsigned int> extraSelectionIndices;
                extraSelectionIndices.reserve(selectionSetIndices.size()); // Reserve space at least as big as the current selected

                std::vector<std::vector<unsigned int>> landmarkMap = points.getProperty("landmarkMap").value<std::vector<std::vector<unsigned int>>>();
                //qDebug() << "Called broaden func: " << selectionSetIndices.size();
                //qDebug() << landmarkMap.size() << landmarkMap[scale].size();
                
                for (unsigned int localIndex : localIndices)
                    extraSelectionIndices.insert(extraSelectionIndices.end(), landmarkMap[localIndex].begin(), landmarkMap[localIndex].end());

                selectionSetIndices.insert(selectionSetIndices.end(), extraSelectionIndices.begin(), extraSelectionIndices.end());
                //qDebug() << "Broadened selection: " << selectionSetIndices.size();
            }
        }
    }

    _core->notifySelectionChanged(points.getName());
}

void ScatterplotPlugin::loadPointData(const QString& dataSetName)
{
    _currentDataSet = dataSetName;
    _currentColorDataSet = "";

    _scatterPlotWidget->resetColorMap();

    setWindowTitle(_currentDataSet);

    emit currentDatasetChanged(_currentDataSet);

    const Points& points = _core->requestData<Points>(_currentDataSet);

    // For source data determine whether to use dimension names or make them up
    if (points.getDimensionNames().size() == points.getNumDimensions())
        _settingsAction.getPositionAction().setDimensions(points.getDimensionNames());
    else
        _settingsAction.getPositionAction().setDimensions(points.getNumDimensions());

    // For derived data determine whether to use dimension names or make them up
    if (DataSet::getSourceData(points).getDimensionNames().size() == DataSet::getSourceData(points).getNumDimensions())
        _settingsAction.getColoringAction().setDimensions(DataSet::getSourceData(points).getDimensionNames());
    else
        _settingsAction.getColoringAction().setDimensions(DataSet::getSourceData(points).getNumDimensions());

    updateData();

    _pixelSelectionTool->setEnabled(!_currentDataSet.isEmpty());

    updateWindowTitle();

    _scatterPlotWidget->setColoringMode(ScatterplotWidget::ColoringMode::ConstantColor);
    _settingsAction.getColoringAction().getColorDataAction().getDatasetNameAction().setString("None loaded");

    _dropWidget->setShowDropIndicator(false);

    setFocus();
}

void ScatterplotPlugin::loadColorData(const QString& dataSetName)
{
    _currentColorDataSet = dataSetName;

    DataSet& dataSet = _core->requestData(dataSetName);

    DataType dataType = dataSet.getDataType();

    if (dataType == PointType)
    {
        Points& points = static_cast<Points&>(dataSet);

        std::vector<float> scalars;
        if (points.getNumPoints() != _numPoints)
        {
            qWarning("Number of points used for coloring does not match number of points in data, aborting attempt to color plot");
            return;
        }

        points.visitFromBeginToEnd([&scalars](auto begin, auto end)
            {
                scalars.insert(scalars.begin(), begin, end);
            });

        _scatterPlotWidget->setScalars(scalars);
        _scatterPlotWidget->setScalarEffect(PointEffect::Color);
        updateData();
    }
    if (dataType == ClusterType)
    {
        Clusters& clusters = static_cast<Clusters&>(dataSet);
        
        std::vector<Vector3f> colors(_points.size());
        for (const Cluster& cluster : clusters.getClusters())
        {
            for (const int& index : cluster.indices)
            {
                if (index < 0 || index > colors.size())
                {
                    qWarning("Cluster index is out of range of data, aborting attempt to color plot");
                    return;
                }
                colors[index] = Vector3f(cluster._color.redF(), cluster._color.greenF(), cluster._color.blueF());
            }
        }

        _scatterPlotWidget->setColors(colors);

        updateData();
    }

    _scatterPlotWidget->setColoringMode(ScatterplotWidget::ColoringMode::ColorData);
    _settingsAction.getColoringAction().getColorDataAction().getDatasetNameAction().setString(dataSetName);

    _dropWidget->setShowDropIndicator(false);

    setFocus();
}

ScatterplotWidget* ScatterplotPlugin::getScatterplotWidget()
{
    return _scatterPlotWidget;
}

hdps::CoreInterface* ScatterplotPlugin::getCore()
{
    return _core;
}

void ScatterplotPlugin::updateData()
{
    // Check if the scatter plot is initialized, if not, don't do anything
    if (!_scatterPlotWidget->isInitialized())
        return;
    
    // If no dataset has been selected, don't do anything
    if (_currentDataSet.isEmpty())
        return;

    // Get the dataset belonging to the currently displayed dataset
    const Points& points = _core->requestData<Points>(_currentDataSet);

    // Get the selected dimensions to use as X and Y dimension in the plot
    int xDim = _settingsAction.getPositionAction().getXDimension();
    int yDim = _settingsAction.getPositionAction().getYDimension();

    // If one of the dimensions was not set, do not draw anything
    if (xDim < 0 || yDim < 0)
        return;

    // Determine number of points depending on if its a full dataset or a subset
    _numPoints = points.getNumPoints();

    // Extract 2-dimensional points from the data set based on the selected dimensions
    calculatePositions(points);

    // Pass the 2D points to the scatter plot widget
    _scatterPlotWidget->setData(&_points);

    updateSelection();
}

void ScatterplotPlugin::calculatePositions(const Points& points)
{
    points.extractDataForDimensions(_points, _settingsAction.getPositionAction().getXDimension(), _settingsAction.getPositionAction().getYDimension());
}

void ScatterplotPlugin::calculateScalars(std::vector<float>& scalars, const Points& points, int colorIndex)
{
    if (colorIndex >= 0) {
        points.extractDataForDimension(scalars, colorIndex);
    }
}

void ScatterplotPlugin::updateSelection()
{
    const Points& points = _core->requestData<Points>(_currentDataSet);
    Points& selection = static_cast<Points&>(points.getSelection());

    std::vector<bool> selected;
    std::vector<char> highlights;

    points.selectedLocalIndices(selection.indices, selected);

    highlights.resize(points.getNumPoints(), 0);

    for (int i = 0; i < selected.size(); i++)
        highlights[i] = selected[i] ? 1 : 0;

    _scatterPlotWidget->setHighlights(highlights);

    emit selectionChanged();
}

PixelSelectionTool* ScatterplotPlugin::getSelectionTool()
{
    return _pixelSelectionTool;
}

QString ScatterplotPlugin::getCurrentDataset() const
{
    return _currentDataSet;
}

std::uint32_t ScatterplotPlugin::getNumberOfPoints() const
{
    if (_currentDataSet.isEmpty())
        return 0;

    const Points& points = _core->requestData<Points>(_currentDataSet);

    return points.getNumPoints();
}

std::uint32_t ScatterplotPlugin::getNumberOfSelectedPoints() const
{
    if (_currentDataSet.isEmpty())
        return 0;

    const Points& points    = _core->requestData<Points>(_currentDataSet);
    const Points& selection = static_cast<Points&>(points.getSelection());

    return static_cast<std::uint32_t>(selection.indices.size());
}

void ScatterplotPlugin::setXDimension(const std::int32_t& dimensionIndex)
{
    updateData();
}

void ScatterplotPlugin::setYDimension(const std::int32_t& dimensionIndex)
{
    updateData();
}

void ScatterplotPlugin::setColorDimension(const std::int32_t& dimensionIndex)
{
    const Points& points = DataSet::getSourceData(_core->requestData<Points>(_currentDataSet));

    std::vector<float> scalars;
    calculateScalars(scalars, points, dimensionIndex);

    _scatterPlotWidget->setScalars(scalars);
    _scatterPlotWidget->setScalarEffect(PointEffect::Color);

    updateData();
}

bool ScatterplotPlugin::canSelect() const
{
    return !getCurrentDataset().isEmpty() && getNumberOfPoints() >= 0;
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
    if (_currentDataSet.isEmpty())
        return;

    auto& points            = _core->requestData<Points>(_currentDataSet);
    auto& selectionSet      = dynamic_cast<Points&>(points.getSelection());
    auto& selectionIndices  = selectionSet.indices;

    if (points.isFull()) {
        selectionIndices.resize(points.getNumPoints());
        std::iota(selectionIndices.begin(), selectionIndices.end(), 0);
    } else {
        selectionIndices = points.indices;
    }

    _core->notifySelectionChanged(_currentDataSet);
}

void ScatterplotPlugin::clearSelection()
{
    if (_currentDataSet.isEmpty())
        return;

    auto& points                = _core->requestData<Points>(_currentDataSet);
    auto& selectionSet          = dynamic_cast<Points&>(points.getSelection());
    auto& selectionSetIndices   = selectionSet.indices;

    selectionSetIndices.clear();

    _core->notifySelectionChanged(_currentDataSet);
}

void ScatterplotPlugin::invertSelection()
{
    if (_currentDataSet.isEmpty())
        return;

    auto& points        = _core->requestData<Points>(_currentDataSet);
    auto& pointsIndices = points.indices;
    auto& selectionSet  = dynamic_cast<Points&>(points.getSelection());

    if (points.isFull()) {
        pointsIndices.resize(points.getNumPoints());
        std::iota(pointsIndices.begin(), pointsIndices.end(), 0);
    }

    auto selectionIndicesSet = QSet<std::uint32_t>(pointsIndices.begin(), pointsIndices.end());

    for (auto selectionSetIndex : selectionSet.indices)
        selectionIndicesSet.remove(selectionSetIndex);

    selectionSet.indices = std::vector<std::uint32_t>(selectionIndicesSet.begin(), selectionIndicesSet.end());

    _core->notifySelectionChanged(_currentDataSet);
}

void ScatterplotPlugin::updateWindowTitle()
{
    if (_currentDataSet.isEmpty())
        setWindowTitle(getGuiName());
    else
        setWindowTitle(QString("%1: %2").arg(getGuiName(), _currentDataSet));
}

// =============================================================================
// Factory
// =============================================================================

ViewPlugin* ScatterplotPluginFactory::produce()
{
    return new ScatterplotPlugin();
}
