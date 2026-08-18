#pragma once

#include <actions/OptionAction.h>
#include <actions/TriggerAction.h>
#include <actions/VerticalGroupAction.h>

#include <PointData/DimensionPickerAction.h>

using namespace mv::gui;

class QMenu;
class ScatterplotPlugin;

/** Action for choosing how overlapping points are ordered along the z-axis. */
class ZOrderingAction : public VerticalGroupAction
{
public:
    enum class Mode {
        InsertionOrder,
        Dimension,
        Randomized
    };

    Q_INVOKABLE ZOrderingAction(QObject* parent, const QString& title);

    void initialize(ScatterplotPlugin* scatterplotPlugin);
    void updateScatterplotWidget();

    QMenu* getContextMenu(QWidget* parent = nullptr) override;

protected: // Linking
    void connectToPublicAction(WidgetAction* publicAction, bool recursive) override;
    void disconnectFromPublicAction(bool recursive) override;

public: // Serialization
    void fromVariantMap(const QVariantMap& variantMap) override;
    QVariantMap toVariantMap() const override;

public: // Action getters
    OptionAction& getModeAction() { return _modeAction; }
    DimensionPickerAction& getDimensionPickerAction() { return _dimensionPickerAction; }
    TriggerAction& getUseZOrderDimensionForSelectionAction() { return _useZOrderDimensionForSelectionAction; }

private:
    void updateZOrderScalars();

    ScatterplotPlugin*      _scatterplotPlugin = nullptr;
    OptionAction            _modeAction;
    DimensionPickerAction   _dimensionPickerAction;
    TriggerAction           _useZOrderDimensionForSelectionAction;
    std::vector<float>      _zOrderScalars;

    friend class mv::AbstractActionsManager;
};

Q_DECLARE_METATYPE(ZOrderingAction)

inline const auto zOrderingActionMetaTypeId = qRegisterMetaType<ZOrderingAction*>("ZOrderingAction");
