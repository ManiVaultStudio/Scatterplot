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
    setIcon(Application::getIconFont("FontAwesome").getIcon("ellipsis-h"));

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
    WidgetAction::Widget(parent, miscellaneousAction, state)
{
    switch (state)
    {
        case Widget::State::Standard:
        {
            auto layout = new QHBoxLayout();
            
            layout->setMargin(0);
            layout->addWidget(new QLabel("Background color:"));
            layout->addWidget(miscellaneousAction->_backgroundColorAction.createWidget(this));

            setLayout(layout);
            break;
        }

        case Widget::State::Popup:
        {
            auto layout = new QGridLayout();

            layout->addWidget(new QLabel("Background color:"), 0, 0);
            layout->addWidget(miscellaneousAction->_backgroundColorAction.createWidget(this), 0, 1);
            
            setPopupLayout(layout);
            break;
        }

        default:
            break;
    }
}