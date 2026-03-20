![C++20](https://img.shields.io/badge/C++-20-blue.svg)
![DirectX 12](https://img.shields.io/badge/Graphics-DirectX%2012-lightgrey.svg)
![CMake](https://img.shields.io/badge/Build-CMake-green.svg)
![Platform](https://img.shields.io/badge/Platform-Windows-0078d7.svg)

Lumina Engine is a modern, data-driven 3D rendering engine built from scratch using **C++** and **DirectX 12**.

Designed with a focus on clean architecture and high performance, Lumina Engine serves as a robust framework for exploring modern real-time rendering techniques, featuring a deferred rendering pipeline, Physically Based Rendering (PBR).

## ✨ Key Features

### 🎨 Rendering & Graphics
* **Deferred Shading Pipeline:** Separated geometry and lighting passes for efficient multi-light rendering.
* **Physically Based Rendering (PBR):** Full support for Albedo, Normal, Metallic, Roughness, Emissive, and AO mapping.
* **HDR Skybox:** Spherical mapping and background rendering relying on early-Z depth magic.
* **Dynamic Camera System:** Smooth, delta-time independent FPS-style camera.
* **Real-time UI:** Integrated `Dear ImGui` for dynamic parameter tweaking (Light Direction, Color, Material parameters).

### 📦 Asset Pipeline
* **Model Loading:** Integrated `Assimp` for loading complex `.gltf` and `.fbx` meshes and submeshes.
* **Shader Compilation:** Real-time HLSL to DXIL compilation using the modern `DirectX Shader Compiler (DXC)`.

---

## 🗺️ Roadmap (Future Works)

Lumina Engine is actively in development. The upcoming milestones include:

- [ ] **Shadow Mapping:** Directional shadows with depth bias and omnidirectional point light shadows.
- [ ] **Post-Processing Stack:** Compute-shader based Bloom, Tone Mapping, and FXAA/TAA.
- [ ] **Forward+ Translucency:** Support for glass, water, and transparent materials.
- [ ] **Mesh Shaders:** Next-gen geometry pipeline integration.
- [ ] **DirectX Raytracing (DXR):** Hardware-accelerated ray-traced shadows and reflections.
- [ ] **Compute GPU Culling:** Frustum and occlusion culling driven by compute shaders.

---

## 🚀 Getting Started

### Prerequisites
* **OS:** Windows 10 / Windows 11
* **IDE:** CLion or Visual Studio 2022
* **Compiler:** MSVC (v143 or newer)
* **SDK:** Windows 10 SDK (10.0.19041.0 or newer)
* **Build System:** CMake 3.20+

### Build Instructions

1. **Clone the repository:**
   ```bash
   git clone [https://github.com/YourUsername/LuminaEngine.git](https://github.com/YourUsername/LuminaEngine.git)
   cd LuminaEngine
   ```
2. **Generate and Build (CMake):**

    The project uses CMake to automatically handle dependencies (Assimp, DXC, etc.) and post-build copy events.

```Bash
mkdir build && cd build
cmake ./
```

3. **Running the Engine:**

    Visual Studio: The CMake script automatically sets the VS_DEBUGGER_WORKING_DIRECTORY to the Assets/ folder. Just press F5!

    CLion: Edit your Run/Debug Configuration and set the Working Directory to $ProjectFileDir$/Assets.

## 📁 Directory Structure
```Plaintext
LuminaEngine/
├── Assets/             # 3D Models, Textures
├── Lumina/             # Engine C++ Source Code
│   ├── Engine/         # Input, ModelLoader, Basic Loop
│   ├── Renderer/       # DX12 Core, Deferred Renderer, TextureManager
│   ├── Platform/       # Window 
│   ├── Samples/        # Application entry points
│   └── Utils/          # External dependencies (Assimp, DXC, ImGui, etc.) and some useful utils
├── Shaders/            # Shaders
├── CMakeLists.txt      # Main build script
└── README.md
```
## 🤝 Dependencies
Lumina Engine stands on the shoulders of these amazing open-source libraries:

[DirectX-Headers](https://github.com/microsoft/DirectX-Headers) & [DXC](https://github.com/microsoft/DirectXShaderCompiler)

[Assimp](https://github.com/assimp/assimp) - Open Asset Import Library

[Dear ImGui](https://github.com/ocornut/imgui) - Bloat-free Graphical User interface

[stb_image](https://github.com/nothings/stb) - Image loading/decoding