// Scene.cpp
#include "Scene.h"
#include "yaml.h"
#include <iostream>
#include "Skybox.h"
#include <imgui.h>

using namespace YAML;
using namespace std;

void Scene::LoadFromYAML(const string& filepath) {
    Node sceneFile = LoadFile(filepath);
    Node skyboxNode = sceneFile["Scene"]["Skybox"]["files"];

    skyboxFaces.clear();
    for (const auto& file : skyboxNode) {
        skyboxFaces.push_back(file.as<string>());
    }
}

void loadScene(const std::string& scenePath, Scene& scene, Skybox& skybox, std::vector<Shader>& shaders) {
    // Mostrar mensaje estilo Source Engine
    ImGui::Begin("Loading");
    ImGui::Text("Cargando escena... por favor espera.");
    ImGui::End();

    // Cargar escena
    scene.LoadFromYAML(scenePath);

    // Inicializar skybox con las rutas
    skybox = Skybox(scene.skyboxFaces);

    // Aquí más adelante cargarás GameObjects
}

