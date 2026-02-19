#include "ScalarSourceModel.h"

#include <DataHierarchyItem.h>
#include <Application.h>
#include <Set.h>

using namespace mv;
using namespace mv::util;

ScalarSourceModel::ScalarSourceModel(QObject* parent /*= nullptr*/) :
    QStandardItemModel(parent),
    _showFullPathName(true)
{
    appendRow(Row({}));     // Constant source
    appendRow(Row({}));     // Selection source
}

ScalarSourceModel::Item::Item(const mv::Dataset<>& scalarDataset) :
    _scalarDataset(scalarDataset)
{
}

QVariant ScalarSourceModel::Item::data(int role) const
{
    const auto rowIndex = row();

    switch (role)
    {
        // Return ruler icon for constant point size and dataset icon otherwise
	    case Qt::DecorationRole:
	    {
	        if (rowIndex == DefaultRow::Constant)
	            return StyledIcon("ruler");

	        if (rowIndex == DefaultRow::Selection)
	            return StyledIcon("mouse-pointer");

	        if (rowIndex >= DefaultRow::DatasetStart)
	            return _scalarDataset->icon();

	        break;
	    }

	    // Return 'Constant' for constant point size and dataset (full path) GUI name otherwise
        case Qt::DisplayRole:
	    {
		    if (rowIndex >= DefaultRow::DatasetStart)
		    {
			    if (rowIndex == 2)
			    	return _scalarDataset->text();
			    //else
			    //    return _showFullPathName ? scalarDataset->getLocation() : scalarDataset->text();
		    }
		    else {
			    if (rowIndex == DefaultRow::Constant)
			    	return "Constant";

			    if (rowIndex == DefaultRow::Selection)
			    	return "Selection";
		    }
	    }

	    default:
	        break;
    }

    return {};
}

const mv::Dataset<>& ScalarSourceModel::Item::getScalarDataset() const
{
    return _scalarDataset;
}

ScalarSourceModel::NameItem::NameItem(const mv::Dataset<>& scalarDataset) :
    Item(scalarDataset)
{
	connect(&const_cast<Dataset<>&>(getScalarDataset()), &Dataset<>::guiNameChanged, this, [this]() {
        emitDataChanged();
    });
}

QVariant ScalarSourceModel::NameItem::data(int role) const
{
    const auto rowIndex = row();

    switch (role)
    {
	    case Qt::DisplayRole:
	    case Qt::EditRole:
		{
            if (rowIndex == DefaultRow::Constant)
                return "Constant";

            if (rowIndex == DefaultRow::Selection)
                return "Selection";

            if (rowIndex >= DefaultRow::DatasetStart)
                return getScalarDataset()->getGuiName();

            break;
		}

	    case Qt::ToolTipRole:
        {
            if (rowIndex == DefaultRow::Constant)
                return "Constant";

            if (rowIndex == DefaultRow::Selection)
                return "Selection";

            if (rowIndex >= DefaultRow::DatasetStart)
                return getScalarDataset()->getLocation();

            break;
        }

	    default:
	        break;
    }

    return Item::data(role);
}

QVariant ScalarSourceModel::IdItem::data(int role) const
{
    const auto rowIndex = row();

    switch (role)
    {
	    case Qt::DisplayRole:
	    case Qt::EditRole:
	    {
	        if (rowIndex == DefaultRow::Constant || rowIndex == DefaultRow::Selection)
	            return "";

	        if (rowIndex >= DefaultRow::DatasetStart)
	            return getScalarDataset()->getId();

	        break;
	    }

	    case Qt::ToolTipRole:
	    {
            if (rowIndex == DefaultRow::Constant || rowIndex == DefaultRow::Selection)
                return "";

	        if (rowIndex >= DefaultRow::DatasetStart)
	            return "ID: " + getScalarDataset()->getId();

	        break;
	    }

	    default:
	        break;
    }

    return Item::data(role);
}

/*
int ScalarSourceModel::rowIndex(const Dataset<DatasetImpl>& dataset) const
{
    // Only proceed if we have a valid dataset
    if (!dataset.isValid())
        return -1;

    // Return the index of the dataset and add one for the constant point size option
    return _datasets.indexOf(dataset) + DefaultRow::DatasetStart;
}
*/



void ScalarSourceModel::addDataset(const Dataset<DatasetImpl>& dataset)
{
    // Avoid duplicates
    if (hasDataset(dataset))
        return;

    appendRow(Row(dataset));
}

bool ScalarSourceModel::hasDataset(const Dataset<DatasetImpl>& dataset) const
{
    if (!dataset.isValid())
        return false;
    else
		return !match(index(0, static_cast<int>(Column::Id)), Qt::EditRole, dataset->getId(), 1, Qt::MatchExactly).isEmpty();
}

void ScalarSourceModel::removeDataset(const Dataset<DatasetImpl>& dataset)
{
    if (!hasDataset(dataset))
        return;

    const auto matches = match(index(0, static_cast<int>(Column::Id)), Qt::EditRole, dataset->getId(), 1, Qt::MatchExactly);

    if (!matches.isEmpty())
        removeRow(matches.first().row());
}

void ScalarSourceModel::removeAllDatasets()
{
    removeRows(DefaultRow::DatasetStart, rowCount() - DefaultRow::DatasetStart);
}

const Datasets& ScalarSourceModel::getDatasets() const
{
    Datasets datasets;

    for (int rowIndex = DefaultRow::DatasetStart; rowIndex < rowCount(); ++rowIndex)
    {
        if (auto item = dynamic_cast<Item*>(itemFromIndex(index(rowIndex, 0)))) {
            const auto dataset = item->getScalarDataset();

        	if (dataset.isValid())
                datasets.append(dataset);
        }
    }

    return datasets;
}

Dataset<DatasetImpl> ScalarSourceModel::getDataset(const std::int32_t& rowIndex) const
{
    if (auto item = dynamic_cast<Item*>(itemFromIndex(index(rowIndex, 0))))
		return item->getScalarDataset();

    return {};
}

void ScalarSourceModel::setDatasets(const Datasets& datasets)
{
    for (const auto& dataset : datasets)
        addDataset(dataset);
}

std::int32_t ScalarSourceModel::getRowIndex(const Dataset<DatasetImpl>& dataset) const
{
    if (!dataset.isValid())
        return -1;

	const auto matches = match(index(0, static_cast<int>(Column::Id)), Qt::EditRole, dataset->getId(), 1, Qt::MatchExactly);

    if (!matches.isEmpty())
        return matches.first().row();

    return -1;
}

bool ScalarSourceModel::getShowFullPathName() const
{
    return _showFullPathName;
}

void ScalarSourceModel::setShowFullPathName(const bool& showFullPathName)
{
    _showFullPathName = showFullPathName;
}
