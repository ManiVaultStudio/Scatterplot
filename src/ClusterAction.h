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

protected: // Widget

    class Widget : public WidgetActionWidget {
    public:
        Widget(QWidget* parent, ClusterAction* clusterAction, const hdps::gui::WidgetActionWidget::State& state);
    };

    QWidget* getWidget(QWidget* parent, const WidgetActionWidget::State& state = WidgetActionWidget::State::Standard) override {
        return new Widget(parent, this, state);
    };

public:

    /**
     * Constructor
     * @param scatterplotPlugin Pointer to scatter plot plugin
     */
    ClusterAction(ScatterplotPlugin* scatterplotPlugin);

    /**
     * Get the context menu for the action
     * @param parent Parent widget
     * @return Context menu
     */
    QMenu* getContextMenu(QWidget* parent = nullptr) override;;

    /** Get selected indices in the points dataset */
    const std::vector<std::uint32_t>& getSelectedIndices() const;

public: // Action getters

    OptionAction& getTargetAction() { return _targetAction; }
    StringAction& getNameAction() { return _nameAction; }
    ColorAction& getColorAction() { return _colorAction; }
    TriggerAction& getCreateCluster() { return _createClusterAction; }

signals:

    /**
     * Signals that the selected indices changed
     * @param selectedIndices Selected indices
     */
    void selectedIndicesChanged(const std::vector<std::uint32_t>& selectedIndices);

protected:
    DataHierarchyItem*      _inputDataHierarchyItem;        /** Pointer to the input data hierarchy item */
    DataHierarchyItem*      _clusterDataHierarchyItem;      /** Pointer to the output cluster data hierarchy item */
    OptionAction            _targetAction;                  /** Target cluster set action */
    StringAction            _nameAction;                    /** Cluster name action */
    ColorAction             _colorAction;                   /** Cluster color action */
    TriggerAction           _createClusterAction;           /** Edit and create cluster action */
};
