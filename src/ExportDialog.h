#pragma once

#include "ExportImageAction.h"

#include <QDialog>
#include <QTabWidget>

class ScatterplotPlugin;

/**
 * Export dialog class
 *
 * Dialog for exporting to image/video
 *
 * @author Thomas Kroes
 */
class ExportDialog : public QDialog
{
public:

    /**
     * Constructor
     * @param parent Pointer to parent widget
     * @param scatterplotPlugin Reference to scatterplot plugin
     */
    ExportDialog(QWidget* parent, ScatterplotPlugin& scatterplotPlugin);

    /** Get preferred size */
    QSize sizeHint() const override {
        return QSize(600, 500);
    }

    /** Get minimum size hint*/
    QSize minimumSizeHint() const override {
        return sizeHint();
    }

protected:
    ScatterplotPlugin&      _scatterplotPlugin;     /** Reference to scatterplot plugin */
    ExportImageAction       _exportImageAction;     /** Export to image action */
};
