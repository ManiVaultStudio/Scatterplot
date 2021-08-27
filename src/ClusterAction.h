#pragma once

#include "PluginAction.h"
#include "event/EventListener.h"

namespace hdps {
    class DataHierarchyItem;
}

using namespace hdps;
using namespace hdps::gui;

/**
 * Cluster action class
 *
 * Action class for clustering points
 *
 * @author Thomas Kroes
 */
class ClusterAction : public PluginAction, public hdps::EventListener
{
    Q_OBJECT

protected

    class Widget : public WidgetActionWidget
    {
    public:

        /**
         * Constructor
         * @param parent Pointer to parent widget
         * @param clusterAction Pointer to cluster action
         */
        Widget(QWidget* parent, ClusterAction* clusterAction, const WidgetActionWidget::State& state);
    };

    /**
     * Get widget representation of the cluster action
     * @param parent Pointer to parent widget
     * @param state Widget state
     */
    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param scatterplotPlugin Pointer to scatter plot plugin
     */
    ClusterAction(ScatterplotPlugin* scatterplotPlugin);

    /** Updates available cluster datasets */
    void updateTargets();

    /** Create default clusters set if none already exist */
    void createDefaultCustersSet();

public: // Action getters

    OptionAction& getTargetAction() { return _targetAction; }
    StringAction& getNameAction() { return _nameAction; }
    ColorAction& getColorAction() { return _colorAction; }
    TriggerAction& getCreateCluster() { return _addClusterAction; }

protected:
    DataHierarchyItem*      _inputDataHierarchyItem;        /** Pointer to the input data hierarchy item */
    DataHierarchyItem*      _clusterDataHierarchyItem;      /** Pointer to the output cluster data hierarchy item */
    OptionAction            _targetAction;                  /** Target cluster set action */
    StringAction            _nameAction;                    /** Cluster name action */
    ColorAction             _colorAction;                   /** Cluster color action */
    TriggerAction           _addClusterAction;           /** Edit and create cluster action */
};
