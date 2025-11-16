#pragma once

#include <Functions.h>
#include <IPluginInterface.h>
#include <Glacier/ZInput.h>
#include <Glacier/ZItem.h>
#include <Glacier/ZScene.h>
#include <Glacier/ZEntity.h>

class EntityPathTrace : public IPluginInterface {
public:
    void Init() override;
    void OnEngineInitialized() override;
    EntityPathTrace();
    ~EntityPathTrace() override;
    void OnDrawMenu() override;
    void OnDraw3D(IRenderer *p_Renderer) override;
    void OnDepthDraw3D(IRenderer *p_Renderer) override;
private:
    void OnFrameUpdate(const SGameUpdateEvent& p_UpdateEvent);
    void DrawTraceLines(IRenderer *p_Renderer);
    SVector3 Vector3Offset(SVector3 initialPoint, float32 size);

    DECLARE_PLUGIN_DETOUR(EntityPathTrace, bool, ZInputAction_Digital, ZInputAction* th, int a2);
    DECLARE_PLUGIN_DETOUR(EntityPathTrace, bool, PinOutput, ZEntityRef entity, uint32_t pinId, const ZObjectRef& data);
    DECLARE_PLUGIN_DETOUR(EntityPathTrace, void, OnLoadScene, ZEntitySceneContext*, SSceneInitParameters&);
    DECLARE_PLUGIN_DETOUR(EntityPathTrace, void, OnReloadScene, ZEntitySceneContext* th, bool forReload);

private:
    bool m_showTraceLines;
    bool m_useDepth;
    bool m_saveAllTraces;
    bool m_isTaser;
    ZInputAction m_selectTraceItemInputAction;
    ZInputAction m_clearCurrentTraceInputAction;
    ZHM5Item* m_currentTraceItem;
    ZHM5Action* m_currentTraceItemAction;
    std::vector<SVector3> m_traceItemPositions;
    std::vector<std::vector<SVector3>> m_allTracePositions;
    float32 m_tracePathSize;
    SVector4 m_tracePathColor;
};

DEFINE_ZHM_PLUGIN(EntityPathTrace)

