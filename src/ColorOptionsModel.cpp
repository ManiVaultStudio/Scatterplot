#include "ColorOptionsModel.h"

#include "DataHierarchyItem.h"
#include "Application.h"

using namespace hdps;

ColorOptionsModel::ColorOptionsModel(QObject* parent /*= nullptr*/) :
    QAbstractListModel(parent),
    _datasets(),
    _showFullPathName(true)
{
}

int ColorOptionsModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    // Constant color option plus the number of available datasets
    return _datasets.count() + 1;
}

int ColorOptionsModel::rowIndex(const Dataset<DatasetImpl>& dataset) const
{
    // Only proceed if we have a valid dataset
    if (!dataset.isValid())
        return -1;

    // Return the index of the dataset and add one for the constant color option
    return _datasets.indexOf(dataset) + 1;
}

int ColorOptionsModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 1;
}

QVariant ColorOptionsModel::data(const QModelIndex& index, int role) const
{
    // Get row/column and smart pointer to the dataset
    const auto row      = index.row();
    const auto column   = index.column();
    const auto dataset  = getDataset(row);

    switch (role)
    {
        // Return palette icon for constant color and dataset icon otherwise
        case Qt::DecorationRole:
            return row > 0 ? dataset->getIcon() : Application::getIconFont("FontAwesome").getIcon("palette");

        // Return 'Constant' for constant color and dataset (full path) GUI name otherwise
        case Qt::DisplayRole:
        {
            if (row > 0)
                if (row == 1)
                    return dataset->getGuiName();
                else
                    return _showFullPathName ? dataset->getDataHierarchyItem().getFullPathName() : dataset->getGuiName();
            else
                return "Constant";
        }

        default:
            break;
    }

    return QVariant();
}

const Datasets& ColorOptionsModel::getDatasets() const
{
    return _datasets;
}

Dataset<DatasetImpl> ColorOptionsModel::getDataset(const std::int32_t& rowIndex) const
{
    // Return empty smart pointer when out of range
    if (rowIndex <= 0 || rowIndex > _datasets.count())
        return Dataset<DatasetImpl>();

    // Subtract the constant color row
    return _datasets[rowIndex - 1];
}

void ColorOptionsModel::addDataset(const Dataset<DatasetImpl>& dataset)
{
    // Notify others that the model layout is about to be changed
    emit layoutAboutToBeChanged();

    // Add the datasets
    _datasets << dataset;

    // Notify others that the model layout is changed
    emit layoutChanged();

    // Get smart pointer to added dataset
    auto& addedDataset = _datasets.last();

    // Remove a dataset from the model when it is about to be deleted
    connect(&addedDataset, &Dataset<DatasetImpl>::dataAboutToBeRemoved, this, [this, &addedDataset]() {

        // Remove the dataset from the model before it is physically deleted
        removeDataset(addedDataset);
    });

    // Notify others that the model has updated when the dataset GUI name changes
    connect(&addedDataset, &Dataset<DatasetImpl>::dataGuiNameChanged, this, [this, &addedDataset]() {

        // Get row index of the dataset
        const auto datasetRowIndex = rowIndex(addedDataset);

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

void ColorOptionsModel::removeDataset(const Dataset<DatasetImpl>& dataset)
{
    // Notify others that the model layout is about to be changed
    emit layoutAboutToBeChanged();

    // Remove the corresponding row from the model
    beginRemoveRows(QModelIndex(), rowIndex(dataset), rowIndex(dataset));
    {
        // Remove from the vector
        _datasets.removeOne(dataset);
    }
    endRemoveRows();

    // Notify others that the model layout is changed
    emit layoutChanged();
}

void ColorOptionsModel::removeAllDatasets()
{
    // Notify others that the model layout is about to be changed
    emit layoutAboutToBeChanged();

    // Remove all datasets
    _datasets.clear();

    // Notify others that the model layout is changed
    emit layoutChanged();

    // And update model data with altered datasets
    updateData();
}

bool ColorOptionsModel::getShowFullPathName() const
{
    return _showFullPathName;
}

void ColorOptionsModel::setShowFullPathName(const bool& showFullPathName)
{
    _showFullPathName = showFullPathName;

    updateData();
}

void ColorOptionsModel::updateData()
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
