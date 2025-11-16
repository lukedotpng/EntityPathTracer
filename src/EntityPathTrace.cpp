#include "EntityPathTrace.h"

#include <Logging.h>
#include <Globals.h>
#include <Glacier/ZInputActionManager.h>
#include <Glacier/ZGameLoopManager.h>
#include <Glacier/ZScene.h>
#include <Glacier/ZAction.h>
#include <Glacier/ZItem.h>
#include <Glacier/ZEntity.h>
#include <Glacier/ZMath.h>
#include <Glacier/ZObject.h>
#include <Glacier/ZModule.h>
#include <Hooks.h>

void EntityPathTrace::Init() {
    Hooks::ZInputAction_Digital->AddDetour(this, &EntityPathTrace::ZInputAction_Digital);
    Hooks::SignalOutputPin->AddDetour(this, &EntityPathTrace::PinOutput);
    Hooks::ZEntitySceneContext_LoadScene->AddDetour(this, &EntityPathTrace::OnLoadScene);
    Hooks::ZEntitySceneContext_ClearScene->AddDetour(this, &EntityPathTrace::OnReloadScene);
}

void EntityPathTrace::OnEngineInitialized() {
    Logger::Info("EntityPathTrace has been initialized!");

    std::string inputBinds = "EntityPathTraceBinds={SelectTraceItem=tap(kb,p);ClearCurrentTrace=tap(kb,o);};";

    if(ZInputActionManager::AddBindings(inputBinds.c_str())) {
        Logger::Debug("Added EntityPathTrace Keybinds!!");
    }

    // Register a function to be called on every game frame while the game is in play mode.
    const ZMemberDelegate<EntityPathTrace, void(const SGameUpdateEvent&)> s_Delegate(this, &EntityPathTrace::OnFrameUpdate);
    Globals::GameLoopManager->RegisterFrameUpdate(s_Delegate, 1, EUpdateMode::eUpdatePlayMode);
}

EntityPathTrace::~EntityPathTrace() {
    // Unregister our frame update function when the mod unloads.
    const ZMemberDelegate<EntityPathTrace, void(const SGameUpdateEvent&)> s_Delegate(this, &EntityPathTrace::OnFrameUpdate);
    Globals::GameLoopManager->UnregisterFrameUpdate(s_Delegate, 1, EUpdateMode::eUpdatePlayMode);
}

void EntityPathTrace::OnDrawMenu() {
    // Toggle our message when the user presses our button.
    float color[] = {m_tracePathColor.x, m_tracePathColor.y, m_tracePathColor.z, m_tracePathColor.w};

    if(ImGui::BeginMenu("Item Tracer Settings")) {
        ImGui::Checkbox("Enable tracing", &m_showTraceLines);
        ImGui::Checkbox("Use depth rendering", &m_useDepth);
        ImGui::Checkbox("Save all traces lines (May cause crashes)", &m_saveAllTraces);
        ImGui::SliderFloat("Path Size", &m_tracePathSize, 0, 2, "%.2f", ImGuiSliderFlags_None);
        ImGui::SetColorEditOptions(ImGuiColorEditFlags_AlphaBar);
        ImGui::ColorPicker4("Path color", color);

        ImGui::EndMenu();

        m_tracePathColor.x = color[0];
        m_tracePathColor.y = color[1];
        m_tracePathColor.z = color[2];
        m_tracePathColor.w = color[3];
    }
}
void EntityPathTrace::OnFrameUpdate(const SGameUpdateEvent &p_UpdateEvent) {
    if(Functions::ZInputAction_Digital->Call(&m_selectTraceItemInputAction, 1)) {
        if(m_showTraceLines) {
            m_showTraceLines = false;
            Logger::Debug("Hiding trace lines");
        } else {
            m_showTraceLines = true;
            Logger::Debug("Showing trace lines");
        }
    }

    if (!m_saveAllTraces && m_allTracePositions.size() > 0) {
        m_allTracePositions.clear();
    }

    if(Functions::ZInputAction_Digital->Call(&m_clearCurrentTraceInputAction, 1)) {
        if(!m_allTracePositions.empty() && m_traceItemPositions.size() <= 2) {
            Logger::Debug("Clearing past trace line!");
            m_allTracePositions.pop_back();
        } else if(m_traceItemPositions.size() > 2) {
            Logger::Debug("Clearing current trace line!");
            m_traceItemPositions.clear();
        }
    }

    // Log Positions
    if(!m_currentTraceItem) {
        return;
    }

    const auto currentItemSpatial = m_currentTraceItem->m_rPhysicsAccessor.m_ref.QueryInterface<ZSpatialEntity>();
    const SVector3 traceItemPosition = currentItemSpatial->m_mTransform.Trans;

    if(m_traceItemPositions.size() <= 1) {
        m_traceItemPositions.push_back(traceItemPosition);
        m_traceItemPositions.push_back(traceItemPosition);
        return;
    }

    if((traceItemPosition.x <= m_traceItemPositions.back().x + .05 && traceItemPosition.x >= m_traceItemPositions.back().x - .05) &&
        (traceItemPosition.y <= m_traceItemPositions.back().y + .05 && traceItemPosition.y >= m_traceItemPositions.back().y - .05) &&
        (traceItemPosition.z <= m_traceItemPositions.back().z + .05 && traceItemPosition.z >= m_traceItemPositions.back().z - .05))
        {
        // Item is not moving
        return;
    }

    if(m_currentTraceItemAction) {
        if(m_currentTraceItemAction->m_bVisible || m_isTaser) {
            m_traceItemPositions.push_back(traceItemPosition);
        }
    } else {
        m_traceItemPositions.push_back(traceItemPosition);
    }
}

void EntityPathTrace::OnDraw3D(IRenderer *p_Renderer) {
    if (!m_useDepth) {
        DrawTraceLines(p_Renderer);
    }
}

void EntityPathTrace::OnDepthDraw3D(IRenderer *p_Renderer) {
    if (m_useDepth) {
        DrawTraceLines(p_Renderer);
    }
}

void EntityPathTrace::DrawTraceLines(IRenderer *p_Renderer) {
    if(!m_currentTraceItem || !m_showTraceLines) {
        return;
    }

    const auto currentItemSpatial = m_currentTraceItem->m_rPhysicsAccessor.m_ref.QueryInterface<ZSpatialEntity>();

    p_Renderer->DrawOBB3D(
       SVector3(-.1f, -.1f, -.1f),
       SVector3(.1f, .1f, .1f),
       currentItemSpatial->GetWorldMatrix(),
       SVector4(1, 1, 1, .5f)
       );


    if(m_traceItemPositions.size() > 2) {
        for(int i = 0; i < m_traceItemPositions.size() - 1; i++) {
            p_Renderer->DrawQuad3D(
                Vector3Offset(m_traceItemPositions[i], -m_tracePathSize), m_tracePathColor,
                Vector3Offset(m_traceItemPositions[i], m_tracePathSize), m_tracePathColor,
                Vector3Offset(m_traceItemPositions[i + 1], m_tracePathSize), m_tracePathColor,
                Vector3Offset(m_traceItemPositions[i + 1], -m_tracePathSize), m_tracePathColor
                );
        }
    }

    // Print past traces
    if(m_saveAllTraces) {
        for(auto & tracePosition : m_allTracePositions) {
            if(tracePosition.empty()) {
                continue;
            }

            if(tracePosition.size() <= 1) {
                continue;
            }
            for(int i = 0; i < tracePosition.size() - 1; i++) {
                p_Renderer->DrawQuad3D(
                    Vector3Offset(tracePosition[i], -m_tracePathSize), m_tracePathColor,
                    Vector3Offset(tracePosition[i], m_tracePathSize), m_tracePathColor,
                    Vector3Offset(tracePosition[i + 1], m_tracePathSize), m_tracePathColor,
                    Vector3Offset(tracePosition[i + 1], -m_tracePathSize), m_tracePathColor
                    );
            }
        }
    }
}

SVector3 EntityPathTrace::Vector3Offset(const SVector3 initialPoint, const float32 size) {
    SVector3 offsetVector;
    offsetVector.x = initialPoint.x - size;
    offsetVector.y = initialPoint.y - size;
    offsetVector.z = initialPoint.z - size;

    return offsetVector;
}

DEFINE_PLUGIN_DETOUR(EntityPathTrace, bool, ZInputAction_Digital, ZInputAction* th, int a2) {
    return { HookAction::Continue() };
}

DEFINE_PLUGIN_DETOUR(EntityPathTrace, bool, PinOutput, ZEntityRef entity, uint32_t pinId, const ZObjectRef& data) {
    if(pinId != -238204392 && pinId != -2063305321) {
        return {HookAction::Continue()};
    }

    Logger::Debug("Item dropped!");

    const auto traceableItem = entity.QueryInterface<ZHM5Item>();

    if(traceableItem) {
        Logger::Debug("\tZHM5Item found!");
        if(traceableItem->m_rPhysicsAccessor.m_ref.QueryInterface<ZSpatialEntity>()) {
            Logger::Debug("\tZSpatialEntity found!");
            if(traceableItem->m_pItemConfigDescriptor->m_sTitle.ToStringView().contains("Breach")) {
                Logger::Debug("\tBreaching charged dropped, moving on :P");
                return {HookAction::Continue()};
            }
        } else {
            Logger::Debug("\tNo ZSpatialEntity found");
            return {HookAction::Continue()};
        }
    } else {
        Logger::Debug("\tNo ZHM5Item found");
        return {HookAction::Continue()};
    }

    const ZHM5ActionManager* actionManager = Globals::HM5ActionManager;

    for (const auto currAction : actionManager->m_Actions) {
        if(currAction && currAction->m_eActionType == EActionType::AT_PICKUP) {
            if(const ZHM5Item* currItem = currAction->m_Object.QueryInterface<ZHM5Item>()) {
                if(currItem->m_pItemConfigDescriptor == nullptr || traceableItem->m_pItemConfigDescriptor == nullptr) {
                    return { HookAction::Continue() };
                }

                if(currItem->m_pItemConfigDescriptor->m_RepositoryId == traceableItem->m_pItemConfigDescriptor->m_RepositoryId) {
                    Logger::Debug("\tFound matching entity!");

                    if(currItem->m_pItemConfigDescriptor->m_sTitle.ToStringView().contains("Taser") || currItem->m_pItemConfigDescriptor->m_sTitle.ToStringView().contains("EMP")) {
                        m_isTaser = true;
                    } else {
                        m_isTaser = false;
                    }


                    if(m_currentTraceItem == nullptr) {
                        m_currentTraceItem = traceableItem;
                        m_currentTraceItemAction = currAction;
                        return { HookAction::Continue() };
                    }

                    if(traceableItem->m_pItemConfigDescriptor->m_sTitle == m_currentTraceItem->m_pItemConfigDescriptor->m_sTitle) {
                        return { HookAction::Continue() };
                    }

                    if (m_saveAllTraces) {
                        m_allTracePositions.push_back(m_traceItemPositions);
                    }
                    m_traceItemPositions.clear();
                    m_currentTraceItem = traceableItem;
                    m_currentTraceItemAction = currAction;
                    return { HookAction::Continue() };
                }
            }
        }
    }

    Logger::Debug("\tItem had no corresponding item on map");

    return { HookAction::Continue() };
}

DEFINE_PLUGIN_DETOUR(EntityPathTrace, void, OnLoadScene, ZEntitySceneContext* th, SSceneInitParameters&) {
    m_showTraceLines = false;
    m_currentTraceItem = nullptr;
    m_currentTraceItemAction = nullptr;
    m_traceItemPositions.clear();
    m_allTracePositions.clear();
    Logger::Debug("OnLoadScene");
    return { HookAction::Continue() };
}

DEFINE_PLUGIN_DETOUR(EntityPathTrace, void, OnReloadScene, ZEntitySceneContext* th, bool forReload) {
    m_showTraceLines = false;
    m_currentTraceItem = nullptr;
    m_currentTraceItemAction = nullptr;
    m_traceItemPositions.clear();
    m_allTracePositions.clear();
    Logger::Debug("OnReloadScene");
    return { HookAction::Continue() };
}

DECLARE_ZHM_PLUGIN(EntityPathTrace);

