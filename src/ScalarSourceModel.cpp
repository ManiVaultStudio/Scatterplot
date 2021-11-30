#include "ScalarSourceModel.h"

#include "DataHierarchyItem.h"
#include "Application.h"

using namespace hdps;

ScalarSourceModel::ScalarSourceModel(QObject* parent /*= nullptr*/) :
    QAbstractListModel(parent),
    _datasets(),
    _showFullPathName(true)
{
}

int ScalarSourceModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    // Constant point size option plus the number of available datasets
    return _datasets.count() + 1;
}

int ScalarSourceModel::rowIndex(const Dataset<DatasetImpl>& dataset) const
{
    // Only proceed if we have a valid dataset
    if (!dataset.isValid())
        return -1;

    // Return the index of the dataset and add one for the constant point size option
    return _datasets.indexOf(dataset) + 1;
}

int ScalarSourceModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 1;
}

QVariant ScalarSourceModel::data(const QModelIndex& index, int role) const
{
    // Get row/column of and smart pointer to the dataset
    const auto row              = index.row();
    const auto column           = index.column();
    const auto scalarDataset    = getDataset(row);

    switch (role)
    {
        // Return ruler icon for constant point size and dataset icon otherwise
        case Qt::DecorationRole:
            return row > 0 ? scalarDataset->getIcon() : Application::getIconFont("FontAwesome").getIcon("ruler");

         // Return 'Constant' for constant point size and dataset (full path) GUI name otherwise
        case Qt::DisplayRole:
        {
            if (row > 0)
                if (row == 1)
                    return scalarDataset->getGuiName();
                else
                    return _showFullPathName ? scalarDataset->getDataHierarchyItem().getFullPathName() : scalarDataset->getGuiName();
            else
                return "Constant";
        }

        default:
            break;
    }

    return QVariant();
}

void ScalarSourceModel::removeAllDatasets()
{
    // Remove all datasets
    _datasets.clear();

    // And update model data with altered datasets
    updateData();
}

const Datasets& ScalarSourceModel::getDatasets() const
{
    return _datasets;
}

void ScalarSourceModel::addDataset(const Dataset<DatasetImpl>& dataset)
{
    // Add the datasets
    _datasets << dataset;

    // Remove a dataset from the model when it is about to be deleted
    connect(&dataset, &Dataset<DatasetImpl>::dataAboutToBeRemoved, this, [this, &dataset]() {

        // Remove from the vector
        _datasets.removeOne(dataset);

        // And update model data with altered datasets
        updateData();
    });

    // Notify others that the model has updated when the dataset GUI name changes
    connect(&dataset, &Dataset<DatasetImpl>::dataGuiNameChanged, this, [this, &dataset]() {

        // Get row index of the dataset
        const auto colorDatasetRowIndex = rowIndex(dataset);

        // Only proceed if we found a valid row index
        if (colorDatasetRowIndex < 0)
            return;

        // Establish model index
        const auto modelIndex = index(colorDatasetRowIndex, 0);

        // Only proceed if we have a valid model index
        if (!modelIndex.isValid())
            return;

        // Notify others that the data changed
        emit dataChanged(modelIndex, modelIndex);
    });
}

void ScalarSourceModel::removeDataset(const Dataset<DatasetImpl>& dataset)
{

}

Dataset<DatasetImpl> ScalarSourceModel::getDataset(const std::int32_t& rowIndex) const
{
    // Return empty smart pointer when out of range
    if (rowIndex <= 0 || rowIndex > _datasets.count())
        return Dataset<DatasetImpl>();

    // Subtract the constant point size row
    return _datasets[rowIndex - 1];
}

void ScalarSourceModel::setDatasets(const Datasets& datasets)
{
    // Notify others that the model layout is about to be changed
    emit layoutAboutToBeChanged();

    // Add datasets
    for (const auto& dataset : _datasets)
        addDataset(dataset);

    // Notify others that the model layout is changed
    emit layoutChanged();

    // And update model data with datasets
    updateData();
}

bool ScalarSourceModel::getShowFullPathName() const
{
    return _showFullPathName;
}

void ScalarSourceModel::setShowFullPathName(const bool& showFullPathName)
{
    _showFullPathName = showFullPathName;

    updateData();
}

void ScalarSourceModel::updateData()
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
