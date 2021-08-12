#include "MiscellaneousAction.h"
#include "Application.h"
#include "ScatterplotPlugin.h"
#include "ScatterplotWidget.h"

using namespace hdps::gui;

const QColor MiscellaneousAction::DEFAULT_BACKGROUND_COLOR = qRgb(255, 255, 255);

MiscellaneousAction::MiscellaneousAction(ScatterplotPlugin* scatterplotPlugin) :
    PluginAction(scatterplotPlugin, "Miscellaneous"),
    _backgroundColorAction(scatterplotPlugin)
{
    setIcon(Application::getIconFont("FontAwesome").getIcon("cog"));

    _scatterplotPlugin->addAction(&_backgroundColorAction);

    _backgroundColorAction.setColor(DEFAULT_BACKGROUND_COLOR);
    _backgroundColorAction.setDefaultColor(DEFAULT_BACKGROUND_COLOR);

    const auto updateBackgroundColor = [this]() -> void {
        const auto color = _backgroundColorAction.getColor();

        getScatterplotWidget()->setBackgroundColor(color);
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

MiscellaneousAction::Widget::Widget(QWidget* parent, MiscellaneousAction* miscellaneousAction, const Widget::State& state) :
    WidgetActionWidget(parent, miscellaneousAction, state)
{
    auto labelWidget    = miscellaneousAction->_backgroundColorAction.createLabelWidget(this);
    auto colorWidget    = miscellaneousAction->_backgroundColorAction.createWidget(this);

    switch (state)
    {
        case Widget::State::Standard:
        {
            auto layout = new QHBoxLayout();
            
            layout->setMargin(0);
            layout->addWidget(labelWidget);
            layout->addWidget(colorWidget);

            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            auto layout = new QGridLayout();

            layout->addWidget(labelWidget, 0, 0);
            layout->addWidget(colorWidget, 0, 1);
            
            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }
}