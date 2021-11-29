#include "PointSizeByModel.h"

#include "DataHierarchyItem.h"
#include "Application.h"

using namespace hdps;

PointSizeByModel::PointSizeByModel(QObject* parent /*= nullptr*/) :
    QAbstractListModel(parent),
    _datasets(),
    _showFullPathName(true)
{
}

int PointSizeByModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    // Constant point size option plus the number of available datasets
    return _datasets.count() + 1;
}

int PointSizeByModel::rowIndex(const Dataset<DatasetImpl>& dataset) const
{
    // Only proceed if we have a valid dataset
    if (!dataset.isValid())
        return -1;

    // Return the index of the dataset and add one for the constant point size option
    return _datasets.indexOf(dataset) + 1;
}

int PointSizeByModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 1;
}

QVariant PointSizeByModel::data(const QModelIndex& index, int role) const
{
    // Get row/column of and smart pointer to the dataset
    const auto row          = index.row();
    const auto column       = index.column();
    const auto colorDataset = getDataset(row);

    switch (role)
    {
        // Return ruler icon for constant point size and dataset icon otherwise
        case Qt::DecorationRole:
            return row > 0 ? colorDataset->getIcon() : Application::getIconFont("FontAwesome").getIcon("ruler");

         // Return 'Constant' for constant point size and dataset (full path) GUI name otherwise
        case Qt::DisplayRole:
        {
            if (row > 0)
                if (row == 1)
                    return colorDataset->getGuiName();
                else
                    return _showFullPathName ? colorDataset->getDataHierarchyItem().getFullPathName() : colorDataset->getGuiName();
            else
                return "Constant";
        }

        default:
            break;
    }

    return QVariant();
}

const QVector<Dataset<DatasetImpl>>& PointSizeByModel::getDatasets() const
{
    return _datasets;
}

Dataset<DatasetImpl> PointSizeByModel::getDataset(const std::int32_t& rowIndex) const
{
    // Return empty smart pointer when out of range
    if (rowIndex <= 0 || rowIndex > _datasets.count())
        return Dataset<DatasetImpl>();

    // Subtract the constant point size row
    return _datasets[rowIndex - 1];
}

void PointSizeByModel::setDatasets(const QVector<Dataset<DatasetImpl>>& datasets)
{
    if (datasets.count() != _datasets.count()) {

        // Notify others that the model layout is about to be changed
        emit layoutAboutToBeChanged();

        // Assign the datasets
        _datasets = datasets;

        // Notify others that the model layout is changed
        emit layoutChanged();
    }
    else {
        // Assign the datasets
        _datasets = datasets;
    }

    // Update model when a dataset GUI name changes
    for (const auto& dataset : _datasets) {

        // Notify others that the model has updated when the dataset GUI name changes
        connect(&dataset, &Dataset<DatasetImpl>::dataGuiNameChanged, this, [this, &dataset]() {

            // Get row index of the dataset
            const auto datasetRowIndex = rowIndex(dataset);

            // Only proceed if we found a valid row index
            if (datasetRowIndex < 0)
                return;

            // Establish model index
            const auto modelIndex = index(datasetRowIndex, 0);

            // Only proceed if we have a valid model index
            if (!modelIndex.isValid())
                return;

            // Notify others that the data changed
            emit dataChanged(modelIndex, modelIndex);
        });
    }

    // And update model data with datasets
    updateData();
}

void PointSizeByModel::removeDataset(const Dataset<DatasetImpl>& dataset)
{
    // Copy existing datasets
    auto updatedDatasets = _datasets;

    // Remove from the vector
    updatedDatasets.removeOne(dataset);

    // Assign new datasets
    setDatasets(updatedDatasets);

    // And update model data with altered datasets
    updateData();
}

bool PointSizeByModel::getShowFullPathName() const
{
    return _showFullPathName;
}

void PointSizeByModel::setShowFullPathName(const bool& showFullPathName)
{
    _showFullPathName = showFullPathName;

    updateData();
}

void PointSizeByModel::updateData()
{
    // Update the datasets string list model
    for (auto dataset : _datasets) {

        // Continue if the dataset is not valid
        if (!dataset.isValid())
            continue;

        // Get dataset model index
        const auto datasetModelIndex = index(_datasets.indexOf(dataset), 0);

        // Notify others that the data changed
        emit dataChanged(datasetModelIndex, datasetModelIndex);
    }
}
