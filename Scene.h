// Scene.h
#pragma once

#include <string>
#include <vector>

class Scene {
public:
    std::vector<std::string> skyboxFaces;

    void LoadFromYAML(const std::string& filepath);
};
