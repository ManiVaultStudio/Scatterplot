#include "MiscellaneousAction.h"
#include "Application.h"
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
    _backgroundColorAction.setDefaultColor(DEFAULT_BACKGROUND_COLOR);

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

