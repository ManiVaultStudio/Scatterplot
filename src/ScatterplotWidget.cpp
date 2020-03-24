#include "ScatterplotWidget.h"

#include "util/Math.h"

#include <vector>

#include <QSize>
#include <QDebug>
#include <math.h>

using namespace hdps::util;

namespace
{
    Bounds getDataBounds(const std::vector<Vector2f>& points)
    {
        Bounds bounds = Bounds::Max;

        for (const Vector2f& point : points)
        {
            bounds.setLeft(std::min(point.x, bounds.getLeft()));
            bounds.setRight(std::max(point.x, bounds.getRight()));
            bounds.setBottom(std::min(point.y, bounds.getBottom()));
            bounds.setTop(std::max(point.y, bounds.getTop()));
        }

        return bounds;
    }
}

ScatterplotWidget::ScatterplotWidget()
    :
    _densityRenderer(DensityRenderer::RenderMode::DENSITY),
    _colormapWidget(this)
{
    addSelectionListener(&_selectionRenderer);
}

bool ScatterplotWidget::isInitialized()
{
    return _isInitialized;
}

void ScatterplotWidget::setRenderMode(RenderMode renderMode)
{
    _renderMode = renderMode;

    update();
}

void ScatterplotWidget::renderModePicked(const int index)
{
    switch (index)
    {
    case 0: setRenderMode(ScatterplotWidget::RenderMode::SCATTERPLOT); break;
    case 1: setRenderMode(ScatterplotWidget::RenderMode::DENSITY); break;
    case 2: setRenderMode(ScatterplotWidget::RenderMode::LANDSCAPE); break;
    }
    qDebug() << "Render Mode Picked";
}

void ScatterplotWidget::pointSizeChanged(const int size)
{
    _pointRenderer.setPointSize(size / 1000.0f);
    update();
}

void ScatterplotWidget::pointOpacityChanged(const int opacity)
{
    _pointRenderer.setAlpha(opacity / 100.0f);
    update();
}

void ScatterplotWidget::sigmaChanged(const int sigma)
{
    _densityRenderer.setSigma(sigma / 100.0f);
    update();
}

void ScatterplotWidget::colormapChanged(QString colormapName)
{
    _pointRenderer.setColormap(colormapName);
    _densityRenderer.setColormap(colormapName);
    update();
}

void ScatterplotWidget::colormapdiscreteChanged(bool isDiscrete)
{
    //_renderer->setColormapDiscrete(isDiscrete);

    update();
}

// Positions need to be passed as a pointer as we need to store them locally in order
// to be able to find the subset of data that's part of a selection. If passed
// by reference then we can upload the data to the GPU, but not store it in the widget.
void ScatterplotWidget::setData(const std::vector<Vector2f>* points)
{
    Bounds bounds = getDataBounds(*points);
    bounds.ensureMinimumSize(1e-07f, 1e-07f);
    bounds.makeSquare();
    bounds.expand(0.1f);
    _dataBounds = bounds;

    // Pass bounds and data to renderers
    _pointRenderer.setBounds(_dataBounds);
    _densityRenderer.setBounds(_dataBounds);
    _pointRenderer.setData(*points);
    _densityRenderer.setData(points);

    update();
}

void ScatterplotWidget::setHighlights(const std::vector<char>& highlights)
{
    _pointRenderer.setHighlights(highlights);

    update();
}

void ScatterplotWidget::setScalars(const std::vector<float>& scalars)
{
    _pointRenderer.setScalars(scalars);
    
    update();
}

void ScatterplotWidget::setColors(const std::vector<Vector3f>& colors)
{
    _pointRenderer.setColors(colors);
    _pointRenderer.setScalarEffect(None);

    update();
}

void ScatterplotWidget::setPointSize(const float size)
{
    _pointRenderer.setPointSize(size);

    update();
}

void ScatterplotWidget::setAlpha(const float alpha)
{
    _pointRenderer.setAlpha(alpha);
}

void ScatterplotWidget::setPointScaling(hdps::gui::PointScaling scalingMode)
{
    _pointRenderer.setPointScaling(scalingMode);
}

void ScatterplotWidget::setScalarEffect(PointEffect effect)
{
    _pointRenderer.setScalarEffect(effect);
}

void ScatterplotWidget::setSigma(const float sigma)
{
    _densityRenderer.setSigma(sigma);

    update();
}

void ScatterplotWidget::addSelectionListener(plugin::SelectionListener* listener)
{
    _selectionListeners.push_back(listener);
}

Selection ScatterplotWidget::getSelection()
{
    Selection isotropicSelection = toIsotropicCoordinates * _selection;

    Vector2f bottomLeftData(lerp(_dataBounds.getLeft(), _dataBounds.getRight(), isotropicSelection.getLeft()), lerp(_dataBounds.getBottom(), _dataBounds.getTop(), isotropicSelection.getBottom()));
    Vector2f topRightData(lerp(_dataBounds.getLeft(), _dataBounds.getRight(), isotropicSelection.getRight()), lerp(_dataBounds.getBottom(), _dataBounds.getTop(), isotropicSelection.getTop()));
    
    return Selection(bottomLeftData, topRightData);
}

void ScatterplotWidget::initializeGL()
{
    initializeOpenGLFunctions();
    qDebug() << "Initializing scatterplot widget with context: " << context();
    std::string versionString = std::string((const char*) glGetString(GL_VERSION));
    qDebug() << versionString.c_str();

    connect(context(), &QOpenGLContext::aboutToBeDestroyed, this, &ScatterplotWidget::cleanup);

    _colormapWidget.move(width() - 71, 10);
    _colormapWidget.show();

    QObject::connect(&_colormapWidget, &ColormapWidget::colormapSelected, this, &ScatterplotWidget::colormapChanged);
    QObject::connect(&_colormapWidget, &ColormapWidget::discreteSelected, this, &ScatterplotWidget::colormapdiscreteChanged);

    _pointRenderer.init();
    _densityRenderer.init();
    _selectionRenderer.init();

    // Set a default color map for both renderers
    _pointRenderer.setScalarEffect(PointEffect::Color);
    _pointRenderer.setColormap(_colormapWidget.getActiveColormap());
    _densityRenderer.setColormap(_colormapWidget.getActiveColormap());

    _isInitialized = true;
    emit initialized();
}

void ScatterplotWidget::resizeGL(int w, int h)
{
    _windowSize.setWidth(w);
    _windowSize.setHeight(h);

    _pointRenderer.resize(QSize(w, h));
    _densityRenderer.resize(QSize(w, h));
    _selectionRenderer.resize(QSize(w, h));

    // Set matrix for normalizing from pixel coordinates to [0, 1]
    toNormalisedCoordinates = Matrix3f(1.0f / w, 0, 0, 1.0f / h, 0, 0);

    // Take the smallest dimensions in order to calculate the aspect ratio
    int size = w < h ? w : h;

    float wAspect = (float)w / size;
    float hAspect = (float)h / size;
    float wDiff = ((wAspect - 1) / 2.0);
    float hDiff = ((hAspect - 1) / 2.0);

    
    toIsotropicCoordinates = Matrix3f(wAspect, 0, 0, hAspect, -wDiff, -hDiff);

    if (_colormapWidget._isOpen)
    {
        _colormapWidget.move(width() - (64 + 14 + 15 * 36 + 15), 10);
    }
    else {
        _colormapWidget.move(width() - 71, 10);
    }
    _colormapWidget.setColormap(0, true);
}

void ScatterplotWidget::paintGL()
{
    // Bind the framebuffer belonging to the widget
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebufferObject());

    // Clear the widget to the background color
    glClearColor(1, 1, 1, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Reset the blending function
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    switch (_renderMode)
    {
    case SCATTERPLOT: _pointRenderer.render(); break;
    case DENSITY:
    {
        _densityRenderer.setRenderMode(DensityRenderer::DENSITY);
        _densityRenderer.render();
        break;
    }
    case LANDSCAPE:
    {
        _densityRenderer.setRenderMode(DensityRenderer::LANDSCAPE);
        _densityRenderer.render();
        break;
    }
    }
    _selectionRenderer.render();
}

void ScatterplotWidget::mousePressEvent(QMouseEvent *event)
{
    _selecting = true;

    Vector2f point = toNormalisedCoordinates * Vector2f(event->x(), _windowSize.height() - event->y());
    _selection.setStart(point);
}

void ScatterplotWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (!_selecting) return;

    Vector2f point = toNormalisedCoordinates * Vector2f(event->x(), _windowSize.height() - event->y());
    _selection.setEnd(point);

    onSelecting(_selection);

    update();
}

void ScatterplotWidget::mouseReleaseEvent(QMouseEvent *event)
{
    _selecting = false;

    Vector2f point = toNormalisedCoordinates * Vector2f(event->x(), _windowSize.height() - event->y());
    _selection.setEnd(point);

    onSelection(_selection);

    update();
}

void ScatterplotWidget::onSelecting(Selection selection)
{
    _selection.set(selection.getStart(), selection.getEnd());

    for (plugin::SelectionListener* listener : _selectionListeners)
        listener->onSelecting(selection);

    update();
}

void ScatterplotWidget::onSelection(Selection selection)
{
    _selection.set(selection.getStart(), selection.getEnd());

    for (plugin::SelectionListener* listener : _selectionListeners)
        listener->onSelection(_selection);

    update();
}

void ScatterplotWidget::cleanup()
{
    qDebug() << "Deleting scatterplot widget, performing clean up...";
    _isInitialized = false;

    makeCurrent();
    _pointRenderer.destroy();
    _densityRenderer.destroy();
    _selectionRenderer.destroy();
}

ScatterplotWidget::~ScatterplotWidget()
{
    disconnect(QOpenGLWidget::context(), &QOpenGLContext::aboutToBeDestroyed, this, &ScatterplotWidget::cleanup);
    cleanup();
}