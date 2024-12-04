#pragma once

#include <Functions.h>
#include <IPluginInterface.h>
#include <Glacier/ZInput.h>
#include <Glacier/ZItem.h>
#include <Glacier/ZScene.h>

class EntityPathTrace : public IPluginInterface {
public:
    void Init() override;
    void OnEngineInitialized() override;
    ~EntityPathTrace() override;
    void OnDrawMenu() override;
    void OnDraw3D(IRenderer *p_Renderer) override;
private:
    void OnFrameUpdate(const SGameUpdateEvent& p_UpdateEvent);
    SVector3 Vector3Offset(SVector3 initialPoint, float32 size);

    DECLARE_PLUGIN_DETOUR(EntityPathTrace, bool, ZInputAction_Digital, ZInputAction* th, int a2);
    DECLARE_PLUGIN_DETOUR(EntityPathTrace, bool, PinOutput, ZEntityRef entity, uint32_t pinId, const ZObjectRef& data);
    DECLARE_PLUGIN_DETOUR(EntityPathTrace, void, OnLoadScene, ZEntitySceneContext* th, ZSceneData& p_sceneData);
    DECLARE_PLUGIN_DETOUR(EntityPathTrace, void, OnReloadScene, ZEntitySceneContext* th, bool forReload);

private:
    bool m_showTraceLines = false;
    bool m_saveAllTraces = false;
    bool m_isTaser = false;
    ZInputAction m_selectTraceItemInputAction = "SelectTraceItem";
    ZInputAction m_clearCurrentTraceInputAction = "ClearCurrentTrace";
    ZHM5Item* m_currentTraceItem = nullptr;
    ZHM5Action* m_currentTraceItemAction = nullptr;
    std::vector<SVector3> m_traceItemPositions;
    std::vector<std::vector<SVector3>> m_allTracePositions;
    float32 m_tracePathSize = 0.05;
    SVector4 m_tracePathColor = SVector4(.13, .88, .13, 0);
};

DEFINE_ZHM_PLUGIN(EntityPathTrace)

