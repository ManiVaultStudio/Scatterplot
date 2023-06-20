#include "MiscellaneousAction.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

const QColor MiscellaneousAction::DEFAULT_BACKGROUND_COLOR = qRgb(255, 255, 255);

MiscellaneousAction::MiscellaneousAction(QObject* parent, const QString& title) :
    GroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _backgroundColorAction(this, "Background color")
{
    setIcon(Application::getIconFont("FontAwesome").getIcon("cog"));
    setConnectionPermissionsToForceNone(true);

    addAction(&_backgroundColorAction);

    _scatterplotPlugin->getWidget().addAction(&_backgroundColorAction);

    _backgroundColorAction.setColor(DEFAULT_BACKGROUND_COLOR);

    const auto updateBackgroundColor = [this]() -> void {
        _scatterplotPlugin->getScatterplotWidget().setBackgroundColor(_backgroundColorAction.getColor());
    };

    connect(&_backgroundColorAction, &ColorAction::colorChanged, this, [this, updateBackgroundColor](const QColor& color) {
        updateBackgroundColor();
    });

    updateBackgroundColor();
}

QMenu* MiscellaneousAction::getContextMenu()
{
    auto menu = new QMenu("Miscellaneous");

    menu->addAction(&_backgroundColorAction);

    return menu;
}

void MiscellaneousAction::connectToPublicAction(WidgetAction* publicAction, bool recursive)
{
    auto publicMiscellaneousAction = dynamic_cast<MiscellaneousAction*>(publicAction);

    Q_ASSERT(publicMiscellaneousAction != nullptr);

    if (publicMiscellaneousAction == nullptr)
        return;

    if (recursive) {
        actions().connectPrivateActionToPublicAction(&_backgroundColorAction, &publicMiscellaneousAction->getBackgroundColorAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void MiscellaneousAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_backgroundColorAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void MiscellaneousAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _backgroundColorAction.fromParentVariantMap(variantMap);
}

QVariantMap MiscellaneousAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _backgroundColorAction.insertIntoVariantMap(variantMap);

    return variantMap;
}