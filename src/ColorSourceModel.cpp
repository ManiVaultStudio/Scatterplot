#include "ColorSourceModel.h"

#include <DataHierarchyItem.h>
#include <Application.h>
#include <Set.h>

using namespace mv;

QVariant ColorSourceModel::ConstantColorItem::data(int role /*= Qt::UserRole + 1*/) const
{
    switch (role)
    {
        case Qt::DecorationRole:
            return Application::getIconFont("FontAwesome").getIcon("palette");

        case Qt::DisplayRole:
            return "Constant";

        default:
            break;
    }

    return {};
}

QVariant ColorSourceModel::ScatterLayoutItem::data(int role /*= Qt::UserRole + 1*/) const
{
    switch (role)
    {
        case Qt::DecorationRole:
            return Application::getIconFont("FontAwesome").getIcon("palette");

        case Qt::DisplayRole:
            return "Scatter layout";

        default:
            break;
    }

    return {};
}

ColorSourceModel::DatasetItem::DatasetItem(Dataset<DatasetImpl> dataset, ColorSourceModel* colorSourceModel) :
    _dataset(dataset),
    _colorSourceModel(colorSourceModel)
{
    Q_ASSERT(_dataset.isValid());
    Q_ASSERT(_colorSourceModel);

    if (!dataset.isValid() || !_colorSourceModel)
        return;

    connect(&_dataset, &Dataset<DatasetImpl>::aboutToBeRemoved, this, [this]() {
        _colorSourceModel->removeDataset(_dataset);
    });

    connect(_dataset.get(), &DatasetImpl::textChanged, this, [this]() {
        emitDataChanged();
    });

    connect(_colorSourceModel, &ColorSourceModel::showFullPathNameChanged, this, [this]() {
        emitDataChanged();
    });
}

QVariant ColorSourceModel::DatasetItem::data(int role /*= Qt::UserRole + 1*/) const
{
    switch (role)
    {
        case Qt::DecorationRole:
            return _dataset->getIcon();

        case Qt::DisplayRole:
            return row() == 2 ? _dataset->text() : (_colorSourceModel->getShowFullPathName() ? _dataset->getLocation() : _dataset->text());

        case 200:
            return _dataset->getId();

        default:
            break;
    }

    return {};
}

mv::Dataset<mv::DatasetImpl>& ColorSourceModel::DatasetItem::getDataset()
{
    return _dataset;
}

ColorSourceModel::ColorSourceModel(QObject* parent /*= nullptr*/) :
    QStandardItemModel(parent),
    _showFullPathName(true)
{
    appendRow(new ConstantColorItem());
    appendRow(new ScatterLayoutItem());
}

void ColorSourceModel::addDataset(Dataset<DatasetImpl> dataset)
{
    appendRow(new DatasetItem(dataset, this));
}

void ColorSourceModel::removeDataset(Dataset<DatasetImpl> dataset)
{
    const auto matches = match(QModelIndex(), 200, dataset->getId(), 1, Qt::MatchExactly);

    if (matches.isEmpty())
        return;

    removeRows(matches.first().row(), 1);
}

void ColorSourceModel::removeAllDatasets()
{
}

bool ColorSourceModel::getShowFullPathName() const
{
    return _showFullPathName;
}

void ColorSourceModel::setShowFullPathName(const bool& showFullPathName)
{
    if (showFullPathName == _showFullPathName)
        return;

    _showFullPathName = showFullPathName;

    emit showFullPathNameChanged(_showFullPathName);
}
