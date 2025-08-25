#include "MappingUtils.h"

#include <Dataset.h>
#include <LinkedData.h>
#include <PointData/PointData.h>
#include <Set.h>

#include <algorithm>
#include <cstdint>
#include <functional>
#include <map>
#include <tuple>
#include <ranges>
#include <utility>
#include <vector>

using CheckFunc = std::function<bool(const mv::LinkedData& linkedData, const mv::Dataset<Points>& target)>;

static void printLinkedDataNames(const mv::Dataset<Points>& data) {
    const std::vector<mv::LinkedData>& linkedFromColors = data->getLinkedData();
    if (!linkedFromColors.empty()) {
        qDebug() << data->getGuiName();
        qDebug() << linkedFromColors[0].getSourceDataSet()->getGuiName();
        qDebug() << linkedFromColors[0].getTargetDataset()->getGuiName();
    }
}

std::pair<const mv::LinkedData*, unsigned int> getSelectionMapping(const mv::Dataset<Points>& source, const mv::Dataset<Points>& target, CheckFunc checkMapping) {
    const std::vector<mv::LinkedData>& linkedDatas = source->getLinkedData();

    if (linkedDatas.empty())
        return { nullptr, 0 } ;

    // find linked data between source and target OR source and target's parent, if target is derived and they have the same number of points
    if (const auto result = std::ranges::find_if(
            linkedDatas, 
            [&target, &checkMapping](const mv::LinkedData& linkedData) -> bool {
                return checkMapping(linkedData, target);
            });
        result != linkedDatas.end()) 
    {
        return {&(*result), target->getNumPoints() };
    }

    return { nullptr, 0 };
}

std::pair<const mv::LinkedData*, unsigned int> getSelectionMappingColorsToPositions(const mv::Dataset<Points>& colors, const mv::Dataset<Points>& positions) {
    auto testTargetAndParent = [](const mv::LinkedData& linkedData, const mv::Dataset<Points>& positions) -> bool {
        const mv::Dataset<mv::DatasetImpl> mapTargetData = linkedData.getTargetDataset();
        return mapTargetData == positions || parentHasSameNumPoints(mapTargetData, positions);
        };

    return getSelectionMapping(colors, positions, testTargetAndParent);
}

std::pair<const mv::LinkedData*, unsigned int> getSelectionMappingPositionsToColors(const mv::Dataset<Points>& positions, const mv::Dataset<Points>& colors) {
    auto testTarget = [](const mv::LinkedData& linkedData, const mv::Dataset<Points>& colors) -> bool {
        return linkedData.getTargetDataset() == colors;
        };

    auto [mapping, numTargetPoints] = getSelectionMapping(positions, colors, testTarget);

    if (mapping && parentHasSameNumPoints(positions, positions)) {
        const auto positionsParent = positions->getParent<Points>();
        std::tie(mapping, numTargetPoints) = getSelectionMapping(positionsParent, colors, testTarget);
    }

    return { mapping, numTargetPoints };
}
}

bool checkSurjectiveMapping(const mv::LinkedData& linkedData, const std::uint32_t numPointsInTarget) {
    const std::map<std::uint32_t, std::vector<std::uint32_t>>& linkedMap = linkedData.getMapping().getMap();

    std::vector<bool> found(numPointsInTarget, false);
    std::uint32_t count = 0;

    for (const auto& [key, vec] : linkedMap) {
        for (std::uint32_t val : vec) {
            if (val >= numPointsInTarget) continue; // Skip values that are too large

            if (!found[val]) {
                found[val] = true;
                if (++count == numPointsInTarget)
                    return true;
            }
        }
    }

    return false; // The previous loop would have returned early if the entire taget set was covered
}

bool checkSelectionMapping(const mv::Dataset<Points>& colors, const mv::Dataset<Points>& positions) {

    // Check if there is a mapping
    auto mapping = getSelectionMappingColorsToPositions(colors, positions);
    auto numTargetPoints = positions->getNumPoints();

    // Check if there is a mapping
    auto [mapping, numTargetPoints] = getSelectionMappingColorsToPositions(colors, positions);

    if (!mapping)
        std::tie(mapping, numTargetPoints) = getSelectionMappingPositionsToColors(positions, colors);


    if (!mapping)
        return false;

    return checkSurjectiveMapping(*mapping, numTargetPoints);
}
