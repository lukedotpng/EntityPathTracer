#pragma once

#include <IPluginInterface.h>

#include <Glacier/SGameUpdateEvent.h>

class EntityPathTrace : public IPluginInterface {
public:
    void OnEngineInitialized() override;
    ~EntityPathTrace() override;
    void OnDrawMenu() override;
    void OnDrawUI(bool p_HasFocus) override;

private:
    void OnFrameUpdate(const SGameUpdateEvent& p_UpdateEvent);
    DECLARE_PLUGIN_DETOUR(EntityPathTrace, void, OnLoadScene, ZEntitySceneContext* th, ZSceneData& p_SceneData);

private:
    bool m_ShowMessage = false;
};

DEFINE_ZHM_PLUGIN(EntityPathTrace)
