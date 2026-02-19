#pragma once

#include <actions/GroupAction.h>
#include <actions/DatasetPickerAction.h>

using namespace mv::gui;

class ScatterplotPlugin;

class DatasetsAction : public GroupAction
{
    Q_OBJECT

public:

    /**
     * Construct with \p parent and \p title
     * @param parent Pointer to parent object
     * @param title Title of the action
     */
    Q_INVOKABLE DatasetsAction(QObject* parent, const QString& title);

    mv::Dataset<mv::DatasetImpl> getColorDataset() { return _colorDataset; }
    mv::Dataset<mv::DatasetImpl> getPointSizeDataset() { return _pointSizeDataset; }
    mv::Dataset<mv::DatasetImpl> getPointOpacityDataset() { return _pointOpacityDataset; }

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
     * @param variantMap Variant map representation of the widget action
     */
    void fromVariantMap(const QVariantMap& variantMap) override;

    /**
     * Save widget action to variant map
     * @return Variant map representation of the widget action
     */
    QVariantMap toVariantMap() const override;

protected: // Dataset picker action setup

    /**
     * Set up the dataset picker actions with the datasets from the scatter plot plugin
     * @param scatterplotPlugin Pointer to scatter plot plugin whose datasets are used to populate the dataset picker actions
     */
    void setupDatasetPickerActions(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Set up the position dataset picker action with the position datasets from the scatter plot plugin
     * @param scatterplotPlugin Pointer to scatter plot plugin whose position datasets are used to populate the dataset picker action
     */
    void setupPositionDatasetPickerAction(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Set up the color dataset picker action with the color datasets from the scatter plot plugin
     * @param scatterplotPlugin Pointer to scatter plot plugin whose color datasets are used to populate the dataset picker action
     */
    void setupColorDatasetPickerAction(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Set up the point size dataset picker action with the point size datasets from the scatter plot plugin
     * @param scatterplotPlugin Pointer to scatter plot plugin whose point size datasets are used to populate the dataset picker action
     */
    void setupPointSizeDatasetPickerAction(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Set up the point opacity dataset picker action with the point opacity datasets from the scatter plot plugin
     * @param scatterplotPlugin Pointer to scatter plot plugin whose point opacity datasets are used to populate the dataset picker action
     */
    void setupPointOpacityDatasetPickerAction(ScatterplotPlugin* scatterplotPlugin);

public: // Action getters

    DatasetPickerAction& getPositionDatasetPickerAction() { return _positionDatasetPickerAction; }
    DatasetPickerAction& getColorDatasetPickerAction() { return _colorDatasetPickerAction; }
    DatasetPickerAction& getPointSizeDatasetPickerAction() { return _pointSizeDatasetPickerAction; }
    DatasetPickerAction& getPointOpacityDatasetPickerAction() { return _pointOpacityDatasetPickerAction; }

private:
    ScatterplotPlugin*              _scatterplotPlugin;               /** Pointer to scatter plot plugin */
    DatasetPickerAction             _positionDatasetPickerAction;     /** Dataset picker action for position dataset */
    DatasetPickerAction             _colorDatasetPickerAction;        /** Dataset picker action for color dataset */
    DatasetPickerAction             _pointSizeDatasetPickerAction;    /** Dataset picker action for point size */
    DatasetPickerAction             _pointOpacityDatasetPickerAction; /** Dataset picker action for point opacity */
    mv::Dataset<mv::DatasetImpl>    _colorDataset;                    /** Smart pointer to dataset used for coloring (if any) */
    mv::Dataset<mv::DatasetImpl>    _pointSizeDataset;                /** Smart pointer to dataset for driving point size (if any) */
    mv::Dataset<mv::DatasetImpl>    _pointOpacityDataset;             /** Smart pointer to dataset for driving point opacity (if any) */

    friend class mv::AbstractActionsManager;
    friend class ScatterplotPlugin;
};

Q_DECLARE_METATYPE(DatasetsAction)

inline const auto datasetsActionMetaTypeId = qRegisterMetaType<DatasetsAction*>("DatasetsAction");