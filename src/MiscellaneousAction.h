#pragma once

#include <actions/GroupAction.h>
#include <actions/ColorAction.h>

using namespace hdps::gui;

class QMenu;

class ScatterplotPlugin;

class MiscellaneousAction : public GroupAction
{
public:

    /**
     * Construct with \p parent object and \p title
     * @param parent Pointer to parent object
     * @param title Title
     */
    Q_INVOKABLE MiscellaneousAction(QObject* parent, const QString& title);

    QMenu* getContextMenu();

protected:
    ScatterplotPlugin*  _scatterplotPlugin;         /** Pointer to scatter plot plugin */
    ColorAction         _backgroundColorAction;     /** Color action for settings the background color action */

    static const QColor DEFAULT_BACKGROUND_COLOR;
};

Q_DECLARE_METATYPE(MiscellaneousAction)

inline const auto miscellaneousActionMetaTypeId = qRegisterMetaType<MiscellaneousAction*>("MiscellaneousAction");