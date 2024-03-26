#include "MiscellaneousAction.h"

#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <graphics/Bounds.h>

using namespace mv::gui;

const QColor MiscellaneousAction::DEFAULT_BACKGROUND_COLOR = qRgb(255, 255, 255);

MiscellaneousAction::MiscellaneousAction(QObject* parent, const QString& title) :
    VerticalGroupAction(parent, title),
    _scatterplotPlugin(dynamic_cast<ScatterplotPlugin*>(parent->parent())),
    _backgroundColorAction(this, "Background color"),
    _zoomRectangleAction(this, "Zoom ROI"),
    _updateZoom(true)
{
    setIcon(Application::getIconFont("FontAwesome").getIcon("cog"));
    setLabelSizingType(LabelSizingType::Auto);
    setConfigurationFlag(WidgetAction::ConfigurationFlag::ForceCollapsedInGroup);

    addAction(&_backgroundColorAction);
    addAction(&_zoomRectangleAction);

    _backgroundColorAction.setColor(DEFAULT_BACKGROUND_COLOR);

    const auto updateBackgroundColor = [this]() -> void {
        _scatterplotPlugin->getScatterplotWidget().setBackgroundColor(_backgroundColorAction.getColor());
    };

    connect(&_backgroundColorAction, &ColorAction::colorChanged, this, [this, updateBackgroundColor](const QColor& color) {
        updateBackgroundColor();
    });

    connect(&_zoomRectangleAction, &DecimalRectangleAction::rectangleChanged, this, [this](const QRectF& newZoomBounds) {
        qDebug() << "DecimalRectangleAction::rectangleChanged:" << _updateZoom;

        if (_updateZoom)
        {
            auto newBounds = mv::Bounds{ static_cast<float>(newZoomBounds.left()), static_cast<float>(newZoomBounds.right()), static_cast<float>(newZoomBounds.bottom()), static_cast<float>(newZoomBounds.top()) };

            if (newBounds == _scatterplotPlugin->getScatterplotWidget().getZoomBounds())
                return;


            qDebug() << "current: " << _zoomRectangleAction.getRectangle();
            qDebug() << "new :    " << newZoomBounds;

            _updateZoom = false;
            _scatterplotPlugin->getScatterplotWidget().setZoomBounds(mv::Bounds{ static_cast<float>(newZoomBounds.left()), static_cast<float>(newZoomBounds.right()), static_cast<float>(newZoomBounds.bottom()), static_cast<float>(newZoomBounds.top()) });
        }
        else
            _updateZoom = true;

        qDebug() << "_updateZoom:" << _updateZoom;
        },
        Qt::DirectConnection);

    connect(&_scatterplotPlugin->getScatterplotWidget(), &ScatterplotWidget::zoomBoundsChanged, this, [this](const mv::Bounds& newZoomBounds) {
        qDebug() << "ScatterplotWidget:" << getId();
        qDebug() << "ScatterplotWidget::zoomBoundsChanged:" << _updateZoom;

        if (_updateZoom)
        {
            auto newZoomRectangle = QRectF{ newZoomBounds.getLeft(), newZoomBounds.getTop(), newZoomBounds.getWidth(), newZoomBounds.getHeight() };

            if (newZoomRectangle == _zoomRectangleAction.getRectangle())
                return;

            qDebug() << "current: " << _zoomRectangleAction.getRectangle();
            qDebug() << "new :    " << newZoomRectangle;

            _updateZoom = false;
            _zoomRectangleAction.setRectangle(newZoomRectangle);
        }
        else
            _updateZoom = true;

        qDebug() << "_updateZoom:" << _updateZoom;
        },
        Qt::DirectConnection);

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
        actions().connectPrivateActionToPublicAction(&_zoomRectangleAction, &publicMiscellaneousAction->getBackgroundColorAction(), recursive);
    }

    GroupAction::connectToPublicAction(publicAction, recursive);
}

void MiscellaneousAction::disconnectFromPublicAction(bool recursive)
{
    if (!isConnected())
        return;

    if (recursive) {
        actions().disconnectPrivateActionFromPublicAction(&_backgroundColorAction, recursive);
        actions().disconnectPrivateActionFromPublicAction(&_zoomRectangleAction, recursive);
    }

    GroupAction::disconnectFromPublicAction(recursive);
}

void MiscellaneousAction::fromVariantMap(const QVariantMap& variantMap)
{
    GroupAction::fromVariantMap(variantMap);

    _backgroundColorAction.fromParentVariantMap(variantMap);
    _zoomRectangleAction.fromParentVariantMap(variantMap);
}

QVariantMap MiscellaneousAction::toVariantMap() const
{
    auto variantMap = GroupAction::toVariantMap();

    _backgroundColorAction.insertIntoVariantMap(variantMap);
    _zoomRectangleAction.insertIntoVariantMap(variantMap);

    return variantMap;
}