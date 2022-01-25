#include "SettingsAction.h"
#include "ExportDialog.h"

#include "Application.h"
#include "ScatterplotPlugin.h"
#include "PointData.h"

#include <QMenu>

using namespace hdps::gui;

SettingsAction::SettingsAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Settings"),
    _renderModeAction(scatterplotPlugin),
    _positionAction(scatterplotPlugin),
    _coloringAction(scatterplotPlugin),
    _subsetAction(scatterplotPlugin),
    _manualClusteringAction(scatterplotPlugin),
    _selectionAction(*scatterplotPlugin),
    _plotAction(scatterplotPlugin),
    _exportAction(this, "Export to image/video"),
    _miscellaneousAction(scatterplotPlugin)
{
    const auto updateEnabled = [this]() {
        setEnabled(_scatterplotPlugin->getPositionDataset().isValid());
    };

    connect(&scatterplotPlugin->getPositionDataset(), &Dataset<Points>::changed, this, updateEnabled);

    updateEnabled();

    connect(&_exportAction, &TriggerAction::triggered, this, [this]() {
        ExportDialog exportDialog(nullptr, *_scatterplotPlugin);

        exportDialog.exec();
    });
}

QMenu* SettingsAction::getContextMenu()
{
    auto menu = new QMenu();

    menu->addMenu(_renderModeAction.getContextMenu());
    menu->addMenu(_plotAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_positionAction.getContextMenu());
    menu->addMenu(_coloringAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_subsetAction.getContextMenu());
    menu->addMenu(_selectionAction.getContextMenu());
    menu->addSeparator();
    menu->addMenu(_miscellaneousAction.getContextMenu());

    return menu;
}

SettingsAction::Widget::Widget(QWidget* parent, SettingsAction* settingsAction) :
    WidgetActionWidget(parent, settingsAction, Widget::State::Standard),
    _layout(),
    _toolBarWidget(),
    _toolBarLayout(),
    _stateWidgets(),
    _spacerWidgets()
{
    setAutoFillBackground(true);

    _toolBarLayout.setMargin(0);
    _toolBarLayout.setSpacing(0);
    _toolBarLayout.setSizeConstraint(QLayout::SetFixedSize);

    addStateWidget(&settingsAction->_renderModeAction, 4);
    addStateWidget(&settingsAction->_plotAction, 7);
    addStateWidget(&settingsAction->_positionAction, 10);
    addStateWidget(&settingsAction->_coloringAction, 8);
    addStateWidget(&settingsAction->_subsetAction, 3);
    addStateWidget(&settingsAction->_manualClusteringAction, 0);
    addStateWidget(&settingsAction->_selectionAction, 2);
    addStateWidget(&settingsAction->getColoringAction().getColorMapAction(), 100);

    _toolBarLayout.addStretch(1);

    _toolBarWidget.setLayout(&_toolBarLayout);

    _layout.addWidget(&_toolBarWidget);
    _layout.addStretch(1);

    setLayout(&_layout);

    _layout.setMargin(4);

    this->installEventFilter(this);
    _toolBarWidget.installEventFilter(this);
}

bool SettingsAction::Widget::eventFilter(QObject* object, QEvent* event)
{
    switch (event->type())
    {
        case QEvent::Resize:
            updateLayout();
            break;

        default:
            break;
    }

    return QObject::eventFilter(object, event);
}

void SettingsAction::Widget::addStateWidget(WidgetAction* widgetAction, const std::int32_t& priority /*= 0*/)
{
    _stateWidgets << new WidgetActionStateWidget(this, widgetAction, priority);

    if (_stateWidgets.count() >= 2) {
        _spacerWidgets << new SpacerWidget();
        _toolBarLayout.addWidget(_spacerWidgets.back());
    }
    
    _toolBarLayout.addWidget(_stateWidgets.back());
}

void SettingsAction::Widget::updateLayout()
{
    QMap<WidgetActionStateWidget*, Widget::State> states;

    for (auto stateWidget : _stateWidgets)
        states[stateWidget] = Widget::State::Collapsed;

    const auto getWidth = [this, &states]() -> std::uint32_t {
        std::uint32_t width = 2 * _layout.margin();

        for (auto stateWidget : _stateWidgets)
            width += stateWidget->getSizeHint(states[stateWidget]).width();

        for (auto spacerWidget : _spacerWidgets) {
            const auto spacerWidgetIndex    = _spacerWidgets.indexOf(spacerWidget);
            const auto stateWidgetLeft      = _stateWidgets[spacerWidgetIndex];
            const auto stateWidgetRight     = _stateWidgets[spacerWidgetIndex + 1];
            const auto spacerWidgetType     = SpacerWidget::getType(states[stateWidgetLeft], states[stateWidgetRight]);
            const auto spacerWidgetWidth    = SpacerWidget::getWidth(spacerWidgetType);

            width += spacerWidgetWidth;
        }

        return width;
    };

    auto prioritySortedStateWidgets = _stateWidgets;

    std::sort(prioritySortedStateWidgets.begin(), prioritySortedStateWidgets.end(), [](WidgetActionStateWidget* stateWidgetA, WidgetActionStateWidget* stateWidgetB) {
        return stateWidgetA->getPriority() > stateWidgetB->getPriority();
    });

    for (auto stateWidget : prioritySortedStateWidgets) {
        auto cachedStates = states;

        states[stateWidget] = Widget::State::Standard;

        if (getWidth() > static_cast<std::uint32_t>(width())) {
            states = cachedStates;
            break;
        }
    }

    for (auto stateWidget : _stateWidgets)
        stateWidget->setState(states[stateWidget]);

    for (auto spacerWidget : _spacerWidgets) {
        const auto spacerWidgetIndex    = _spacerWidgets.indexOf(spacerWidget);
        const auto stateWidgetLeft      = _stateWidgets[spacerWidgetIndex];
        const auto stateWidgetRight     = _stateWidgets[spacerWidgetIndex + 1];
        const auto spacerWidgetType     = SpacerWidget::getType(states[stateWidgetLeft], states[stateWidgetRight]);

        spacerWidget->setType(spacerWidgetType);
    }
}

SettingsAction::SpacerWidget::SpacerWidget(const Type& type /*= State::Divider*/) :
    QWidget(),
    _type(Type::Divider),
    _layout(new QHBoxLayout()),
    _verticalLine(new QFrame())
{
    _verticalLine->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    _verticalLine->setFrameShape(QFrame::VLine);
    _verticalLine->setFrameShadow(QFrame::Sunken);

    _layout->setMargin(2);
    _layout->setSpacing(0);
    _layout->setAlignment(Qt::AlignCenter);
    _layout->addWidget(_verticalLine);
    
    setType(type);
}

SettingsAction::SpacerWidget::Type SettingsAction::SpacerWidget::getType(const WidgetActionWidget::State& widgetTypeLeft, const WidgetActionWidget::State& widgetTypeRight)
{
    return widgetTypeLeft == WidgetActionWidget::State::Collapsed && widgetTypeRight == WidgetActionWidget::State::Collapsed ? Type::Spacer : Type::Divider;
}

SettingsAction::SpacerWidget::Type SettingsAction::SpacerWidget::getType(const WidgetActionStateWidget* stateWidgetLeft, const WidgetActionStateWidget* stateWidgetRight)
{
    return getType(stateWidgetLeft->getState(), stateWidgetRight->getState());
}

void SettingsAction::SpacerWidget::setType(const Type& type)
{
    _type = type;

    setLayout(_layout);
    setFixedWidth(getWidth(_type));

    _verticalLine->setVisible(_type == Type::Divider ? true : false);
}

std::int32_t SettingsAction::SpacerWidget::getWidth(const Type& type)
{
    switch (type)
    {
        case Type::Divider:
            return 14;

        case Type::Spacer:
            return 6;

        default:
            break;
    }

    return 0;
}