#pragma once

#include <actions/DecimalRangeAction.h>
#include <actions/ToggleAction.h>
#include <actions/VerticalGroupAction.h>

#include <PointData/DimensionPickerAction.h>

class ScatterplotPlugin;

using namespace mv::gui;

/** Action for restricting selection eligibility to a dimension value range. */
class SelectionRestrictionAction : public VerticalGroupAction
{
    Q_OBJECT

public:
    Q_INVOKABLE SelectionRestrictionAction(QObject* parent, const QString& title);

    void initialize(ScatterplotPlugin* scatterplotPlugin);
    void updateSelectionExclusions();

protected: // Linking
    void connectToPublicAction(WidgetAction* publicAction, bool recursive) override;
    void disconnectFromPublicAction(bool recursive) override;

public: // Serialization
    void fromVariantMap(const QVariantMap& variantMap) override;
    QVariantMap toVariantMap() const override;

public: // Action getters
    ToggleAction& getEnabledAction() { return _enabledAction; }
    DimensionPickerAction& getDimensionPickerAction() { return _dimensionPickerAction; }
    DecimalRangeAction& getRangeAction() { return _rangeAction; }

private:
    void updateDataset();
    void updateDimensionValues(bool resetRange);
    void updateActionsReadOnly();

    ScatterplotPlugin*      _scatterplotPlugin = nullptr;
    ToggleAction            _enabledAction;
    DimensionPickerAction   _dimensionPickerAction;
    DecimalRangeAction      _rangeAction;
    std::vector<float>      _dimensionValues;
    bool                    _updatingRange = false;

    friend class mv::AbstractActionsManager;
};

Q_DECLARE_METATYPE(SelectionRestrictionAction)

inline const auto selectionRestrictionActionMetaTypeId = qRegisterMetaType<SelectionRestrictionAction*>("SelectionRestrictionAction");
