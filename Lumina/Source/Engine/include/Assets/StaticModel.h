#pragma once

#include <string>
#include <vector>

#include "Renderer/MeshType.h"

class StaticModel
{
public:
    StaticModel() = default;
    ~StaticModel() = default;

    bool LoadFromFile(const std::string& FilePath);

    [[nodiscard]] const std::vector<FMeshData>& GetMeshesData() const { return mMeshesData; }
    [[nodiscard]] const std::vector<std::string>& GetMaterialNames() const { return mMaterialNames; }

private:
    std::vector<FMeshData> mMeshesData;
    std::vector<std::string> mMaterialNames;
};