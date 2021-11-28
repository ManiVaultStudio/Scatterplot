#pragma once

#include "PluginAction.h"

#include <QAbstractListModel>

using namespace hdps;

/**
 * Color-by model class
 *
 * Serves as model input to the color-by-data option action, it lists a constant color option and all available color datasets
 *
 * @author Thomas Kroes
 */
class ColorByModel : public QAbstractListModel
{
protected:

    /** (Default) constructor */
    ColorByModel(QObject* parent = nullptr);

public:

    /**
     * Get the number of row
     * @param parent Parent model index
     * @return Number of rows in the model
     */
    int rowCount(const QModelIndex& parent = QModelIndex()) const;

    /**
     * Get the row index of a color dataset
     * @param parent Parent model index
     * @return Row index of the color dataset
     */
    int rowIndex(const Dataset<DatasetImpl>& colorDataset) const;

    /**
     * Get the number of columns
     * @param parent Parent model index
     * @return Number of columns in the model
     */
    int columnCount(const QModelIndex& parent = QModelIndex()) const;

    /**
     * Get data
     * @param index Model index to query
     * @param role Data role
     * @return Data
     */
    QVariant data(const QModelIndex& index, int role) const;

    /**
     * Get datasets
     * @return Vector of smart pointers to datasets
     */
    const QVector<Dataset<DatasetImpl>>& getDatasets() const;

    /**
     * Get color dataset at the specified row index
     * @param rowIndex Index of the row
     * @return Smart pointer to color dataset
     */
    Dataset<DatasetImpl> getColorDataset(const std::int32_t& rowIndex) const;

    /**
     * Set color datasets (resets the model)
     * @param colorDatasets Vector of smart pointers to color datasets
     */
    void setColorDatasets(const QVector<Dataset<DatasetImpl>>& colorDatasets);

    /**
     * Remove specific color dataset
     * @param colorDataset Smart pointer to color dataset
     */
    void removeColorDataset(const Dataset<DatasetImpl>& colorDataset);

    /** Get whether to show the full path name in the GUI */
    bool getShowFullPathName() const;

    /**
     * Set whether to show the full path name in the GUI
     * @param showFullPathName Whether to show the full path name in the GUI
     */
    void setShowFullPathName(const bool& showFullPathName);

    /** Updates the model from the color datasets */
    void updateData();

protected:
    QVector<Dataset<DatasetImpl>>   _colorDatasets;         /** Color datasets from which can be picked */
    bool                            _showFullPathName;      /** Whether to show the full path name in the GUI */

    friend class ColoringAction;
};