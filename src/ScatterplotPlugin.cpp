#include "ScatterplotPlugin.h"

#include "PixelSelectionTool.h"
#include "ScatterplotWidget.h"
#include "DataHierarchyItem.h"
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

ScatterplotPlugin::ScatterplotPlugin(const PluginFactory* factory) :
    ViewPlugin(factory),
    _points(),
    _colors(),
    _positions(),
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
        if (!_points.isValid())
            return;

        auto contextMenu = _settingsAction.getContextMenu();
        
        contextMenu->addSeparator();

        _points->populateContextMenu(contextMenu);

        contextMenu->exec(mapToGlobal(point));
    });

    _dropWidget->setDropIndicatorWidget(new DropWidget::DropIndicatorWidget(this, "No data loaded", "Drag an item from the data hierarchy and drop it here to visualize data..."));
    _dropWidget->initialize([this](const QMimeData* mimeData) -> DropWidget::DropRegions {
        DropWidget::DropRegions dropRegions;

        const auto mimeText = mimeData->text();
        const auto tokens   = mimeText.split("\n");

        if (tokens.count() == 1)
            return dropRegions;

        const auto datasetName          = tokens[0];
        const auto dataType             = DataType(tokens[1]);
        const auto dataTypes            = DataTypes({ PointType , ColorType, ClusterType });
        const auto currentDatasetName   = _points.getDatasetName();

        if (!dataTypes.contains(dataType))
            dropRegions << new DropWidget::DropRegion(this, "Incompatible data", "This type of data is not supported", false);

        if (dataType == PointType) {
            const auto candidateDataset     = getCore()->requestData<Points>(datasetName);
            const auto candidateDatasetName = candidateDataset.getName();
            const auto description          = QString("Visualize %1 as points or density/contour map").arg(candidateDatasetName);

            if (!_points.isValid()) {
                dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDatasetName]() {
                    _points.setDatasetName(candidateDatasetName);
                });
            }
            else {
                if (candidateDatasetName == currentDatasetName) {
                    dropRegions << new DropWidget::DropRegion(this, "Warning", "Data already loaded", false);
                }
                else {
                    const auto points = getCore()->requestData<Points>(currentDatasetName);

                    if (points.getNumPoints() != candidateDataset.getNumPoints()) {
                        dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDatasetName]() {
                            _points.setDatasetName(candidateDatasetName);
                        });
                    }
                    else {
                        dropRegions << new DropWidget::DropRegion(this, "Position", description, true, [this, candidateDatasetName]() {
                            _points.setDatasetName(candidateDatasetName);
                        });

                        if (candidateDatasetName != _colors.getDatasetName()) {
                            dropRegions << new DropWidget::DropRegion(this, "Color", QString("Color %1 by %2").arg(currentDatasetName, candidateDatasetName), true, [this, candidateDatasetName]() {
                                _colors.setDatasetName(candidateDatasetName);
                            });
                        }
                    }
                }
            }
        }

        if (dataType == ClusterType) {
            const auto candidateDataset     = getCore()->requestData<Clusters>(datasetName);
            const auto candidateDatasetName = candidateDataset.getName();
            const auto description          = QString("Color points by %1").arg(candidateDatasetName);

            if (_points.isValid()) {
                if (candidateDatasetName == _colors.getDatasetName()) {
                    dropRegions << new DropWidget::DropRegion(this, "Color", "Cluster set is already in use", false, [this]() {});
                }
                else {
                    dropRegions << new DropWidget::DropRegion(this, "Color", description, true, [this, candidateDatasetName]() {
                        _colors.setDatasetName(candidateDatasetName);
                    });
                }
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
        if (!_points.isValid())
            return;

        switch (dataEvent->getType())
        {
            case EventType::DataChanged:
            {
                if (dataEvent->dataSetName != _points.getDatasetName())
                    return;

                updateData();

                break;
            }

            case EventType::SelectionChanged:
            {
                if (!_points.isValid())
                    return;

                if (dataEvent->dataSetName == _points.getDatasetName())
                    updateSelection();

                break;
            }

            default:
                break;
        }
    });

    // Register for cluster dataset events
    registerDataEventByType(ClusterType, [this](hdps::DataEvent* dataEvent) {
        if (!_colors.isValid())
            return;

        if (dataEvent->dataSetName != _colors.getDatasetName())
            return;

        switch (dataEvent->getType())
        {
            case EventType::DataChanged:
            {
                if (dataEvent->dataSetName != _colors.getDatasetName())
                    return;

                loadColors(_colors.getDatasetName());

                break;
            }

            default:
                break;
        }
    });

    connect(_pixelSelectionTool, &PixelSelectionTool::areaChanged, [this]() {
        if (!_pixelSelectionTool->isNotifyDuringSelection())
            return;

        selectPoints();
    });

    connect(_pixelSelectionTool, &PixelSelectionTool::ended, [this]() {
        if (_pixelSelectionTool->isNotifyDuringSelection())
            return;

        selectPoints();
    });

    connect(&_settingsAction.getColoringAction().getColorByAction(), &OptionAction::currentTextChanged, [this](const QString& currentText) {
        if (currentText != "Color data")
            return;

        loadColors(_colors.getDatasetName());
    });

    const auto updateWindowTitle = [this]() -> void {
        if (!_points.isValid())
            setWindowTitle(getGuiName());
        else
            setWindowTitle(QString("%1: %2").arg(getGuiName(), _points.getDatasetName()));
    };

    // Load points when the dataset name of the points dataset reference changes
    connect(&_points, &DatasetRef<Points>::datasetNameChanged, this, [this, updateWindowTitle](const QString& oldDatasetName, const QString& newDatasetName) {
        loadPoints(newDatasetName);
        updateWindowTitle();
    });

    // Load color data when the dataset name of the colors dataset reference changes
    connect(&_colors, &DatasetRef<Colors>::datasetNameChanged, this, [this](const QString& oldDatasetName, const QString& newDatasetName) {
        loadColors(newDatasetName);
    });

    updateWindowTitle();
}

void ScatterplotPlugin::createSubset(const bool& fromSourceData /*= false*/, const QString& name /*= ""*/)
{
    auto& subsetPoints  = _points->isDerivedData() && fromSourceData ? DataSet::getSourceData(*_points) : *_points;

    const auto subsetName = subsetPoints.createSubset(_points->getName());

    _core->getDataHierarchyItem(subsetName)->select();
}

void ScatterplotPlugin::selectPoints()
{
    if (!_points.isValid() || !_pixelSelectionTool->isActive())
        return;

    auto selectionAreaImage = _pixelSelectionTool->getAreaPixmap().toImage();


    Points& selectionSet = static_cast<Points&>(_points->getSelection());

    std::vector<std::uint32_t> targetIndices;

    targetIndices.reserve(_points->getNumPoints());
    std::vector<unsigned int> localGlobalIndices;
    
    _points->getGlobalIndices(localGlobalIndices);

    const auto dataBounds   = _scatterPlotWidget->getBounds();
    const auto width        = selectionAreaImage.width();
    const auto height       = selectionAreaImage.height();
    const auto size         = width < height ? width : height;
    const auto& set         = _points->isDerivedData() ? DataSet::getSourceData(*_points) : *_points;
    const auto& setIndices  = set.indices;

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

    _points->setSelection(targetIndices);

    _core->notifySelectionChanged(_points->getName());
}

DatasetRef<Points>& ScatterplotPlugin::getPointsDataset()
{
    return _points;
}

DatasetRef<DataSet>& ScatterplotPlugin::getColorsDataset()
{
    return _colors;
}

QStringList ScatterplotPlugin::getClusterDatasetNames()
{
    QStringList clusterDatasetNames;

    if (!_points.isValid())
        return clusterDatasetNames;

    auto pointsDataHierarchyItem = _core->getDataHierarchyItem(_points.getDatasetName());

    if (pointsDataHierarchyItem == nullptr)
        return clusterDatasetNames;

    for (auto child : pointsDataHierarchyItem->getChildren()) {
        if (child->getDataType() == ClusterType)
            clusterDatasetNames << child->getDatasetName();
    }

    return clusterDatasetNames;
}

void ScatterplotPlugin::loadPoints(const QString& dataSetName)
{
    _colors.reset();

    if (_points.isValid()) {
        _settingsAction.getColoringAction().getColorMapAction().reset();

        // For source data determine whether to use dimension names or make them up
        if (_points->getDimensionNames().size() == _points->getNumDimensions())
            _settingsAction.getPositionAction().setDimensions(_points->getDimensionNames());
        else
            _settingsAction.getPositionAction().setDimensions(_points->getNumDimensions());

        // For derived data determine whether to use dimension names or make them up
        if (DataSet::getSourceData(*_points).getDimensionNames().size() == DataSet::getSourceData(*_points).getNumDimensions())
            _settingsAction.getColoringAction().setDimensions(DataSet::getSourceData(*_points).getDimensionNames());
        else
            _settingsAction.getColoringAction().setDimensions(DataSet::getSourceData(*_points).getNumDimensions());

        _pixelSelectionTool->setEnabled(_points.isValid());

        _dropWidget->setShowDropIndicator(false);

        setFocus();
    }
    else {
        _settingsAction.getPositionAction().setDimensions(std::vector<QString>());
        _settingsAction.getColoringAction().setDimensions(std::vector<QString>());

        _dropWidget->setShowDropIndicator(true);
    }

    _scatterPlotWidget->setColoringMode(ScatterplotWidget::ColoringMode::ConstantColor);

    updateData();
}

void ScatterplotPlugin::loadColors(const QString& dataSetName)
{
    _settingsAction.getColoringAction().getColorDataAction().getDatasetNameAction().setString(_colors.getDatasetName());

    if (_colors.isValid()) {
        const auto dataType = _colors->getDataType();

        if (dataType == PointType)
        {
            std::vector<float> scalars;

            if (_points->getNumPoints() != _numPoints)
            {
                qWarning("Number of points used for coloring does not match number of points in data, aborting attempt to color plot");
                return;
            }

            _points->visitFromBeginToEnd([&scalars](auto begin, auto end) {
                scalars.insert(scalars.begin(), begin, end);
            });

            _scatterPlotWidget->setScalars(scalars);
            _scatterPlotWidget->setScalarEffect(PointEffect::Color);
        }

        if (dataType == ClusterType)
        {
            auto& clusters = static_cast<Clusters&>(*_colors);

            std::vector<Vector3f> colors(_positions.size());

            for (const Cluster& cluster : clusters.getClusters())
            {
                for (const int& index : cluster.getIndices())
                {
                    if (index < 0 || index > colors.size())
                    {
                        qWarning("Cluster index is out of range of data, aborting attempt to color plot");
                        return;
                    }

                    const auto clusterColor = cluster.getColor();

                    colors[index] = Vector3f(clusterColor.redF(), clusterColor.greenF(), clusterColor.blueF());
                }
            }

            _scatterPlotWidget->setColors(colors);

            updateData();
        }

        _scatterPlotWidget->setColoringMode(ScatterplotWidget::ColoringMode::ColorData);
        _settingsAction.getColoringAction().getColorDataAction().getDatasetNameAction().setString(dataSetName);

        _dropWidget->setShowDropIndicator(false);
    }
    else {
        std::vector<float> scalars;

        if (_points.isValid()) {
            scalars.resize(_points->getNumPoints());
        }
        
        _scatterPlotWidget->setScalars(scalars);
        _scatterPlotWidget->setScalarEffect(PointEffect::Color);
    }

    updateData();
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
    if (_points.isValid()) {
        // Get the selected dimensions to use as X and Y dimension in the plot
        int xDim = _settingsAction.getPositionAction().getXDimension();
        int yDim = _settingsAction.getPositionAction().getYDimension();

        // If one of the dimensions was not set, do not draw anything
        if (xDim < 0 || yDim < 0)
            return;

        // Determine number of points depending on if its a full dataset or a subset
        _numPoints = _points->getNumPoints();

        // Extract 2-dimensional points from the data set based on the selected dimensions
        calculatePositions(*_points);

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
    points.extractDataForDimensions(_positions, _settingsAction.getPositionAction().getXDimension(), _settingsAction.getPositionAction().getYDimension());
}

void ScatterplotPlugin::calculateScalars(std::vector<float>& scalars, const Points& points, int colorIndex)
{
    if (colorIndex >= 0) {
        points.extractDataForDimension(scalars, colorIndex);
    }
}

void ScatterplotPlugin::updateSelection()
{
    if (!_points.isValid())
        return;

    auto& selection = static_cast<Points&>(_points->getSelection());

    std::vector<bool> selected;
    std::vector<char> highlights;

    _points->selectedLocalIndices(selection.indices, selected);

    highlights.resize(_points->getNumPoints(), 0);

    for (int i = 0; i < selected.size(); i++)
        highlights[i] = selected[i] ? 1 : 0;

    _scatterPlotWidget->setHighlights(highlights);

    emit selectionChanged();
}

PixelSelectionTool* ScatterplotPlugin::getSelectionTool()
{
    return _pixelSelectionTool;
}

std::uint32_t ScatterplotPlugin::getNumberOfPoints() const
{
    if (!_points.isValid())
        return 0;

    return _points->getNumPoints();
}

std::uint32_t ScatterplotPlugin::getNumberOfSelectedPoints() const
{
    if (!_points.isValid())
        return 0;

    const Points& selection = static_cast<Points&>(_points->getSelection());

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
    std::vector<float> scalars;
    calculateScalars(scalars, *_points, dimensionIndex);

    _scatterPlotWidget->setScalars(scalars);
    _scatterPlotWidget->setScalarEffect(PointEffect::Color);

    updateData();
}

bool ScatterplotPlugin::canSelect() const
{
    return _points.isValid() && getNumberOfPoints() >= 0;
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
    if (!_points.isValid())
        return;

    auto& selectionSet      = dynamic_cast<Points&>(_points->getSelection());
    auto& selectionIndices  = selectionSet.indices;

    if (_points->isFull()) {
        selectionIndices.resize(_points->getNumPoints());
        std::iota(selectionIndices.begin(), selectionIndices.end(), 0);
    } else {
        selectionIndices = _points->indices;
    }

    _points.notifySelectionChanged();
}

void ScatterplotPlugin::clearSelection()
{
    if (!_points.isValid())
        return;

    auto& selectionSet          = dynamic_cast<Points&>(_points->getSelection());
    auto& selectionSetIndices   = selectionSet.indices;

    selectionSetIndices.clear();

    _points.notifySelectionChanged();
}

void ScatterplotPlugin::invertSelection()
{
    if (!_points.isValid())
        return;

    auto& pointsIndices = _points->indices;
    auto& selectionSet  = dynamic_cast<Points&>(_points->getSelection());

    if (_points->isFull()) {
        pointsIndices.resize(_points->getNumPoints());
        std::iota(pointsIndices.begin(), pointsIndices.end(), 0);
    }

    auto selectionIndicesSet = QSet<std::uint32_t>(pointsIndices.begin(), pointsIndices.end());

    for (auto selectionSetIndex : selectionSet.indices)
        selectionIndicesSet.remove(selectionSetIndex);

    selectionSet.indices = std::vector<std::uint32_t>(selectionIndicesSet.begin(), selectionIndicesSet.end());

    _points.notifySelectionChanged();
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
