/**
 * @file RendererModule.h
 * @brief Modular initialization interface for renderer subsystems.
 *
 * Defines a common interface for all renderer modules (Scene, RenderGraph, etc.)
 * to initialize, shutdown, and tick in a consistent manner.
 */
#pragma once

class IRendererModule
{
public:
    virtual ~IRendererModule() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;

    virtual void Tick(float DeltaTime) { (void)DeltaTime; }

    [[nodiscard]] virtual bool IsInitialized() const = 0;
};

class FRendererModuleRegistry
{
public:
    // Future: RegisterModule, InitializeAll, ShutdownAll
};