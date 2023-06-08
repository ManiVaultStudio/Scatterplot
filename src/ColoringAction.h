#pragma once

#include <actions/GroupAction.h>
#include <actions/ColorMapAction.h>

#include <PointData/DimensionPickerAction.h>

#include "ColorSourceModel.h"

#include <QHBoxLayout>
#include <QLabel>
#include <QStackedWidget>

using namespace hdps::gui;

class ScatterplotPlugin;

/**
 * Coloring action class
 *
 * Action class for configuring the coloring of points
 *
 * @author Thomas Kroes
 */
class ColoringAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE ColoringAction(QObject* parent, const QString& title);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;

    /**
     * Add color dataset to the list
     * @param colorDataset Smart pointer to color dataset
     */
    void addColorDataset(const Dataset<DatasetImpl>& colorDataset);

    /** Determines whether a given color dataset is already loaded */
    bool hasColorDataset(const Dataset<DatasetImpl>& colorDataset) const;

    /** Get smart pointer to current color dataset (if any) */
    Dataset<DatasetImpl> getCurrentColorDataset() const;

    /**
     * Set the current color dataset
     * @param colorDataset Smart pointer to color dataset
     */
    void setCurrentColorDataset(const Dataset<DatasetImpl>& colorDataset);

protected:

    /** Update the color by action options */
    void updateColorByActionOptions();

    /** Update the colors of the points in the scatter plot widget */
    void updateScatterPlotWidgetColors();

protected: // Color map

    /** Updates the scalar range in the color map */
    void updateColorMapActionScalarRange();

    /** Update the scatter plot widget color map */
    void updateScatterplotWidgetColorMap();

    /** Update the color map range in the scatter plot widget */
    void updateScatterPlotWidgetColorMapRange();

    /** Determine whether the color map should be enabled */
    bool shouldEnableColorMap() const;

    /** Enables/disables the color map */
    void updateColorMapActionReadOnly();

protected: // Linking

    /**
     * Connect this action to a public action
     * @param publicAction Pointer to public action to connect to
     * @param recursive Whether to also connect descendant child actions
     */
    void connectToPublicAction(WidgetAction* publicAction, bool recursive) override;

    /**
     * Disconnect this action from its public action
     * @param recursive Whether to also disconnect descendant child actions
     */
    void disconnectFromPublicAction(bool recursive) override;

public: // Serialization

    /**
     * Load widget action from variant map
     * @param Variant map representation of the widget action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save widget action to variant map
     * @return Variant map representation of the widget action
     */
    QVariantMap toVariantMap() const override;

public: // Action getters

    OptionAction& getColorByAction() { return _colorByAction; }
    ColorAction& getConstantColorAction() { return _constantColorAction; }
    DimensionPickerAction& getDimensionAction() { return _dimensionAction; }
    ColorMapAction& getColorMapAction() { return _colorMapAction; }

signals:
    void currentColorDatasetChanged(Dataset<DatasetImpl> currentColorDataset);

private:
    ScatterplotPlugin*      _scatterplotPlugin;         /** Pointer to scatter plot plugin */
    ColorSourceModel        _colorByModel;              /** Color by model (model input for the color by action) */
    OptionAction            _colorByAction;             /** Action for picking the coloring type */
    ColorAction             _constantColorAction;       /** Action for picking the constant color */
    DimensionPickerAction   _dimensionAction;           /** Dimension picker action */
    ColorMapAction          _colorMapAction;            /** Color map action */

    /** Default constant color */
    static const QColor DEFAULT_CONSTANT_COLOR;

    friend class ScatterplotPlugin;
    friend class hdps::AbstractActionsManager;
};

Q_DECLARE_METATYPE(ColoringAction)

inline const auto coloringActionMetaTypeId = qRegisterMetaType<ColoringAction*>("ColoringAction");