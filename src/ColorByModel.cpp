#include "ColorByModel.h"

#include "DataHierarchyItem.h"
#include "Application.h"

using namespace hdps;

ColorByModel::ColorByModel(QObject* parent /*= nullptr*/) :
    QAbstractListModel(parent),
    _colorDatasets(),
    _showFullPathName(true)
{
}

int ColorByModel::rowCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    // Constant color option plus the number of available color datasets
    return _colorDatasets.count() + 1;
}

int ColorByModel::rowIndex(const Dataset<DatasetImpl>& colorDataset) const
{
    // Only proceed if we have a valid color dataset
    if (!colorDataset.isValid())
        return -1;

    // Return the index of the color dataset and add one for the constant color option
    return _colorDatasets.indexOf(colorDataset) + 1;
}

int ColorByModel::columnCount(const QModelIndex& parent /*= QModelIndex()*/) const
{
    return 1;
}

QVariant ColorByModel::data(const QModelIndex& index, int role) const
{
    // Get row/column and smart pointer to the color dataset
    const auto row          = index.row();
    const auto column       = index.column();
    const auto colorDataset = getColorDataset(row);

    switch (role)
    {
        // Return palette icon for constant color and dataset icon otherwise
        case Qt::DecorationRole:
            return row > 0 ? colorDataset->getIcon() : Application::getIconFont("FontAwesome").getIcon("palette");

        // Return 'Constant' for constant color and dataset (full path) GUI name otherwise
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

const QVector<Dataset<DatasetImpl>>& ColorByModel::getDatasets() const
{
    return _colorDatasets;
}

Dataset<DatasetImpl> ColorByModel::getColorDataset(const std::int32_t& rowIndex) const
{
    // Return empty smart pointer when out of range
    if (rowIndex <= 0 || rowIndex > _colorDatasets.count())
        return Dataset<DatasetImpl>();

    // Subtract the constant color row
    return _colorDatasets[rowIndex - 1];
}

void ColorByModel::setColorDatasets(const QVector<Dataset<DatasetImpl>>& colorDatasets)
{
    if (colorDatasets.count() != _colorDatasets.count()) {

        // Notify others that the model layout is about to be changed
        emit layoutAboutToBeChanged();

        // Assign the datasets
        _colorDatasets = colorDatasets;

        // Notify others that the model layout is changed
        emit layoutChanged();
    }
    else {
        // Assign the datasets
        _colorDatasets = colorDatasets;
    }

    // Update model when a dataset GUI name changes
    for (const auto& colorDataset : _colorDatasets) {

        // Notify others that the model has updated when the dataset GUI name changes
        connect(&colorDataset, &Dataset<DatasetImpl>::dataGuiNameChanged, this, [this, &colorDataset]() {

            // Get row index of the color dataset
            const auto colorDatasetRowIndex = rowIndex(colorDataset);

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

    // And update model data with datasets
    updateData();
}

void ColorByModel::removeColorDataset(const hdps::Dataset<hdps::DatasetImpl>& colorDataset)
{
    // Copy existing datasets
    auto updatedDatasets = _colorDatasets;

    // Remove from the vector
    updatedDatasets.removeOne(colorDataset);

    // Assign new datasets
    setColorDatasets(updatedDatasets);

    // And update model data with altered datasets
    updateData();
}

bool ColorByModel::getShowFullPathName() const
{
    return _showFullPathName;
}

void ColorByModel::setShowFullPathName(const bool& showFullPathName)
{
    _showFullPathName = showFullPathName;

    updateData();
}

void ColorByModel::updateData()
{
    // Update the datasets string list model
    for (auto dataset : _colorDatasets) {

        // Continue if the dataset is not valid
        if (!dataset.isValid())
            continue;

        // Get dataset model index
        const auto datasetModelIndex = index(_colorDatasets.indexOf(dataset), 0);

        // Notify others that the data changed
        emit dataChanged(datasetModelIndex, datasetModelIndex);
    }
}
