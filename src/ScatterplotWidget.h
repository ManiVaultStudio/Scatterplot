#pragma once

#include <renderers/DensityRenderer.h>
#include <renderers/PointRenderer.h>

#include <util/PixelSelectionTool.h>

#include <actions/DecimalRectangleAction.h>

#include <graphics/Bounds.h>
#include <graphics/Vector2f.h>
#include <graphics/Vector3f.h>

#include <QOpenGLFunctions_3_3_Core>
#include <QOpenGLWidget>
#include <QPoint>

using namespace mv::gui;
using namespace mv::util;

namespace mv::plugin
{
    class ViewPlugin;
}

class ScatterplotWidget : public QOpenGLWidget, protected QOpenGLFunctions_3_3_Core
{
    Q_OBJECT

public:
    enum RenderMode {
        SCATTERPLOT,
        DENSITY,
        LANDSCAPE
    };

    /** The way that point colors are determined */
    enum class ColoringMode {
        Constant,      /** Point color is a constant color */
        Data,          /** Determined by external dataset */
        Scatter,       /** Determined by scatter layout using a 2D colormap */
    };

public:
    ScatterplotWidget(mv::plugin::ViewPlugin* parentPlugin = nullptr);

    ~ScatterplotWidget();

    /** Returns true when the widget was initialized and is ready to be used. */
    bool isInitialized() const;

    /** Get/set render mode */
    RenderMode getRenderMode() const;
    void setRenderMode(const RenderMode& renderMode);

    /** Get/set background color */
    QColor getBackgroundColor() const;
    void setBackgroundColor(QColor color);

    /** Get/set coloring mode */
    ColoringMode getColoringMode() const;
    void setColoringMode(const ColoringMode& coloringMode);

    /**
     * Get the pixel selection tool
     * @return Reference to the pixel selection tool
     */
    PixelSelectionTool& getPixelSelectionTool();

    /**
     * Get the sampler pixel selection tool
     * @return Reference to the sampler pixel selection tool
     */
    PixelSelectionTool& getSamplerPixelSelectionTool();

    /**
     * Feed 2-dimensional data to the scatterplot.
     */
    void setData(const std::vector<mv::Vector2f>* data);
    void setHighlights(const std::vector<char>& highlights, const std::int32_t& numSelectedPoints);
    void setScalars(const std::vector<float>& scalars);

    /**
     * Set colors for each individual data point
     * @param colors Vector of colors (size must match that of the loaded points dataset)
     */
    void setColors(const std::vector<mv::Vector3f>& colors);

    /**
     * Set point size scalars
     * @param pointSizeScalars Point size scalars
     */
    void setPointSizeScalars(const std::vector<float>& pointSizeScalars);

    /**
     * Set point opacity scalars
     * @param pointOpacityScalars Point opacity scalars (assume the values are normalized)
     */
    void setPointOpacityScalars(const std::vector<float>& pointOpacityScalars);

    void setScalarEffect(PointEffect effect);
    void setPointScaling(PointScaling scalingMode);

    void showHighlights(bool show);

    /**
     * Set sigma value for kernel density estimation.
     * @param sigma kernel width as a fraction of the output square width. Typical values are [0.01 .. 0.5]
     */
    void setSigma(const float sigma);

    mv::Bounds getBounds() const {
        return _dataRectangleAction.getBounds();
    }

    mv::Vector3f getColorMapRange() const;
    void setColorMapRange(const float& min, const float& max);

    void setWeightDensity(bool useWeights);
    float getWeightDensity() const { return _weightDensity; }

    /**
     * Create screenshot
     * @param width Width of the screen shot (in pixels)
     * @param height Height of the screen shot (in pixels)
     * @param backgroundColor Background color of the screen shot
     */
    void createScreenshot(std::int32_t width, std::int32_t height, const QString& fileName, const QColor& backgroundColor);

public: // Selection

    /**
     * Get the selection display mode color
     * @return Selection display mode
     */
    PointSelectionDisplayMode getSelectionDisplayMode() const;

    /**
     * Set the selection display mode
     * @param selectionDisplayMode Selection display mode
     */
    void setSelectionDisplayMode(PointSelectionDisplayMode selectionDisplayMode);

    /**
     * Get the selection outline color
     * @return Color of the selection outline
     */
    QColor getSelectionOutlineColor() const;

    /**
     * Set the selection outline color
     * @param selectionOutlineColor Selection outline color
     */
    void setSelectionOutlineColor(const QColor& selectionOutlineColor);

    /**
     * Get whether the selection outline color should be the point color or a custom color
     * @return Boolean determining whether the selection outline color should be the point color or a custom color
     */
    bool getSelectionOutlineOverrideColor() const;

    /**
     * Set whether the selection outline color should be the point color or a custom color
     * @param selectionOutlineOverrideColor Boolean determining whether the selection outline color should be the point color or a custom color
     */
    void setSelectionOutlineOverrideColor(bool selectionOutlineOverrideColor);

    /**
     * Get the selection outline scale
     * @return Scale of the selection outline
     */
    float getSelectionOutlineScale() const;

    /**
     * Set the selection outline scale
     * @param selectionOutlineScale Scale of the selection outline
     */
    void setSelectionOutlineScale(float selectionOutlineScale);

    /**
     * Get the selection outline opacity
     * @return Opacity of the selection outline
     */
    float getSelectionOutlineOpacity() const;

    /**
     * Set the selection outline opacity
     * @param selectionOutlineOpacity Opacity of the selection outline
     */
    void setSelectionOutlineOpacity(float selectionOutlineOpacity);

    /**
     * Get whether the selection outline halo is enabled or not
     * @return Boolean determining whether the selection outline halo is enabled or not
     */
    bool getSelectionOutlineHaloEnabled() const;

    /**
     * Set whether the selection outline halo is enabled or not
     * @param randomizedDepth Boolean determining whether the selection outline halo is enabled or not
     */
    void setRandomizedDepthEnabled(bool randomizedDepth);

    /**
     * Set whether the z-order of each point is to be randomized or not
     * @param selectionOutlineHaloEnabled Boolean determining whether the z-order of each point is to be randomized or not
     */
    void setSelectionOutlineHaloEnabled(bool selectionOutlineHaloEnabled);

    /**
     * Get whether the z-order of each point is to be randomized or not
     * @return Boolean determining whether the z-order of each point is to be randomized or not
     */
    bool getRandomizedDepthEnabled() const;

protected:
    void initializeGL()         Q_DECL_OVERRIDE;
    void resizeGL(int w, int h) Q_DECL_OVERRIDE;
    void paintGL()              Q_DECL_OVERRIDE;
    void paintPixelSelectionToolNative(PixelSelectionTool& pixelSelectionTool, QImage& image, QPainter& painter) const;

    void cleanup();
    
    void showEvent(QShowEvent* event) Q_DECL_OVERRIDE
    {
        emit created();
        QWidget::showEvent(event);
    }

    bool event(QEvent* event) override;

public: // Const access to renderers

    const PointRenderer& getPointRenderer() const {
        return _pointRenderer;
    }

    const DensityRenderer& getDensityRenderer() const {
        return _densityRenderer;
    }

public:

    /** Assign a color map image to the point and density renderers */
    void setColorMap(const QImage& colorMapImage);

public: // Navigators

    /**
     * Get the navigator for the point renderer
     * @return Reference to the navigator
     */
    mv::Navigator2D& getPointRendererNavigator() { return _pointRenderer.getNavigator(); }

    /**
     * Get the navigator for the density renderer
     * @return Reference to the navigator
     */
    mv::Navigator2D& getDensityRendererNavigator() { return _densityRenderer.getNavigator(); }

signals:
    void initialized();
    void created();

    /**
     * Signals that the render mode changed
     * @param renderMode Signals that the render mode has changed
     */
    void renderModeChanged(const RenderMode& renderMode);

    /**
     * Signals that the coloring mode changed
     * @param coloringMode Signals that the coloring mode has changed
     */
    void coloringModeChanged(const ColoringMode& coloringMode);

    /** Signals that the density computation has started */
    void densityComputationStarted();

    /** Signals that the density computation has ended */
    void densityComputationEnded();

    /** Signals that zoom rectangle has changed  */
    //void zoomBoundsChanged(const mv::Bounds& newZoomBounds);

public slots:
    void computeDensity();
    
private slots:
    void updatePixelRatio();

protected:
    PointRenderer               _pointRenderer;                 /** For rendering point data as points */
    DensityRenderer             _densityRenderer;               /** For rendering point data as a density plot */

private:
    bool                        _isInitialized;                 /** Boolean determining whether the widget it properly initialized or not */
    RenderMode                  _renderMode;                    /** Current render mode */
    QColor                      _backgroundColor;               /** Background color */
    ColoringMode                _coloringMode;                  /** Type of point/density coloring */
    DecimalRectangleAction      _dataRectangleAction;           /** Rectangle action for the bounds of the loaded data */
    QImage                      _colorMapImage;                 /** 1D/2D color map image */
    PixelSelectionTool          _pixelSelectionTool;            /** 2D pixel selection tool */
    PixelSelectionTool          _samplerPixelSelectionTool;     /** 2D pixel selection tool */
    float                       _pixelRatio;                    /** Current pixel ratio */
    bool                        _weightDensity;                 /** Use point scalar sizes to weight density */

    mv::plugin::ViewPlugin*     _parentPlugin = nullptr;

    friend class ScatterplotPlugin;
    friend class NavigationAction;
};
