# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## Build System

This is a Visual Studio project (no CMake or Makefile). Build via:
- Open `EngineFramework.sln` in Visual Studio 2019+
- Target: Win32 or x64, Debug or Release
- Toolset: v143

There are no automated tests.

## Architecture Overview

A **Vulkan-based 3D graphics engine** with a component-entity scene graph.

**Entry point:** `main.cpp` → `Application::getInstance()->run()`

**Initialization order:** `AppWindow` → `VulkanInstance` → `Renderer` → Materials → main loop

### Core Subsystems

**Application Core** (`Core/App/`)
- `Application`: Singleton orchestrating the engine lifecycle
- `AppWindow`: GLFW window wrapper (800×800)

**Vulkan Layer** (`Core/Vulkan/`)
- `VulkanInstance`, `VulkanDevice`, `VulkanSwapChain`, `VulkanValidator`
- Validation layers enabled by default for debug builds

**Renderer & Graphics Pipeline** (`Core/Graphics/`)
- `Renderer` (singleton): swap chain, frame sync, render loop
- `GraphicsPipeline`: per-pipeline shaders, descriptors, render commands
- Three pipelines: default (opaque), skybox, UI text (transparent)
- `Material`: abstracts shader uniform binding; created via `Material::create()`, looked up via `Material::find()`
- `GPUBuffer`, `GPUTexture`: GPU resource management with static factory/lookup methods
- `MeshLoader`: static OBJ loader; supports `.obj` meshes

**Entity-Component System** (`Core/Entity/`)
- `Entity`: base transform (position/rotation, `matrix4f`)
- `SceneNode`: extends Entity with parent-child hierarchy
- Components: `MeshRenderer` (renderable), `Camera`; base interfaces `IEntityComponent`, `IRenderable`
- `RenderNode`: SceneNode with attached MeshRenderer

**Scene Management** (`Core/Scene/`)
- `World` (singleton): owns the scene graph, processes `EntityCommandBuffer`
- Update lifecycle per node: `preUpdate()` → `onUpdate()` → `update()` → `lateUpdate()`
- `EntityCommandBuffer`: deferred entity operations (add/remove) applied end-of-frame

**Input & Events** (`Core/Input/`, `Core/Events/`)
- `EventDispatcher<T>`: type-safe event system
- Convenience macros: `ADD_KEYBOARD_EVENT_LISTENER`, `SEND_WINDOW_EVENT`
- `InputHandler`: bridges GLFW callbacks to the event system

**UI** (`Core/UI/`)
- `FontAsset`: TTF font loading + rasterization
- `TextMesh`: text rendering using the transparent UI pipeline

**Data Structures** (`Core/Structure/`)
- `linkedList<T>`: custom generic linked list
- `IDVector<T>`: ID-keyed vector
- `DataPtr`: generic data pointer wrapper

**Math:** external `shml` library — `vec3f`, `vec4f`, `matrix4f`, `quat`

### Key Patterns

- **Singleton:** `Application`, `AppWindow`, `VulkanInstance`, `Renderer`, `World` — all accessed via `::getInstance()` / `::getWorld()`
- **Static factory + cache:** `Material::create()` / `Material::find()`, `GPUTexture::createTexture()` / `GPUTexture::getTexture()`
- **Deferred rendering:** `RenderCommandList` collects draw calls during update; `Renderer` executes them at frame end
- **Resource lifetime:** manual `new`/`delete`; explicit teardown methods required

### Assets

- `Assets/Shaders/` — pre-compiled SPIR-V (`.spv`); source `.vert`/`.frag` files alongside
- `Assets/Meshes/` — OBJ files
- `Assets/Textures/` — PNG/JPG images
- `Assets/Fonts/` — TTF files
