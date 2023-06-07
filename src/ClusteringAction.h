#pragma once

#include <actions/GroupAction.h>
#include <actions/DatasetPickerAction.h>

using namespace hdps;
using namespace hdps::gui;
using namespace hdps::util;

class Clusters;
class ScatterplotPlugin;

/**
 * Clustering action class
 *
 * Action class for clustering
 *
 * @author Thomas Kroes
 */
class ClusteringAction : public GroupAction
{
public:

    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE ClusteringAction(QObject* parent, const QString& title);

    /** Adds a clusters dataset to the position dataset as a child */
    void createDefaultClusterDataset();

    /** Update the target cluster datasets action (creates default set if no cluster sets are available) */
    void updateTargetClusterDatasets();

protected:

    /** Update the state of the actions */
    void updateActions();

public: // Action getters

    StringAction& getNameAction() { return _nameAction; }
    ColorAction& getColorAction() { return _colorAction; }
    TriggerAction& getCreateCluster() { return _addClusterAction; }
    TriggerAction& getAddClusterAction() { return _addClusterAction; }
    DatasetPickerAction& getTargetClusterDataset() { return _targetClusterDataset; }

private:
    ScatterplotPlugin*      _scatterplotPlugin;         /** Pointer to scatter plot plugin */
    StringAction            _nameAction;                /** Cluster name action */
    ColorAction             _colorAction;               /** Cluster color action */
    TriggerAction           _addClusterAction;          /** Add manual cluster action */
    DatasetPickerAction     _targetClusterDataset;      /** Target cluster dataset action */
};
