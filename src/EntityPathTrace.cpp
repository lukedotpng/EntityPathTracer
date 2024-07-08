#include "EntityPathTrace.h"
 #include "EntityPathTrace.h"

#include <Logging.h>
#include <Glacier/ZRender.h>
#include <Globals.h>
#include <Glacier/ZInputActionManager.h>
#include <Glacier/ZGameLoopManager.h>
#include <Glacier/ZScene.h>
#include <Glacier/ZAction.h>
#include <Glacier/ZItem.h>
#include <Glacier/ZEntity.h>
#include <Glacier/ZMath.h>
#include <Glacier/ZObject.h>
#include <Hooks.h>

void EntityPathTrace::Init() {
    Hooks::ZInputAction_Digital->AddDetour(this, &EntityPathTrace::ZInputAction_Digital);
    Hooks::SignalOutputPin->AddDetour(this, &EntityPathTrace::PinOutput);
    Hooks::ZEntitySceneContext_LoadScene->AddDetour(this, &EntityPathTrace::OnLoadScene);
    Hooks::ZEntitySceneContext_ClearScene->AddDetour(this, EntityPathTrace::OnReloadScene);


}

void EntityPathTrace::OnEngineInitialized() {
    Logger::Info("EntityPathTrace has been initialized!");

    const char* inputBinds = "EntityPathTraceBinds={SelectTraceItem=tap(kb,p);ClearCurrentTrace=tap(kb,o);};";

    if(ZInputActionManager::AddBindings(inputBinds)) {
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
        ImGui::SliderFloat("Path Size", &m_tracePathSize, 0, 2, "%.2f", ImGuiSliderFlags_None);
        ImGui::Checkbox("Show Item Name on Screen", &m_nameSelectedItemOnScreen);
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
        Logger::Debug("Pressed P");
        if(m_isTracing) {
            Logger::Debug("{} is no longer being traced", m_currentTraceItem->m_pItemConfigDescriptor->m_sTitle);
            m_isTracing = false;
            // m_currentTraceItem = nullptr;
            m_currentTraceItemAction = nullptr;
            m_currentTraceItemSpatial = nullptr;
            // m_traceItemPositions.clear();
        } else if(m_lastEntityCaptured != nullptr) {
            Logger::Debug("Starting a new capture!");
            if(m_currentTraceItem) {
                if(m_lastEntityCaptured.QueryInterface<ZHM5Item>()->m_pItemConfigDescriptor->m_RepositoryId != m_currentTraceItem->m_pItemConfigDescriptor->m_RepositoryId) {
                    m_traceItemPositions.clear();
                    m_currentTraceItem = m_lastEntityCaptured.QueryInterface<ZHM5Item>();
                }
            } else {
                m_traceItemPositions.clear();
                m_currentTraceItem = m_lastEntityCaptured.QueryInterface<ZHM5Item>();
            }

            if(!m_currentTraceItem) {
                Logger::Debug("No ZHM5Item on entity");
                return;
            }
            m_currentTraceItemSpatial = m_currentTraceItem->m_rPhysicsAccessor.m_ref.QueryInterface<ZSpatialEntity>();
            if(!m_currentTraceItemSpatial) {
                Logger::Debug("No spatial on item");
                return;
            }
            Logger::Debug("Checking for matching entity on map!");
            const ZHM5ActionManager* actionManager = Globals::HM5ActionManager;
            m_currentTraceItemAction = nullptr;
            for (const auto currAction : actionManager->m_Actions) {
                if(currAction && currAction->m_eActionType == EActionType::AT_PICKUP) {
                   if(const ZHM5Item* currItem = currAction->m_Object.QueryInterface<ZHM5Item>()) {
                       if(currItem->m_pItemConfigDescriptor->m_RepositoryId == m_currentTraceItem->m_pItemConfigDescriptor->m_RepositoryId) {
                           Logger::Debug("Found matching entity!");
                           m_currentTraceItemAction = currAction;
                           break;
                       }
                   }
                }
            }

            if(m_currentTraceItemAction == nullptr) {
                Logger::Debug("Item had no cooresponding item on map");
                return;
            }

            Logger::Debug("{} is now being traced", m_currentTraceItem->m_pItemConfigDescriptor->m_sTitle);
            m_isTracing = true;
        }
    }

    if(Functions::ZInputAction_Digital->Call(&m_clearCurrentTraceInputAction, 1)) {
        m_traceItemPositions.clear();
    }
}

void EntityPathTrace::OnDraw3D(IRenderer *p_Renderer) {
    if(!m_currentTraceItem || !m_currentTraceItemSpatial || !m_currentTraceItemAction || !m_isTracing) {
        return;
    }

    const SVector3 traceItemPosition = m_currentTraceItemSpatial->m_mTransform.Trans;

    if(m_nameSelectedItemOnScreen) {
        const ImGuiIO& guiIO = ImGui::GetIO();
        const float32 screenWidth = guiIO.DisplaySize.x * guiIO.DisplayFramebufferScale.x;

        p_Renderer->DrawText2D(
            m_currentTraceItem->m_pItemConfigDescriptor->m_sTitle,
            SVector2(screenWidth - 20, 20),
            SVector4(1, 1, 1, 1),
            0,
            .5,
            TextAlignment::Right
            );
    }

    if(m_currentTraceItemAction->m_bVisible) {
        p_Renderer->DrawBox3D(
           SVector3(traceItemPosition.x + .1f, traceItemPosition.y + .1f, traceItemPosition.z + .1f),
           SVector3(traceItemPosition.x - .1f, traceItemPosition.y - .1f, traceItemPosition.z -.1f),
           SVector4(1, 1, 1, 0)
           );
    }

    if(m_traceItemPositions.size() <= 1) {
        m_traceItemPositions.push_back(traceItemPosition);
        Logger::Debug("Vector is too small");
        return;
    }
    if(traceItemPosition.x == m_traceItemPositions.back().x && traceItemPosition.y == m_traceItemPositions.back().y && traceItemPosition.z == m_traceItemPositions.back().z) {
        // Item is not moving
    } else if(m_currentTraceItemAction->m_bVisible) {
        m_traceItemPositions.push_back(traceItemPosition);
    }

    for(int i = 0; i < m_traceItemPositions.size() - 1; i++) {
        if(m_traceItemPositions.empty()) {
            Logger::Debug("Rug was pulled D:");
            break;
        }

        p_Renderer->DrawQuad3D(
            Vector3Offset(m_traceItemPositions[i], -m_tracePathSize), m_tracePathColor,
            Vector3Offset(m_traceItemPositions[i], m_tracePathSize), m_tracePathColor,
            Vector3Offset(m_traceItemPositions[i + 1], m_tracePathSize), m_tracePathColor,
            Vector3Offset(m_traceItemPositions[i + 1], -m_tracePathSize), m_tracePathColor
            );
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

    if(const auto traceableItem = entity.QueryInterface<ZHM5Item>()) {
        if(traceableItem->m_rPhysicsAccessor.m_ref.QueryInterface<ZSpatialEntity>()) {
            if(!traceableItem->m_pItemConfigDescriptor->m_sTitle.ToStringView().contains("Charge")) {
                m_lastEntityCaptured = entity;
            }
        }
    }

    return { HookAction::Continue() };
}

DEFINE_PLUGIN_DETOUR(EntityPathTrace, void, OnLoadScene, ZEntitySceneContext* th, ZSceneData& p_sceneData) {
    m_isTracing = false;
    m_lastEntityCaptured = nullptr;
    m_currentTraceItem = nullptr;
    m_traceItemPositions.clear();
    Logger::Debug("OnLoadScene");
    return { HookAction::Continue() };
}

DEFINE_PLUGIN_DETOUR(EntityPathTrace, void, OnReloadScene, ZEntitySceneContext* th, bool forReload) {
    m_isTracing = false;
    m_currentTraceItem = nullptr;
    m_lastEntityCaptured = nullptr;
    m_traceItemPositions.clear();
    Logger::Debug("OnReloadScene");
    return { HookAction::Continue() };
}

DECLARE_ZHM_PLUGIN(EntityPathTrace);
