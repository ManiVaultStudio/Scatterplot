#include "SelectionAction.h"
#include "PixelSelectionTool.h"
#include "Application.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QPainter>
#include <QPainterPath>

using namespace hdps::gui;

SelectionAction::SelectionAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Selection"),
    _typeAction(this, "Type"),
    _rectangleAction(this, "Rectangle"),
    _brushAction(this, "Brush"),
    _lassoAction(this, "Lasso"),
    _polygonAction(this, "Polygon"),
    _typeActionGroup(this),
    _brushRadiusAction(this, "Brush radius", PixelSelectionTool::BRUSH_RADIUS_MIN, PixelSelectionTool::BRUSH_RADIUS_MAX, PixelSelectionTool::BRUSH_RADIUS_DEFAULT, PixelSelectionTool::BRUSH_RADIUS_DEFAULT),
    _modifierAddAction(this, "Add to selection"),
    _modifierRemoveAction(this, "Remove from selection"),
    _modifierActionGroup(this),
    _clearSelectionAction(this, "Select none"),
    _selectAllAction(this, "Select all"),
    _invertSelectionAction(this, "Invert selection"),
    _notifyDuringSelectionAction(this, "Notify during selection")
{
    setIcon(hdps::Application::getIconFont("FontAwesome").getIcon("mouse-pointer"));

    _rectangleAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _brushAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _lassoAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _polygonAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _clearSelectionAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _selectAllAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _invertSelectionAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);
    _notifyDuringSelectionAction.setShortcutContext(Qt::WidgetWithChildrenShortcut);

    _rectangleAction.setIcon(getIcon(PixelSelectionTool::Type::Rectangle));
    _brushAction.setIcon(getIcon(PixelSelectionTool::Type::Brush));
    _lassoAction.setIcon(getIcon(PixelSelectionTool::Type::Lasso));
    _polygonAction.setIcon(getIcon(PixelSelectionTool::Type::Polygon));

    scatterplotPlugin->addAction(&_rectangleAction);
    scatterplotPlugin->addAction(&_brushAction);
    scatterplotPlugin->addAction(&_lassoAction);
    scatterplotPlugin->addAction(&_polygonAction);
    scatterplotPlugin->addAction(&_brushRadiusAction);
    scatterplotPlugin->addAction(&_modifierAddAction);
    scatterplotPlugin->addAction(&_modifierRemoveAction);
    scatterplotPlugin->addAction(&_clearSelectionAction);
    scatterplotPlugin->addAction(&_selectAllAction);
    scatterplotPlugin->addAction(&_invertSelectionAction);
    scatterplotPlugin->addAction(&_notifyDuringSelectionAction);

    _rectangleAction.setCheckable(true);
    _brushAction.setCheckable(true);
    _lassoAction.setCheckable(true);
    _polygonAction.setCheckable(true);
    _modifierAddAction.setCheckable(true);
    _modifierRemoveAction.setCheckable(true);
    _notifyDuringSelectionAction.setCheckable(true);

    _rectangleAction.setShortcut(QKeySequence("R"));
    _brushAction.setShortcut(QKeySequence("B"));
    _lassoAction.setShortcut(QKeySequence("L"));
    _polygonAction.setShortcut(QKeySequence("P"));
    _clearSelectionAction.setShortcut(QKeySequence("E"));
    _selectAllAction.setShortcut(QKeySequence("A"));
    _invertSelectionAction.setShortcut(QKeySequence("I"));
    _notifyDuringSelectionAction.setShortcut(QKeySequence("U"));

    _rectangleAction.setToolTip("Select data points inside a rectangle (R)");
    _brushAction.setToolTip("Select data points using a brush tool (B)");
    _lassoAction.setToolTip("Select data points using a lasso (L)");
    _polygonAction.setToolTip("Select data points by drawing a polygon (P)");
    _brushRadiusAction.setToolTip("Brush selection tool radius");
    _modifierAddAction.setToolTip("Add items to the existing selection");
    _modifierRemoveAction.setToolTip("Remove items from the existing selection");
    _clearSelectionAction.setToolTip("Clears the selection (E)");
    _selectAllAction.setToolTip("Select all data points (A)");
    _invertSelectionAction.setToolTip("Invert the selection (I)");
    _notifyDuringSelectionAction.setToolTip("Notify during selection or only at the end of the selection process (U)");

    const auto& fontAwesome = hdps::Application::getIconFont("FontAwesome");

    _modifierAddAction.setIcon(fontAwesome.getIcon("plus"));
    _modifierRemoveAction.setIcon(fontAwesome.getIcon("minus"));

    _typeAction.setOptions(_scatterplotPlugin->getSelectionTool()->types.keys());

    _typeActionGroup.addAction(&_rectangleAction);
    _typeActionGroup.addAction(&_brushAction);
    _typeActionGroup.addAction(&_lassoAction);
    _typeActionGroup.addAction(&_polygonAction);
    
    _brushRadiusAction.setSuffix("px");

    _modifierActionGroup.addAction(&_modifierAddAction);
    _modifierActionGroup.addAction(&_modifierRemoveAction);

    connect(&_typeAction, &OptionAction::currentTextChanged, [this](const QString& currentText) {
        _scatterplotPlugin->getSelectionTool()->setType(PixelSelectionTool::getTypeEnum(currentText));
    });

    connect(&_rectangleAction, &QAction::triggered, this, [this]() {
        _scatterplotPlugin->getSelectionTool()->setType(PixelSelectionTool::Type::Rectangle);
    });

    connect(&_brushAction, &QAction::triggered, this, [this]() {
        _scatterplotPlugin->getSelectionTool()->setType(PixelSelectionTool::Type::Brush);
    });

    connect(&_lassoAction, &QAction::triggered, this, [this]() {
        _scatterplotPlugin->getSelectionTool()->setType(PixelSelectionTool::Type::Lasso);
    });

    connect(&_polygonAction, &QAction::triggered, this, [this]() {
        _scatterplotPlugin->getSelectionTool()->setType(PixelSelectionTool::Type::Polygon);
    });

    const auto updateType = [this]() {
        const auto type = _scatterplotPlugin->getSelectionTool()->getType();

        _typeAction.setCurrentText(PixelSelectionTool::getTypeName(type));
        _rectangleAction.setChecked(type == PixelSelectionTool::Type::Rectangle);
        _brushAction.setChecked(type == PixelSelectionTool::Type::Brush);
        _lassoAction.setChecked(type == PixelSelectionTool::Type::Lasso);
        _polygonAction.setChecked(type == PixelSelectionTool::Type::Polygon);
        _brushRadiusAction.setEnabled(type == PixelSelectionTool::Type::Brush);
    };

    connect(_scatterplotPlugin->getSelectionTool(), &PixelSelectionTool::typeChanged, this, [this, updateType](const PixelSelectionTool::Type& type) {
        updateType();
    });

    updateType();

    connect(_scatterplotPlugin->getSelectionTool(), &PixelSelectionTool::brushRadiusChanged, this, [this](const float& brushRadius) {
        _brushRadiusAction.setValue(brushRadius);
    });

    connect(&_brushRadiusAction, &DecimalAction::valueChanged, this, [this](const double& value) {
        _scatterplotPlugin->getSelectionTool()->setBrushRadius(value);
    });

    connect(&_modifierAddAction, &QAction::toggled, [this](bool checked) {
        _scatterplotPlugin->getSelectionTool()->setModifier(checked ? PixelSelectionTool::Modifier::Add : PixelSelectionTool::Modifier::Replace);
    });

    connect(&_modifierRemoveAction, &QAction::toggled, [this](bool checked) {
        _scatterplotPlugin->getSelectionTool()->setModifier(checked ? PixelSelectionTool::Modifier::Remove : PixelSelectionTool::Modifier::Replace);
    });

    connect(&_selectAllAction, &QAction::triggered, [this]() {
        _scatterplotPlugin->selectAll();
    });

    connect(&_clearSelectionAction, &QAction::triggered, [this]() {
        _scatterplotPlugin->clearSelection();
    });

    connect(&_invertSelectionAction, &QAction::triggered, [this]() {
        _scatterplotPlugin->invertSelection();
    });

    const auto updateNotifyDuringSelection = [this]() -> void {
        _notifyDuringSelectionAction.setChecked(_scatterplotPlugin->getSelectionTool()->isNotifyDuringSelection());
    };
    
    connect(_scatterplotPlugin->getSelectionTool(), &PixelSelectionTool::notifyDuringSelectionChanged, this, [this, updateNotifyDuringSelection](const bool& notifyDuringSelection) {
        updateNotifyDuringSelection();
    });

    updateNotifyDuringSelection();

    connect(&_notifyDuringSelectionAction, &QAction::toggled, [this](bool toggled) {
        _scatterplotPlugin->getSelectionTool()->setNotifyDuringSelection(toggled);
    });

    const auto updateSelectionButtons = [this]() {
        _clearSelectionAction.setEnabled(_scatterplotPlugin->canClearSelection());
        _selectAllAction.setEnabled(_scatterplotPlugin->canSelectAll());
        _invertSelectionAction.setEnabled(_scatterplotPlugin->canInvertSelection());
    };

    connect(_scatterplotPlugin, qOverload<>(&ScatterplotPlugin::selectionChanged), this, [this, updateSelectionButtons]() {
        updateSelectionButtons();
    });

    updateSelectionButtons();

    _scatterplotPlugin->installEventFilter(this);
}

QMenu* SelectionAction::getContextMenu()
{
    auto menu = new QMenu("Selection");

    const auto addActionToMenu = [menu](QAction* action) -> void {
        auto actionMenu = new QMenu(action->text());

        actionMenu->addAction(action);

        menu->addMenu(actionMenu);
    };

    menu->setEnabled(_scatterplotPlugin->getScatterplotWidget()->getRenderMode() == ScatterplotWidget::RenderMode::SCATTERPLOT);

    menu->addAction(&_rectangleAction);
    menu->addAction(&_brushAction);
    menu->addAction(&_lassoAction);
    menu->addAction(&_polygonAction);

    menu->addSeparator();

    addActionToMenu(&_brushRadiusAction);

    menu->addSeparator();

    menu->addAction(&_modifierAddAction);
    menu->addAction(&_modifierRemoveAction);

    menu->addSeparator();

    menu->addAction(&_clearSelectionAction);
    menu->addAction(&_selectAllAction);
    menu->addAction(&_invertSelectionAction);

    menu->addSeparator();
    
    menu->addAction(&_notifyDuringSelectionAction);

    return menu;
}

bool SelectionAction::eventFilter(QObject* object, QEvent* event)
{
    const auto keyEvent = dynamic_cast<QKeyEvent*>(event);

    if (!keyEvent)
        return QObject::eventFilter(object, event);

    if (keyEvent->isAutoRepeat())
        return QObject::eventFilter(object, event);

    switch (keyEvent->type())
    {
        case QEvent::KeyPress:
        {
            switch (keyEvent->key())
            {
                case Qt::Key_Shift:
                    _modifierAddAction.setChecked(true);
                    break;

                case Qt::Key_Control:
                    _modifierRemoveAction.setChecked(true);
                    break;

                default:
                    break;
            }

            break;
        }

        case QEvent::KeyRelease:
        {
            switch (keyEvent->key())
            {
                case Qt::Key_Shift:
                    _modifierAddAction.setChecked(false);
                    break;

                case Qt::Key_Control:
                    _modifierRemoveAction.setChecked(false);
                    break;

                default:
                    break;
            }

            break;
        }

        default:
            break;
    }

    return QObject::eventFilter(object, event);
}

QIcon SelectionAction::getIcon(const PixelSelectionTool::Type& selectionType)
{
    const auto margin           = 5;
    const auto pixmapSize       = QSize(100, 100);
    const auto pixmapDeflated   = QRect(QPoint(), pixmapSize).marginsRemoved(QMargins(margin, margin, margin, margin));

    // Create pixmap
    QPixmap pixmap(pixmapSize);

    // Fill with a transparent background
    pixmap.fill(Qt::transparent);

    // Create a painter to draw in the pixmap
    QPainter painter(&pixmap);

    // Enable anti-aliasing
    painter.setRenderHint(QPainter::Antialiasing);

    // Get the text color from the application
    const auto textColor = QApplication::palette().text().color();

    // Configure painter
    painter.setPen(QPen(textColor, 10, Qt::DotLine, Qt::SquareCap, Qt::SvgMiterJoin));

    switch (selectionType)
    {
        case PixelSelectionTool::Type::Rectangle:
        {
            painter.drawRect(pixmapDeflated);
            break;
        }

        case PixelSelectionTool::Type::Brush:
        {
            painter.drawEllipse(pixmapDeflated.center(), 45, 45);
            break;
        }

        case PixelSelectionTool::Type::Lasso:
        {
            QVector<QPoint> polygonPoints;
            
            polygonPoints << QPoint(5, 8);
            polygonPoints << QPoint(80, 28);
            polygonPoints << QPoint(92, 90);
            polygonPoints << QPoint(45, 60);
            polygonPoints << QPoint(10, 80);

            polygonPoints << polygonPoints[0];
            polygonPoints << polygonPoints[1];

            QPainterPath testCurve;

            QVector<QVector<QPoint>> curves;

            for (int pointIndex = 1; pointIndex < polygonPoints.count() - 1; pointIndex++) {
                const auto pPrevious    = polygonPoints[pointIndex - 1];
                const auto p            = polygonPoints[pointIndex];
                const auto pNext        = polygonPoints[pointIndex + 1];
                const auto pC0          = pPrevious + ((p - pPrevious) / 2);
                const auto pC1          = p + ((pNext - p) / 2);

                //qDebug() << pPrevious << p << pNext << pC0 << pC1;

                curves << QVector<QPoint>({ pC0, p, pC1 });
            }

            testCurve.moveTo(curves.first().first());

            for (auto curve : curves)
                testCurve.cubicTo(curve[0], curve[1], curve[2]);

            painter.drawPath(testCurve);

            break;
        }

        case PixelSelectionTool::Type::Polygon:
        {
            QVector<QPoint> points;

            points << QPoint(10, 10);
            points << QPoint(90, 45);
            points << QPoint(25, 90);

            painter.drawPolygon(points);

            break;
        }

        default:
            break;
    }

    return QIcon(pixmap);
}

SelectionAction::Widget::Widget(QWidget* parent, SelectionAction* selectionAction, const Widget::State& state) :
    WidgetActionWidget(parent, selectionAction, state)
{
    auto typeWidget                     = selectionAction->_typeAction.createWidget(this);
    auto brushRadiusWidget              = selectionAction->_brushRadiusAction.createWidget(this);
    auto modifierAddWidget              = selectionAction->_modifierAddAction.createPushButtonWidget(this);
    auto modifierRemoveWidget           = selectionAction->_modifierRemoveAction.createPushButtonWidget(this);
    auto clearSelectionWidget           = selectionAction->_clearSelectionAction.createWidget(this);
    auto selectAllWidget                = selectionAction->_selectAllAction.createWidget(this);
    auto invertSelectionWidget          = selectionAction->_invertSelectionAction.createWidget(this);
    auto notifyDuringSelectionWidget    = selectionAction->_notifyDuringSelectionAction.createCheckBoxWidget(this);

    switch (state)
    {
        case Widget::State::Standard:
        {
            auto layout = new QHBoxLayout();

            layout->setMargin(0);
            layout->addWidget(typeWidget);
            layout->addWidget(brushRadiusWidget);
            layout->addWidget(modifierAddWidget);
            layout->addWidget(modifierRemoveWidget);
            layout->addWidget(clearSelectionWidget);
            layout->addWidget(selectAllWidget);
            layout->addWidget(invertSelectionWidget);
            layout->addWidget(notifyDuringSelectionWidget);

            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            const auto getTypeWidget = [&, this]() -> QWidget* {
                modifierAddWidget->getPushButton()->setText("");
                modifierRemoveWidget->getPushButton()->setText("");

                auto layout = new QHBoxLayout();

                layout->setMargin(0);
                layout->addWidget(typeWidget);
                layout->addWidget(modifierAddWidget);
                layout->addWidget(modifierRemoveWidget);
                layout->itemAt(0)->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

                auto widget = new QWidget();

                widget->setLayout(layout);

                return widget;
            };

            const auto getSelectWidget = [&, this]() -> QWidget* {
                auto layout = new QHBoxLayout();

                layout->setMargin(0);
                layout->addWidget(clearSelectionWidget);
                layout->addWidget(selectAllWidget);
                layout->addWidget(invertSelectionWidget);
                layout->addStretch(1);

                auto widget = new QWidget();

                widget->setLayout(layout);

                return widget;
            };

            auto layout = new QGridLayout();

            layout->addWidget(selectionAction->_typeAction.createLabelWidget(this), 0, 0);
            layout->addWidget(getTypeWidget(), 0, 1);
            layout->addWidget(selectionAction->_brushRadiusAction.createLabelWidget(this), 1, 0);
            layout->addWidget(brushRadiusWidget, 1, 1);
            layout->addWidget(getSelectWidget(), 2, 1);
            layout->addWidget(notifyDuringSelectionWidget, 3, 1);
            layout->itemAtPosition(1, 1)->widget()->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }
}
