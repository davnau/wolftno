#include "tungsten/game/ai/ai2_core_support.h"

#define RECOVERED_METHOD(owner, method) \
  idRecoveredValue owner::method(...) { return {}; }

idAction_MoveToPoint::~idAction_MoveToPoint() = default;
idAIActionManager::~idAIActionManager() = default;
idAIDamageInfo::~idAIDamageInfo() = default;
RECOVERED_METHOD(idAIDamageInfo, GetAttacker)
RECOVERED_METHOD(idAIDamageInfo, GetInflictor)
idAIDeathInfo::idAIDeathInfo() = default;
idAIDeathInfo::~idAIDeathInfo() = default;

idAIEditable::idAIPerception::~idAIPerception() = default;
idAIEditable::idAISpawn::idAISpawn() = default;
idAIEditable::idAISpawn::~idAISpawn() = default;
idAIEditable::idInteractions::idInteractions() = default;
idAIEditable::idInteractions::~idInteractions() = default;
idAIEditable::idAIDeath::idAIDeath() = default;
idAIEditable::idAIDeath::~idAIDeath() = default;
idAIEditable::idAIActionSettings::idAICustomRobot::idAICustomRobot() = default;
idAIEditable::idAIActionSettings::idAIFireSettings::idAIFireSettings() = default;
idAIEditable::idAIActionSettings::~idAIActionSettings() = default;
RECOVERED_METHOD(idAIEditable::idAIActionSettings, Clear)
RECOVERED_METHOD(idAIEditable::idAIActionSettings, Init)
idAIEditable::idAIEditable() = default;
idAIEditable::~idAIEditable() = default;

RECOVERED_METHOD(idAIEvent, GetOriginator)
idAIFSMCallback::~idAIFSMCallback() = default;
RECOVERED_METHOD(idAIFSMCallback, OnError)
RECOVERED_METHOD(idAIFSMCallback, OnRestart)
RECOVERED_METHOD(idAIFSMCallback, OnTransition)
idAIMoveState::~idAIMoveState() = default;
RECOVERED_METHOD(idAIMoveState, GetNextPoint)
RECOVERED_METHOD(idAIScriptProxy, ForceFSMIntoExecutingScript)
RECOVERED_METHOD(idAIScriptProxy, InternalCallEvent)
RECOVERED_METHOD(idAIScriptProxy, PauseCurrentFiberTillActionCompletes)
RECOVERED_METHOD(idAIScriptProxy, PauseCurrentThreadTillActionCompletes)
RECOVERED_METHOD(idAIScriptProxy, PauseFiberTillActionCompletes)
RECOVERED_METHOD(idAIScriptProxy, PauseThreadTillActionCompletes)
RECOVERED_METHOD(idAnimatedEntity, SetModelByName)
idAnimator_Drone::~idAnimator_Drone() = default;
idAnimatorParms_AnimWeb::idAnimatorParms_AnimWeb() = default;
idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb() = default;
RECOVERED_METHOD(idCheckSurroundingsState, CanUse)
RECOVERED_METHOD(idCheckSurroundingsState, FindBestLookInterestPoint)
RECOVERED_METHOD(idCheckSurroundingsState, FindNetDirectionInterest)
RECOVERED_METHOD(idCheckSurroundingsState, GetLookedAtElapsedTime)
RECOVERED_METHOD(idCheckSurroundingsState, SetLastLookedAtTime)
RECOVERED_METHOD(idCheckSurroundingsState, TraceVector)
RECOVERED_METHOD(idCheckSurroundingsState, Update)
RECOVERED_METHOD(idCheckSurroundingsState, UpdateDirectionDistanceExponent)
RECOVERED_METHOD(idCheckSurroundingsState, UpdateLookInterests)
RECOVERED_METHOD(idFocusInfo, HasFocus)
idFSMCallback::~idFSMCallback() = default;
idFSMManager::idFSMStack::idFSMStack() = default;
idFSMManager::~idFSMManager() = default;
idLegsFSM::~idLegsFSM() = default;
std::intptr_t idPresentable::SetModelFade(...) { return 0; }
idScriptFunction::~idScriptFunction() = default;
idSpawnFocusWithRequirement::idSpawnFocusWithRequirement() = default;
RECOVERED_METHOD(idSpawnFocusWithRequirement, RequirementFilled)

std::intptr_t idBounds::Compare(...) const { return 0; }
std::intptr_t idStr::GetFileName(...) const { return 0; }
std::intptr_t idGameLocal::GetDebugEntity(...) { return 0; }
std::intptr_t idGameLocal::GetTensionManager(...) { return 0; }

template <int JointCount>
idAnimator_TurretNxN<JointCount>::~idAnimator_TurretNxN() = default;
template <int JointCount>
idRecoveredValue idAnimator_TurretNxN<JointCount>::SetParameters(...) { return {}; }
template class idAnimator_TurretNxN<3>;

template <typename T, int Count>
idArray<T, Count>::~idArray() = default;
template class idArray<idAnimWebAI, 1>;
template class idArray<idFocusInfo, 2>;
template class idArray<idTrajectory_Parabolic, 4>;

template <typename Key, typename Value>
idHashTableT<Key, Value>::~idHashTableT() = default;
template <typename Key, typename Value>
idRecoveredValue idHashTableT<Key, Value>::Clear(...) { return {}; }
template class idHashTableT<int, idAIAction*>;

template class idHealthT<aiHealthComponent_t, 2, 0>;

template class idShieldHealthT<aiHealthComponent_t, 2, 0, 1>;

template class idInterpolate<int>;

template class idRange<float>;

using idAIEventHandle = idHandle<int, invalidAIEvent_t, -1>;
template int idList<eboltControlNode_t, 5>::Append(const eboltControlNode_t&);
template bool idList<idCrosshairCustomIcon, 5>::Resize(int);
template void idList<idCrosshairCustomIcon, 5>::SetNum(int);
template int idList<idAIEventHandle, 5>::Append(const idAIEventHandle&);
template int idList<idAIEventHandle, 5>::Insert(const idAIEventHandle&, int);
template bool idList<idAIEventHandle, 5>::RemoveIndex(int);
template idList<idPonytail, 5>::~idList();
template bool idList<idVec3, 5>::RemoveIndexFast(int);

template <>
idEntitySpriteAnimator* idEntityPtr<idEntitySpriteAnimator>::GetEntity() const { return entity; }
template <>
idEntityPtr<idProxyMove>::operator idProxyMove*() const { return entity; }
template <>
idTarget_AlarmSystem* idEntityPtr<idTarget_AlarmSystem>::GetEntity() const { return entity; }
template <>
idEntityPtr<idTarget_FakeEnemy>::operator idTarget_FakeEnemy*() const { return entity; }
template <>
idVolume_CustomDeath* idEntityPtr<idVolume_CustomDeath>::operator->() const { return entity; }

const char* aiWalkModifierNames[7] = {
    "none", "walk", "run", "crouch", "sprint", "injured", "scripted"};

template <const char** Values, int Count>
idRecoveredValue idCmdSystem::ArgCompletion_String(...) {
  return {Count + (Values != nullptr ? 1 : 0)};
}
template idRecoveredValue idCmdSystem::ArgCompletion_String<aiWalkModifierNames, 7>(...);

#undef RECOVERED_METHOD
