#include "tungsten/game/ai/ai2_actions.h"
#include "tungsten/game/ai/aidamageinfo.h"
#include "tungsten/game/ai/animwebai.h"
#include "tungsten/game/ai/ai2_core_support.h"
#include "tungsten/game/ai/aivoicecontroller.h"
#include "tungsten/game/ai/aiorientation.h"
#include "tungsten/game/ai/aifirecontrol.h"
#include "tungsten/game/ai/aicamera.h"
#include "tungsten/game/ai/aaspositionmgr.h"
#include "tungsten/game/ai/aimemory.h"
#include "tungsten/game/ai/aimovestate.h"
#include "tungsten/game/ai/aidefs.h"
#include "tungsten/game/ai/gamestate/aientitystate.h"
#include "tungsten/game/ai/gamestate/aiworldstate.h"
#include "tungsten/game/ai/encountergroupmanager.h"
#include "tungsten/game/animation/facemgr.h"
#include "tungsten/game/clientgame/presentable/presentable.h"
#include "engine/animation/md6.h"
#include "engine/decls/animwebs/declanimweb.h"
#include "engine/framework/keyinput.h"
#include "engine/framework/coderesource.h"
#include "shared/idlib/lib.h"
#include "shared/idlib/math/mathlib.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <string_view>

namespace {

struct overrideWebInfo_t {
  aiSubWeb_t subWeb;
  const char* stateName;
  aiSubWeb_t destinationSubWeb;
  const char* destinationStateName;
};
static_assert(sizeof(overrideWebInfo_t) == 32,
              "override web-info ABI drift");

constexpr std::array<overrideWebInfo_t, 110> overrideWebInfo{{
    {static_cast<aiSubWeb_t>(24), "", static_cast<aiSubWeb_t>(0), ""},
    {aiSubWeb_t::react, "alarm", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::react, "first_spot", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::react, "first_detect_forward", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::react, "first_detect_left", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::react, "first_detect_right", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::react, "first_detect_backward", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::surprise, "forward", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::surprise, "backward", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::surprise, "left", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::surprise, "right", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::search, "forward", aiSubWeb_t::hostile, "idle"},
    {aiSubWeb_t::search, "left", aiSubWeb_t::hostile, "idle"},
    {aiSubWeb_t::search, "right", aiSubWeb_t::hostile, "idle"},
    {aiSubWeb_t::search, "above", aiSubWeb_t::hostile, "idle"},
    {aiSubWeb_t::search, "below", aiSubWeb_t::hostile, "idle"},
    {aiSubWeb_t::dive, "forward", aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::dive, "right", aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::dive, "left", aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::dive, "back", aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::dodge, "forward", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::dodge, "right", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::dodge, "left", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::dodge, "back", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::dodge, "flinch_left", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::dodge, "flinch_right", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::dodge, "cower", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::throwAttack, "throw", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::throwAttack, "throw_low", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::fullCover, "throw_left", aiSubWeb_t::fullCover, "idle"},
    {aiSubWeb_t::fullCover, "throw_right", aiSubWeb_t::fullCover, "idle"},
    {aiSubWeb_t::fullCover, "throw_over", aiSubWeb_t::fullCover,
     "idle_crouch"},
    {aiSubWeb_t::melee, "punch", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "kick", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "forward", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "back", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "left", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "right", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "crouch_forward", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "medium", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "long", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "moving_forward", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "takedown", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::melee, "leap_attack", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::throwAttack, "shoot", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::overrideAnimation, "blocked", aiSubWeb_t::max, "idle"},
    {aiSubWeb_t::overrideAnimation, "blocked_outsider", aiSubWeb_t::max,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "blocked_accepted", aiSubWeb_t::max,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "blocked_heroic", aiSubWeb_t::max,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "bumped", aiSubWeb_t::max, "idle"},
    {aiSubWeb_t::overrideAnimation, "investigate_body", aiSubWeb_t::max,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "angry_rage", aiSubWeb_t::combat,
     "idle"},
    {aiSubWeb_t::combat, "taunt", aiSubWeb_t::combat, "idle"},
    {aiSubWeb_t::overrideAnimation, "climb256jumpleft", aiSubWeb_t::combat,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "climbledge64", aiSubWeb_t::combat,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "climbledge128", aiSubWeb_t::combat,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "climbledge256", aiSubWeb_t::combat,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "climbledge512", aiSubWeb_t::combat,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "climbpipecage96", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "climbrail128", aiSubWeb_t::combat,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "climbrail256", aiSubWeb_t::combat,
     "idle"},
    {aiSubWeb_t::overrideAnimation, "crawldownpipe0", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "crawldownpipe64", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "crawllargepipe64", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "crawluppipe0", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "floorroundcoverup0", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "floorroundpanelup0", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "floorsquarepanelup0", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpforward128", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpforward256", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpforward384", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpledgedown64", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpledgedown128", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpledgedown192", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpledgedown256", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpledgeout256", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumprailup64then192", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumprailup256", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpraildown0", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpraildown64", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpraildown128", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpraildown128double",
     aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::overrideAnimation, "jumpraildown256", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumprailout128", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumprailout256", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpleftwallup256", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "pipedropdown192", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "pipedropdowncling192",
     aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::overrideAnimation, "pipedropout192", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "poleshimmyarms512", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "poleshimmyarms768", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "poleshimmyarms1024", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "poleshimmyquad512", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "poleshimmyquad768", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "poleshimmyquad1024", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "slidepipe0", aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::overrideAnimation, "slidepipe64", aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::overrideAnimation, "swoopraildown128", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "trackdrop240", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "wallentrance", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "rafterentrance320", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "swooprailentrance128",
     aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::overrideAnimation, "attackbarswing", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "attackbounceleft", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "attackbounceright", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "attackrailrunleft", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "attackwallrunright", aiSubWeb_t::combat,
     "run"},
    {aiSubWeb_t::overrideAnimation, "jumpcover", aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::overrideAnimation, "divecover", aiSubWeb_t::combat, "run"},
    {aiSubWeb_t::max, "", aiSubWeb_t::max, ""},
}};

idVec3 NormalizedOrZero(const idVec3& value) {
  const float lengthSquared = value.LengthSqr();
  if (lengthSquared <= 0.0F) return {};
  return value * (1.0F / std::sqrt(
      std::max(lengthSquared, std::numeric_limits<float>::min())));
}

void OrthoNormalizeFireAxis(idMat3& axis) {
  axis[0] = NormalizedOrZero(axis[0]);
  idVec3 up = axis[0].Cross(axis[1]);
  if (up.LengthSqr() <= std::numeric_limits<float>::min()) {
    axis = axis[0].ToMat3();
    return;
  }
  up.Normalize();
  axis[2] = up;
  axis[1] = axis[2].Cross(axis[0]);
  axis[1].Normalize();
}

idAngles QuaternionToAngles(const idQuat& rotation) {
  const float x2 = rotation.x + rotation.x;
  const float y2 = rotation.y + rotation.y;
  const float z2 = rotation.z + rotation.z;
  const float xx = rotation.x * x2;
  const float xy = rotation.x * y2;
  const float xz = rotation.x * z2;
  const float yy = rotation.y * y2;
  const float yz = rotation.y * z2;
  const float zz = rotation.z * z2;
  const float wx = rotation.w * x2;
  const float wy = rotation.w * y2;
  const float wz = rotation.w * z2;

  const float matrix00 = 1.0F - (yy + zz);
  const float matrix01 = xy - wz;
  const float matrix02 = xz + wy;
  const float matrix10 = xy + wz;
  const float matrix11 = 1.0F - (xx + zz);
  const float matrix12 = yz - wx;
  const float matrix22 = 1.0F - (xx + yy);

  const float sinePitch = std::clamp(matrix02, -1.0F, 1.0F);
  const float theta = -std::asin(sinePitch);
  const float cosinePitch = std::cos(theta);
  constexpr float radiansToDegrees = 57.29577951308232F;
  if (cosinePitch > 1.0e-6F) {
    return idAngles{theta * radiansToDegrees,
                    std::atan2(matrix01, matrix00) * radiansToDegrees,
                    std::atan2(matrix12, matrix22) * radiansToDegrees};
  }
  return idAngles{sinePitch > 0.0F ? -90.0F : 90.0F,
                  std::atan2(-matrix10, matrix11) * radiansToDegrees, 0.0F};
}

constexpr float kDegreesToRadians = 0.017453292F;

float CosineDegrees(const float degrees) {
  constexpr float fullCircle = 6.2831855F;
  constexpr float inverseFullCircle = 0.15915494F;
  float radians = degrees * kDegreesToRadians;
  if (radians < 0.0F || radians >= fullCircle) {
    radians -= std::floor(radians * inverseFullCircle) * fullCircle;
  }
  return std::cos(radians);
}

}  // namespace

idCodeResource<idDeclRenderParm> rp_aiRobotGlow("aiRobotGlow");
idCodeResource<idDeclRenderParm> rp_aiSelfDestructGlow(
    "aiSelfDestructGlow");
idCVar g_showTrajectories("g_showTrajectories", "0", 0,
                          "show deferred AI trajectory queries");
idCVar m_sensitivity("m_sensitivity", "1", 0, "mouse sensitivity");
idCVar in_invertLook("in_invertLook", "0", 0, "invert vertical look");
idCVar joy_pitchSpeed("joy_pitchSpeed", "1", 0,
                      "controller pitch speed");
idCVar joy_yawSpeed("joy_yawSpeed", "1", 0, "controller yaw speed");
extern idCVar ai_debugScenePoints;
extern idCVar takedownID;
extern idCVar ai_enableScrambles;
extern idCVar ai_scrambleTime;

const idEventDef EV_DisintigrateHide{};
const idEventDef EV_Hide{};
const idEventDef EV_Activate{};
const idEventDef EV_AnimWeb_Wait{};
const idEventDef EV_AnimWeb_BeginSyncedAnim{};
const idEventDef ACTION_CrouchToStandEnter{1, idEventDef::EVENT_FSMEVENT};
const idEventDef ACTION_StandToCrouchEnter{2, idEventDef::EVENT_FSMEVENT};
const idEventDef ACTION_PlayOverrideAnimEnter{3,
                                              idEventDef::EVENT_FSMEVENT};

idRecoveredValue idAI2::RecordCoreCall(const char* methodName) {
  const std::string_view call = methodName != nullptr ? methodName : "";
  coreState.lastCall = methodName != nullptr ? methodName : "";
  ++coreState.sequence;
  if (call == "SetSleeping") coreState.sleeping = true;
  if (call == "SetBlinded") coreState.blinded = true;
  if (call == "DeactivateExtendedClipModel") coreState.extendedClipModel = false;
  if (call == "ActivateExtendedClipModel") coreState.extendedClipModel = true;
  if (call == "SetAIClip") coreState.aiClip = true;
  if (call == "ClearAIClip") coreState.aiClip = false;
  if (call == "KillAI" || call == "Killed" || call == "Death") {
    animEventState.dead = true;
    animEventState.health = 0;
  }
  if (call == "GetModifiedSkill") return {coreState.modifiedSkill};
  if (call == "PerceptionFlagIsSet") return {coreState.perceptionFlags != 0};
  if (call == "TeslaIsActive") return {aiEventState.teslaActive};
  if (call == "GetEnableHeadTracking") return {animEventState.headTracking};
  if (call == "IsIncapacitated") return {animEventState.dead};
  return {coreState.sequence};
}

idAI2::idAI2()
    : localVoiceController(std::make_unique<idAIVoiceController>(VC_LOCAL)),
      playerControlCamera(std::make_unique<idAICamera_Control>()),
      worldState(std::make_unique<idAIWorldState>()),
      aasPositionManager(std::make_unique<idAASPositionMgr>()),
      animationInfo(
          std::make_unique<idAIVolatile::idAIAnimationInfo>()),
      healthComponent(std::make_unique<idAIHealth>()),
      damageInfo(std::make_unique<idAIDamageInfo>()),
      componentTimeLine(std::make_unique<idComponentActionScript>()),
      dynamicCoverManager(std::make_unique<idFormationCoverMgr>()),
      goreComponent(std::make_unique<idGoreComponent>()),
      faceManager(std::make_unique<idFaceMgr>()),
      bodyOrientation(std::make_unique<idAIOrientation>()),
      moveState(std::make_unique<idAIMoveState>()),
      actionFSM(std::make_unique<idLegsFSM>()),
      fsmManager(std::make_unique<idFSMManager>()),
      actionManager(std::make_unique<idAIActionManager>()),
      fsmCallback(std::make_unique<idAIFSMCallback>()),
      painAnimator(std::make_unique<idAnimator_Channel>()),
      memory(std::make_unique<idAIMemory>()),
      shootAnimator(std::make_unique<idAnimator_Channel>()) {
  robotDeathFXManager = std::make_unique<idFXManager>();
  robotDeathDroneAnimator = std::make_unique<idAnimator_Drone>();
  animWebAnimators[AIANIMWEB_BODY] = std::make_unique<idAnimator_AnimWeb>();
  animWebAnimators[AIANIMWEB_HEAD] = std::make_unique<idAnimator_AnimWeb>();
  focusInfos[0] = std::make_unique<idFocusInfo>();
  focusInfos[1] = std::make_unique<idFocusInfo>();
  focusInfos[0]->Init(nullptr, AIFOCI_AIM);
  focusInfos[1]->Init(nullptr, AIFOCI_LOOK);
}
idAI2::~idAI2() = default;

void idAI2::SetGoreAvailable(const char* goreName, bool available) {
  goreComponent->SetGoreAvailable(goreName, available);
}

void idAI2::SetAnimWebEventPendingForTest(aiAnimWeb_t web,
                                          animWebEvent_t eventType) {
  animWebAnimators[static_cast<std::size_t>(web)]->SetEvent(eventType);
}

bool idAI2::IsAnimWebEventPendingForTest(aiAnimWeb_t web,
                                         animWebEvent_t eventType) const {
  return animWebAnimators[static_cast<std::size_t>(web)]->IsEventPending(
      eventType);
}

idAnimator_AnimWeb* idAI2::GetAnimWebAnimatorForTest(aiAnimWeb_t web) {
  return animWebAnimators[static_cast<std::size_t>(web)].get();
}

const idAnimator_AnimWeb* idAI2::GetAnimWebAnimatorForTest(
    aiAnimWeb_t web) const {
  return animWebAnimators[static_cast<std::size_t>(web)].get();
}

void idAI2::SetAnimWebDeclarationForTest(const idDeclAnimWeb* value) {
  animWebState.declaration = value;
  for (const auto& animator : animWebAnimators) {
    animator->SetDeclarationForTest(value);
  }
}

void idAI2::SetAnimWebDeclarationForTest(
    const aiAnimWeb_t web, const idDeclAnimWeb* value) {
  animWebAnimators[static_cast<std::size_t>(web)]->SetDeclarationForTest(
      value);
}

void idAI2::ConfigureEmergencyCoverExitForTest(
    bool usesEmergencyExit, bool alreadyUsed, coverAction_t action,
    bool hostileStance) {
  usesEmergencyCoverExit = usesEmergencyExit;
  memory->SetEmergencyCoverExit(alreadyUsed);
  coreState.coverAction = action;
  coverOrientation = COVERORIENT_BACKWARD;
  allowHostileStance = hostileStance;
}

bool idAI2::HasUsedEmergencyCoverExitForTest() const {
  return memory->HasEmergencyCoverExit();
}

// DIA: idAnimatedEntity::SetModelByName
// DIA: idAIEditable::idAIPerception::~idAIPerception
// DIA: idAnimator_Drone::~idAnimator_Drone
// DIA: idArray<idFocusInfo,2>::~idArray<idFocusInfo,2>
// DIA: idAI2::idAIVolatile::idAIWorldStateInfo::~idAIWorldStateInfo
idAI2::idAIVolatile::idAIWorldStateInfo::~idAIWorldStateInfo() = default;
// DIA: idAI2::idAIVolatile::idPlayerAIControl::~idPlayerAIControl
idAI2::idAIVolatile::idPlayerAIControl::~idPlayerAIControl() = default;
// DIA: idAI2::idAIVolatile::idAIDebugging::~idAIDebugging
idAI2::idAIVolatile::idAIDebugging::~idAIDebugging() = default;
// DIA: idArray<idTrajectory_Parabolic,4>::~idArray<idTrajectory_Parabolic,4>
// DIA: idArray<idAnimWebAI,1>::~idArray<idAnimWebAI,1>
// DIA: idAnimator_TurretNxN<3>::~idAnimator_TurretNxN<3>
// DIA: idAI2::ApplyDamage
idRecoveredValue idAI2::ApplyDamage(...) { return RecordCoreCall("ApplyDamage"); }
// DIA: idAI2::GetModifiedSkill
int idAI2::GetModifiedSkill() { return coreState.modifiedSkill; }
// DIA: idAI2::InitFireControl
void idAI2::InitFireControl() {
  fireControl = std::make_unique<idAIFireControl_Human>();
}
// DIA: idAI2::PerceptionFlagIsSet
bool idAI2::PerceptionFlagIsSet(int flags) {
  return (flags & coreState.perceptionFlags) != 0;
}
// DIA: idAI2::SaveFSM
void idAI2::SaveFSM(idFile_String* file, const char* fsmName,
                    const char* indent) {
  const idFiniteStateMachine* fsm = fsmManager->FindFSM(fsmName);
  if (fsm != nullptr) {
    fsm->Save(file, indent);
  } else {
    idLib::Printf("No FSM named '%s' was found.\n", fsmName);
  }
}
// DIA: idScriptFunction::~idScriptFunction
// DIA: idAI2::GetEventResponseFlags
unsigned int idAI2::GetEventResponseFlags() {
  return ~static_cast<unsigned int>(aiEventState.overloadDisableFlags) &
         static_cast<unsigned int>(coreState.aiEventResponseFlags |
                                   aiEventState.overloadEnableFlags);
}
// DIA: idAI2::RespondsToAIEvent
bool idAI2::RespondsToAIEvent(const idAIEvent* event) const {
  const unsigned int responseFlags =
      ~static_cast<unsigned int>(aiEventState.overloadDisableFlags) &
      static_cast<unsigned int>(coreState.aiEventResponseFlags |
                                aiEventState.overloadEnableFlags);
  return (event->GetEventClass() & responseFlags) != 0;
}
// DIA: idBounds::Compare
// DIA: idAI2::DeactivateExtendedClipModel
void idAI2::DeactivateExtendedClipModel() {
  extendedClipModelStatus = 0;
  extendedClipModelAuxiliaryStatus = 0;
  if (extendedClipModelActive) extendedClipModelActive = false;
  coreState.extendedClipModel = false;
}
// DIA: idStr::GetFileName
// DIA: idAI2::SetMovePushStatus
void idAI2::SetMovePushStatus(aiMovePushStatus_t newStatus,
                              std::int64_t frameCount) {
  movePushStatus = newStatus;
  movePushEndTime = newStatus != AIMOVEPUSHSTATUS_PUSH_NONE
                        ? GetEndTimeByFrames(static_cast<int>(frameCount))
                        : 0;

  idPhysics* physicsObject = GetPhysics();
  int currentClipMask = physicsObject->GetClipMask(-1);
  int configuredClipMask = defaultPhysicsClipMask;
  if (newStatus == AIMOVEPUSHSTATUS_PUSH_PLAYERS ||
      newStatus == AIMOVEPUSHSTATUS_PUSH_PLAYERS_AND_NPCS) {
    currentClipMask &= ~0x8000;
    configuredClipMask &= ~0x8000;
  } else {
    currentClipMask |= 0x8000;
    configuredClipMask |= 0x8000;
  }
  physicsObject->SetClipMask(currentClipMask, 0);
  defaultPhysicsClipMask = configuredClipMask;
}
// DIA: idAI2::SetWorldCollision
void idAI2::SetWorldCollision(bool collideWithWorld) {
  idPhysics* physicsObject = GetPhysics();
  const int numberOfClipModels = physicsObject->GetNumClipModels();
  if (numberOfClipModels == 1) {
    physicsObject->SetClipMask(collideWithWorld ? defaultPhysicsClipMask : 0, 0);
  } else if (numberOfClipModels == 2) {
    physicsObject->SetClipMask(collideWithWorld ? defaultPhysicsClipMask : 0, 0);
    physicsObject->SetClipMask(0, 1);
  }
}
// DIA: idAI2::GetWorldCollisionBounds
const idBounds* idAI2::GetWorldCollisionBounds() {
  return &GetPhysics()->GetAbsBounds();
}
// DIA: idAI2::GetEnemyLookDot
float idAI2::GetEnemyLookDot(const idEntity* enemy) const {
  if (enemy == nullptr) return 1.0F;
  const auto* enemyActor = dynamic_cast<const idActor*>(enemy);
  if (enemyActor == nullptr) return 1.0F;

  idVec3 ourEyePosition;
  idVec3 enemyEyePosition;
  idMat3 enemyEyeAxis;
  GetLastEyePos(&ourEyePosition);
  enemyActor->GetLastEyePos(&enemyEyePosition);
  enemyActor->GetEyeAxis(&enemyEyeAxis);

  const idVec3 toUs = ourEyePosition - enemyEyePosition;
  const float lengthSquared = toUs.x * toUs.x + toUs.y * toUs.y +
                              toUs.z * toUs.z;
  const float inverseLength =
      1.0F / std::sqrt(std::max(lengthSquared,
                                std::numeric_limits<float>::min()));
  return enemyEyeAxis[0].x * (toUs.x * inverseLength) +
         enemyEyeAxis[0].y * (toUs.y * inverseLength) +
         enemyEyeAxis[0].z * (toUs.z * inverseLength);
}
// DIA: idAI2::CalcTurnRadius
float idAI2::CalcTurnRadius(float speed, float turnRate) {
  return ((360.0F / turnRate) * speed) / (3.1415927F * 2.0F);
}
// DIA: OkToFocus
bool OkToFocus(const idVec3* to, const idVec3* from,
               const idVec3* actualPosition) {
  const idVec3 intendedDelta = *to - *from;
  const float intendedLengthSquared = intendedDelta.LengthSqr();
  const float intendedInverseLength =
      1.0F / std::sqrt(std::max(intendedLengthSquared,
                                std::numeric_limits<float>::min()));
  if (intendedInverseLength * intendedLengthSquared <= 16.0F) {
    return false;
  }

  const idVec3 actualDelta = *actualPosition - *from;
  const float actualInverseLength =
      1.0F / std::sqrt(std::max(actualDelta.LengthSqr(),
                                std::numeric_limits<float>::min()));
  return ((actualDelta * actualInverseLength) *
          (intendedDelta * intendedInverseLength)) >
         -0.00000011920929F;
}
// DIA: idAI2::GetFacialAnimationSet
const idDeclFacialAnimationSet* idAI2::GetFacialAnimationSet() const {
  return currentFacialAnimationSet != nullptr ? currentFacialAnimationSet
                                               : defaultFacialAnimationSet;
}
// DIA: idAI2::Draw
bool idAI2::Draw(idPlayer* player) {
  playerControlCamera->Draw(player);
  return true;
}
// DIA: idAI2::AlertCycleToSubWeb
int idAI2::AlertCycleToSubWeb(int alertCycle) {
  static const int alertCycleToSubWebMap[11] = {
      24, 0, 3, 1, 4, 8, 8, 24, 24, 24, 24};
  return alertCycle < 11 ? alertCycleToSubWebMap[alertCycle] : 0;
}
// DIA: idAI2::GetAlertCycleFSM
idFiniteStateMachine* idAI2::GetAlertCycleFSM() {
  return fsmManager->FindAlertCycleFSM();
}
// DIA: idAI2::EnableAIEventResponse
void idAI2::EnableAIEventResponse(int flags) {
  coreState.aiEventResponseFlags |= flags;
}
// DIA: idAI2::DisableAIEventResponse
void idAI2::DisableAIEventResponse(int flags) {
  coreState.aiEventResponseFlags &= ~flags;
}
// DIA: idAI2::GetEnableHeadTracking
bool idAI2::GetEnableHeadTracking() { return animEventState.headTracking; }
// DIA: idAI2::SetCoverAction
void idAI2::SetCoverAction(coverAction_t action, int currentTime) {
  coreState.coverAction = action;
  coreState.coverActionTimestamp = currentTime;
}
// DIA: idAI2::GetCoverActionDuration
int idAI2::GetCoverActionDuration(coverAction_t action, int currentTime) {
  return coreState.coverAction == action
             ? currentTime - coreState.coverActionTimestamp
             : 0;
}
// DIA: idAI2::GetBodyOrientation
idAIOrientation* idAI2::GetBodyOrientation() { return bodyOrientation.get(); }
// DIA: idAI2::SetBodyOrientation
void idAI2::SetBodyOrientation(const idMat3& axis) {
  bodyOrientation->Init(axis);
}
// DIA: idAI2::CheckForGoreByName
bool idAI2::CheckForGoreByName(const idAtomicString* goreName) {
  return goreComponent->CheckForGoreByName(goreName);
}
// DIA: idAI2::PlayHurtSound
void idAI2::PlayHurtSound() {
  if (!damageChannelPlaying && hurtSound != nullptr) {
    lastDamageSound = hurtSound;
    damageChannelPlaying = true;
    ++damageSoundStartCount;
  }
}
// DIA: idAI2::GetRenderParam
idCodeResource<idDeclRenderParm>* idAI2::GetRenderParam(
    aiRenderParam_t parameter) {
  if (parameter == AIRENDERPARAM_ROBOTGLOW) {
    return &rp_aiRobotGlow;
  }
  if (parameter == AIRENDERPARAM_SELFDESTRUCTGLOW) {
    return &rp_aiSelfDestructGlow;
  }
  return nullptr;
}
// DIA: idRange<float>::GetRatio
template <>
float idRange<float>::GetRatio(float current) const {
  const float range = maxRange - minRange;
  const float clampedCurrent = std::max(0.0F, current - minRange);
  return range != 0.0F ? std::min(1.0F, clampedCurrent / range) : 0.0F;
}
// DIA: idCmdSystem::ArgCompletion_String<&aiWalkModifierNames,7>
// DIA: idList<idHandle<int,enum invalidAIEvent_t,-1>,5>::RemoveIndex
// DIA: idList<idVec3,5>::RemoveIndexFast
// DIA: idFXManager::SetExternalTransform
void idFXManager::SetExternalTransform(const idVec3& position,
                                       const idMat3& rotation) {
  externalPosition = position;
  externalRotation = rotation;
  hasExternalPositionAndRotation = true;
}

// DIA: idBaseHealth::SetCurBaseHealth
void idBaseHealth::SetCurBaseHealth(float health) {
  if (health > 0.0F) {
    ClearDeadFlag();
    SetCurBaseHealth_Impl(health);
  }
}
// DIA: idBaseHealth::DamageShield_Impl
bool idBaseHealth::DamageShield_Impl(float damage, bool allowDeath,
                                     bool overflow, float* remainingDamage,
                                     bool ignoreArmor) {
  (void)overflow;
  return Damage_Impl(damage, allowDeath, remainingDamage, ignoreArmor);
}
// DIA: idCoverActions::Clear
void idCoverActions::Clear() { actionMask &= 0xE0u; }
// DIA: idFSMCallback::~idFSMCallback
// DIA: idFSMCallback::`scalar deleting destructor'
// DIA: idFSMManager::~idFSMManager
// DIA: idFSMManager::`scalar deleting destructor'
// DIA: idAnimatedEntity::GetTreeAnimatorFromPresentable
idTreeAnimator* idAnimatedEntity::GetTreeAnimatorFromPresentable() const {
  if (presentable == nullptr ||
      presentable->GetAnimatedEntityInterface() == nullptr) {
    return nullptr;
  }
  idPresentable* animatedPresentable =
      presentable != nullptr ? presentable->GetAnimatedEntityInterface()
                             : nullptr;
  return static_cast<idTreeAnimator*>(
      animatedPresentable->GetAnimatedTreeAnimator());
}
// DIA: idDamageGroup::HealthRatio
float idDamageGroup::HealthRatio() const {
  return maxHealth > 0.0F ? health / maxHealth : 0.0F;
}
// DIA: idActor::GetWalkState
walkState_t idActor::GetWalkState() const { return walkState; }
// DIA: idActor::SetWalkState
void idActor::SetWalkState(walkState_t state) { walkState = state; }
// DIA: idPlayer::GetUCmdTracker1
idRecoveredValue idPlayer::GetUCmdTracker1(...) { return {}; }
// DIA: idPlayer::GetIsSprinting
idRecoveredValue idPlayer::GetIsSprinting(...) { return {}; }
// DIA: idPlayer::GetFocusTracker
idRecoveredValue idPlayer::GetFocusTracker(...) { return {}; }
// DIA: idPlayer::IsZoomedIn
idRecoveredValue idPlayer::IsZoomedIn(...) { return {}; }
// DIA: idAIFSMCallback::~idAIFSMCallback
// DIA: `idAIFSMCallback::~idAIFSMCallback'::`1'::dtor$0
// DIA: idAIFSMCallback::`scalar deleting destructor'
// DIA: idAI2::UsesTurrets
bool idAI2::UsesTurrets() const {
  return turretAnimationSupported && turretActionSetting;
}
// DIA: idAI2::SetAIClip
void idAI2::SetAIClip() {
  aiClipMask |= 0x400;
  coreState.aiClip = true;
}
// DIA: idAI2::ClearAIClip
void idAI2::ClearAIClip() {
  aiClipMask &= ~0x400;
  coreState.aiClip = false;
}
// DIA: idAction_MoveToPoint::~idAction_MoveToPoint
// DIA: idAI2::TeslaIsActive
bool idAI2::TeslaIsActive() const {
  return teslaDischargeTime >= 0 || teslaExplodeTime >= 0;
}
// DIA: idAI2::IsTeslaDangerous
bool idAI2::IsTeslaDangerous(idVec3* position, float* radius) const {
  if (teslaDischargeTime >= 0 || teslaExplodeTime >= 0) {
    if (teslaHeavyGuardGun) {
      if (teslaExplodeTime >= 0 && teslaEvaluationTime < teslaExplodeTime &&
          teslaDamageDecl != nullptr) {
        *position = teslaExplodePosition;
        *radius = teslaDamageDecl->radius;
        return true;
      }
    } else if (teslaDischargeTime >= 0 &&
               teslaEvaluationTime < teslaDischargeTime) {
      *position = GetPhysics()->GetOrigin();
      *radius = teslaRange;
      return true;
    }
  }
  return false;
}
// DIA: idVec3::DistanceFast
float idVec3::DistanceFast(const idVec3& other) const {
  const float deltaX = other.x - x;
  const float deltaY = other.y - y;
  const float deltaZ = other.z - z;
  const float squaredDistance = deltaX * deltaX + deltaY * deltaY +
                                deltaZ * deltaZ;
  const float reciprocalLength =
      1.0F /
      std::sqrt(std::max(squaredDistance, std::numeric_limits<float>::min()));
  return reciprocalLength * squaredDistance;
}
// DIA: idAI2::SetBlinded
bool idAI2::SetBlinded(bool state, bool forceForever,
                       int blindedOverrideTimeMilliseconds) {
  if (!usesBlindness || volumeDying || animEventState.dead) {
    return false;
  }
  if (!state) {
    if (endBlindedTime == 0) {
      return false;
    }
    const float cooldown = reblindCooldown;
    endBlindedTime = 0;
    coreState.blinded = false;
    if (cooldown != -1.0F) {
      const float scaledCooldown = incomingDamageScale * cooldown;
      endReblindCooldown =
          aiEvaluationTime - static_cast<int>(scaledCooldown * -1000.0F) - 1;
      return true;
    }
    endReblindCooldown = -1;
    return true;
  }
  if (endBlindedTime != 0 || endStunnedTime != 0) {
    return false;
  }
  if (blindedDuration == -1.0F || forceForever) {
    endBlindedTime = -1;
    endReblindCooldown = -1;
    coreState.blinded = true;
    return true;
  }
  if (blindedOverrideTimeMilliseconds == -1) {
    blindedOverrideTimeMilliseconds =
        static_cast<int>(blindedDuration * 1000.0F);
  }
  endReblindCooldown = -1;
  endBlindedTime = blindedOverrideTimeMilliseconds + aiEvaluationTime;
  coreState.blinded = true;
  return true;
}
// DIA: idAI2::OkToLKWBlind
bool idAI2::OkToLKWBlind() const {
  return !animEventState.dead && endBlindedTime == 0 && painAnimationDone;
}
// DIA: idAI2::IsIncapacitated
bool idAI2::IsIncapacitated() const { return endStunnedTime != 0; }
// DIA: idAI2::SetSleeping
void idAI2::SetSleeping(bool sleep) {
  if (!usesSleeping) {
    return;
  }
  if (sleep) {
    if (!coreState.sleeping) {
      coreState.sleeping = true;
    }
  } else if (coreState.sleeping && !volumeDying && !animEventState.dead) {
    coreState.sleeping = false;
  }
}
// DIA: idAI2::ApplyWaterEffects
idRecoveredValue idAI2::ApplyWaterEffects(...) { return RecordCoreCall("ApplyWaterEffects"); }
// DIA: idAI2::InitEncounterGroup
void idAI2::InitEncounterGroup() {
  if (encounterGroupHandle != -1) return;

  const char* selectedGroupName = encounterGroupName.IsEmpty()
                                      ? GetName()
                                      : encounterGroupName.data;
  idEncounterGroupManager* manager =
      gameLocal->GetEncounterGroupManager();
  const encounterGroupHandle_t handle = manager->AddGroup(selectedGroupName);
  encounterGroupHandle = handle.value;
  if (encounterGroupHandle != -1) {
    idEncounterGroup* group = manager->GroupForHandle(handle);
    if (group != nullptr) {
      group->Add(this);
    }
  }
}
// DIA: idAI2::OnActivate
void idAI2::OnActivate(idEntity* activator) {
  (void)activator;
  if (!robotState.usesRobot) return;
  const int currentTime = gameLocal->GetGameTimeMs();
  if (robotState.usesRobot && robotState.isCharging &&
      robotState.chargeDoneTime == -1) {
    robotState.chargeStartTime = currentTime;
    robotState.chargeDoneTime =
        currentTime - static_cast<int>(robotState.chargeTime * -1000.0F);
  }
  if (!IsDead()) (void)IsDying();
}
// DIA: idAI2::Dropped
void idAI2::Dropped(idEntity* user,
                    const idDeclInventory* inventoryDeclaration) {
  inventoryDecl = inventoryDeclaration;
  const idSpawnId userId{
      user != nullptr ? user->GetEntityNumber() : idSpawnId{}.value};
  idAILogic logic(this);
  worldState->AddEntityState(gameLocal->GetAIGameState(), &logic, userId,
                             static_cast<aiStimulus_t>(7), 1.0F, userId);
}
// DIA: idAI2::IsModesAllowed
bool idAI2::IsModesAllowed(aiModeRestricFlag_t flags) const {
  return (!stationaryMode || (flags & 4) == 0) &&
         (flags & movementModeRestrictionFlags) == 0;
}
// DIA: idAI2::IsModesRestricted
bool idAI2::IsModesRestricted(aiModeRestricFlag_t flags) const {
  return (stationaryMode && (flags & 4) != 0) ||
         (flags & movementModeRestrictionFlags) != 0;
}
// DIA: idAI2::SetWalkState
void idAI2::SetWalkState(walkState_t state) {
  walkState_t clampedState = state;
  if (state == WALKSTATE_RUNNING &&
      (movementModeRestrictionFlags & 8) != 0) {
    clampedState = WALKSTATE_WALKING;
  }
  if (state <= maximumWalkState) {
    if (state < minimumWalkState) {
      clampedState = minimumWalkState;
    }
    idActor::SetWalkState(clampedState);
  } else {
    idActor::SetWalkState(maximumWalkState);
  }
}
// DIA: idAI2::InitScriptObjects
void idAI2::InitScriptObjects() {
  if (!hasScriptObject) return;
  if (scriptState.threads.Num() != 0) {
    primaryScriptThread = idThread::Find(scriptState.threads[0].value);
    if (primaryScriptThread != nullptr) {
      primaryScriptThread->ManualDelete();
    }
  } else {
    idLib::Warning(
        "AI %s has a script object but no thread. Make sure the script object "
        "has a constructor (init) defined.",
        GetName());
  }
}
// DIA: idAI2::SendGrappleActivationTest
void idAI2::SendGrappleActivationTest(idEntity* target) {
  if (target == nullptr) return;

  idVec3 start = GetPhysics()->GetOrigin();
  start.z += 32.0F;
  idVec3 end = target->GetPhysics()->GetOrigin();
  end.z += 32.0F;
  idMat3 identity;
  identity[0] = {1.0F, 0.0F, 0.0F};
  identity[1] = {0.0F, 1.0F, 0.0F};
  identity[2] = {0.0F, 0.0F, 1.0F};
  grappleQueryIndex = gameLocal->SubmitTranslationQuery(
      start, end, 32, identity, 106521, GetEntityNumber(), true,
      "w:\\build\\tech5\\tungsten\\game\\ai\\AI2.cpp(3008) : GrappleTest");
  meleeCountBetweenGrappleChecks = 0;
  lastGrappleCheck = gameLocal->GetGameTimeMs();
}
// DIA: idAI2::IsWeaponEquipped
bool idAI2::IsWeaponEquipped() const {
  return GetEquippedWeapon(2) != nullptr || GetEquippedWeapon(1) != nullptr;
}
// DIA: idAI2::TurretFire
void idAI2::TurretFire() { lastFireTime = aiEvaluationTime; }
// DIA: idAI2::FindWeaponToDraw
const idWeapon* idAI2::FindWeaponToDraw() const {
  for (int slot = 14; slot >= 0; --slot) {
    const idWeapon* weapon = GetEquippedWeapon(static_cast<unsigned int>(slot));
    if (weapon != nullptr &&
        weapon->GetHolsterSlot() == static_cast<unsigned int>(slot)) {
      return weapon;
    }
  }
  return nullptr;
}
// DIA: idAI2::GetEffectiveAccuracy
int idAI2::GetEffectiveAccuracy() const {
  if ((groupStatusCondition & 0x2000) != 0) {
    return 6;
  }
  return editableAimAccuracy != 0 ? editableAimAccuracy : skillAimAccuracy;
}
// DIA: idAI2::InitHeadTracking
idRecoveredValue idAI2::InitHeadTracking(...) { return RecordCoreCall("InitHeadTracking"); }
// DIA: idAI2::SetModel
idRecoveredValue idAI2::SetModel(...) { return RecordCoreCall("SetModel"); }
// DIA: idAI2::InitAAS
idRecoveredValue idAI2::InitAAS(...) { return RecordCoreCall("InitAAS"); }
// DIA: idAI2::InitMovement
void idAI2::InitMovement() {
  moveState->Init(this);
  moveState->Stop(MOVE_STATUS_DONE);
  moveState->SetVelocityType(AIVELOCITY_ANIMSPEED);
  moveState->SetMovementModeForReal(
      static_cast<aiMovementMode_t>(defaultMovementMode));
  moveState->SetAccelRate(initialAccelRate);
  moveState->SetMaxMoveSpeed(initialMaxMoveSpeed);
  moveState->SetDecelRate(initialDecelRate);
}
// DIA: idAI2::SetPerceptionFlags
void idAI2::SetPerceptionFlags(int flags, bool setFlag) {
  if (setFlag) {
    coreState.perceptionFlags |= static_cast<std::uint64_t>(flags);
    if (fireControl != nullptr) {
      fireControl->SetFireMode(this, FIREMODE_HOLDFIRE);
    }
  } else {
    coreState.perceptionFlags &= ~static_cast<std::uint64_t>(flags);
  }
}
// DIA: idAI2::JobSync
void idAI2::JobSync() {
  idActor::JobSync();
  ++animatedEntityJobSyncCount;
  for (idTrajectory_Parabolic& trajectory : trajectories) {
    trajectory.UpdateDeferredQueries(g_showTrajectories.valueInteger != 0);
  }
}
// DIA: idAI2::ApplyImpulse
void idAI2::ApplyImpulse(int physicsId, int bodyId, const idVec3* point,
                         const idVec3* impulse) {
  (void)physicsId;
  if (!animEventState.dead ||
      (articulatedFigureLoaded &&
       (articulatedFigureActive || articulatedFigureClientActive))) {
    GetPhysics()->ApplyImpulse(bodyId, *point, *impulse);
  }
}
// DIA: idAI2::ApplyForce
void idAI2::ApplyForce(int physicsId, int bodyId, const idVec3* point,
                       const idVec3* force) {
  (void)physicsId;
  if (!animEventState.dead ||
      (articulatedFigureLoaded &&
       (articulatedFigureActive || articulatedFigureClientActive))) {
    GetPhysics()->ApplyForce(bodyId, *point, *force);
  }
}
// DIA: idAI2::GetModelTransform
void idAI2::GetModelTransform(idVec3* modelOrigin,
                              idMat3* modelAxis) const {
  if (articulatedFigureActive || articulatedFigureClientActive) {
    *modelOrigin = articulatedFigureOrigin;
    *modelAxis = articulatedFigureAxis;
    return;
  }
  *modelAxis = bodyOrientation->GetAxis() * GetPhysics()->GetAxis();
  *modelOrigin = GetPhysics()->GetOrigin() + (*modelAxis * modelOffset);
}
// DIA: idAI2::GetSoundTransform
void idAI2::GetSoundTransform(idVec3* modelOrigin, idMat3* modelAxis) const {
  idEntity::GetSoundTransform(modelOrigin, modelAxis);
  if (usesDrone) {
    *modelOrigin = dronePresentOrigin;
  }
}
// DIA: idAI2::ShouldLeaveDormancy
bool idAI2::ShouldLeaveDormancy() const {
  return ai_enableDormancy.GetInteger() == 0 ||
         idEntity::ShouldLeaveDormancy();
}
// DIA: idAI2::InternalCallEvent
idEventArg* idAI2::InternalCallEvent(idEventArg* result,
                                     const idEventDef* event,
                                     idEventArg* arguments) {
  if (event->type == idEventDef::EVENT_FSMEVENT) {
    return GetAlertCycleFSM()->ProcessEventArgPtr(result, event, arguments);
  }
  idLib::Warning(
      "idAI2::InternalCallEvent() doesn't handle event type %d.",
      static_cast<int>(event->type));
  result->type = 0;
  result->value.s = nullptr;
  return result;
}
// DIA: idAI2::InternalRespondsTo
bool idAI2::InternalRespondsTo(const idEventDef* event) {
  if (event->type != idEventDef::EVENT_FSMEVENT) return false;
  return GetAlertCycleFSM()->RespondsTo(event);
}
// DIA: idAI2::CheckForOneShotKill
bool idAI2::CheckForOneShotKill(idEntity* attacker) {
  if ((dropState != 1 && dropState != 2) || GetEquipped(4) == nullptr ||
      ignoreDamageWhileDropping) {
    return false;
  }
  if (dynamic_cast<idPlayer*>(attacker) != nullptr) return true;
  const std::uint32_t seed =
      1664525U * gameLocal->GetRandomSeed() + 1013904223U;
  gameLocal->SetRandomSeed(seed);
  const float randomValue =
      static_cast<float>((seed >> 10U) & 0x7FFFU) * 0.000030518509F;
  return nonPlayerOneShotChance > randomValue;
}
// DIA: idAI2::GetJointIndexFromTrace
idJointIndex idAI2::GetJointIndexFromTrace(const trace_t* trace) const {
  idJointIndex result;
  result.value = trace->contact.type == contactInfo_t::CONTACT_SPHERE
                     ? static_cast<short>(trace->contact.trmFeature)
                     : static_cast<short>(-1);
  return result;
}
// DIA: idAI2::ApplyFakeDamageImpulse
void idAI2::ApplyFakeDamageImpulse(const idVec3* impulse) {
  const idVec3 point = GetPhysics()->GetOrigin();
  damageInfo->Update(this, nullptr, nullptr, nullptr, 0.0F, 0.0F, 0.0F,
                     0.0F, point, *impulse, idJointIndex{-1}, false, false,
                     false, false);
}
// DIA: idAI2::PlayAdditivePainAnim
bool idAI2::PlayAdditivePainAnim(const aiAnimationHandle_t* painAlias) {
  if (painAlias->value == 0xFFFFU) return false;
  return painAnimator->PlayAnim(painAlias->value, gameLocal->GetGameTimeMs(),
                                1.0F, 5, 5, 1.0F);
}
// DIA: idAI2::PlayAdditiveFireAnim
void idAI2::PlayAdditiveFireAnim() {
  const idList<unsigned short, 5>* animations = nullptr;
  if (IsCrouching() && additiveCrouchFireAnimations.Num() > 0) {
    animations = &additiveCrouchFireAnimations;
  } else {
    animations = &additiveFireAnimations;
  }

  const int animationCount = animations->Num();
  if (animationCount > 0) {
    const int animationIndex = RandomInt(animationCount);
    const unsigned short aliasHandle = (*animations)[animationIndex];
    if (aliasHandle != 0xFFFFU) {
      const int blendOutDurationMs =
          static_cast<int>(additiveFireAnimationBlendTime * 1000.0F);
      const int blendDurationMs = (std::max)(blendOutDurationMs, 100);
      const float rateScale = RandomFloat(0.0F, 0.75F) + 0.25F;
      shootAnimator->PlayAnim(aliasHandle, gameLocal->GetGameTimeMs(),
                              rateScale, blendDurationMs,
                              blendOutDurationMs, 1.0F);
    }
  }
}
// DIA: idAI2::PlayAdditiveAlarmAnimation
void idAI2::PlayAdditiveAlarmAnimation() {
  if (additiveAlarmAnimation == 0xFFFFU) return;
  painAnimator->PlayAnim(additiveAlarmAnimation, gameLocal->GetGameTimeMs(),
                         1.0F, 46, 0, 1.0F);
}
// DIA: idAI2::CheckPain
idRecoveredValue idAI2::CheckPain(...) { return RecordCoreCall("CheckPain"); }
// DIA: idAI2::DeathPending
bool idAI2::DeathPending() const {
  if (deathImpulseIndex >= 0) {
    return true;
  }
  return animEventState.dead;
}
// DIA: idAI2::KillAI
void idAI2::KillAI() {
  if (!animEventState.dead) {
    animEventState.health = 0;
    animEventState.dead = true;
  }
  walkIKEnabled = false;
}
// DIA: idAI2::StartBodyDeathFade
void idAI2::StartBodyDeathFade() {
  const int duration = static_cast<int>(bodyFadeTime * 1000.0F);
  if (fadeOutTimestamp == -1) {
    fadeOutDuration = duration;
    fadeOutTimestamp = gameLocal->GetGameTimeMs();
  }
}
// DIA: idAI2::StartDisintigrateDeathFade
void idAI2::StartDisintigrateDeathFade() {
  constexpr float sFadeTime = 1.75F;
  const int duration = static_cast<int>(sFadeTime * 1000.0F);
  if (fadeOutTimestamp == -1) {
    fadeOutDuration = duration;
    fadeOutTimestamp = gameLocal->GetGameTimeMs();
  }
  PostEventMS(&EV_DisintigrateHide, duration);
  PostEventMS(&EV_Hide, 10000);
}
// DIA: idAI2::DisableAnimatorsOnDeath
void idAI2::DisableAnimatorsOnDeath() {
  faceManager->SetEnableAnimation(this, false);
  faceManager->SetEnableEyeTracking(this, false);
  deathAnimatorsEnabled = false;
  walkIKEnabled = false;
}
// DIA: idAI2::DoMeleeDamage
idRecoveredValue idAI2::DoMeleeDamage(...) { return RecordCoreCall("DoMeleeDamage"); }
// DIA: idAI2::DoBlockedMeleeReaction
void idAI2::DoBlockedMeleeReaction() {
  actionFSM->ProcessMeleeBlocked(this);
  stunnedByPlayerBlock = true;
}
// DIA: idAI2::GetMeleeInterval
int idAI2::GetMeleeInterval(const aiMelee_t meleeType) {
  int minimumInterval;
  int maximumInterval;
  if (meleeType == AIMELEE_MEDIUM) {
    minimumInterval = minimumMediumMeleeInterval;
    if (minimumInterval == -1) {
      minimumInterval = behaviorDeclaration->minMediumMeleeInterval;
    }
    maximumInterval = maximumMediumMeleeInterval;
    if (maximumInterval == -1) {
      maximumInterval = behaviorDeclaration->maxMediumMeleeInterval;
    }
  } else if (meleeType == AIMELEE_LONG) {
    minimumInterval = minimumLongMeleeInterval;
    if (minimumInterval == -1) {
      minimumInterval = behaviorDeclaration->minLongMeleeInterval;
    }
    maximumInterval = maximumLongMeleeInterval;
    if (maximumInterval == -1) {
      maximumInterval = behaviorDeclaration->maxLongMeleeInterval;
    }
  } else {
    minimumInterval = minimumCloseMeleeInterval;
    if (minimumInterval == -1) {
      minimumInterval = behaviorDeclaration->minCloseMeleeInterval;
    }
    maximumInterval = maximumCloseMeleeInterval;
    if (maximumInterval == -1) {
      maximumInterval = behaviorDeclaration->maxCloseMeleeInterval;
    }
  }

  const int intervalRange = maximumInterval - minimumInterval;
  if (intervalRange == 0) return minimumInterval;

  const unsigned int seed =
      1664525U * gameLocal->GetRandomSeed() + 1013904223U;
  gameLocal->SetRandomSeed(seed);
  return static_cast<int>((seed >> 10U) & 0x7FFFU) % intervalRange +
         minimumInterval;
}
// DIA: idAI2::IsTargetInMeleeRange
bool idAI2::IsTargetInMeleeRange(const idVec3* targetPosition,
                                 const aiMelee_t meleeType) {
  const idDeclAIBehavior* declaration = behaviorDeclaration;
  float meleeDistance = declaration->closeMeleeDist;
  if (!declaration->meleeEnabled) return false;

  if ((meleeType & AIMELEE_LONG) != 0) {
    meleeDistance = (std::max)(declaration->longMeleeDist, meleeDistance);
  }
  if ((meleeType & AIMELEE_MEDIUM) != 0) {
    meleeDistance = (std::max)(declaration->mediumMeleeDist, meleeDistance);
  }
  if ((meleeType & AIMELEE_MOVING_FORWARD) != 0 &&
      (groupStatusCondition & 8) == 0) {
    meleeDistance = (std::max)(declaration->movingMeleeDist, meleeDistance);
  }
  meleeDistance = (std::max)(declaration->closeMeleeDist, meleeDistance);

  const idVec3& origin = GetPhysics()->GetOrigin();
  const float deltaX = targetPosition->x - origin.x;
  const float deltaY = targetPosition->y - origin.y;
  const float verticalDistance = std::fabs(targetPosition->z - origin.z);
  const float horizontalDistance = std::sqrt((std::max)(
      deltaX * deltaX + deltaY * deltaY,
      std::numeric_limits<float>::min()));
  if (meleeDistance < horizontalDistance) return false;

  if (declaration->meleeHeight <= 0.0F) {
    const idBounds& bounds = GetPhysics()->GetAbsBounds();
    if ((bounds[1].z - bounds[0].z) * 0.75F <= verticalDistance) {
      return false;
    }
  } else if (declaration->meleeHeight <= verticalDistance) {
    return false;
  }
  return true;
}
// DIA: idAI2::MeleeMaskForTarget
unsigned int idAI2::MeleeMaskForTarget(const idEntity* target) {
  const auto* targetActor = dynamic_cast<const idActor*>(target);
  const bool armoredTarget = dynamic_cast<const idTurret*>(target) != nullptr ||
                             (targetActor != nullptr &&
                              targetActor->GetActorClass() == ACTORCLASS_ROBOT);
  const unsigned int flags = IsCrouching() ? crouchMeleeAnimationFlags
                                           : meleeAnimationFlags;
  if (armoredTarget) {
    return (flags & 0x20U) != 0 ? 0x20U : 1U;
  }
  return flags;
}
// DIA: idAI2::GetDirectionTo
aiDirection_t idAI2::GetDirectionTo(const idVec3* position,
                                    bool fourDirectionLimit) {
  const idPhysics* physicsObject = GetPhysics();
  const idVec3 delta = *position - physicsObject->GetOrigin();
  const float horizontalLengthSqr =
      delta.x * delta.x + delta.y * delta.y;
  const float inverseHorizontalLength =
      1.0F / std::sqrt((std::max)(horizontalLengthSqr,
                                 std::numeric_limits<float>::min()));
  const idVec3 normalizedDelta{delta.x * inverseHorizontalLength,
                               delta.y * inverseHorizontalLength,
                               delta.z * inverseHorizontalLength};

  if (ai_debugScenePoints.valueInteger >= 4) {
    constexpr idColor directionDebugOrange{1.0F, 0.5F, 0.0F, 1.0F};
    idVec3 arrowStart = physicsObject->GetOrigin();
    idVec3 arrowEnd = *position;
    arrowStart.z += 8.0F;
    arrowEnd.z += 8.0F;
    gameLocal->GetRenderWorld()->GetRenderDebug()->DebugArrow(
        directionDebugOrange, arrowStart, arrowEnd, 8.0F, 2000, false);
  }

  const idVec3& gravityNormal = physicsObject->GetGravityNormal();
  idVec3 forward = bodyOrientation->GetAxis()[0];
  idVec3 left = bodyOrientation->GetAxis()[1];
  forward -= gravityNormal * (forward * gravityNormal);
  left -= gravityNormal * (left * gravityNormal);

  const float forwardDot = forward * normalizedDelta;
  const float leftDot = left * normalizedDelta;
  if (fourDirectionLimit) {
    if (leftDot > 0.70710671F) return AIDIR_LEFT;
    if (leftDot >= -0.70710671F) {
      return forwardDot >= 0.0F ? AIDIR_FRONT : AIDIR_BEHIND;
    }
    return AIDIR_RIGHT;
  }

  if (forwardDot < 0.0F) {
    if (forwardDot < -0.9238795F) return AIDIR_BEHIND;
    if (leftDot <= 0.0F) {
      return forwardDot > -0.38268346F ? AIDIR_BEHIND_RIGHT
                                      : AIDIR_RIGHT;
    }
    return forwardDot >= -0.38268346F ? AIDIR_LEFT
                                      : AIDIR_BEHIND_LEFT;
  }
  if (forwardDot > 0.9238795F) return AIDIR_FRONT;
  if (leftDot <= 0.0F) {
    return forwardDot > 0.38268346F ? AIDIR_FRONT_RIGHT : AIDIR_RIGHT;
  }
  return forwardDot <= 0.38268346F ? AIDIR_LEFT : AIDIR_FRONT_LEFT;
}
// DIA: idAI2::GetDirectionToForReactions
aiDirection_t idAI2::GetDirectionToForReactions(const idVec3* position) {
  const idPhysics* physicsObject = GetPhysics();
  const idVec3 delta = *position - physicsObject->GetOrigin();
  const float horizontalLengthSqr =
      delta.x * delta.x + delta.y * delta.y;
  const float inverseHorizontalLength =
      1.0F / std::sqrt((std::max)(horizontalLengthSqr,
                                 std::numeric_limits<float>::min()));
  const idVec3 normalizedDelta{delta.x * inverseHorizontalLength,
                               delta.y * inverseHorizontalLength,
                               delta.z * inverseHorizontalLength};

  if (ai_debugScenePoints.valueInteger >= 4) {
    constexpr idColor directionDebugOrange{1.0F, 0.5F, 0.0F, 1.0F};
    idVec3 arrowStart = physicsObject->GetOrigin();
    idVec3 arrowEnd = *position;
    arrowStart.z += 8.0F;
    arrowEnd.z += 8.0F;
    gameLocal->GetRenderWorld()->GetRenderDebug()->DebugArrow(
        directionDebugOrange, arrowStart, arrowEnd, 8.0F, 2000, false);
  }

  const idVec3& gravityNormal = physicsObject->GetGravityNormal();
  idVec3 forward = bodyOrientation->GetAxis()[0];
  forward -= gravityNormal * (forward * gravityNormal);
  const float inverseForwardHorizontalLength =
      1.0F / std::sqrt((std::max)(forward.x * forward.x +
                                     forward.y * forward.y,
                                 std::numeric_limits<float>::min()));
  const float forwardDot =
      (forward.x * inverseForwardHorizontalLength) * normalizedDelta.x +
      (forward.y * inverseForwardHorizontalLength) * normalizedDelta.y +
      forward.z * normalizedDelta.z;
  if (forwardDot > 0.0F) return AIDIR_FRONT;

  idVec3 left = bodyOrientation->GetAxis()[1];
  left -= gravityNormal * (left * gravityNormal);
  const float inverseLeftHorizontalLength =
      1.0F / std::sqrt((std::max)(left.x * left.x + left.y * left.y,
                                 std::numeric_limits<float>::min()));
  const float leftDot =
      (left.x * inverseLeftHorizontalLength) * normalizedDelta.x +
      (left.y * inverseLeftHorizontalLength) * normalizedDelta.y +
      left.z * normalizedDelta.z;
  if (leftDot > 0.49999994F) return AIDIR_LEFT;
  return leftDot >= -0.49999994F ? AIDIR_BEHIND : AIDIR_RIGHT;
}
// DIA: idAI2::GetTakedownName
bool idAI2::GetTakedownName(const idList<idStr, 5>* takedownList,
                            idStr* takedownName) {
  const int count = takedownList->Num();
  if (count <= 0) {
    idLib::Printf(" invalid takedown ID \n");
    return false;
  }

  const int requestedId = takedownID.valueInteger;
  if (requestedId <= -1) {
    const std::uint32_t seed =
        1664525U * gameLocal->GetRandomSeed() + 1013904223U;
    gameLocal->SetRandomSeed(seed);
    *takedownName = (*takedownList)[
        static_cast<int>(((seed >> 10U) & 0x7FFFU) %
                         static_cast<std::uint32_t>(count))];
    return true;
  }

  const int selectedId = (std::min)(requestedId, count - 1);
  const bool validId = requestedId < count;
  *takedownName = (*takedownList)[selectedId];
  if (!validId) {
    idLib::Printf(" invalid takedown ID \n");
    return false;
  }
  return true;
}
// DIA: idAI2::TakedownWindoIsOpen
bool idAI2::TakedownWindoIsOpen(idEntity* activator) {
  if (!takedownRestrictedToBehind) return true;
  if (gameLocal->GetAIIncomingDamageScale() * 20.0F >= GetBaseHealth()) {
    return true;
  }

  const idVec3 fromActivator =
      GetPhysics()->GetOrigin() - activator->GetPhysics()->GetOrigin();
  if (fromActivator * bodyOrientation->GetAxis()[0] <= 0.0F &&
      (takedownWindowClosed == -1 ||
       gameLocal->GetGameTimeMs() > takedownWindowClosed)) {
    return false;
  }
  return true;
}
// DIA: idAI2::IsCrosshairSubdued
bool idAI2::IsCrosshairSubdued(const idEntity* activator,
                               const idFocusTrace* focusTrace,
                               usableState_t usable) const {
  (void)activator;
  (void)focusTrace;
  return usable == USABLE_AI_TALK || usable == USABLE_AI_TOWN;
}
// DIA: idAI2::HandleLoot
idRecoveredValue idAI2::HandleLoot(...) { return RecordCoreCall("HandleLoot"); }
// DIA: idAI2::SetDelayedVoice
void idAI2::SetDelayedVoice(voiceMsg_t message, float delay) {
  delayedVoiceType = static_cast<int>(message);
  delayedVoiceTime =
      aiEvaluationTime - static_cast<int>(delay * -1000.0F);
}
// DIA: idAI2::CheckDelayedVoice
bool idAI2::CheckDelayedVoice(int currentTime) {
  if (delayedVoiceType == static_cast<int>(AIVOICE_MAX)) {
    return false;
  }
  if (delayedVoiceTime <= 0 || currentTime < delayedVoiceTime) {
    return false;
  }
  if (!animEventState.dead && !volumeDying) {
    localVoiceController->PlayVoice(
        this, static_cast<voiceMsg_t>(delayedVoiceType));
  }
  delayedVoiceType = static_cast<int>(AIVOICE_MAX);
  delayedVoiceTime = -1;
  return true;
}
// DIA: idAI2::IsThrowingGrenade
bool idAI2::IsThrowingGrenade() const {
  return usesGrenades && grenadeItemDeclaration != nullptr &&
         static_cast<unsigned int>(grenadeOverrideAnimation - 27) <= 4U;
}
// DIA: idAI2::GetPlayerViewDataFitness
float idAI2::GetPlayerViewDataFitness() const {
  if (playerViewRange < 200.0F || playerViewRange > 1024.0F) {
    return 0.0F;
  }
  float spottedScale = 0.0F;
  const float spottedSeconds =
      static_cast<float>(playerViewSpottedTime) * 0.001F;
  if (spottedSeconds >= 0.0F) {
    spottedScale = spottedSeconds <= 1.0F ? spottedSeconds : 1.0F;
  }
  return spottedScale * playerViewDot * 1024.0F /
         (1024.0F - playerViewRange);
}
// DIA: idAI2::CanReceiveAIEvents
bool idAI2::CanReceiveAIEvents(int currentTime) const {
  return !IsHidden() && currentTime >= nextAIEventTime;
}
// DIA: idAI2::OnAIEvent_VehicleSound
idRecoveredValue idAI2::OnAIEvent_VehicleSound(...) { return RecordCoreCall("OnAIEvent_VehicleSound"); }
// DIA: idAI2::OnAIEvent_Body
void idAI2::OnAIEvent_Body(idAIEventSphere* event) {
  GetAlertCycleFSM()->PostEvent(
      static_cast<int>(event->GetEventClass()), event,
      static_cast<unsigned int>(gameLocal->GetGameTimeMs()));
}
// DIA: idAI2::OnAIEvent_PlayerInteraction
void idAI2::OnAIEvent_PlayerInteraction(idAIEventSphere* event) {
  if (gameLocal->GetFrameNum() <= 10) return;
  if (!mutantActorClass) {
    if (speakingMoodCapability) {
      faceManager->SetEnableMoodAnimation(this, true);
    }
    if (speakingLipSyncCapability) {
      faceManager->SetEnableLipSyncAnimation(this, true);
    }
    if (speakingGestureCapability) {
      faceManager->SetEnableGestureAnimation(this, true);
    }
    if (speakingEyeTrackingCapability) {
      faceManager->SetEnableEyeTracking(this, true);
    }
    if (speakingBlinkingCapability) {
      faceManager->SetEnableBlinking(this, true);
    }
    if (speakingLidDeformationCapability) {
      faceManager->SetEnableLidDeformation(this, true);
    }
  }
  lastSignal = 10;
  ++signalCount;
  GetAlertCycleFSM()->PostEvent(
      static_cast<int>(event->GetEventClass()), event,
      static_cast<unsigned int>(gameLocal->GetGameTimeMs()));
}
// DIA: idAI2::SetSubWeb
bool idAI2::SetSubWeb(aiSubWeb_t subWeb) {
  constexpr aiModeRestricFlag_t subWebRestriction = 0x10;
  if ((movementModeRestrictionFlags & subWebRestriction) != 0) return false;
  if (forceRelaxedSubWeb) {
    animWebState.subWeb = aiSubWeb_t::relaxed;
    movementModeRestrictionFlags |= subWebRestriction;
    return false;
  }
  if (forceCombatSubWeb) {
    animWebState.subWeb = aiSubWeb_t::combat;
    movementModeRestrictionFlags |= subWebRestriction;
    return false;
  }
  animWebState.subWeb = subWeb;
  return true;
}
// DIA: idAI2::UpdateAttachments
idRecoveredValue idAI2::UpdateAttachments(...) { return RecordCoreCall("UpdateAttachments"); }
// DIA: `idAI2::UpdateAttachments'::`1'::dtor$0
// DIA: idAI2::ActivateExtendedClipModel
idRecoveredValue idAI2::ActivateExtendedClipModel(...) { return RecordCoreCall("ActivateExtendedClipModel"); }
// DIA: `idAI2::ActivateExtendedClipModel'::`1'::dtor$0
// DIA: `idAI2::ActivateExtendedClipModel'::`1'::dtor$1
// DIA: idAI2::AimDebug
void idAI2::AimDebug(const char* text, int level) const {
  if (ai_debugAim.valueInteger < level || ai_debugLevel.valueInteger <= 0) {
    return;
  }
  lastAimDebugText = text != nullptr ? text : "";
  lastAimDebugLevel = level;
  lastAimDebugPosition =
      GetPhysics() != nullptr ? GetPhysics()->GetOrigin() + aimDebugEyeOffset
                              : aimDebugEyeOffset;
  ++aimDebugCount;
}
// DIA: idAI2::LookDebug
void idAI2::LookDebug(const char* text, int level, int duration) const {
  if (ai_debugLook.valueInteger < level ||
      ai_debugLevel.valueInteger <= 0) {
    return;
  }

  lastLookDebugText = text != nullptr ? text : "";
  lastLookDebugLevel = level;
  lastLookDebugDuration = duration;
  lastLookDebugPosition =
      GetPhysics()->GetOrigin() + GetEyeOffset();
  ++lookDebugCount;

  if (ai_debugLook.valueInteger > level) {
    idLib::Printf("[%d] %d %s\n", gameLocal->GetGameTimeMs(),
                  GetEntityNumber(), text != nullptr ? text : "");
  }
}
// DIA: idAI2::Debug_ScenePoints
idRecoveredValue idAI2::Debug_ScenePoints(...) { return RecordCoreCall("Debug_ScenePoints"); }
// DIA: idAI2::idVisibilityObscurity::GetDestination
idRecoveredValue idAI2::idVisibilityObscurity::GetDestination(...) { return {}; }
// DIA: idAI2::CanTaunt
bool idAI2::CanTaunt() const {
  return (tauntDodgeFlags & 0x200) != 0 &&
         aiEvaluationTime > nextAITauntTime;
}
// DIA: idAI2::GetTauntInterval
int idAI2::GetTauntInterval() {
  const float intervalRange = tauntMaximumInterval - tauntMinimumInterval;
  sharedAIRandomSeed = 1664525U * sharedAIRandomSeed + 1013904223U;
  const float randomFraction =
      static_cast<float>((sharedAIRandomSeed >> 10U) & 0x7FFFU) *
      0.000030518509F;
  return static_cast<int>((randomFraction * intervalRange) * 1000.0F +
                          tauntMinimumInterval * 1000.0F);
}
// DIA: idAI2::GetEncounterGroup
idEncounterGroup* idAI2::GetEncounterGroup() const {
  return aiEventEncounterGroup;
}
// DIA: idAI2::GetWalkState
walkState_t idAI2::GetWalkState() const {
  const int forcedWalkState = ai_forceWalk.GetInteger();
  if (forcedWalkState > 0 && forcedWalkState < 4) {
    return static_cast<walkState_t>(forcedWalkState);
  }
  return idActor::GetWalkState();
}
// DIA: idAI2::SetSolid
void idAI2::SetSolid(bool isSolid) {
  idPhysics* physicsObject = GetPhysics();
  const int numberOfClipModels = physicsObject->GetNumClipModels();
  if (isSolid) {
    physicsObject->EnableClip();
    physicsObject->LinkClip();
    if (numberOfClipModels == 1) {
      physicsObject->SetContents(defaultPhysicsContents, 0);
    } else if (numberOfClipModels == 2) {
      physicsObject->SetContents(defaultPhysicsContents, 0);
      physicsObject->SetContents(8U, 1);
    }
    solidState = true;
  } else {
    if (numberOfClipModels == 1) {
      physicsObject->SetContents(0U, 0);
    } else if (numberOfClipModels == 2) {
      physicsObject->SetContents(0U, 0);
      physicsObject->SetContents(0U, 1);
    }
    physicsObject->UnlinkClip();
    solidState = false;
  }
}
// DIA: idAI2::SetAimFocusEntity
void idAI2::SetAimFocusEntity(const idEntity* entity, bool keepFocusInView,
                              aimPoint_t aimPoint, int timeout) {
  if (!robotState.usesRobot || endBlindedTime == 0) {
    focusInfos[0]->SetFocusEntity(reinterpret_cast<idAnimatedEntity*>(this),
                                  entity, keepFocusInView, aimPoint, timeout);
  }
}
// DIA: idAI2::SetAimFocusAimPoint
void idAI2::SetAimFocusAimPoint(aimPoint_t point) {
  aimFocusAimPoint = point;
}
// DIA: idAI2::GetMissTimeAimPosition
idVec3 idAI2::GetMissTimeAimPosition(
    const idEntity* target, idVec3 fireOrigin, const idMat3& fireAxis,
    bool trackAlongGround, float missTimeFractionLeft) const {
  const idBounds& targetBounds = target->GetPhysics()->GetAbsBounds();
  const idVec3 targetCenter = (targetBounds[0] + targetBounds[1]) * 0.5F;
  const float targetRadius =
      std::max(16.0F, (targetBounds[1] - targetBounds[0]).Length() * 0.5F);

  idVec3 fireUp = fireAxis[2];
  idVec3 fireLeft = fireAxis[1];
  if (fireUp.z < 0.0F) {
    fireUp = -fireUp;
    fireLeft = -fireLeft;
  }

  idVec3 missPosition;
  if (trackAlongGround && missTimeFractionLeft != -1.0F &&
      missTimeFractionLeft > 0.1F) {
    float endFraction = 0.1F;
    float missDistance = (targetCenter - fireOrigin).Length() * 0.1F -
                         targetRadius;
    if (missDistance >= 100.0F) {
      missDistance =
          std::min(300.0F, (missDistance - 100.0F) * 0.2F + 100.0F);
    } else {
      endFraction = (1.0F - missDistance / 100.0F) * 0.9F + 0.1F;
    }

    if (missTimeFractionLeft > endFraction && endFraction < 1.0F) {
      const float normalizedFraction =
          (missTimeFractionLeft - endFraction) / (1.0F - endFraction);
      const float verticalDistance =
          normalizedFraction * normalizedFraction * missDistance +
          targetRadius;
      missPosition = targetCenter - fireUp * verticalDistance;
      if (ai_debugMissTime.valueInteger != 0) {
        lastMissTimeDebugPoint = missPosition;
        ++missTimeDebugPointCount;
        ++missTimeDebugLineCount;
      }
      return missPosition;
    }
  }

  const idAIEntityState* targetState = worldState->StateForEntity(target);
  float angleCenter = -90.0F;
  float halfAngleRange = 120.0F;
  if (targetState != nullptr &&
      targetState->lastVisible == VISIBLE_IN_COVER) {
    angleCenter = 90.0F;
    halfAngleRange = 60.0F;
  }

  if (const auto* player = dynamic_cast<const idPlayer*>(target)) {
    const float fieldOfView = player->CalcFov(true);
    const float halfFieldOfView = fieldOfView * 0.5F * kDegreesToRadians;
    if (std::cos(halfFieldOfView) > GetEnemyLookDot(player)) {
      idMat3 playerEyeAxis;
      idVec3 playerEyePosition;
      player->GetEyeAxis(&playerEyeAxis);
      player->GetLastEyePos(&playerEyePosition);

      const float playerUpDot = playerEyeAxis[0] * fireUp;
      const float verticalScale = 1.0F - std::fabs(playerUpDot);
      const float verticalAdjustment =
          ((playerEyePosition.z - targetCenter.z) * 60.0F / targetRadius) *
          verticalScale * verticalScale;
      if ((playerEyeAxis[0] * fireLeft) <= 0.0F) {
        angleCenter = 180.0F - playerUpDot * 90.0F - verticalAdjustment;
      } else {
        angleCenter = playerUpDot * 90.0F + verticalAdjustment;
      }
      halfAngleRange =
          (std::max(0.0F, playerEyeAxis[0] * fireAxis[0]) *
               (360.0F - fieldOfView) +
           fieldOfView) *
          0.5F;
    }
  }

  const float minimumAngle = angleCenter - halfAngleRange;
  const float maximumAngle = angleCenter + halfAngleRange;
  const float randomAngle =
      RandomFloat(minimumAngle, maximumAngle) * kDegreesToRadians;
  const float upDistance =
      (RandomFloat(32.0F) + targetRadius) * std::sin(randomAngle);
  const float leftDistance =
      (RandomFloat(32.0F) + targetRadius) * std::cos(randomAngle);
  missPosition = targetCenter + fireLeft * leftDistance + fireUp * upDistance;

  if (ai_debugMissTime.valueInteger != 0) {
    lastMissTimeDebugPoint = missPosition;
    ++missTimeDebugPointCount;
    missTimeDebugLineCount +=
        static_cast<int>(std::ceil((maximumAngle - minimumAngle) / 15.0F)) +
        1;
  }
  return missPosition;
}
// DIA: idAI2::ReloadWeapon
void idAI2::ReloadWeapon(idWeapon* weapon) {
  if (weapon != nullptr && weapon->fireState == idWeapon::FIRESTATE_RELOADING) {
    weapon->Reload(GetInventoryCollection(), GetFXMgrForAttachment(), false,
                   false, false);
    idLib::Warning(
        "idAI2::ReloadWeapon was called - make sure a frame command is not "
        "missing on the reload animation!");
  }
}
// DIA: idAI2::GetRepullTriggerInterval
void idAI2::GetRepullTriggerInterval(const idWeapon* weapon, int* minimum,
                                     int* maximum) const {
  if (usesConfiguredFireTiming) {
    *minimum = static_cast<int>(configuredMinBurstPause * 1000.0F);
    *maximum = static_cast<int>(configuredMaxBurstPause * 1000.0F);
  } else {
    *minimum = weapon->repullMin;
    *maximum = weapon->repullMax;
  }
}
// DIA: idAI2::GetBurstDuration
void idAI2::GetBurstDuration(const idWeapon* weapon, int* minimum,
                             int* maximum) const {
  if (usesConfiguredFireTiming) {
    *minimum = static_cast<int>(configuredMinBurstDuration * 1000.0F);
    *maximum = static_cast<int>(configuredMaxBurstDuration * 1000.0F);
  } else {
    *minimum = weapon->burstMin;
    *maximum = weapon->burstMax;
  }
}
// DIA: idAI2::IsInsideTrailUpdateRange
bool idAI2::IsInsideTrailUpdateRange(
    const idVec3* suspiciousPosition) const {
  float range = 96.0F;
  if (usesCloseCover) {
    range = std::max(96.0F, closeMinRange);
  }
  return range >= GetPhysics()->GetOrigin().DistanceFast(*suspiciousPosition);
}
// DIA: idAI2::IsRecentlySpawned
bool idAI2::IsRecentlySpawned(float seconds) const {
  return aiEvaluationTime - firstThinkTimestamp <=
         static_cast<int>(seconds * 1000.0F);
}
// DIA: idAI2::IsRecentlyGroupSpotted
bool idAI2::IsRecentlyGroupSpotted(const idEntity* enemy, float time) const {
  return aiEventEncounterGroup != nullptr &&
         aiEventEncounterGroup->IsRecentlyGroupSpotted(enemy, time);
}
// DIA: idAI2::GetSpottedDuration
int idAI2::GetSpottedDuration(const idEntity* enemy, int currentTime) const {
  (void)currentTime;
  idAIEntityState* entityState = worldState->StateForEntity(enemy);
  if (entityState == nullptr || !entityState->active) {
    return -1;
  }
  return entityState->spotDuration >= 0 ? entityState->spotDuration : -1;
}
// DIA: idAI2::Event_DetectPlayer
eventVoid* idAI2::Event_DetectPlayer(eventVoid* result) {
  idPlayer* player = gameLocal->GetPlayer(0);
  if (player != nullptr) {
    eventVoid ignoredResult;
    Action_SetEnemy(&ignoredResult, player);
  }
  return result;
}
// DIA: idAI2::GetEnemyInsideMaxFireRange
bool idAI2::GetEnemyInsideMaxFireRange(const idEntity* enemy) const {
  if (enemy == nullptr || maxFireRange <= 0.0F) {
    return false;
  }
  const idVec3 origin = usesDrone ? dronePresentOrigin
                                  : GetPhysics()->GetOrigin();
  const idVec3 delta = enemy->GetPhysics()->GetOrigin() - origin;
  return maxFireRange * maxFireRange >= delta.LengthSqr();
}
// DIA: idAI2::GetEnemyInsideGoNutsRange
bool idAI2::GetEnemyInsideGoNutsRange() const {
  return currentEnemyRange > 0.0F && goNutsRange >= currentEnemyRange;
}
// DIA: idAI2::SetFOV
void idAI2::SetFOV(const float* fieldOfView) {
  aiEventState.fieldOfView = *fieldOfView;
  float halfFOVRadians = (*fieldOfView * 0.5F) * 0.017453292F;
  if (halfFOVRadians < 0.0F || halfFOVRadians >= 6.2831855F) {
    halfFOVRadians -=
        std::floor(halfFOVRadians * 0.15915494F) * 6.2831855F;
  }
  fieldOfViewDot = std::cos(halfFOVRadians);
}
// DIA: idAI2::Teleport
void idAI2::Teleport(const idVec3* origin, idAngles* angles) {
  idActor::Teleport(origin, angles);
  bodyOrientation->SetAxis(angles->ToMat3());
  idActor::GetAASPosition(GetAAS());
}
// DIA: idAI2::SetEnableAutoFocus
void idAI2::SetEnableAutoFocus(bool enable) {
  if (!enable || autoFocusEnabled) {
    autoFocusEnabled = enable;
  } else {
    lookFocusTimeout = aiEvaluationTime;
    autoFocusEnabled = enable;
  }
}
// DIA: idAI2::ShowFlareEffect
idRecoveredValue idAI2::ShowFlareEffect(...) { return RecordCoreCall("ShowFlareEffect"); }
// DIA: idAI2::StartWeaponFX
int idAI2::StartWeaponFX(fxCondition_t condition) {
  idWeapon* weapon = GetEquippedWeapon();
  if (weapon == nullptr) return 0;
  idFXManager* effects = GetFXMgrForAttachment(weapon);
  idTreeAnimator* animator = GetEquippedWeaponAnimator();
  if (effects == nullptr || animator == nullptr) return 0;
  idVec3 origin;
  idMat3 axis;
  weapon->GetMuzzleFlashWorldTransform(animator, 0, &origin, &axis);
  return effects->StartFX(&origin, &axis, gameLocal->GetGameTimeMs(), condition);
}
// DIA: idAI2::StopWeaponFX
void idAI2::StopWeaponFX(fxCondition_t condition) {
  idWeapon* weapon = GetEquippedWeapon();
  if (weapon == nullptr) return;
  idFXManager* effects = GetFXMgrForAttachment(weapon);
  if (effects != nullptr) {
    effects->StopFX(gameLocal->GetGameTimeMs(), condition, false);
  }
}
// DIA: idCheckSurroundingsState::UpdateLookInterests
// DIA: idCheckSurroundingsState::FindNetDirectionInterest
// DIA: idCheckSurroundingsState::GetLookedAtElapsedTime
// DIA: idCheckSurroundingsState::SetLastLookedAtTime
// DIA: `idAI2::FindClosestFocus'::`2'::idLookDebug::~idLookDebug
// DIA: idAI2::SetSpeakingVO
void idAI2::SetSpeakingVO() {
  if (mutantActorClass) {
    return;
  }
  if (speakingMoodCapability) {
    faceManager->SetEnableMoodAnimation(this, true);
  }
  if (speakingLipSyncCapability) {
    faceManager->SetEnableLipSyncAnimation(this, true);
    faceManager->StartLipsync();
  }
  if (speakingGestureCapability) {
    faceManager->SetEnableGestureAnimation(this, true);
  }
  if (speakingEyeTrackingCapability) {
    faceManager->SetEnableEyeTracking(this, true);
  }
  if (speakingBlinkingCapability) {
    faceManager->SetEnableBlinking(this, true);
  }
  if (speakingLidDeformationCapability) {
    faceManager->SetEnableLidDeformation(this, true);
  }
}
// DIA: idAI2::SetActionScript
void idAI2::SetActionScript(const idList<idScriptAction, 5>* actions,
                            idEntity* source, idEntity* activator) {
  if (!IsDead()) {
    componentTimeLine->SetActionScript(this, actions, source, activator,
                                       false);
  }
}
// DIA: idAI2::EndActionScript
void idAI2::EndActionScript() {
  if (actionScriptCount <= 0) {
    actionScriptRunning = false;
    actionScriptFlags = 0;
  }
}
// DIA: idAI2::OnActionScriptFinished
void idAI2::OnActionScriptFinished(int currentTime) {
  (void)currentTime;
  painAllowedAfterActionScript =
      !behaviorDeclaration->disablePainAfterActionScript;
}
// DIA: idAI2::SetActionScriptFlag
void idAI2::SetActionScriptFlag(int flags, bool setFlag) {
  if (setFlag) {
    actionScriptFlags |= flags;
  } else {
    actionScriptFlags &= ~flags;
  }
}
// DIA: idAI2::ActionScriptFlagIsSet
bool idAI2::ActionScriptFlagIsSet(int flags) const {
  return actionScriptRunning && (flags & actionScriptFlags) != 0;
}
// DIA: idAI2::GetActionFSM
idLegsFSM* idAI2::GetActionFSM() { return actionFSM.get(); }
// DIA: idAI2::GetActionStatus
int idAI2::GetActionStatus() const { return actionFSM->GetStatus(); }
// DIA: idSpawnFocusWithRequirement::RequirementFilled
// DIA: idAIFSMCallback::OnTransition
// DIA: idAIFSMCallback::OnRestart
// DIA: idAIFSMCallback::OnError
// DIA: idAI2::HideMeshNoAutoRestore
bool idAI2::HideMeshNoAutoRestore(const char* meshName) {
  if (presentable == nullptr ||
      presentable->GetAnimatedEntityInterface() == nullptr) {
    return false;
  }
  idPresentable* animatedPresentable =
      presentable != nullptr ? presentable->GetAnimatedEntityInterface()
                             : nullptr;
  auto* animator = static_cast<idTreeAnimator*>(
      animatedPresentable->GetAnimatedTreeAnimator());
  if (animator == nullptr) return false;
  const int meshIndex = animator->GetMeshIndex(meshName);
  if (meshIndex < 0) return false;
  animator->HideMesh(meshIndex);
  return true;
}
// DIA: idAI2::PerformStandUpAction
void idAI2::PerformStandUpAction() {
  if (componentTimeLine->NumScriptActions(this) <= 0) {
    componentTimeLine->ClearActionScript(this);
  }
  idFiniteStateMachine* fsm = GetAlertCycleFSM();
  fsm->SetActionCallerState(fsm->GetCurrentStateForTest());
  fsm->ProcessEvent(&ACTION_CrouchToStandEnter, this);
}
// DIA: `idAI2::PerformStandUpAction'::`1'::dtor$0
// DIA: idAI2::PerformCrouchAction
void idAI2::PerformCrouchAction() {
  if (componentTimeLine->NumScriptActions(this) <= 0) {
    componentTimeLine->ClearActionScript(this);
  }
  idFiniteStateMachine* fsm = GetAlertCycleFSM();
  fsm->SetActionCallerState(fsm->GetCurrentStateForTest());
  fsm->ProcessEvent(&ACTION_StandToCrouchEnter, this);
}
// DIA: idAI2::PerformOverrideAnimAction
void idAI2::PerformOverrideAnimAction(const int overrideAnim,
                                      const bool holdFireUntilDone) {
  if (componentTimeLine->NumScriptActions(this) <= 0) {
    componentTimeLine->ClearActionScript(this);
  }

  idFiniteStateMachine* fsm = GetAlertCycleFSM();
  fsm->SetActionCallerState(fsm->GetCurrentStateForTest());

  idEventArg ownerArgument{};
  ownerArgument.type = static_cast<unsigned char>('e');
  ownerArgument.value.pointer = this;
  idEventArg overrideArgument{};
  overrideArgument.type = static_cast<unsigned char>('i');
  overrideArgument.value.i = overrideAnim;
  idEventArg holdFireArgument{};
  holdFireArgument.type = static_cast<unsigned char>('i');
  holdFireArgument.value.i = holdFireUntilDone;
  (void)fsm->ProcessEvent(&ACTION_PlayOverrideAnimEnter, ownerArgument,
                          overrideArgument, holdFireArgument);
}
// DIA: `idAI2::PerformOverrideAnimAction'::`1'::dtor$0
// DIA: `idAI2::PerformOverrideAnimAction'::`1'::dtor$1
// DIA: `idAI2::PerformOverrideAnimAction'::`1'::dtor$2
// DIA: idAI2::SetIdealAimFocusPoint
void idAI2::SetIdealAimFocusPoint(const idVec3& point, int timeout,
                                  aiFocus_t focusType) {
  focusInfos[0]->SetExplicitFocusPoint(nullptr, point, false, timeout,
                                       focusType);
}
// DIA: idAI2::SetIdealLookFocusPoint
void idAI2::SetIdealLookFocusPoint(const idVec3& point, int timeout,
                                   aiFocus_t focusType) {
  focusInfos[1]->SetExplicitFocusPoint(nullptr, point, false, timeout,
                                       focusType);
}
// DIA: idAI2::ClearAimFocus
void idAI2::ClearAimFocus() { focusInfos[0]->Clear(nullptr); }
// DIA: idAI2::ClearLookFocus
void idAI2::ClearLookFocus() { focusInfos[1]->Clear(nullptr); }
// DIA: idAI2::SetEnableHeadTracking
void idAI2::SetEnableHeadTracking(bool enable) {
  animEventState.headTracking = enable;
  for (const auto& focus : focusInfos) {
    focus->SetEnableTracking(nullptr, enable);
  }
}
// DIA: idAI2::SetLockHeadTracking
void idAI2::SetLockHeadTracking(bool lock) {
  for (const auto& focus : focusInfos) {
    focus->SetLockTracking(lock);
  }
  headTrackingLocked = lock;
}
// DIA: idAI2::IsInCover
bool idAI2::IsInCover() const {
  const auto* combatScenePoint =
      dynamic_cast<const idScenePoint_Combat*>(currentScenePoint);
  return combatScenePoint != nullptr && combatScenePoint->IsTactical() &&
         currentScenePointActivationTime > 0 &&
         coreState.coverAction == COVERACTION_HIDE;
}
// DIA: idAI2::IsBlindFiring
bool idAI2::IsBlindFiring() const {
  const auto* combatScenePoint =
      dynamic_cast<const idScenePoint_Combat*>(currentScenePoint);
  if (combatScenePoint == nullptr || !combatScenePoint->IsTactical() ||
      currentScenePointActivationTime <= 0) {
    return false;
  }

  const coverAction_t action = coreState.coverAction;
  return action == COVERACTION_BLINDFIRE_OVER ||
         action == COVERACTION_BLINDFIRE_LEFT ||
         action == COVERACTION_BLINDFIRE_RIGHT;
}
// DIA: idAI2::GetAnimationDelta
bool idAI2::GetAnimationDelta(const idMD6Anim* animation,
                              idVec3& outDelta) const {
  if (animation == nullptr) {
    return false;
  }

  const float* startDelta = animation->animData->startDelta;
  const float* endDelta = animation->animData->endDelta;
  const idVec3 delta{endDelta[8] - startDelta[8],
                     endDelta[9] - startDelta[9],
                     endDelta[10] - startDelta[10]};
  const float lengthSquared =
      startDelta[0] * startDelta[0] + startDelta[1] * startDelta[1] +
      startDelta[2] * startDelta[2] + startDelta[3] * startDelta[3];
  const float inverseLength =
      1.0F / std::sqrt(std::max(lengthSquared,
                               std::numeric_limits<float>::min()));
  const idQuat rotation{-startDelta[0] * inverseLength,
                        -startDelta[1] * inverseLength,
                        -startDelta[2] * inverseLength, startDelta[3]};

  const float xx = rotation.x * rotation.x;
  const float yy = rotation.y * rotation.y;
  const float zz = rotation.z * rotation.z;
  const float ww = rotation.w * rotation.w;
  const float xy2 = rotation.x * rotation.y * 2.0F;
  const float xz2 = rotation.x * rotation.z * 2.0F;
  const float yz2 = rotation.y * rotation.z * 2.0F;
  const float wx2 = rotation.w * rotation.x * 2.0F;
  const float wy2 = rotation.w * rotation.y * 2.0F;
  const float wz2 = rotation.w * rotation.z * 2.0F;
  outDelta.x = (((xx - yy) - zz) + ww) * delta.x +
               (wz2 + xy2) * delta.y + (xz2 - wy2) * delta.z;
  outDelta.z = (wy2 + xz2) * delta.x + (yz2 - wx2) * delta.y +
               ((zz - (yy + xx)) + ww) * delta.z;
  outDelta.y = (((yy - xx) - zz) + ww) * delta.y +
               (xy2 - wz2) * delta.x + (yz2 + wx2) * delta.z;
  return true;
}
// DIA: idAI2::GetRunCycleIndexForType
int idAI2::GetRunCycleIndexForType(runIndexType_t type) const {
  switch (type) {
    case ANIMWEBAI_RUNINDEXTYPE_NORMAL:
      return 0;
    case ANIMWEBAI_RUNINDEXTYPE_INJURED:
      idLib::Warning("idAI2::GetRunCycleIndexForType run index type "
                     "ANIMWEBAI_RUNINDEXTYPE_INJURED cannot be selected this way");
      return -1;
    case ANIMWEBAI_RUNINDEXTYPE_SCRAMBLE:
      return behaviorDeclaration->runTypeBehaviors.scrambleRunIndex;
    case ANIMWEBAI_RUNINDEXTYPE_FAR:
      return behaviorDeclaration->runTypeBehaviors.farRunIndex;
    case ANIMWEBAI_RUNINDEXTYPE_GUNDOWN:
      return behaviorDeclaration->runTypeBehaviors.gunDownIndex;
    case ANIMWEBAI_RUNINDEXTYPE_NARROW:
      return behaviorDeclaration->runTypeBehaviors.narrowIndex;
    case ANIMWEBAI_RUNINDEXTYPE_UP_STAIRS:
      return behaviorDeclaration->runTypeBehaviors.upStairsIndex;
    case ANIMWEBAI_RUNINDEXTYPE_DOWN_STAIRS:
      return behaviorDeclaration->runTypeBehaviors.downStairsIndex;
    case ANIMWEBAI_RUNINDEXTYPE_DUCK:
      return behaviorDeclaration->runTypeBehaviors.duckIndex;
    default:
      return -1;
  }
}
// DIA: idAI2::GetIdleIndexForType
int idAI2::GetIdleIndexForType(runIndexType_t type) const {
  switch (type) {
    case ANIMWEBAI_RUNINDEXTYPE_INJURED:
      idLib::Warning("idAI2::GetIdleIndexForType idle index type "
                     "ANIMWEBAI_RUNINDEXTYPE_INJURED cannot be selected this way");
      return -1;
    case ANIMWEBAI_RUNINDEXTYPE_GUNDOWN:
      return behaviorDeclaration->idleBehaviors.gunDownIndex;
    case ANIMWEBAI_RUNINDEXTYPE_NARROW:
      return behaviorDeclaration->idleBehaviors.narrowIndex;
    case ANIMWEBAI_RUNINDEXTYPE_DUCK:
      return behaviorDeclaration->idleBehaviors.duckIndex;
    default:
      return 0;
  }
}
// DIA: idAI2::PlayAnimWebPathPerfect
idRecoveredValue idAI2::PlayAnimWebPathPerfect(...) { return RecordCoreCall("PlayAnimWebPathPerfect"); }
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$0
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$1
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$2
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$3
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$4
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$5
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$6
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$7
// DIA: `idAI2::PlayAnimWebPathPerfect'::`1'::dtor$8
// DIA: GetCoverTransionAnimName
idRecoveredValue GetCoverTransionAnimName(...) { return {}; }
// DIA: `GetCoverTransionAnimName'::`1'::dtor$0
// DIA: `GetCoverTransionAnimName'::`1'::dtor$1
// DIA: `GetCoverTransionAnimName'::`1'::dtor$2
// DIA: `GetCoverTransionAnimName'::`1'::dtor$3
// DIA: `GetCoverTransionAnimName'::`1'::dtor$4
// DIA: idAI2::AllocPresentable
idRecoveredValue idAI2::AllocPresentable(...) { return RecordCoreCall("AllocPresentable"); }
// DIA: idAI2::DisableInfluenceTrail
void idAI2::DisableInfluenceTrail() {
  if (presentable != nullptr && ai_enableInfluenceTrails.valueInteger != 0) {
    influenceTrailAllocated = false;
  }
}
// DIA: idAI2::IsOnElevator
idRecoveredValue idAI2::IsOnElevator(...) { return RecordCoreCall("IsOnElevator"); }
// DIA: idAI2::StartRagdoll
idAnimator_AF::testSolidResult_t idAI2::StartRagdoll(
    const idRagdollInfo* info, const char* ragdollReason) {
  reasonForRagdoll = ragdollReason != nullptr ? ragdollReason : "";
  return StartRagdoll(info);
}
// DIA: idAI2::StopRagdoll
void idAI2::StopRagdoll() {
  if (dyingAIRagdollPending) gameLocal->RemoveDyingAI(this);
  dyingAIRagdollPending = false;
  idActor::StopRagdoll();
}
// DIA: idAI2::BeginSync
void idAI2::BeginSync() {
  if (deathSyncEnabled) {
    if (!dyingAIRagdollPending) gameLocal->AddDyingAI(this);
    dyingAIRagdollPending = true;
    idAnimator_AF* articulatedFigure = GetAFMutable();
    if (articulatedFigure != nullptr && articulatedFigure->CanBeginSync()) {
      articulatedFigure->BeginSync(gameLocal->GetGameTimeMs());
    }
  }
}
// DIA: idAI2::EndSync
void idAI2::EndSync() {
  if (dyingAIRagdollPending) gameLocal->RemoveDyingAI(this);
  dyingAIRagdollPending = false;
  idAnimator_AF* articulatedFigure = GetAFMutable();
  if (articulatedFigure != nullptr && articulatedFigure->IsSyncing()) {
    articulatedFigure->EndSync();
  }
}
// DIA: idAI2::GetMoveOrientation
idAIOrientation* idAI2::GetMoveOrientation() {
  return moveState->GetMoveOrientation();
}
// DIA: idAI2::IsMoveDone
bool idAI2::IsMoveDone(std::int64_t checkAlignment) const {
  (void)checkAlignment;
  return moveState->GetMoveStatus() == MOVE_STATUS_DONE;
}
// DIA: idAI2::GetMoveInterface
idAIMoveState* idAI2::GetMoveInterface() { return moveState.get(); }
// DIA: idAI2::PlayDeathSound
void idAI2::PlayDeathSound() {
  if (!deathSoundPlayed && deathSound != nullptr) {
    if (!hasPainJointGroup ||
        idStr::Find(painJointGroupName.data, "head", false, 0, -1) == -1) {
      lastDamageSound = deathSound;
      damageChannelPlaying = true;
      ++damageSoundStartCount;
    }
    deathSoundPlayed = true;
  }
}
// DIA: idAI2::UpdateWetness
idRecoveredValue idAI2::UpdateWetness(...) { return RecordCoreCall("UpdateWetness"); }
// DIA: idAI2::UpdateRenderParamBlends
idRecoveredValue idAI2::UpdateRenderParamBlends(...) { return RecordCoreCall("UpdateRenderParamBlends"); }
// DIA: idAIScriptProxy::PauseFiberTillActionCompletes
// DIA: idAIScriptProxy::PauseThreadTillActionCompletes
// DIA: idAnimator_TurretNxN<3>::`scalar deleting destructor'
// DIA: idAnimator_TurretNxN<3>::SetParameters
// DIA: idEntityPtr<idEntitySpriteAnimator>::GetEntity
// DIA: idEntityPtr<idTarget_AlarmSystem>::GetEntity
// DIA: idInterpolate<int>::GetCurrentValue
// DIA: idHashTableT<int,idAIAction * __ptr64>::Clear
// DIA: idPresentable::SetModelFade
// DIA: idEntity::GetPresentableLeftDir
const idVec3* idEntity::GetPresentableLeftDir() {
  return &presentableAxis[1];
}
// DIA: idEntity::GetPresentableUpDir
const idVec3* idEntity::GetPresentableUpDir() {
  return &presentableAxis[2];
}
// DIA: idActor::SaveDamageGroup
idRecoveredValue idActor::SaveDamageGroup(...) { return {}; }
// DIA: idScenePoint::Disable
void idScenePoint::Disable(int currentTime, float disableSeconds) {
  currentUser = nullptr;
  requestTime = disableSeconds < 0.0F
                    ? -1
                    : currentTime - static_cast<int>(disableSeconds * -1000.0F);
}
// DIA: idScenePoint_Combat::HasEnterAnimation
bool idScenePoint_Combat::HasEnterAnimation() const {
  return !enterAnimation.IsEmpty() || extraEnterAnimationCount > 0;
}
// DIA: idEncounterGroup::GetCurrentCombatStage
idCombatStage* idEncounterGroup::GetCurrentCombatStage() {
  return curCombatStage < 0 || curCombatStage >= combatStages.Num()
             ? nullptr
             : &combatStages[curCombatStage];
}
const idCombatStage* idEncounterGroup::GetCurrentCombatStage() const {
  return curCombatStage < 0 || curCombatStage >= combatStages.Num()
             ? nullptr
             : &combatStages[curCombatStage];
}
// DIA: idAIEditable::idAIActionSettings::Init
// DIA: idAI2::GetCurrentCombatStage
const idCombatStage* idAI2::GetCurrentCombatStage() const {
  const idEncounterGroup* group = GetEncounterGroup();
  return group != nullptr ? group->GetCurrentCombatStage() : nullptr;
}
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::SetDeadFlag
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::Damage_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::Kill_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::Reset_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::HealComponent_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::GetTotalCurHealth_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::GetTotalMaxHealth_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::ApplyDamageWithOverflow
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::ApplyDamageAndClampToZero
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::DamageComponent_Impl
// DIA: idAIEditable::idAIActionSettings::idAICustomRobot::idAICustomRobot
// DIA: idAIDeathInfo::~idAIDeathInfo
// DIA: idAI2::SetRobotPowerOut
void idAI2::SetRobotPowerOut() {
  if (robotState.isCharging) return;
  if (robotState.powerOutTriggerValid) {
    ++robotState.powerOutTriggerActivations;
  }
  robotState.cooldownEffectActive = true;
  robotState.cooldownEffectStartTime = gameLocal->GetGameTimeMs();
  robotState.currentStageHealth =
      robotState.chargeHealthFraction * robotState.stageHealth;
  if (!robotState.autoRecharge) {
    coreState.perceptionFlags |= 1U;
    if (fireControl != nullptr) {
      fireControl->SetFireMode(this, FIREMODE_HOLDFIRE);
    }
  }
  robotState.usesLookScenePoints = false;
  robotState.chargeDoneTime = -1;
  robotState.isCharging = true;
  if (robotState.useRechargeAnimation && endBlindedTime == 0 &&
      endStunnedTime == 0) {
    robotState.chargingAnimationActive = true;
  }
}
// DIA: idAI2::SetRobotChargingDone
void idAI2::SetRobotChargingDone() {
  if (!robotState.isCharging) return;
  if (robotState.chargingDoneTriggerValid) {
    ++robotState.chargingDoneTriggerActivations;
  }
  robotState.cooldownEffectActive = false;
  robotState.cooldownEffectStopTime = gameLocal->GetGameTimeMs();
  if (!robotState.autoRecharge) coreState.perceptionFlags &= ~1ULL;
  robotState.usesLookScenePoints = true;
  robotState.currentStageHealth = robotState.stageHealth;
  robotState.chargeStartTime = -1;
  robotState.chargeDoneTime = -1;
  robotState.isCharging = false;
  if (robotState.useRechargeAnimation && !animEventState.dead) {
    robotState.chargingAnimationActive = false;
  }
}
// DIA: idAI2::SetEMPStunned
idRecoveredValue idAI2::SetEMPStunned(...) { return RecordCoreCall("SetEMPStunned"); }
// DIA: idAI2::InhibitRunning
void idAI2::InhibitRunning(bool inhibit) {
  if (stationaryMode ||
      currentRunIndexType == ANIMWEBAI_RUNINDEXTYPE_INJURED) {
    idLib::Warning(
        "Failed to change InhibitRunning on AI %s. Failed from having "
        "StationaryMode set or was limping/downed",
        GetName());
  } else if (inhibit) {
    movementModeRestrictionFlags |= 8;
  } else {
    movementModeRestrictionFlags &= ~8;
  }
}
// DIA: idAI2::InitFSMs
void idAI2::InitFSMs() {
  actionManager->Init(this);
  fsmManager->Init(this);
  for (int index = 0; index < fsmManager->NumFSMs(); ++index) {
    fsmManager->GetFSM(index)->SetCallback(fsmCallback.get());
  }
}
// DIA: idAI2::UpdateMeleeCapabilities
void idAI2::UpdateMeleeCapabilities() {
  static constexpr std::array<const char*, 15> meleeAnimNodeNames{{
      "none",           "forward",      "back",         "left",
      "right",          "kick",         "punch",        "medium",
      "long",           "crouch_forward", "crouch_back", "crouch_left",
      "crouch_right",   "moving_forward", "takedown",
  }};

  unsigned int meleeFlag = 1U;
  meleeAnimationFlags = 1U;
  const idDeclAnimWeb* declaration =
      animWebAnimators[AIANIMWEB_BODY]->GetDeclaration();
  if (declaration == nullptr) return;

  idStr subWebName;
  GetSubWebPrefix(aiSubWeb_t::melee, &subWebName);
  const animWebSubWebIndex_t subWebIndex =
      declaration->GetSubWebIndexForName(subWebName.data);
  if (subWebIndex.value < 0) return;

  for (const char* stateName : meleeAnimNodeNames) {
    const animWebStateIndex_t stateIndex =
        declaration->GetStateIndexForName(stateName);
    if (stateIndex.value >= 0 &&
        declaration->FindNodeIndex(subWebIndex, stateIndex).value >= 0) {
      meleeAnimationFlags |= meleeFlag;
    }
    meleeFlag = (meleeFlag << 1U) | (meleeFlag >> 31U);
  }
}
// DIA: idAI2::EquipItem
bool idAI2::EquipItem(idInventoryItem* item, equipSlot_t equipSlot) {
  const bool equipped = idActor::EquipItem(item, equipSlot);
  if (idWeapon::CastTo(item) != nullptr) UpdateMeleeCapabilities();
  return equipped;
}
// DIA: idAI2::UnequipItem
bool idAI2::UnequipItem(idInventoryItem* item) {
  const bool unequipped = idActor::UnequipItem(item);
  if (idWeapon::CastTo(item) != nullptr) UpdateMeleeCapabilities();
  return unequipped;
}
// DIA: idAI2::SetAccuracy
void idAI2::SetAccuracy(aiAccuracy_t accuracy) {
  editableAimAccuracy = static_cast<int>(accuracy);
  if ((groupStatusCondition & 0x2000) != 0) {
    focusInfos[0]->SetTrackingParmsIndex(6);
    return;
  }
  if (accuracy == ACCURACY_FROMSKILL) {
    accuracy = static_cast<aiAccuracy_t>(skillAimAccuracy);
  }
  focusInfos[0]->SetTrackingParmsIndex(static_cast<int>(accuracy));
}
// DIA: idAI2::InitPhysics
idRecoveredValue idAI2::InitPhysics(...) { return RecordCoreCall("InitPhysics"); }
// DIA: `idAI2::InitPhysics'::`1'::dtor$0
// DIA: `idAI2::InitPhysics'::`1'::dtor$1
// DIA: idAI2::InitPerception
void idAI2::InitPerception(alertCycle_t cycle) {
  float radius = actorPerceptionRadius;
  if (cycle == ALERTCYCLE_RELAXED && actorIdlePerceptionRadius >= 0.0F) {
    radius = actorIdlePerceptionRadius;
  } else if (cycle == ALERTCYCLE_SEARCH &&
             actorSearchPerceptionRadius >= 0.0F) {
    radius = actorSearchPerceptionRadius;
  }
  worldState->SetRadius(radius);
  SetFOV(&configuredFieldOfView);
  maxFireRange = (std::max)(actorPerceptionRadius, maxFireRange);
}
// DIA: idAI2::OnHide
idRecoveredValue idAI2::OnHide(...) { return RecordCoreCall("OnHide"); }
// DIA: idAI2::UpdateSpriteAnimator
idRecoveredValue idAI2::UpdateSpriteAnimator(...) { return RecordCoreCall("UpdateSpriteAnimator"); }
// DIA: idAI2::OnIgnoreDamage
idRecoveredValue idAI2::OnIgnoreDamage(...) { return RecordCoreCall("OnIgnoreDamage"); }
// DIA: idAI2::UpdateDrop
idRecoveredValue idAI2::UpdateDrop(...) { return RecordCoreCall("UpdateDrop"); }
// DIA: idAI2::PlayAdditivePain
idRecoveredValue idAI2::PlayAdditivePain(...) { return RecordCoreCall("PlayAdditivePain"); }
// DIA: idAI2::PushFromCollision
idRecoveredValue idAI2::PushFromCollision(...) { return RecordCoreCall("PushFromCollision"); }
// DIA: idAI2::CheckDeathCollision
idRecoveredValue idAI2::CheckDeathCollision(...) { return RecordCoreCall("CheckDeathCollision"); }
// DIA: idAI2::DoMeleeDamage
idRecoveredValue idAI2::DoMeleeDamage(idRecoveredOverloadTag<1>) { return RecordCoreCall("DoMeleeDamage"); }
// DIA: idAI2::GetMeleeDirection
idRecoveredValue idAI2::GetMeleeDirection(...) { return RecordCoreCall("GetMeleeDirection"); }
// DIA: idAI2::IsFacingMe
bool idAI2::IsFacingMe(const idEntity* entity, float dotThreshold) {
  idAIEntityState* entityState = worldState->StateForEntity(entity);
  if (entity == nullptr || entityState == nullptr) return false;

  aiSenseState_t* sightSense = entityState->senses[1];
  if (sightSense->lastUnconfirmedStimTime < 0) return false;

  const aiPhysicalState_t* physical =
      sightSense->GetConfirmedPhysicalSafe();
  const idVec3 delta = GetPhysics()->GetOrigin() - physical->origin;
  const float inverseLength =
      1.0F / std::sqrt((std::max)(delta.LengthSqr(),
                                  std::numeric_limits<float>::min()));
  const idVec3 direction = delta * inverseLength;
  return physical->viewfwd * direction > dotThreshold;
}
// DIA: idAI2::StartReloading
idRecoveredValue idAI2::StartReloading(...) { return RecordCoreCall("StartReloading"); }
// DIA: `idAI2::StartReloading'::`1'::dtor$0
// DIA: `idAI2::StartReloading'::`1'::dtor$1
// DIA: idAI2::IsDowned
bool idAI2::IsDowned(float delay) const {
  return GetPosture() == 2 &&
         animWebState.subWeb == aiSubWeb_t::combat &&
         currentRunIndexType == ANIMWEBAI_RUNINDEXTYPE_INJURED &&
         aiEvaluationTime - injuredTime >=
             static_cast<int>(delay * 1000.0F);
}
// DIA: idAI2::CanBeTakenDown
idRecoveredValue idAI2::CanBeTakenDown(...) { return RecordCoreCall("CanBeTakenDown"); }
// DIA: idAI2::SetTakeDownWindow
void idAI2::SetTakeDownWindow(bool open) {
  const int currentTime = gameLocal->GetGameTimeMs();
  if (open) {
    ++stackedTakedownWindows;
    takedownWindowClosed = takedownWindoMaxTime.GetInteger() + currentTime;
  } else if (--stackedTakedownWindows <= 0) {
    stackedTakedownWindows = 0;
    stunnedByPlayerBlock = false;
    takedownWindowClosed = -1;
  }
  counterBlockedUntil = counterBlockTime.GetInteger() + currentTime;
}
// DIA: idAI2::GetUsableState
int idAI2::GetUsableState(const idEntity* activator,
                          const idFocusTrace* focusTrace) {
  (void)focusTrace;
  if (disassembleLootBoxCount > 0) return USABLE_DISASSEMBLE;

  if (IsDead() || IsDying() || usableDeathState) {
    if (!usableDeathLootSuppressed && IsLootable() &&
        (!looted || HasLootableItems())) {
      return USABLE_LOOT;
    }
  } else {
    const auto* player = dynamic_cast<const idPlayer*>(activator);
    if (player != nullptr &&
        !player->PlayerBehavior_Takedown_IsInTakedown()) {
      const int focusState = player->GetTakedownFocusState();
      if (focusState == 1 || focusState == 6) return USABLE_INFO;
    }
  }
  return USABLE_NONE;
}
// DIA: idAI2::GetOnlineUsableState
int idAI2::GetOnlineUsableState() {
  if (disassembleLootBoxCount > 0) return USABLE_DISASSEMBLE;
  if (IsDead() && IsLootable() && (!looted || HasLootableItems())) {
    if (gameLocal->GetGameTimeMs() >
        deathStartTime -
            static_cast<int>(ai_lootDelay.valueFloat * -1000.0F)) {
      return USABLE_LOOT;
    }
  }
  return USABLE_NONE;
}
// DIA: idAI2::UpdateEnemyInAAS
idRecoveredValue idAI2::UpdateEnemyInAAS(...) { return RecordCoreCall("UpdateEnemyInAAS"); }
// DIA: idAI2::SetAnimWebEvent
void idAI2::SetAnimWebEvent(aiAnimWeb_t web,
                            idAnimWebEvent::priority_t priority,
                            idAnimWebIndex subWebIndex,
                            idAnimWebIndex stateIndex,
                            animWebEvent_t eventType,
                            const idEventDef* eventDefinition) {
  animWebAnimators[static_cast<std::size_t>(web)]->SetEvent(
      eventType, subWebIndex, stateIndex, eventDefinition, priority);
}
// DIA: idAI2::ClearAnimWebEvent
void idAI2::ClearAnimWebEvent(aiAnimWeb_t web,
                              idAnimWebEvent::priority_t priority,
                              animWebEvent_t eventType) {
  (void)priority;
  animWebAnimators[static_cast<std::size_t>(web)]->ClearEvent(eventType);
  if (isPlayingEntranceAnim && web == AIANIMWEB_BODY &&
      eventType == ANIMWEB_EVENT_DEST_CLAMPED) {
    isPlayingEntranceAnim = false;
  }
}
// DIA: idAI2::ClearAllAnimWebEvents
void idAI2::ClearAllAnimWebEvents(aiAnimWeb_t web,
                                  idAnimWebEvent::priority_t priority) {
  (void)priority;
  animWebAnimators[static_cast<std::size_t>(web)]->ClearEvents();
  if (isPlayingEntranceAnim && web == AIANIMWEB_BODY) {
    isPlayingEntranceAnim = false;
  }
}
// DIA: idAI2::ClearWaitForOverrideAnim
void idAI2::ClearWaitForOverrideAnim(idAnimWebEvent::priority_t priority) {
  (void)priority;
  animWebEventPending = false;
}
// DIA: idAI2::AnimEventReceived
bool idAI2::AnimEventReceived(idAnimWebEvent::priority_t priority) const {
  (void)priority;
  return !animWebEventPending;
}
// DIA: idAI2::WaitForAnimToEnd
bool idAI2::WaitForAnimToEnd(
    const idAnimWebEvent::priority_t priority, const char* subWebName,
    const char* stateName, const animWebEvent_t eventType) {
  idAnimator_AnimWeb* bodyAnimator =
      animWebAnimators[AIANIMWEB_BODY].get();
  const idAnimWebIndex currentSubWeb =
      bodyAnimator->GetCurrentSubWebIndex();
  const idAnimWebIndex currentState = bodyAnimator->GetCurrentStateIndex();
  const idDeclAnimWeb* declaration = bodyAnimator->GetDeclaration();
  const animWebSubWebIndex_t requestedSubWeb =
      declaration->GetSubWebIndexForName(subWebName);
  const animWebStateIndex_t requestedState =
      declaration->GetStateIndexForName(stateName);
  if (requestedSubWeb.value < 0) {
    idLib::Warning(
        "idAI2::WaitForAnimToEnd - subweb '%s' not found in animWeb '%s'",
        subWebName, declaration->name.str);
    return false;
  }
  if (requestedState.value < 0) {
    idLib::Warning(
        "idAI2::WaitForAnimToEnd - state '%s' not found in animWeb '%s'",
        stateName, declaration->name.str);
    return false;
  }

  const idAnimWebIndex viaSubWeb{requestedSubWeb.value};
  const idAnimWebIndex viaState{requestedState.value};
  const int pathResult = bodyAnimator->ChangeStateVia(
      currentSubWeb, currentState, viaSubWeb, viaState, 1, 0, 0);
  if (pathResult < 0) return false;
  if (pathResult == 0) {
    bodyAnimator->SetEvent(eventType, viaSubWeb, viaState,
                           &EV_AnimWeb_Wait, priority);
  }
  return true;
}
// DIA: idAI2::WaitForAnimToStart
bool idAI2::WaitForAnimToStart(
    const aiAnimWeb_t lookupWeb,
    const idAnimWebEvent::priority_t priority, const char* subWebName,
    const char* stateName, const animWebEvent_t eventType) {
  const idDeclAnimWeb* declaration =
      animWebAnimators[static_cast<std::size_t>(lookupWeb)]->GetDeclaration();
  const animWebSubWebIndex_t subWebIndex =
      declaration->GetSubWebIndexForName(subWebName);
  const animWebStateIndex_t stateIndex =
      declaration->GetStateIndexForName(stateName);
  idAnimator_AnimWeb* bodyAnimator =
      animWebAnimators[AIANIMWEB_BODY].get();
  const idAnimWebIndex destinationSubWeb{subWebIndex.value};
  const idAnimWebIndex destinationState{stateIndex.value};
  const int pathResult = bodyAnimator->ChangeState(
      destinationSubWeb, destinationState, 1, 0, 0);
  if (pathResult < 0) return false;
  if (pathResult == 0) {
    bodyAnimator->SetEvent(eventType, destinationSubWeb, destinationState,
                           &EV_AnimWeb_Wait, priority);
  }
  return true;
}
// DIA: idAI2::WaitForAnimToStartVia
bool idAI2::WaitForAnimToStartVia(
    const aiAnimWeb_t web, const idAnimWebEvent::priority_t priority,
    const idAnimWebIndex destinationSubWeb,
    const idAnimWebIndex destinationState, const idAnimWebIndex viaSubWeb,
    const idAnimWebIndex viaState, const animWebEvent_t eventType) {
  const int pathResult = animWebAnimators[static_cast<std::size_t>(web)]
                             ->ChangeStateVia(
                                 destinationSubWeb, destinationState,
                                 viaSubWeb, viaState, 1, 0, 0);
  if (pathResult < 0) return false;
  if (pathResult == 0) {
    animWebAnimators[AIANIMWEB_BODY]->SetEvent(
        eventType, destinationSubWeb, destinationState, &EV_AnimWeb_Wait,
        priority);
  }
  return true;
}
// DIA: idAI2::WaitForAnimToStartVia
bool idAI2::WaitForAnimToStartVia(
    const aiAnimWeb_t web, const idAnimWebEvent::priority_t priority,
    const char* destinationSubWebName,
    const char* destinationStateName, const char* viaSubWebName,
    const char* viaStateName) {
  const idDeclAnimWeb* declaration =
      animWebAnimators[static_cast<std::size_t>(web)]->GetDeclaration();
  const animWebSubWebIndex_t viaSubWebIndex =
      declaration->GetSubWebIndexForName(viaSubWebName);
  const animWebStateIndex_t viaStateIndex =
      declaration->GetStateIndexForName(viaStateName);
  const animWebSubWebIndex_t destinationSubWebIndex =
      declaration->GetSubWebIndexForName(destinationSubWebName);
  const animWebStateIndex_t destinationStateIndex =
      declaration->GetStateIndexForName(destinationStateName);
  return WaitForAnimToStartVia(
      web, priority, idAnimWebIndex{destinationSubWebIndex.value},
      idAnimWebIndex{destinationStateIndex.value},
      idAnimWebIndex{viaSubWebIndex.value},
      idAnimWebIndex{viaStateIndex.value}, ANIMWEB_EVENT_START_BLEND_IN);
}
// DIA: idAI2::UpdateExtendedClipModel
idRecoveredValue idAI2::UpdateExtendedClipModel(...) { return RecordCoreCall("UpdateExtendedClipModel"); }
// DIA: idAI2::GetVoiceController
idAIVoiceController* idAI2::GetVoiceController(
    voiceController_t controller) {
  if (controller == VC_LOCAL) {
    return localVoiceController.get();
  }
  if (controller == VC_GROUP) {
    return voiceGroupController;
  }
  if (controller == VC_GLOBAL) {
    return idAIVoiceController::GetGlobalController();
  }
  return nullptr;
}
// DIA: idAI2::GetDefaultSpeedByCurrentWalkState
float idAI2::GetDefaultSpeedByCurrentWalkState() const {
  return GetWalkState() == WALKSTATE_RUNNING ? maximumMovementSpeed
                                              : minimumMovementSpeed;
}
// DIA: idAI2::DormantBegin
idRecoveredValue idAI2::DormantBegin(...) { return RecordCoreCall("DormantBegin"); }
// DIA: idAI2::FinishFireWeapon
idRecoveredValue idAI2::FinishFireWeapon(...) { return RecordCoreCall("FinishFireWeapon"); }
// DIA: idAI2::GetSpotLostFireTime
float idAI2::GetSpotLostFireTime() const {
  const int blindedTimeRemaining = endBlindedTime - aiEvaluationTime;
  const float blindedSeconds =
      blindedTimeRemaining > 0
          ? static_cast<float>(blindedTimeRemaining) * 0.001F
          : 0.0F;
  return std::max(spotLostFireTime, blindedSeconds);
}
// DIA: idAI2::GetSpotGainedFireTime
float idAI2::GetSpotGainedFireTime() const {
  const int blindedTimeRemaining = endBlindedTime - aiEvaluationTime;
  const float blindedSeconds =
      blindedTimeRemaining > 0
          ? static_cast<float>(blindedTimeRemaining) * 0.001F
          : 0.0F;
  return std::max(spotGainedFireTime, blindedSeconds);
}
// DIA: idAI2::IsRecentlySpotted
bool idAI2::IsRecentlySpotted(const idAIEntityState* entityState,
                              float time) const {
  if (entityState == nullptr || !entityState->active) return false;
  const aiSenseState_t* sight = entityState->senses[1];
  return sight->lastUnconfirmedStimTime >= 0 &&
         (time < 0.0F ||
          gameLocal->GetGameTimeMs() - sight->lastConfirmedStimTime <
              static_cast<int>(time * 1000.0F));
}
// DIA: idAI2::IsRecentlyHeard
bool idAI2::IsRecentlyHeard(const idAIEntityState* entityState,
                            float time) const {
  if (entityState == nullptr || !entityState->active) return false;
  const aiSenseState_t* hearing = entityState->senses[0];
  return hearing->lastUnconfirmedStimTime >= 0 &&
         (time < 0.0F ||
          gameLocal->GetGameTimeMs() - hearing->lastConfirmedStimTime <
              static_cast<int>(time * 1000.0F));
}
// DIA: idAI2::GetViewTransform
idRecoveredValue idAI2::GetViewTransform(...) { return RecordCoreCall("GetViewTransform"); }
// DIA: idAI2::GetEyeOffset
const idVec3 idAI2::GetEyeOffset() const {
  return idActor::GetEyeOffset();
}
// DIA: idAI2::TeleportPush
idRecoveredValue idAI2::TeleportPush(...) { return RecordCoreCall("TeleportPush"); }
// DIA: idAI2::SpawnFlareEffect
idRecoveredValue idAI2::SpawnFlareEffect(...) { return RecordCoreCall("SpawnFlareEffect"); }
// DIA: idCheckSurroundingsState::TraceVector
// DIA: idCheckSurroundingsState::UpdateDirectionDistanceExponent
// DIA: idAI2::UpdateActions
void idAI2::UpdateActions(int curTime) {
  actionFSM->Update(true, curTime, nullptr);
  componentTimeLine->Update();
}
// DIA: idAI2::SetAlertCycle
void idAI2::SetAlertCycle(alertCycle_t alertCycle) {
  alertCycle_t effectiveCycle = alertCycle;
  if (alwaysInCombat && alertCycle != ALERTCYCLE_DYING &&
      alertCycle != ALERTCYCLE_DEAD) {
    effectiveCycle = ALERTCYCLE_COMBAT;
  }

  if (aiEventState.alertCycle != effectiveCycle) {
    previousAlertCycle = aiEventState.alertCycle;
    alertCycleTimestamp = gameLocal->GetGameTimeMs();
    InitPerception(alertCycle);
  }
  aiEventState.alertCycle = effectiveCycle;

  if (effectiveCycle == ALERTCYCLE_COMBAT) {
    highestAlertCycle = ALERTCYCLE_COMBAT;
    if (previousAlertCycle != ALERTCYCLE_COMBAT) {
      lastSurprisedTime = gameLocal->GetGameTimeMs();
    }
  } else if (effectiveCycle == ALERTCYCLE_SEARCH) {
    highestAlertCycle = (std::max)(highestAlertCycle,
                                   static_cast<int>(ALERTCYCLE_SEARCH));
  }

  const aiSubWeb_t subWeb =
      effectiveCycle < ALERTCYCLE_MAX
          ? static_cast<aiSubWeb_t>(
                AlertCycleToSubWeb(static_cast<int>(effectiveCycle)))
          : aiSubWeb_t::relaxed;
  SetSubWeb(subWeb);
}
// DIA: idAI2::DoEmergencyCoverExit
bool idAI2::DoEmergencyCoverExit() {
  const coverAction_t coverAction = coreState.coverAction;
  if (!usesEmergencyCoverExit || memory->HasEmergencyCoverExit() ||
      coverAction == COVERACTION_NONE || coverAction == COVERACTION_HIDE ||
      coverAction == COVERACTION_FLINCH) {
    return false;
  }

  const int currentTime = gameLocal->GetGameTimeMs();
  memory->SetCurScenePoint(this, currentTime, nullptr);
  coreState.coverAction = COVERACTION_NONE;
  coreState.coverActionTimestamp = currentTime;
  coverOrientation = COVERORIENT_NONE;
  SetPosture(POSTURE_STANDING);
  moveState->Stop(MOVE_STATUS_DONE);
  SetEnableHeadTracking(true);

  idStr targetState("idle");
  idStr targetSubWeb;
  GetSubWebPrefix(
      static_cast<aiSubWeb_t>(static_cast<int>(allowHostileStance) + 1),
      &targetSubWeb);
  const idDeclAnimWeb* declaration =
      animWebAnimators[AIANIMWEB_BODY]->GetDeclaration();
  const animWebSubWebIndex_t subWebIndex =
      declaration->GetSubWebIndexForName(targetSubWeb.data);
  const animWebStateIndex_t stateIndex =
      declaration->GetStateIndexForName(targetState.data);

  blendParms_t forceBlendParms;
  forceBlendParms.SetDuration(10);
  forceBlendParms.SetDestStartFrame(0);
  forceBlendParms.parms.originBlend = 2;
  animWebAnimators[AIANIMWEB_BODY]->ForceState(
      idAnimWebIndex{subWebIndex.value}, idAnimWebIndex{stateIndex.value},
      forceBlendParms, 0);
  memory->SetEmergencyCoverExit(true);
  return true;
}
// DIA: `idAI2::DoEmergencyCoverExit'::`1'::dtor$0
// DIA: `idAI2::DoEmergencyCoverExit'::`1'::dtor$1
// DIA: idAI2::GetAnimationPathReachability
bool idAI2::GetAnimationPathReachability(
    const idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t> subWeb,
    const idIndex<short, idDeclAnimWeb::invalidStateIndex_t> state) const {
  idAnimator_AnimWeb* animator = animWebAnimators[AIANIMWEB_BODY].get();
  idAnimWebRoute testPath;
  for (idAnimWebIndex& nodeIndex : testPath.path) {
    nodeIndex.value = -1;
  }
  testPath.num = 0;
  testPath.cost = std::numeric_limits<int>::max();
  return animator->FindPath(
      animator->GetCurrentNodeIndex(), idAnimWebIndex{state.value},
      idAnimWebIndex{subWeb.value}, idAnimWebIndex{-1}, &testPath);
}
// DIA: idAI2::GetAnimationDuration
idRecoveredValue idAI2::GetAnimationDuration(...) { return RecordCoreCall("GetAnimationDuration"); }
// DIA: idAI2::GetAnimationDelta
bool idAI2::GetAnimationDelta(
    const idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t> subWeb,
    const idIndex<short, idDeclAnimWeb::invalidStateIndex_t> state,
    idVec3& outDelta, const bool verifyAWPath) const {
  if (verifyAWPath && !GetAnimationPathReachability(subWeb, state)) {
    return false;
  }
  const idMD6Anim* animation = GetAnimForAnimWebNode(subWeb, state);
  return animation != nullptr && GetAnimationDelta(animation, outDelta);
}
// DIA: idAI2::GetAnimationRot
bool idAI2::GetAnimationRot(
    const idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t> subWeb,
    const idIndex<short, idDeclAnimWeb::invalidStateIndex_t> state,
    idAngles& outRotation, const bool verifyAWPath) const {
  if (verifyAWPath && !GetAnimationPathReachability(subWeb, state)) {
    return false;
  }
  const idMD6Anim* animation = GetAnimForAnimWebNode(subWeb, state);
  if (animation == nullptr) return false;

  idQuat endRotation{0.0F, 0.0F, 0.0F, 1.0F};
  if (animation->animData != nullptr) {
    endRotation = idQuat{animation->animData->endDelta[0],
                         animation->animData->endDelta[1],
                         animation->animData->endDelta[2],
                         animation->animData->endDelta[3]};
  }
  outRotation = QuaternionToAngles(endRotation);
  return true;
}
// DIA: idAI2::PlayAnimWebPath
idRecoveredValue idAI2::PlayAnimWebPath(...) { return RecordCoreCall("PlayAnimWebPath"); }
// DIA: idAI2::GetCoverTransitionRadius
idRecoveredValue idAI2::GetCoverTransitionRadius(...) { return RecordCoreCall("GetCoverTransitionRadius"); }
// DIA: `idAI2::GetCoverTransitionRadius'::`1'::dtor$0
// DIA: `idAI2::GetCoverTransitionRadius'::`1'::dtor$1
// DIA: `idAI2::GetCoverTransitionRadius'::`1'::dtor$2
// DIA: `idAI2::GetCoverTransitionRadius'::`1'::dtor$3
// DIA: idAI2::ShouldScramble
bool idAI2::ShouldScramble() {
  if (ai_enableScrambles.GetInteger() == 0) return false;

  const int currentTime = gameLocal->GetGameTimeMs();
  if (currentTime - lastFireTime < ai_scrambleTime.GetInteger() ||
      (groupStatusCondition & 0x10) != 0) {
    return true;
  }

  const idCombatStage* stage = GetCurrentCombatStage();
  if (stage != nullptr &&
      stage->stageType == idCombatStage::COMBAT_STAGE_RETREAT) {
    const idEncounterGroup* group = GetEncounterGroup();
    if (group->stageChangeTime + 8000 > currentTime) return true;
  }
  return false;
}
// DIA: idAI2::StartRagdoll
idAnimator_AF::testSolidResult_t idAI2::StartRagdoll(
    const idRagdollInfo* info) {
  if (!ragdollEnabled || ragdollSuppressed) {
    return idAnimator_AF::TESTSOLID_IN_SOLID;
  }
  PlayDeathSound();
  if (!dyingAIRagdollPending) gameLocal->AddDyingAI(this);
  dyingAIRagdollPending = true;
  if (!info->useOverrideVelocity || !usesDrone) {
    return idActor::StartRagdoll(info);
  }
  idRagdollInfo droneInfo = *info;
  const float gameHz = static_cast<float>(gameLocal->GetGameHz(1));
  droneInfo.overrideVelocity = dronePastVelocity * gameHz;
  return idActor::StartRagdoll(&droneInfo);
}
// DIA: idAI2::UpdateGoreLevel
idRecoveredValue idAI2::UpdateGoreLevel(...) { return RecordCoreCall("UpdateGoreLevel"); }
// DIA: idAIScriptProxy::PauseCurrentThreadTillActionCompletes
// DIA: idAIScriptProxy::PauseCurrentFiberTillActionCompletes
// DIA: idEntityPtr<idTarget_FakeEnemy>::operator idTarget_FakeEnemy * __ptr64
// DIA: idLeakyIntegrator<0,1>::Update
// DIA: idEntityPtr<idVolume_CustomDeath>::operator->
// DIA: idHashTableT<int,idAIAction * __ptr64>::~idHashTableT<int,idAIAction * __ptr64>
// DIA: idAIEvent::GetOriginator
// DIA: idEntity::GetTarget
idEntity* idEntity::GetTarget(int index) {
  return index >= 0 && index < targets.Num() ? targets[index] : nullptr;
}
// DIA: idAIDamageInfo::GetInflictor
idEntity* idAIDamageInfo::GetInflictor() {
  return const_cast<idEntity*>(inflictor);
}
// DIA: idAIDamageInfo::GetAttacker
idEntity* idAIDamageInfo::GetAttacker() {
  return const_cast<idEntity*>(attacker);
}
// DIA: idAIMemory::GetEnemy
const idEntity* idAIMemory::GetEnemy() const { return currentEnemy; }
// DIA: idAIMemory::GetFriendly
const idEntity* idAIMemory::GetFriendly() const { return currentFriendly; }
// DIA: idAIMemory::GetFriendlyBlocker
const idEntity* idAIMemory::GetFriendlyBlocker() const {
  return friendlyBlocker;
}
// DIA: idAIMemory::GetLastAttacker
idEntity* idAIMemory::GetLastAttacker() const { return lastAttacker; }
// DIA: idFocusInfo::HasFocus
bool idFocusInfo::HasFocus() {
  return focusEntity != nullptr || hasExplicitFocusPoint;
}
// DIA: idAIDeathInfo::idAIDeathInfo
// DIA: idAI2::HasFocus
bool idAI2::HasFocus() {
  return focusInfos[0]->HasFocus() || focusInfos[1]->HasFocus();
}
// DIA: idAI2::GetControllingPlayer
idPlayer* idAI2::GetControllingPlayer() const { return controllingPlayer; }
// DIA: idAI2::IsPlayerControlled
bool idAI2::IsPlayerControlled() const { return controllingPlayer != nullptr; }
// DIA: idGameLocal::GetDebugEntity
// DIA: idGameLocal::GetTensionManager
// DIA: idEntityPtr<idProxyMove>::operator idProxyMove * __ptr64
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::WriteComponents_Impl
// DIA: idHealthT<enum aiHealthComponent_t,2,0>::ReadComponents_Impl
// DIA: idShieldHealthT<enum aiHealthComponent_t,2,0,1>::HealShield_Impl
// DIA: idShieldHealthT<enum aiHealthComponent_t,2,0,1>::DamageShield_Impl
// DIA: idShieldHealthT<enum aiHealthComponent_t,2,0,1>::Update_Impl
// DIA: idAIDeathInfo::Set
void idAIDeathInfo::Set(idAIDamageInfo* info) {
  attacker = info->GetAttackerEntity();
  inflictor = info->GetInflictorEntity();
  const idDamageImpulse* deathImpulse = info->GetDeathImpulse();
  impulse = deathImpulse != nullptr ? *deathImpulse : idDamageImpulse{};
  startDeathTime = gameLocal->GetGameTimeMs();
}
// DIA: idAIEditable::idInteractions::idInteractions
// DIA: idAIEditable::idInteractions::~idInteractions
// DIA: idAIEditable::idAIActionSettings::idAIFireSettings::idAIFireSettings
// DIA: idAI2::idAnimWebDefaults::idAnimWebDefaults
idAI2::idAnimWebDefaults::idAnimWebDefaults() = default;
// DIA: idAI2::idAIVolatile::idAIFocusInfo::idAIFocusInfo
idAI2::idAIVolatile::idAIFocusInfo::idAIFocusInfo() = default;
// DIA: `idAI2::idAIVolatile::idAIFocusInfo::idAIFocusInfo'::`1'::dtor$0
// DIA: idAI2::idAIVolatile::idAIDeath::idAIDeath
idAI2::idAIVolatile::idAIDeath::idAIDeath() = default;
// DIA: `idAI2::idAIVolatile::idAIDeath::idAIDeath'::`1'::dtor$0
// DIA: idAI2::idAIVolatile::idAIDeath::~idAIDeath
idAI2::idAIVolatile::idAIDeath::~idAIDeath() = default;
// DIA: idAI2::idAnimWebDefaults::~idAnimWebDefaults
idAI2::idAnimWebDefaults::~idAnimWebDefaults() = default;
// DIA: idAI2::RobotBatteryExplodes
idRecoveredValue idAI2::RobotBatteryExplodes(...) { return RecordCoreCall("RobotBatteryExplodes"); }
// DIA: idAI2::CanFireRobot
bool idAI2::CanFireRobot(int currentTime) {
  if (!robotState.usesRobot) return true;
  if (robotState.isCharging || robotState.noFireTime >= currentTime ||
      (robotState.usesTesla &&
       (teslaDischargeTime >= 0 || teslaExplodeTime >= 0))) {
    robotState.lastShootTime = -1;
    return false;
  }
  if (robotState.shootHealthDrain <= 0.0F) return true;

  const int lastShootTime = robotState.lastShootTime;
  if (lastShootTime <= 0 ||
      (robotState.currentStageHealth -=
           static_cast<float>(currentTime - lastShootTime) * 0.001F *
           robotState.stageHealth * robotState.shootHealthDrain,
       robotState.chargeHealthFraction <
           robotState.currentStageHealth / robotState.stageHealth)) {
    robotState.lastShootTime = currentTime;
    robotState.bossHealth =
        (static_cast<float>(robotState.numberOfStages -
                            robotState.currentStage - 1) +
         robotState.currentStageHealth / robotState.stageHealth) /
        static_cast<float>(robotState.numberOfStages);
    return true;
  }

  SetRobotPowerOut();
  if (robotState.usesRobot) robotState.lastShootTime = -1;
  robotState.bossHealth =
      (static_cast<float>(robotState.numberOfStages - robotState.currentStage -
                          1) +
       robotState.currentStageHealth / robotState.stageHealth) /
      static_cast<float>(robotState.numberOfStages);
  return false;
}
// DIA: idAI2::StopFireRobot
void idAI2::StopFireRobot(int currentTime) {
  if (!robotState.usesRobot) return;
  const int lastShootTime = robotState.lastShootTime;
  if (lastShootTime > 0 && !robotState.isCharging &&
      robotState.shootHealthDrain > 0.0F) {
    const float stageHealth = robotState.stageHealth;
    robotState.currentStageHealth -=
        static_cast<float>(currentTime - lastShootTime) * 0.001F *
        stageHealth * robotState.shootHealthDrain;
    if (robotState.chargeHealthFraction >=
        robotState.currentStageHealth / stageHealth) {
      SetRobotPowerOut();
      robotState.currentStageHealth =
          stageHealth * robotState.chargeHealthFraction;
    }
    robotState.bossHealth =
        (static_cast<float>(robotState.numberOfStages -
                            robotState.currentStage - 1) +
         robotState.currentStageHealth / stageHealth) /
        static_cast<float>(robotState.numberOfStages);
  }
  if (robotState.usesRobot) robotState.lastShootTime = -1;
}
// DIA: idAI2::CanTeslaBlindFire
bool idAI2::CanTeslaBlindFire(idAIEntityState* enemyState, int duration) {
  return teslaBlindFireDuration > 0.0F && enemyState != nullptr &&
         teslaBlindFireTimeStamp != 0 &&
         gameLocal->GetGameTimeMs() <= duration + teslaBlindFireTimeStamp &&
         !IsRecentlySpotted(enemyState, spotLostFireTime) &&
         aiEventState.teslaBlindFirePositions.Num() >= 1;
}
// DIA: idAI2::SpawnTeslaBomb
idRecoveredValue idAI2::SpawnTeslaBomb(...) { return RecordCoreCall("SpawnTeslaBomb"); }
// DIA: idAI2::StopTesla
idRecoveredValue idAI2::StopTesla(...) { return RecordCoreCall("StopTesla"); }
// DIA: idAI2::LKWBlind
bool idAI2::LKWBlind() {
  const int currentTime = gameLocal->GetGameTimeMs();
  if (endReblindCooldown == -1 || currentTime <= endReblindCooldown ||
      !usesBlindness || volumeDying || animEventState.dead ||
      endBlindedTime != 0 || endStunnedTime != 0) {
    return false;
  }
  endBlindedTime = blindedDuration == -1.0F ? -1 : currentTime + 1000;
  endReblindCooldown = -1;
  ++lkwBlindEffectCount;
  PlayAdditivePain();
  return true;
}
// DIA: idAI2::UpdateGrappleActivation
idRecoveredValue idAI2::UpdateGrappleActivation(...) { return RecordCoreCall("UpdateGrappleActivation"); }
// DIA: idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb
// DIA: `idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb'::`1'::dtor$0
// DIA: `idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb'::`1'::dtor$1
// DIA: `idAnimatorParms_AnimWeb::~idAnimatorParms_AnimWeb'::`1'::dtor$2
// DIA: idAI2::Remove
idRecoveredValue idAI2::Remove(...) { return RecordCoreCall("Remove"); }
// DIA: idAI2::ShouldEnterDormancy
bool idAI2::ShouldEnterDormancy() const {
  if (ai_enableDormancy.GetInteger() == 0) return false;
  if (immediatePhysicsDormancyGate &&
      GetPhysics()->ShouldEnterDormancyImmediately()) {
    return true;
  }
  if (controllingPlayer != nullptr) return false;
  if (animWebTransitionActive) return false;
  if ((articulatedFigureActive || articulatedFigureClientActive) &&
      !articulatedFigureAtRest) {
    return false;
  }
  if (const_cast<idAI2*>(this)->IsExecutingScript()) return false;
  return idEntity::ShouldEnterDormancy();
}
// DIA: idAI2::UpdateFSMs
idRecoveredValue idAI2::UpdateFSMs(...) { return RecordCoreCall("UpdateFSMs"); }
// DIA: idAI2::UpdateMovement
void idAI2::UpdateMovement() {
  moveState->UpdateMovement(gameLocal->GetGameTimeMs());
  UpdateExtendedClipModel();
}
// DIA: idAI2::UpdateDeathMovement
void idAI2::UpdateDeathMovement() {
  moveState->UpdateDeathMovement(gameLocal->GetGameTimeMs());
  UpdateExtendedClipModel();
}
// DIA: idAI2::UpdatePathToEnemy
idRecoveredValue idAI2::UpdatePathToEnemy(...) { return RecordCoreCall("UpdatePathToEnemy"); }
// DIA: idAI2::SendDeathTrigger
void idAI2::SendDeathTrigger() {
  const idEntity* player = gameLocal->GetPlayer(0);
  if (deathTrigger != nullptr) {
    if (player == nullptr) player = this;
    deathTrigger->PostEventMS(&EV_Activate, 0, player);
    deathTrigger = nullptr;
  }
}
// DIA: idAI2::ApplyDeathAnimationLogic
idRecoveredValue idAI2::ApplyDeathAnimationLogic(...) { return RecordCoreCall("ApplyDeathAnimationLogic"); }
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$0
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$1
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$2
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$3
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$4
// DIA: `idAI2::ApplyDeathAnimationLogic'::`1'::dtor$5
// DIA: idAI2::DamageFeedback
idRecoveredValue idAI2::DamageFeedback(...) { return RecordCoreCall("DamageFeedback"); }
// DIA: idAI2::GetMeleeTypeForTarget
idRecoveredValue idAI2::GetMeleeTypeForTarget(...) { return RecordCoreCall("GetMeleeTypeForTarget"); }
// DIA: idAI2::HandleAffirmativeResponse
idRecoveredValue idAI2::HandleAffirmativeResponse(...) { return RecordCoreCall("HandleAffirmativeResponse"); }
// DIA: idAI2::GetSuspectedEnemyPos_PushedIntoAAS
idRecoveredValue idAI2::GetSuspectedEnemyPos_PushedIntoAAS(...) { return RecordCoreCall("GetSuspectedEnemyPos_PushedIntoAAS"); }
// DIA: idAI2::AwareOfEnemy
bool idAI2::AwareOfEnemy() const { return currentEnemy != nullptr; }
// DIA: idAI2::IsBlockedByFriendly
idRecoveredValue idAI2::IsBlockedByFriendly(...) { return RecordCoreCall("IsBlockedByFriendly"); }
// DIA: idAI2::IsAASBlocked
idRecoveredValue idAI2::IsAASBlocked(...) { return RecordCoreCall("IsAASBlocked"); }
// DIA: idAI2::GetFakeEnemyWeight
float idAI2::GetFakeEnemyWeight() {
  const auto* fakeEnemyTarget = dynamic_cast<const idTarget_FakeEnemy*>(fakeEnemy);
  return fakeEnemyTarget != nullptr ? fakeEnemyTarget->weight : 0.0F;
}
// DIA: idAI2::OnAIEvent_Investigate
idRecoveredValue idAI2::OnAIEvent_Investigate(...) { return RecordCoreCall("OnAIEvent_Investigate"); }
// DIA: idAI2::OnAIEvent_Projectile
idRecoveredValue idAI2::OnAIEvent_Projectile(...) { return RecordCoreCall("OnAIEvent_Projectile"); }
// DIA: idAI2::OnAIEvent_PlayerAim
idRecoveredValue idAI2::OnAIEvent_PlayerAim(...) { return RecordCoreCall("OnAIEvent_PlayerAim"); }
// DIA: idAI2::ForceStopOverrideAnim
void idAI2::ForceStopOverrideAnim(
    const idAnimWebEvent::priority_t priority, int blendDurationMs) {
  (void)priority;
  idAnimator_AnimWeb* animator = animWebAnimators[AIANIMWEB_BODY].get();
  if (animator->IsAnyEventPending()) {
    idAnimWebIndex destSubWebIndex{-1};
    idAnimWebIndex destStateIndex{-1};
    if (animator->GetDestinationIndices(&destSubWebIndex,
                                        &destStateIndex)) {
      animator->ClearEvent(ANIMWEB_EVENT_START_BLEND_IN);
      blendParms_t blendParms;
      const int millisecondsPerFrame = gameLocal->GetGameMsPerFrame();
      const float durationFrames = std::floor(
          static_cast<float>(blendDurationMs) /
                  static_cast<float>(millisecondsPerFrame) +
              0.5F);
      blendParms.SetDuration(static_cast<int>(durationFrames));
      blendParms.SetDestStartFrame(0);
      animator->ForceState(destSubWebIndex, destStateIndex, blendParms, 0);
    }
  }
}
// DIA: idAI2::ClearSyncedAnimData
void idAI2::ClearSyncedAnimData() {
  syncedOverrideAnimation = 0;
  syncedTargetSpawnId = 0x1FFF;
}
// DIA: idAI2::IsOverrideAnimSupported
bool idAI2::IsOverrideAnimSupported(const overrideAnim_t anim) const {
  if (anim == ANIMOVERRIDE_NONE) return false;

  const overrideWebInfo_t& webInfo =
      overrideWebInfo[static_cast<std::size_t>(anim)];
  idStr subWebName;
  GetSubWebPrefix(webInfo.subWeb, &subWebName);
  const idDeclAnimWeb* declaration =
      animWebAnimators[AIANIMWEB_BODY]->GetDeclaration();
  const animWebSubWebIndex_t subWebIndex =
      declaration->GetSubWebIndexForName(subWebName.data);
  if (subWebIndex.value < 0) return false;

  const animWebStateIndex_t stateIndex =
      declaration->GetStateIndexForName(webInfo.stateName);
  if (stateIndex.value < 0) return false;

  return GetAnimationPathReachability(
      idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t>{subWebIndex.value},
      idIndex<short, idDeclAnimWeb::invalidStateIndex_t>{stateIndex.value});
}
// DIA: idAI2::WaitForOverrideAnim
bool idAI2::WaitForOverrideAnim(
    const idAnimWebEvent::priority_t priority,
    const overrideAnim_t overrideAnimation, const animWebEvent_t eventType,
    const bool force, const bool randomizeAnimation,
    const idEntity* syncedTarget) {
  (void)force;
  const overrideWebInfo_t& webInfo =
      overrideWebInfo[static_cast<std::size_t>(overrideAnimation)];
  idAnimator_AnimWeb* bodyAnimator =
      animWebAnimators[AIANIMWEB_BODY].get();
  idAnimWebIndex destinationSubWeb = bodyAnimator->GetCurrentSubWebIndex();
  idAnimWebIndex destinationState = bodyAnimator->GetCurrentStateIndex();

  if (webInfo.destinationSubWeb != aiSubWeb_t::max) {
    idStr destinationSubWebName;
    GetSubWebPrefix(webInfo.destinationSubWeb, &destinationSubWebName);
    const idDeclAnimWeb* declaration = bodyAnimator->GetDeclaration();
    const animWebSubWebIndex_t destinationSubWebIndex =
        declaration->GetSubWebIndexForName(destinationSubWebName.data);
    destinationSubWeb.value = destinationSubWebIndex.value;

    const idAIMemory::aiMemoryAnimAfterOverride animAfterOverride =
        memory->GetAnimAfterOverride();
    if (animAfterOverride ==
        idAIMemory::AIMEMORY_ANIMAFTER_OVERRIDE_DEFAULT) {
      destinationState.value =
          declaration->GetStateIndexForName(webInfo.destinationStateName)
              .value;
    } else if (animAfterOverride ==
               idAIMemory::AIMEMORY_ANIMAFTER_OVERRIDE_IDLE) {
      destinationState.value =
          declaration->GetStateIndexForName("idle").value;
    } else if (animAfterOverride ==
               idAIMemory::AIMEMORY_ANIMAFTER_OVERRIDE_RUN) {
      destinationState.value =
          declaration->GetStateIndexForName("run").value;
    }
    memory->SetAnimAfterOverride(
        idAIMemory::AIMEMORY_ANIMAFTER_OVERRIDE_DEFAULT);

    if (destinationSubWeb.value < 0) {
      idLib::Warning(
          "idAI2::WaitForOverrideAnim - subweb '%s' not found in animWeb "
          "'%s'",
          destinationSubWebName.data, declaration->name.str);
      return false;
    }
    if (destinationState.value < 0) {
      idLib::Warning(
          "idAI2::WaitForOverrideAnim - state '%s' not found in animWeb "
          "'%s'",
          webInfo.destinationStateName, declaration->name.str);
      return false;
    }
  }

  idStr overrideSubWebName;
  GetSubWebPrefix(webInfo.subWeb, &overrideSubWebName);
  const idDeclAnimWeb* declaration = bodyAnimator->GetDeclaration();
  const animWebSubWebIndex_t overrideSubWebIndex =
      declaration->GetSubWebIndexForName(overrideSubWebName.data);
  const animWebStateIndex_t overrideStateIndex =
      declaration->GetStateIndexForName(webInfo.stateName);
  if (overrideSubWebIndex.value < 0) {
    idLib::Warning(
        "idAI2::WaitForOverrideAnim - subweb '%s' not found in animWeb '%s'",
        overrideSubWebName.data, declaration->name.str);
    return false;
  }
  if (overrideStateIndex.value < 0) {
    idLib::Warning(
        "idAI2::WaitForOverrideAnim - state '%s' not found in animWeb '%s'",
        webInfo.stateName, declaration->name.str);
    return false;
  }

  const idAnimWebIndex overrideSubWeb{overrideSubWebIndex.value};
  const idAnimWebIndex overrideState{overrideStateIndex.value};
  const int pathResult = bodyAnimator->ChangeStateVia(
      destinationSubWeb, destinationState, overrideSubWeb, overrideState, 1, 0,
      0);
  if (pathResult < 1) return false;
  if (pathResult == 1) {
    if (randomizeAnimation) {
      const animWebNodeIndex_t overrideNodeIndex =
          declaration->FindNodeIndex(overrideSubWebIndex, overrideStateIndex);
      if (overrideNodeIndex.value >= 0) {
        const idDeclAnimWebNode* overrideNode =
            declaration->nodes[overrideNodeIndex.value];
        if (overrideNode != nullptr) {
          int animationIndex = 0;
          const int animationCount =
              overrideNode->NumAnims(animWebModelIndex_t{0});
          if (animationCount != 0) {
            const std::uint32_t seed =
                1664525U * gameLocal->GetRandomSeed() + 1013904223U;
            gameLocal->SetRandomSeed(seed);
            animationIndex =
                static_cast<int>((seed >> 10U) & 0x7FFFU) % animationCount;
          }
          bodyAnimator->SetSelect01ForTest(
              std::min(1.0F, static_cast<float>(animationIndex)));
          bodyAnimator->SetMeleeAttackForTest(
              static_cast<float>(animationIndex));
        }
      }
    }

    bodyAnimator->SetEvent(eventType, overrideSubWeb, overrideState,
                           &EV_AnimWeb_Wait, priority);
    if (syncedTarget != nullptr) {
      bodyAnimator->SetEvent(ANIMWEB_EVENT_START_BLEND_IN, overrideSubWeb,
                             overrideState, &EV_AnimWeb_BeginSyncedAnim,
                             priority);
      syncedOverrideAnimation = static_cast<int>(overrideAnimation);
      syncedTargetSpawnId = syncedTarget->GetEntityNumber();
    }
  }
  return true;
}
// DIA: idAI2::SetLookFocusEntity
void idAI2::SetLookFocusEntity(const idEntity* entity,
                               bool keepFocusInView, aimPoint_t aimPoint,
                               int timeout) {
  if (!robotState.usesRobot || endBlindedTime == 0) {
    focusInfos[1]->SetFocusEntity(reinterpret_cast<idAnimatedEntity*>(this),
                                  entity, keepFocusInView, aimPoint, timeout);
    lastLookedAtEntity = entity;
    lastLookedAtTime = gameLocal->GetGameTimeMs();
  }
}
// DIA: idAI2::TestFireWeapon
bool idAI2::TestFireWeapon(idWeapon* weapon, idFireParms& fireParms,
                           idTestFireResults& testFireResults,
                           const testFireWeaponParms_t& testParms) {
  fireParms.Clear();
  testFireResults.numTraces = 0;
  testFireResults.useOverrideFxDir = false;
  fireParms.weapon = weapon;
  fireParms.freeAmmo = testParms.freeAmmo;

  if (weapon->triggerState == idWeapon::TRIGGERSTATE_RELEASED) {
    AimDebug("TR", 1);
    return false;
  }

  idVec3 muzzlePosition;
  idMat3 muzzleAxis;
  if (!GetMuzzleTransform(weapon, &muzzlePosition, &muzzleAxis)) {
    idLib::Warning(
        "idAI2::TestFireWeapon - could not determine weapon muzzle point.");
    if (robotState.usesRobot) robotState.lastShootTime = -1;
    return false;
  }

  if (memory->GetFriendlyBlocker() != nullptr) return false;

  const int currentTime =
      gameLocal != nullptr ? gameLocal->GetGameTimeMs() : 0;
  idFocusInfo* aimFocus = focusInfos[0].get();
  idEntity* focusEntity = aimFocus->GetFocusEntity();

  fireParms.muzzleOffset = muzzleAxis[0] * 26.0F;
  muzzlePosition -= fireParms.muzzleOffset;
  if ((testFireMuzzleContents & 1U) != 0U) {
    AimDebug("muzzle in solid", 1);
    if (robotState.usesRobot) robotState.lastShootTime = -1;
    return false;
  }

  idVec3 targetPosition = aimFocus->GetCurrentFocusPoint();
  if (focusEntity != nullptr) {
    if (focusCompensationTime >= 0.0F) {
      const int spottedDuration =
          GetSpottedDuration(focusEntity, currentTime);
      if (spottedDuration > 0) {
        if (focusCompensationTime == 0.0F) {
          targetPosition = aimFocus->GetIdealFocusPoint();
        } else {
          const float compensation = std::min(
              (static_cast<float>(spottedDuration) * 0.001F) /
                  focusCompensationTime,
              1.0F);
          targetPosition +=
              (aimFocus->GetIdealFocusPoint() - targetPosition) * compensation;
        }
      }
    }
    if (predictTargetMovement) {
      const float frameSeconds =
          (gameLocal != nullptr ? gameLocal->GetGameMsPerFrame() : 16) *
          0.001F;
      targetPosition += focusEntity->GetLinearVelocity() * frameSeconds;
    }
  }

  float requiredAimDot = testFireAimDot;
  if (muzzleAxis[2].z < 0.0F) {
    requiredAimDot = testFireAimDotUpsideDown;
  }
  if (memory->IsGangstaPoseEnabled()) {
    requiredAimDot = testFireGangstaAimDot;
  }

  const idVec3 muzzleToTarget = targetPosition - muzzlePosition;
  const float targetDistanceSquared = muzzleToTarget.LengthSqr();
  const idVec3 targetDirection = NormalizedOrZero(muzzleToTarget);
  const float muzzleDirectionDot = muzzleAxis[0] * targetDirection;

  idMat3 fireAxis;
  if (!testParms.useMuzzleDirForFireAxis) {
    const float alwaysAimingDistanceSquared =
        testParms.alwaysAimingDistance * testParms.alwaysAimingDistance;
    if (targetDistanceSquared > alwaysAimingDistanceSquared &&
        requiredAimDot > muzzleDirectionDot) {
      AimDebug("muzzle direction", 1);
      if (robotState.usesRobot) robotState.lastShootTime = -1;
      return false;
    }
    fireAxis = GetPhysics()->GetAxis();
    fireAxis[0] = targetDirection;
    OrthoNormalizeFireAxis(fireAxis);
  } else {
    if (!testParms.ignoreAimFocus &&
        requiredAimDot > muzzleDirectionDot) {
      AimDebug("muzzle direction", 1);
      if (robotState.usesRobot) robotState.lastShootTime = -1;
      return false;
    }
    fireAxis = muzzleAxis;
  }

  const bool useWeaponSpread = testParms.useWeaponSpread;
  if (fireMissTime != 0.0F && fireControl != nullptr &&
      (fireControl->SetMissTimeTarget(this, focusEntity, currentTime),
       fireControl->UseMissTime(this, focusEntity, currentTime))) {
    const float missTimeFraction =
        fireControl->GetMissTimeFractionRemaining(this, currentTime);
    const idVec3 missPosition = GetMissTimeAimPosition(
        focusEntity, muzzlePosition, fireAxis, missTrackAlongGround,
        missTimeFraction);
    fireAxis = NormalizedOrZero(missPosition - muzzlePosition).ToMat3();
  } else if (useWeaponSpread) {
    float spreadModifier = 1.0F;
    if (!robotState.usesRobot && !usesDrone &&
        GetLinearVelocity().LengthSqr() > 100.0F) {
      spreadModifier =
          GetWalkState() < WALKSTATE_RUNNING ? 1.25F : 1.5F;
    }
    weapon->AddSpreadToFireAxis(
        fireAxis, static_cast<aiAccuracy_t>(GetEffectiveAccuracy()),
        spreadModifier, fireAxis);
  }

  weapon->DeferredFire(
      GetPresentable(), nullptr, muzzlePosition, fireAxis, fireParms,
      weapon->weaponState == idWeapon::WEAPONSTATE_SECONDARY);

  // Retail checks the freshly-cleared trace count here. DeferredFire does not
  // receive testFireResults, so this loop has no iterations in this function.
  AimDebug("TestFire passed deferred fire submitted", 1);
  return true;
}
// DIA: idAI2::TestForBlockedTrace
testForBlockedTraceResult_t idAI2::TestForBlockedTrace(
    const idFireParms& fireParms,
    const idTestFireResults& testFireResults,
    const testForBlockedTraceParms_t& testParms) const {
  testForBlockedTraceResult_t result = TEST_BLOCKED_TRACE_CLEAR;
  const float maximumBlockingDistance = testParms.maxBlockingDistance + 26.0F;
  const float targetDistance = std::sqrt(std::max(
      (focusInfos[0]->GetCurrentFocusPoint() - fireParms.start).LengthSqr(),
      idMath::FLT_SMALLEST_NON_DENORMAL));

  for (int traceIndex = 0; traceIndex < testFireResults.numTraces;
       ++traceIndex) {
    const multiHitTrace_t& multiTrace =
        testFireResults.multiTraces[static_cast<std::size_t>(traceIndex)];
    const float traceLength =
        testFireResults.traceLens[static_cast<std::size_t>(traceIndex)];
    for (int hitIndex = 0; hitIndex < multiTrace.numHits; ++hitIndex) {
      const trace_t& hit =
          multiTrace.hits[static_cast<std::size_t>(hitIndex)];
      if (hit.fraction >= 1.0F) continue;

      idEntity* hitEntity = gameLocal->GetEntityByNumber(hit.contact.entityNum);
      if (hitEntity == nullptr) continue;

      if (fireParms.target != nullptr &&
          fireParms.target->GetEntityNumber() ==
              hitEntity->GetEntityNumber()) {
        return TEST_BLOCKED_TRACE_CLEAR;
      }

      if (hit.contact.surfaceType == 7 || hit.contact.surfaceType == 6 ||
          hitEntity->IsWeaponStaticProp()) {
        continue;
      }

      idPlayer* hitPlayer = dynamic_cast<idPlayer*>(hitEntity);
      if (hitPlayer != nullptr && hitPlayer->GetVehicle() != nullptr &&
          fireParms.target != nullptr &&
          hitPlayer->GetVehicle()->GetEntityNumber() ==
              fireParms.target->GetEntityNumber()) {
        return TEST_BLOCKED_TRACE_CLEAR;
      }

      const float hitDistance = traceLength * hit.fraction;
      if (hitDistance >= targetDistance) continue;
      if (hitDistance <= maximumBlockingDistance) {
        if (ai_debugLevel.valueInteger >= 4 && gameLocal != nullptr &&
            gameLocal->GetRenderWorld() != nullptr) {
          idRenderDebug* renderDebug =
              gameLocal->GetRenderWorld()->GetRenderDebug();
          if (renderDebug != nullptr) {
            renderDebug->DebugArrow(idColor{1.0F, 1.0F, 1.0F, 1.0F},
                                    fireParms.start,
                                    hit.endpos, 4.0F, 1000, false);
          }
        }
        return TEST_BLOCKED_TRACE_CLOSE;
      }
      result = TEST_BLOCKED_TRACE_DISTANT;
    }
  }
  return result;
}
// DIA: idAI2::GetFireAtLastKnownDuration
void idAI2::GetFireAtLastKnownDuration(const idWeapon* weapon, int& minimum,
                                       int& maximum) {
  (void)weapon;
  const int duration = static_cast<int>(GetSpotLostFireTime() * 1000.0F);
  minimum = duration;
  maximum = duration;
}
// DIA: idAI2::GetEnemySenseState
const aiSenseState_t* idAI2::GetEnemySenseState(
    aiSense_t senseType) const {
  idAIEntityState* entityState = worldState->StateForEntity(currentEnemy);
  if (currentEnemy == nullptr || entityState == nullptr) {
    return nullptr;
  }
  if (senseType == AISENSE_MAX) {
    return entityState->senses[entityState->lastSense];
  }
  return entityState->senses[static_cast<unsigned char>(senseType)];
}
// DIA: idAI2::IsRecentlySpotted
bool idAI2::IsRecentlySpotted(const idEntity* enemy, float time) const {
  return IsRecentlySpotted(worldState->StateForEntity(enemy), time);
}
// DIA: idAI2::UpdateAimFocus
void idAI2::UpdateAimFocus() {
  idFocusInfo* focus = focusInfos[0].get();
  auto* focusOwner = reinterpret_cast<idAnimatedEntity*>(this);
  focus->SetOffset({});

  const int currentTime = gameLocal != nullptr ? gameLocal->GetGameTimeMs() : 0;
  if (focus->GetFocusTimeout() != -1 &&
      currentTime >= focus->GetFocusTimeout()) {
    focus->Clear(focusOwner);
  }

  idEntity* focusEntity = focus->GetFocusEntity();
  if (focusEntity == nullptr) {
    if (!focus->HasExplicitFocusPoint()) {
      const idVec3 eyePosition =
          GetPhysics()->GetOrigin() + GetEyeOffset();
      focus->SetIdealFocusPoint(
          eyePosition + bodyOrientation->GetAxis()[0] * 64.0F);
      focus->SetOffset({});
    }
    return;
  }

  focus->SetFocusType(AIFOCUS_ENTITY);
  bool controlledStaticWeapon = false;
  if (const auto* player = dynamic_cast<const idPlayer*>(focusEntity)) {
    controlledStaticWeapon =
        player->GetControl() != nullptr &&
        player->GetControl()->IsWeaponStaticProp();
  }

  idAIEntityState* entityState = worldState->StateForEntity(focusEntity);
  if (entityState == nullptr) {
    idVec3 focusPoint;
    if (focus->GetAimPointSelection() == AIMPOINT_EYELEVEL) {
      focusPoint = focusEntity->GetPhysics()->GetOrigin();
      focusPoint.z = GetPhysics()->GetOrigin().z + GetEyeOffset().z;
    } else {
      focusEntity->GetAimPoint(focus->GetAimPointSelection(), &focusPoint);
    }
    focus->SetTempFocusPoint(focusOwner, focusPoint, AIFOCUS_MISC);
    focus->SetOffset({});
    return;
  }

  aiSenseState_t* sense =
      entityState->senses[static_cast<std::size_t>(entityState->lastSense)];
  if (sense->lastUnconfirmedStimTime < 0 ||
      sense->lastConfirmedStimTime <= 0) {
    focus->Clear(focusOwner);
    return;
  }

  const idVec3 aiOrigin = GetPhysics()->GetOrigin();
  if (!entityState->IsVisible()) {
    aiSenseState_t* sight = entityState->senses[AISENSE_SIGHT];
    if (sight->lastUnconfirmedStimTime >= 0 &&
        sight->lastConfirmedStimTime >= 0) {
      if (OkToFocus(&sight->GetConfirmedPhysicalSafe()->eyePos, &aiOrigin,
                    &sense->GetConfirmedPhysicalSafe()->origin)) {
        focus->SetTempFocusPoint(
            focusOwner, sight->GetConfirmedPhysicalSafe()->eyePos,
            AIFOCUS_LAST_SEEN_POS);
      } else if (OkToFocus(&sight->aiOrigin, &aiOrigin,
                           &sense->GetConfirmedPhysicalSafe()->origin)) {
        idVec3 focusPoint = sight->aiOrigin;
        focusPoint.z += GetEyeOffset().z;
        focus->SetTempFocusPoint(focusOwner, focusPoint,
                                 AIFOCUS_POS_WHEN_LAST_SEEN);
      }
    }

    if (focus->GetFocusType() == AIFOCUS_ENTITY) {
      const aiPhysicalState_t* confirmed = sense->GetConfirmedPhysicalSafe();
      if (OkToFocus(&confirmed->origin, &aiOrigin, &confirmed->origin)) {
        idVec3 focusPoint = confirmed->eyePos;
        if (focus->GetAimPointSelection() == AIMPOINT_EYELEVEL) {
          focusPoint.z = aiOrigin.z + GetEyeOffset().z;
        }
        focus->SetTempFocusPoint(focusOwner, focusPoint,
                                 AIFOCUS_LAST_SENSED_POS);
      } else if (OkToFocus(&sense->aiOrigin, &aiOrigin,
                           &confirmed->origin)) {
        const idVec3 focusPoint = sense->aiOrigin + GetEyeOffset();
        focus->SetTempFocusPoint(focusOwner, focusPoint,
                                 AIFOCUS_POS_WHEN_LAST_SENSED);
      } else {
        focus->Clear(focusOwner);
      }
    }
  }

  if (focus->GetFocusType() != AIFOCUS_ENTITY) return;

  const aimPoint_t aimPoint = focus->GetAimPointSelection();
  if (aimPoint < AIMPOINT_ORIGIN) return;

  idVec3 focusPoint;
  if (aimPoint <= AIMPOINT_FEET || aimPoint == AIMPOINT_LOOKAHEAD) {
    entityState->GetAimPoint(entityState->lastSense, aimPoint, &focusPoint);
    focus->SetIdealFocusPoint(focusPoint);
    focus->SetOffset({});
    return;
  }

  switch (aimPoint) {
    case AIMPOINT_BEST: {
      aimPoint_t selectedPoint = AIMPOINT_TORSO;
      if (controlledStaticWeapon || focusEntity->IsDead()) {
        selectedPoint = AIMPOINT_HEAD;
      } else if (entityState->lastVisible == VISIBLE_IN_COVER &&
                 (GetPhysics()->GetOrigin() -
                  focusEntity->GetPhysics()->GetOrigin()).LengthSqr() <=
                     22500.0F) {
        selectedPoint = AIMPOINT_HEAD;
      }

      idWeapon* equippedWeapon = GetEquippedWeapon();
      if (equippedWeapon != nullptr && equippedWeapon->ShouldAimAtHead()) {
        selectedPoint = AIMPOINT_HEAD;
      }

      entityState->GetAimPoint(entityState->lastSense, selectedPoint,
                               &focusPoint);
      focus->SetIdealFocusPoint(focusPoint);
      const float aimOffset = selectedPoint == AIMPOINT_TORSO
                                  ? behaviorDeclaration->aimOffset
                                  : 0.0F;
      focus->SetOffset({0.0F, 0.0F, aimOffset});
      return;
    }
    case AIMPOINT_EYELEVEL: {
      const aiPhysicalState_t* confirmed = sense->GetConfirmedPhysicalSafe();
      focusPoint.x = confirmed->origin.x;
      focusPoint.y = confirmed->origin.y;
      focusPoint.z = aiOrigin.z + GetEyeOffset().z;
      focus->SetIdealFocusPoint(focusPoint);
      focus->SetOffset({});
      return;
    }
    default:
      return;
  }
}
// DIA: idAI2::SetupRobotDead
void idAI2::SetupRobotDead() {
  if (robotDeathSetup) return;

  const int gameTime = gameLocal->GetGameTimeMs();
  if (robotTimeToCritical <= 0.0F) {
    if (robotStopEffectsDelay > 0.0F && usesDrone) {
      const int stopTime =
          gameTime + static_cast<int>(robotStopEffectsDelay * 1000.0F);
      robotDeathFXManager->StopAllFX(stopTime, false);
    }
  } else {
    robotCriticalTimestamp =
        gameTime - static_cast<int>(robotTimeToCritical * -1000.0F);
    robotExplodeTimestamp = robotCriticalTimestamp;
    if (robotTimeCriticalToExplode >= 0.0F) {
      robotExplodeTimestamp -=
          static_cast<int>(robotTimeCriticalToExplode * -1000.0F);
    }
    robotDeadTimestamp = robotExplodeTimestamp + 3000;

    if (usesDrone) {
      const idMat3 bodyAxis = bodyOrientation->GetAxis();
      SetPresentableAxis(bodyAxis);
      const idVec3 velocity = GetPhysics()->GetLinearVelocity(0);
      robotDeathDroneAnimator->Update(this, bodyAxis, velocity);
    }

    robotDeathFXManager->StartFX(
        &vec3_origin, &mat3_identity, gameTime,
        static_cast<fxCondition_t>(0x62));
  }

  if (deathTrigger != nullptr) {
    deathTrigger->PostEventMS(&EV_Activate, 0, this);
    deathTrigger = nullptr;
  }
  robotDeathSetup = true;
}
// DIA: idAI2::FindClosestFocus
bool idAI2::FindClosestFocus() {
  const int debugDuration = gameLocal->GetGameMsPerFrame();
  const idEntity* neutral = memory->GetCurrentNeutral();
  if (neutral == nullptr) return false;

  idAIEntityState* neutralState = worldState->StateForEntity(neutral);
  if (neutralState == nullptr) return false;
  const aiSenseState_t* sightState = neutralState->senses[AISENSE_SIGHT];
  if (sightState->lastUnconfirmedStimTime < 0 ||
      gameLocal->GetGameTimeMs() - sightState->lastConfirmedStimTime >=
          5000) {
    return false;
  }

  idPhysics* physics = GetPhysics();
  idPhysics* neutralPhysics = neutral->GetPhysics();
  const idVec3 selfOrigin = physics->GetOrigin();
  const idVec3 neutralOrigin = neutralPhysics->GetOrigin();
  const idVec3 toNeutral = neutralOrigin - selfOrigin;
  const float distanceSquared = toNeutral.LengthSqr();
  const char* debugText = nullptr;

  if (distanceSquared <= 16384.0F) {
    const idVec3 neutralDirection = NormalizedOrZero(toNeutral);
    const char* closeText = nullptr;

    if (moveState->GetMoveStatus() == MOVE_STATUS_MOVING) {
      const idVec3 velocityDirection =
          NormalizedOrZero(physics->GetLinearVelocity(0));
      if ((velocityDirection * neutralDirection) > -0.70710671F) {
        const float rayDistance = toNeutral * velocityDirection;
        if (rayDistance >= -16.0F && rayDistance <= 64.0F) {
          const idVec3 closestPosition =
              selfOrigin + velocityDirection * rayDistance;
          const float closestDistance = std::sqrt(std::max(
              (neutralOrigin - closestPosition).LengthSqr(),
              std::numeric_limits<float>::min()));
          if (closestDistance <= 48.0F) {
            closeText = "squeeze by";
          } else {
            debugText = "moving close";
          }
        }
      }
    } else if ((neutralDirection * (*GetPresentableAxis())[0]) >=
               -0.49999994F) {
      closeText = "standing close";
    }

    if (closeText != nullptr) {
      SetLookFocusEntity(neutral, false, AIMPOINT_HEAD, debugDuration - 1);
      faceManager->SetMood(faceManager->GetDefaultMood());

      if (!greetingsEnabled) {
        LookDebug(closeText, 1, debugDuration);
        return true;
      }

      if ((neutralPhysics->GetOrigin() - physics->GetOrigin()).LengthSqr() >
          16384.0F) {
        LookDebug(closeText, 1, debugDuration);
        return true;
      }

      if (const auto* actor = dynamic_cast<const idActor*>(neutral)) {
        const actorClass_t actorClass = actor->GetActorClass();
        if ((actorClass == ACTORCLASS_PLAYER &&
             (behaviorDeclaration == nullptr ||
              !behaviorDeclaration->canGreetPlayers)) ||
            (actorClass == ACTORCLASS_HUMAN &&
             (behaviorDeclaration == nullptr ||
              !behaviorDeclaration->canGreetHumans))) {
          LookDebug(closeText, 1, debugDuration);
          return false;
        }
      }

      idRelationship::attitude_t attitude =
          idRelationship::ATTITUDE_NEUTRAL;
      if (GetFaction() != nullptr) {
        attitude = GetFaction()->GetAttitudeTowards(neutral);
      }
      const idRelationship::attitudeStage_t attitudeStage =
          idRelationship::AttitudeToAttitudeStage(attitude);
      mood_t mood = MOOD_ANGRY;
      if (attitudeStage == idRelationship::ATTITUDESTAGE_ACCEPTED) {
        mood = MOOD_CLEAR;
      } else if (attitudeStage == idRelationship::ATTITUDESTAGE_HEROIC) {
        mood = MOOD_HAPPY;
      }
      faceManager->SetMood(mood);
      LookDebug("mood change", 1, debugDuration);
      return true;
    }
  }

  faceManager->SetMood(faceManager->GetDefaultMood());
  if (debugText != nullptr) {
    LookDebug(debugText, 1, debugDuration);
  }
  return false;
}
// DIA: `idAI2::FindClosestFocus'::`1'::dtor$0
// DIA: idAI2::GetAlertCycle
alertCycle_t idAI2::GetAlertCycle() const {
  return controllingPlayer != nullptr
             ? ALERTCYCLE_COMBAT
             : static_cast<alertCycle_t>(aiEventState.alertCycle);
}
// DIA: idAI2::GetAlertCycleDuration
int idAI2::GetAlertCycleDuration() const {
  if (controllingPlayer != nullptr || alertCycleTimestamp < 0) {
    return -1;
  }
  return gameLocal->GetGameTimeMs() - alertCycleTimestamp;
}
// DIA: idAI2::LeaveActionNode
void idAI2::LeaveActionNode(bool force) {
  (void)force;
  if (idActionNode* actionNode = idActionNode::CastTo(currentActionNode)) {
    actionNode->LeaveActionNode(this);
  }
}
// DIA: idAI2::UsesSmoothAnimations
bool idAI2::UsesSmoothAnimations() const {
  const int alertCycle = controllingPlayer != nullptr ? 3 : aiEventState.alertCycle;
  if (alertCycle == 1) return usesSmoothAnimRelaxed;
  if (alertCycle == 2) return usesSmoothAnimSearch;
  if (alertCycle == 3) return usesSmoothAnimCombat;
  return false;
}
// DIA: idAI2::ActivateStealthKillTrigger
void idAI2::ActivateStealthKillTrigger() {
  const int currentTime = gameLocal->GetGameTimeMs();
  const int alarmTimestamp = gameLocal->GetAlarmTimestamp();
  if (alarmTimestamp < 0 || currentTime < alarmTimestamp + 1000) {
    if (stealthKillTrigger != nullptr) {
      stealthKillTrigger->PostEventMS(&EV_Activate, 0, this);
      stealthKillTrigger = nullptr;
    }
  }
}
// DIA: idAI2::GetAnimationPathReachability
bool idAI2::GetAnimationPathReachability(
    const idAnimWebPath& animPath) const {
  if (animPath.IsEmpty()) return false;

  idStr subWebName;
  idStr stateName;
  GetSubWebAndStateFromAnimWebPath(animPath.data, subWebName, stateName);

  const idDeclAnimWeb* declaration =
      animWebAnimators[AIANIMWEB_BODY]->GetDeclaration();
  const animWebSubWebIndex_t subWebIndex =
      declaration->GetSubWebIndexForName(subWebName.data);
  const animWebStateIndex_t stateIndex =
      declaration->GetStateIndexForName(stateName.data);
  if (subWebIndex.value < 0 || stateIndex.value < 0) return false;

  return GetAnimationPathReachability(
      idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t>{subWebIndex.value},
      idIndex<short, idDeclAnimWeb::invalidStateIndex_t>{stateIndex.value});
}
// DIA: `idAI2::GetAnimationPathReachability'::`1'::dtor$0
// DIA: `idAI2::GetAnimationPathReachability'::`1'::dtor$1
// DIA: idAI2::GetAnimationDelta
bool idAI2::GetAnimationDelta(const idAnimWebPath& animPath,
                              idVec3& outDelta,
                              const bool verifyAWPath) const {
  if (animPath.IsEmpty()) return false;

  idStr subWebName;
  idStr stateName;
  GetSubWebAndStateFromAnimWebPath(animPath.data, subWebName, stateName);

  const idDeclAnimWeb* declaration =
      animWebAnimators[AIANIMWEB_BODY]->GetDeclaration();
  const animWebSubWebIndex_t subWebIndex =
      declaration->GetSubWebIndexForName(subWebName.data);
  const animWebStateIndex_t stateIndex =
      declaration->GetStateIndexForName(stateName.data);
  if (subWebIndex.value < 0 || stateIndex.value < 0) return false;

  return GetAnimationDelta(
      idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t>{subWebIndex.value},
      idIndex<short, idDeclAnimWeb::invalidStateIndex_t>{stateIndex.value},
      outDelta, verifyAWPath);
}
// DIA: idAI2::GetAnimationRot
bool idAI2::GetAnimationRot(const idAnimWebPath& animPath,
                            idAngles& outRotation,
                            const bool verifyAWPath) const {
  if (animPath.IsEmpty()) return false;

  idStr subWebName;
  idStr stateName;
  GetSubWebAndStateFromAnimWebPath(animPath.data, subWebName, stateName);

  const idDeclAnimWeb* declaration =
      animWebAnimators[AIANIMWEB_BODY]->GetDeclaration();
  const animWebSubWebIndex_t subWebIndex =
      declaration->GetSubWebIndexForName(subWebName.data);
  const animWebStateIndex_t stateIndex =
      declaration->GetStateIndexForName(stateName.data);
  if (subWebIndex.value < 0 || stateIndex.value < 0) return false;

  return GetAnimationRot(
      idIndex<short, idDeclAnimWeb::invalidSubWebIndex_t>{subWebIndex.value},
      idIndex<short, idDeclAnimWeb::invalidStateIndex_t>{stateIndex.value},
      outRotation, verifyAWPath);
}
// DIA: idList<idPonytail,5>::~idList<idPonytail,5>
// DIA: idList<idCrosshairCustomIcon,5>::Resize
// DIA: idAnimatorParms_AnimWeb::idAnimatorParms_AnimWeb
// DIA: idFSMManager::idFSMStack::idFSMStack
// DIA: idActor::SetLastDamageInfo
void idActor::SetLastDamageInfo(const idEntity* inflictor,
                                const idDeclDamage* damageDefinition) {
  lastDamageInflictor = inflictor;
  lastDamageDefinition = damageDefinition;
}
// DIA: idPlayer::GetProxyMove
idProxyMove* idPlayer::GetProxyMove() const {
  return dynamic_cast<idProxyMove*>(proxyMoveEntity);
}
// DIA: idProp_Moveable::SetOwner
void idProp_Moveable::SetOwner(idEntity* ownerEntity) {
  owner = ownerEntity;
}
// DIA: idAIMemory::ShouldAvoid
bool idAIMemory::ShouldAvoid(const int currentTime) {
  return avoidEntity != nullptr && currentTime < avoidFinishedTime &&
         avoidRadius != 0.0F;
}
// DIA: idAIMemory::SetAvoidEntity
void idAIMemory::SetAvoidEntity(const int currentTime,
                                const idEntity* entity, const float radius,
                                const float durationSeconds) {
  avoidEntity = entity;
  avoidRadius = radius;
  avoidFinishedTime =
      currentTime - static_cast<int>(durationSeconds * -1000.0F);
}
// DIA: idAIMemory::SetSpottedEnemyTrigger
void idAIMemory::SetSpottedEnemyTrigger(const idEntity* entity) {
  spottedEnemyTrigger = entity;
  spottedEnemyTriggerNumber =
      entity != nullptr ? entity->GetEntityNumber() : -1;
}
// DIA: idAIMemory::GetSpottedEnemyTrigger
const idEntity* idAIMemory::GetSpottedEnemyTrigger() const {
  if (spottedEnemyTrigger == nullptr) return nullptr;
  if (gameLocal == nullptr || spottedEnemyTriggerNumber < 0) {
    return spottedEnemyTrigger;
  }
  return gameLocal->GetEntityByNumber(spottedEnemyTriggerNumber) ==
                 spottedEnemyTrigger
             ? spottedEnemyTrigger
             : nullptr;
}
// DIA: idAIMemory::SetDetectedEnemyTrigger
void idAIMemory::SetDetectedEnemyTrigger(const idEntity* entity) {
  detectedEnemyTrigger = entity;
  detectedEnemyTriggerNumber =
      entity != nullptr ? entity->GetEntityNumber() : -1;
}
// DIA: idAIMemory::GetDetectedEnemyTrigger
const idEntity* idAIMemory::GetDetectedEnemyTrigger() const {
  if (detectedEnemyTrigger == nullptr) return nullptr;
  if (gameLocal == nullptr || detectedEnemyTriggerNumber < 0) {
    return detectedEnemyTrigger;
  }
  return gameLocal->GetEntityByNumber(detectedEnemyTriggerNumber) ==
                 detectedEnemyTrigger
             ? detectedEnemyTrigger
             : nullptr;
}
// DIA: idAIMemory::SetTriggeredInteraction
void idAIMemory::SetTriggeredInteraction(idPlayer* entity) {
  triggeredInteraction = entity;
  triggeredInteractionNumber =
      entity != nullptr ? entity->GetEntityNumber() : -1;
}
idPlayer* idAIMemory::GetTriggeredInteractionForTest() const {
  if (triggeredInteraction == nullptr) return nullptr;
  if (gameLocal == nullptr || triggeredInteractionNumber < 0) {
    return triggeredInteraction;
  }
  return gameLocal->GetEntityByNumber(triggeredInteractionNumber) ==
                 triggeredInteraction
             ? triggeredInteraction
             : nullptr;
}
// DIA: idSpawnFocusWithRequirement::idSpawnFocusWithRequirement
idSpawnFocusWithRequirement::idSpawnFocusWithRequirement() = default;
// DIA: idAIEditable::idAIActionSettings::Clear
void idAIEditable::idAIActionSettings::Clear() {
  usesTurrets = false;
  stationaryMode = false;
  noLowerBodyTurns = false;
  enemyTrailUpdatePeriod = 4.0F;
  isElite = false;
  isDualWielder = false;
  isCommander = false;
  isHeavy = false;
  isKampHund = false;
  isSuperSoldat = false;

  cover.usesCombatScenePoints = true;
  cover.usesClose = true;
  cover.usesIdle = true;
  cover.usesEmergencyExit = false;
  cover.dodgeMovingToScenepoint = false;
  cover.scenePointSelectionFlags = 3;
  cover.coverFlags = 0x36;
  cover.coverTargetMinRange = 200.0F;
  cover.coverMinRange = 50.0F;
  cover.coverMaxRange = 1000.0F;
  cover.coverRandomWeight = 0.1F;
  cover.coverHistoryWeight = 1.0F;
  cover.coverVerticalWeight = 1.0F;
  cover.coverTacticalWeight = 0.25F;
  cover.coverTargetTimeout = -1.0F;
  cover.returnToCoverDelay = 1.0F;

  melee.meleeMaxRange = 75.0F;
  melee.meleeLongMaxRange = 0.0F;
  melee.playerTakedownRange = 0.0F;
  melee.playerTakedownMaxHealth = 0.0F;
  melee.playerTakedownDecl = nullptr;
  melee.instantHitExcludeFromTrace = false;

  search.usesSearchScenePoints = true;
  search.searchMinRange = 50.0F;
  search.searchMaxRange = -1.0F;
  roam.usesRoamScenePoints = true;
  roam.roamMinRange = 50.0F;
  roam.roamMaxRange = -1.0F;
  roam.idleTalkPeriod = -1.0F;
  look.usesLookScenePoints = true;
  look.lookMinRange = 50.0F;
  look.lookMaxRange = -1.0F;
  look.minLookSPPeriod = 5.0F;
  look.combatTargetDelayLook = -1.0F;

  fire.aimAngle = 15.0F;
  fire.gangstaPoseFraction = 0.0F;
  fire.gangstaDuration = 3.0F;
  fire.maxFireRange = -1.0F;
  fire.usesGrenades = false;
  fire.fakeGrenadeMD6 = nullptr;
  fire.fakeGrenadeExplosion = nullptr;
  fire.secondaryFireRange = -1.0F;
  fire.useReloadAnimWhenMoving = false;
  fire.additiveFireAnims.ClearFree();
  fire.additiveFireAnimsCrouch.ClearFree();
  fire.additiveFireAnimBlendTime = 0.0F;
  fire.freeAmmoDistance = -1.0F;
  fire.goNutsRange = 0.0F;
  fire.safeReloadFraction = 0.0F;
  fire.aimDot = CosineDegrees(fire.aimAngle);
  fire.gangstaAimDot = CosineDegrees(fire.aimAngle + 10.0F);
  fire.aimDotUpsideDown = CosineDegrees(fire.aimAngle + 20.0F);
  fire.fireTiming.bUseFireTiming = false;
  fire.fireTiming.minBurstDuration = 1.0F;
  fire.fireTiming.maxBurstDuration = 3.0F;
  fire.fireTiming.minBurstPause = 0.2F;
  fire.fireTiming.maxBurstPause = 1.0F;
  fire.fireTiming.missTime = 0.0F;
  fire.fireTiming.missTimeMinRange = 300.0F;
  fire.fireTiming.missTrackAlongGround = false;
  fire.fireTiming.spotLostFire = 2.0F;
  fire.fireTiming.spotGainedFire = 0.0F;
  fire.fireTiming.minSpotLostRange = 75.0F;
  fire.fireTiming.predictTargetMovement = false;
  fire.fireTiming.focusCompensationTime = -1.0F;

  close.closeFlags = 1;
  close.closeMinRange = 0.0F;
  close.closeMaxRange = -1.0F;
  close.stopOnSpotRange = 500.0F;
  close.noSpotTimeBeforeWalking = 3.0F;
  close.maxRunRange = -1.0F;
  close.useEvasiveManeuvers = false;
  close.noStopOnSpotWhenBoreBlocked = false;
  close.evadeInterval = 1000;
  close.evadeMinDistance = 300;
  close.useOutOfReach = false;
  close.outOfReachGentlemanRange = 0.0F;
  close.periodicVoice.use = false;
  close.periodicVoice.minTime = 1000;
  close.periodicVoice.maxTime = 1000;

  dodge.dodgeFlags = 0xF6;
  dodge.dodgeMinRange = 100.0F;
  dodge.dodgeMaxRange = -1.0F;
  dodge.tauntMinRange = 100.0F;
  dodge.dodgePeriod = 2.0F;
  dodge.dodgeAnimPeriod = 4.0F;
  dodge.detectGrenadesInAir = false;
  taunt.tauntMinInterval = 4.0F;
  taunt.tauntMaxInterval = 7.0F;

  squad.squadLeader.value = 0x1FFF;
  squad.playerIsLeader = false;
  squad.leaderRange = 200.0F;
  squad.scenePointRange = 500.0F;
  squad.squadActive = true;
  outOfReach.distance = 32.0F;

  customRobot.usesRobot = false;
  customRobot.isBoss = true;
  customRobot.stageGroup.Set("battery");
  customRobot.stageHealth = 300.0F;
  customRobot.curStageHealth = 300.0F;
  customRobot.bossHealth = 1.0F;
  customRobot.chargeHealthFract = 0.15000001F;
  customRobot.chargeTime = 5.0F;
  customRobot.shootHealthDrain = 0.0F;
  customRobot.autoRecharge = false;
  customRobot.useRechargeAnim = false;
  customRobot.isCharging = false;
  customRobot.chargeStartTime = -1;
  customRobot.chargeDoneTime = -1;
  customRobot.numStages = 1;
  customRobot.triggerOnPowerOut.value = 0x1FFF;
  customRobot.triggerOnChargingDone.value = 0x1FFF;
  customRobot.curStage = 0;
  customRobot.noFireTime = -1;
  customRobot.immuneDoneTime = -1;
  customRobot.turnfastSound = nullptr;
  customRobot.lastBodyDir = idVec3{1.0F, 0.0F, 0.0F};
  customRobot.nextTurnfastTime = -1;
  customRobot.lastShootTime = -1;
  customRobot.nextMTSPCheck = -1;

  robotBlinding.bUsesBlindness = false;
  robotBlinding.blindedDuration = -1.0F;
  robotBlinding.reblindCooldown = 0.0F;
  robotBlinding.endBlindedTime = 0;
  robotBlinding.endReblindCooldown = 0;
  robotBlinding.blindedDamageGroup = nullptr;
  EMPStun.bUsesEMPStun = false;
  EMPStun.stunDuration = -1.0F;
  EMPStun.isEMPMagnetic = false;
  EMPStun.isNeverEMPMagnetic = false;
  EMPStun.endStunnedTime = 0;
  sleeping.bUsesSleeping = false;
  sleeping.sleepRadius = 100.0F;
  sleeping.isSleeping = false;

  robotDeath.timeToCritical = -1.0F;
  robotDeath.timeCriticalToExplode = -1.0F;
  robotDeath.explodeDamage = nullptr;
  robotDeath.stopEffectsDelay = 1.0F;
  robotDeath.criticalTimestamp = -1;
  robotDeath.explodeTimestamp = -1;
  robotDeath.deadTimeStamp = -1;
  robotDeath.deathSetup = false;

  tesla.usesTesla = false;
  tesla.useTeslaAnim = true;
  tesla.isHeavyGuardGun = false;
  tesla.teslaMeleeRange = 0.0F;
  tesla.teslaAASRange = 0.0F;
  tesla.teslaMinRange = 0.0F;
  tesla.teslaRange = 500.0F;
  tesla.teslaChargeTime = 5.0F;
  tesla.teslaCoolDownTime = 5.0F;
  tesla.teslaExplodeDelay = 0.25F;
  tesla.teslaMaxRangeDamageScale = 1.0F;
  tesla.useTeslaWhileBlinded = false;
  tesla.useTeslaBombGroundEffects = false;
  tesla.spawnTeslaBombInFOV = false;
  tesla.teslaTargetOffset = 64.0F;
  tesla.inhibitFireTime = 0.0F;
  tesla.teslaDamageDecl = nullptr;
  tesla.teslaBombDamageDecl = nullptr;
  tesla.teslaBombDangerZone = 0.0F;
  tesla.teslaBoltDecl = nullptr;
  tesla.teslaBoltFlickerInterval = 0.5F;
  tesla.teslaBlindFireDuration = 0.0F;
  tesla.teslaBombFxDecl = nullptr;
}
// DIA: idAI2::GetFakeEnemy
const idEntity* idAI2::GetFakeEnemy() const { return fakeEnemy; }
// DIA: idAIEditable::idAIActionSettings::~idAIActionSettings
// DIA: `idAIEditable::idAIActionSettings::~idAIActionSettings'::`1'::dtor$0
// DIA: idAIEditable::idAIDeath::idAIDeath
// DIA: `idAIEditable::idAIDeath::idAIDeath'::`1'::dtor$0
// DIA: `idAIEditable::idAIDeath::idAIDeath'::`1'::dtor$1
// DIA: `idAIEditable::idAIDeath::idAIDeath'::`1'::dtor$2
// DIA: idAIEditable::idAIDeath::~idAIDeath
// DIA: idAI2::UpdateGroupStatusConditions
idRecoveredValue idAI2::UpdateGroupStatusConditions(...) { return RecordCoreCall("UpdateGroupStatusConditions"); }
// DIA: idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo
idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo() = default;
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$0
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$1
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$2
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$3
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$4
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$5
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::idAIAnimationInfo'::`1'::dtor$6
// DIA: idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo
idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo() = default;
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$0
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$1
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$2
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$3
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$4
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$5
// DIA: `idAI2::idAIVolatile::idAIAnimationInfo::~idAIAnimationInfo'::`1'::dtor$6
// DIA: idAI2::idAIVolatile::idAIScriptInfo::idAIScriptInfo
idAI2::idAIVolatile::idAIScriptInfo::idAIScriptInfo() = default;
// DIA: idAI2::idAIVolatile::idAIScriptInfo::~idAIScriptInfo
idAI2::idAIVolatile::idAIScriptInfo::~idAIScriptInfo() = default;
// DIA: idAIDamageInfo::~idAIDamageInfo
// DIA: idAI2::idAIConstant::idAIConstant
idAI2::idAIConstant::idAIConstant() = default;
// DIA: `idAI2::idAIConstant::idAIConstant'::`1'::dtor$0
// DIA: `idAI2::idAIConstant::idAIConstant'::`1'::dtor$3
// DIA: idAI2::idAIConstant::~idAIConstant
idAI2::idAIConstant::~idAIConstant() = default;
// DIA: `idAI2::idAIConstant::~idAIConstant'::`1'::dtor$0
// DIA: `idAI2::idAIConstant::~idAIConstant'::`1'::dtor$2
// DIA: idAI2::DamageRobot
idRecoveredValue idAI2::DamageRobot(...) { return RecordCoreCall("DamageRobot"); }
// DIA: idAI2::ThinkRobot
idRecoveredValue idAI2::ThinkRobot(...) { return RecordCoreCall("ThinkRobot"); }
// DIA: idAI2::TurretShouldFire
idRecoveredValue idAI2::TurretShouldFire(...) { return RecordCoreCall("TurretShouldFire"); }
// DIA: idAI2::InitAnimation
idRecoveredValue idAI2::InitAnimation(...) { return RecordCoreCall("InitAnimation"); }
// DIA: `idAI2::InitAnimation'::`1'::dtor$0
// DIA: `idAI2::InitAnimation'::`1'::dtor$1
// DIA: `idAI2::InitAnimation'::`1'::dtor$11
// DIA: `idAI2::InitAnimation'::`1'::dtor$12
// DIA: `idAI2::InitAnimation'::`1'::dtor$13
// DIA: `idAI2::InitAnimation'::`1'::dtor$2
// DIA: `idAI2::InitAnimation'::`1'::dtor$16
// DIA: `idAI2::InitAnimation'::`1'::dtor$17
// DIA: `idAI2::InitAnimation'::`1'::dtor$18
// DIA: `idAI2::InitAnimation'::`1'::dtor$3
// DIA: `idAI2::InitAnimation'::`1'::dtor$4
// DIA: `idAI2::InitAnimation'::`1'::dtor$5
// DIA: `idAI2::InitAnimation'::`1'::dtor$6
// DIA: `idAI2::InitAnimation'::`1'::dtor$7
// DIA: `idAI2::InitAnimation'::`1'::dtor$8
// DIA: `idAI2::InitAnimation'::`1'::dtor$9
// DIA: `idAI2::InitAnimation'::`1'::dtor$10
// DIA: idAI2::UpdateTravelFlags
idRecoveredValue idAI2::UpdateTravelFlags(...) { return RecordCoreCall("UpdateTravelFlags"); }
// DIA: `idAI2::UpdateTravelFlags'::`1'::dtor$0
// DIA: idAI2::ShouldDeadThink
bool idAI2::ShouldDeadThink() {
  if (usesDrone || robotState.usesRobot) {
    return robotDeathSetup && robotTimeToCritical > 0.0F &&
           gameLocal->GetGameTimeMs() > droneLastDamageTick;
  }
  if (GetAlertCycle() != 6) return false;
  return gameLocal->GetGameTimeMs() - deathStartTime > 500;
}
// DIA: idAI2::UpdateEffects
idRecoveredValue idAI2::UpdateEffects(...) { return RecordCoreCall("UpdateEffects"); }
// DIA: idAI2::UpdateBodyOrientationWhileMoving
idRecoveredValue idAI2::UpdateBodyOrientationWhileMoving(...) { return RecordCoreCall("UpdateBodyOrientationWhileMoving"); }
// DIA: idAI2::StartStaggeringPain
idRecoveredValue idAI2::StartStaggeringPain(...) { return RecordCoreCall("StartStaggeringPain"); }
// DIA: idAI2::StartDeathSystem
idRecoveredValue idAI2::StartDeathSystem(...) { return RecordCoreCall("StartDeathSystem"); }
// DIA: `idAI2::StartDeathSystem'::`1'::dtor$2
// DIA: idAI2::KilledNotification
idRecoveredValue idAI2::KilledNotification(...) { return RecordCoreCall("KilledNotification"); }
// DIA: idAI2::Takedown
idRecoveredValue idAI2::Takedown(...) { return RecordCoreCall("Takedown"); }
// DIA: `idAI2::Takedown'::`1'::dtor$0
// DIA: `idAI2::Takedown'::`1'::dtor$1
// DIA: `idAI2::Takedown'::`1'::dtor$2
// DIA: `idAI2::Takedown'::`1'::dtor$3
// DIA: `idAI2::Takedown'::`1'::dtor$4
// DIA: `idAI2::Takedown'::`1'::dtor$5
// DIA: `idAI2::Takedown'::`1'::dtor$6
// DIA: `idAI2::Takedown'::`1'::dtor$7
// DIA: `idAI2::Takedown'::`1'::dtor$8
// DIA: `idAI2::Takedown'::`1'::dtor$9
// DIA: idAI2::Use
idRecoveredValue idAI2::Use(...) { return RecordCoreCall("Use"); }
// DIA: idAI2::InteractionCameraDeactivateCallback
void idAI2::InteractionCameraDeactivateCallback(idPlayer* player) {
  SetLookFocusEntity(player, true, AIMPOINT_HEAD, -1);
}
// DIA: idAI2::SetActionNodeGroupName
void idAI2::SetActionNodeGroupName(const char* groupName) {
  const char* newGroupName = groupName != nullptr ? groupName : "";
  if (std::strcmp(actionNodeGroupName.data, newGroupName) == 0) return;
  actionNodeGroupName = newGroupName;
  if (idActionNode* actionNode = idActionNode::CastTo(currentActionNode)) {
    actionNode->LeaveActionNode(this);
  }
  currentActionNode = nullptr;
}
// DIA: idAI2::UpdateAvoidCharacter
idRecoveredValue idAI2::UpdateAvoidCharacter(...) { return RecordCoreCall("UpdateAvoidCharacter"); }
// DIA: idAI2::ShouldHoldFire
idRecoveredValue idAI2::ShouldHoldFire(...) { return RecordCoreCall("ShouldHoldFire"); }
// DIA: idAI2::SetFakeEnemy
void idAI2::SetFakeEnemy(idEntity* entity) {
  if (fakeEnemy != entity) {
    fakeEnemy = entity;
    currentEnemy = nullptr;
  }
}
// DIA: idAI2::ClearFakeEnemy
void idAI2::ClearFakeEnemy() {
  if (fakeEnemy != nullptr) {
    fakeEnemy = nullptr;
    currentEnemy = nullptr;
  }
}
// DIA: idAI2::ClearWorldState
void idAI2::ClearWorldState() {
  currentEnemy = nullptr;
  worldState->Clear();
  fakeEnemy = nullptr;
}
// DIA: idAI2::SetupFakePosition
void idAI2::SetupFakePosition(const idEntity* instigator,
                              const idVec3& fakePosition, bool detected) {
  idAIGameState* gameState = gameLocal->GetAIGameState();
  idAILogic logic(idSpawnId{GetEntityNumber()});
  logic.SetGameState(gameState);
  const idSpawnId instigatorId{
      instigator != nullptr ? instigator->GetEntityNumber() : 0x1fff};
  const idSpawnId sourceId{GetEntityNumber()};

  if (detected) {
    worldState->AddEntityState(gameState, &logic, instigatorId,
                               AISTIM_GUNFIRE, 1.0F, sourceId);
  } else {
    if (controllingPlayer == nullptr &&
        aiEventState.alertCycle == ALERTCYCLE_RELAXED) {
      localVoiceController->PlayVoice(this, AIVOICE_SEARCH_NOTICE);
    }
    worldState->AddEntityState(gameState, &logic, instigatorId,
                               AISTIM_HEARING, 1.0F, sourceId);
  }

  idAIEntityState* entityState = worldState->StateForEntity(instigator);
  if (entityState == nullptr) return;

  aiPhysicalState_t physical;
  gameState->GetPhysicalState(gameLocal, instigator, &physical);
  const idVec3 eyeOffset = physical.eyePos - physical.origin;
  physical.origin = fakePosition;
  physical.eyePos = fakePosition + eyeOffset;
  entityState->SetSenseState(gameState, &logic, AISENSE_HEARING, &physical,
                             AISTIM_HEARING, sourceId);
  if (detected) {
    entityState->SetSenseState(gameState, &logic, AISENSE_SIGHT, &physical,
                               AISTIM_SIGHT, sourceId);
  }
}
// DIA: idAI2::OnAIEvent_Voice
idRecoveredValue idAI2::OnAIEvent_Voice(...) { return RecordCoreCall("OnAIEvent_Voice"); }
// DIA: idAI2::OnAIEvent_Footstep
idRecoveredValue idAI2::OnAIEvent_Footstep(...) { return RecordCoreCall("OnAIEvent_Footstep"); }
// DIA: idAI2::OnAIEvent_Gunfire
idRecoveredValue idAI2::OnAIEvent_Gunfire(...) { return RecordCoreCall("OnAIEvent_Gunfire"); }
// DIA: `idAI2::OnAIEvent_Gunfire'::`1'::dtor$0
// DIA: `idAI2::OnAIEvent_Gunfire'::`1'::dtor$1
// DIA: idAI2::OnAIEvent_Explosion
idRecoveredValue idAI2::OnAIEvent_Explosion(...) { return RecordCoreCall("OnAIEvent_Explosion"); }
// DIA: idAI2::OnAIEvent_ProjectileImpact
idRecoveredValue idAI2::OnAIEvent_ProjectileImpact(...) { return RecordCoreCall("OnAIEvent_ProjectileImpact"); }
// DIA: idAI2::OnAIEvent_DeadBodyLand
idRecoveredValue idAI2::OnAIEvent_DeadBodyLand(...) { return RecordCoreCall("OnAIEvent_DeadBodyLand"); }
// DIA: idAI2::OnAIEvent_DoorMotion
idRecoveredValue idAI2::OnAIEvent_DoorMotion(...) { return RecordCoreCall("OnAIEvent_DoorMotion"); }
// DIA: idAI2::OnAIEvent_DeploySound
idRecoveredValue idAI2::OnAIEvent_DeploySound(...) { return RecordCoreCall("OnAIEvent_DeploySound"); }
// DIA: idAI2::OnAIEvent_Grenade
idRecoveredValue idAI2::OnAIEvent_Grenade(...) { return RecordCoreCall("OnAIEvent_Grenade"); }
// DIA: idAI2::OnAIEvent_Vehicle
idRecoveredValue idAI2::OnAIEvent_Vehicle(...) { return RecordCoreCall("OnAIEvent_Vehicle"); }
// DIA: idAI2::UpdateAnimationControllers
idRecoveredValue idAI2::UpdateAnimationControllers(...) { return RecordCoreCall("UpdateAnimationControllers"); }
// DIA: idAI2::Debug_Perception
idRecoveredValue idAI2::Debug_Perception(...) { return RecordCoreCall("Debug_Perception"); }
// DIA: idAI2::idVisibilityObscurity::Update
idRecoveredValue idAI2::idVisibilityObscurity::Update(...) { return {}; }
// DIA: idAI2::ShouldReloadWeapon
bool idAI2::ShouldReloadWeapon(idWeapon* weapon, bool secondary) {
  if (weapon == nullptr ||
      weapon->fireState == idWeapon::FIRESTATE_RELOADING) {
    return false;
  }
  const float loadedFraction = weapon->GetLoadedFraction(secondary);
  if (loadedFraction >= 1.0F) return false;
  if (loadedFraction <= 0.0F) return true;

  const idEntity* enemy = currentEnemy;
  if (enemy != nullptr) {
    if (safeReloadFraction < loadedFraction) return false;
    const int currentTime = gameLocal->GetGameTimeMs();
    if (GetSpottedDuration(enemy, currentTime) >= 0) return false;
    if (IsRecentlySpotted(worldState->StateForEntity(enemy), 3.0F)) {
      return false;
    }
  }
  return true;
}
// DIA: idAI2::CheckForSuspectedToTrailUpdate
idRecoveredValue idAI2::CheckForSuspectedToTrailUpdate(...) { return RecordCoreCall("CheckForSuspectedToTrailUpdate"); }
// DIA: idAI2::CalcMovementTurnRate
idRecoveredValue idAI2::CalcMovementTurnRate(...) { return RecordCoreCall("CalcMovementTurnRate"); }
// DIA: idCheckSurroundingsState::CanUse
// DIA: idCheckSurroundingsState::FindBestLookInterestPoint
// DIA: idCheckSurroundingsState::Update
// DIA: idAI2::FindAutoFocusEntity
idRecoveredValue idAI2::FindAutoFocusEntity(...) { return RecordCoreCall("FindAutoFocusEntity"); }
// DIA: `idAI2::FindAutoFocusEntity'::`1'::dtor$0
// DIA: `idAI2::FindAutoFocusEntity'::`1'::dtor$1
// DIA: idAI2::FindAutoFocus
idRecoveredValue idAI2::FindAutoFocus(...) { return RecordCoreCall("FindAutoFocus"); }
// DIA: idAI2::StartActionScript
void idAI2::StartActionScript(int currentTime) {
  scriptedActionStartTime = currentTime;
  ++scriptedActionStartCount;
  if (IsDead()) return;

  if (!mutantActorClass) {
    if (speakingMoodCapability) {
      faceManager->SetEnableMoodAnimation(this, true);
    }
    if (speakingLipSyncCapability) {
      faceManager->SetEnableLipSyncAnimation(this, true);
      faceManager->StartLipsync();
    }
    if (speakingGestureCapability) {
      faceManager->SetEnableGestureAnimation(this, true);
    }
    if (speakingEyeTrackingCapability) {
      faceManager->SetEnableEyeTracking(this, true);
    }
    if (speakingBlinkingCapability) {
      faceManager->SetEnableBlinking(this, true);
    }
    if (speakingLidDeformationCapability) {
      faceManager->SetEnableLidDeformation(this, true);
    }
  }

  idFSMManager::idFSMStack stack;
  stack.Add(GetAlertCycleFSM(), "ActionScript");
  fsmManager->SetFSMStack(stack);
  SetAlertCycle(ALERTCYCLE_RELAXED);
  componentTimeLine->Start(this);
}
// DIA: idAI2::GiveAwarenessOfEntity
void idAI2::GiveAwarenessOfEntity(const idEntity* entity,
                                  alertCycle_t alertCycle) {
  (void)alertCycle;
  if (entity == nullptr) {
    idLib::Warning("idAI2::GiveAwarenessOfEntity - entity is NULL");
    return;
  }

  const int currentTime = gameLocal->GetGameTimeMs();
  memory->ClearAllTrailPositions(currentTime);
  memory->SetEnemy(const_cast<idEntity*>(entity));
  currentEnemy = const_cast<idEntity*>(entity);

  const idVec3& origin = entity->GetPhysics()->GetOrigin();
  SetupFakePosition(entity, origin, false);
  if (GetAAS()->PointAreaNumMaxheight(0, origin, 16.0F) > 0) {
    memory->SetLastSafeEnemyPos(origin);
  }
}
// DIA: idAI2::DoSpawnSettings
idRecoveredValue idAI2::DoSpawnSettings(...) { return RecordCoreCall("DoSpawnSettings"); }
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$5
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$0
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$1
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$2
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$3
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$4
// DIA: `idAI2::DoSpawnSettings'::`1'::dtor$9
// DIA: idAI2::SpawnCorpse
idRecoveredValue idAI2::SpawnCorpse(...) { return RecordCoreCall("SpawnCorpse"); }
// DIA: `idAI2::SpawnCorpse'::`1'::dtor$0
// DIA: `idAI2::SpawnCorpse'::`1'::dtor$1
// DIA: idAI2::GetAnimationDeltaAndRot
idRecoveredValue idAI2::GetAnimationDeltaAndRot(...) { return RecordCoreCall("GetAnimationDeltaAndRot"); }
// DIA: idAIScriptProxy::ForceFSMIntoExecutingScript
// DIA: idList<idHandle<int,enum invalidAIEvent_t,-1>,5>::Append
// DIA: idList<idHandle<int,enum invalidAIEvent_t,-1>,5>::Insert
// DIA: idList<eboltControlNode_t,5>::Append
// DIA: idList<idCrosshairCustomIcon,5>::SetNum
// DIA: idAIMoveState::~idAIMoveState
// DIA: idAIMoveState::GetMoveStatus
aiMoveStatus_t idAIMoveState::GetMoveStatus() const { return moveStatus; }
// DIA: idAIMoveState::GetFinalPoint
const idVec3& idAIMoveState::GetFinalPoint() const { return finalPoint; }
// DIA: idAIMoveState::GetNextPoint
// DIA: idAIMoveState::IsPlayingTraversalAnim
bool idAIMoveState::IsPlayingTraversalAnim() const {
  return traversalPlaying;
}
// DIA: idAIMoveState::SetTranslationDeltaScale
void idAIMoveState::SetTranslationDeltaScale(const idVec3& scale) {
  translationScale = scale;
}
// DIA: idAIMoveState::`scalar deleting destructor'
// DIA: idAI2::idAIVolatile::idAIPhysics::~idAIPhysics
idAI2::idAIVolatile::idAIPhysics::~idAIPhysics() = default;
// DIA: `idAI2::idAIVolatile::idAIPhysics::~idAIPhysics'::`1'::dtor$0
// DIA: idAI2::idAIVolatile::idAIPain::~idAIPain
idAI2::idAIVolatile::idAIPain::~idAIPain() = default;
// DIA: idAI2::UpdateTesla
idRecoveredValue idAI2::UpdateTesla(...) { return RecordCoreCall("UpdateTesla"); }
// DIA: `idAI2::UpdateTesla'::`1'::dtor$0
// DIA: idAI2::DamageGroupPopOffArmor
idRecoveredValue idAI2::DamageGroupPopOffArmor(...) { return RecordCoreCall("DamageGroupPopOffArmor"); }
// DIA: `idAI2::DamageGroupPopOffArmor'::`1'::dtor$0
// DIA: idAI2::CheckForGore
idRecoveredValue idAI2::CheckForGore(...) { return RecordCoreCall("CheckForGore"); }
// DIA: idAI2::UpdateDeaths
idRecoveredValue idAI2::UpdateDeaths(...) { return RecordCoreCall("UpdateDeaths"); }
// DIA: idAI2::AlarmNotification
void idAI2::AlarmNotification() {
  const idEntity* enemy = memory->GetEnemy();
  if (enemy == nullptr) return;

  idEncounterGroup* group = GetEncounterGroup();
  if (group == nullptr) return;

  idStaticList<idAI2*, 64> livingGroupMembers;
  group->GetLivingMembers(&livingGroupMembers);
  for (int index = 0; index < livingGroupMembers.Num(); ++index) {
    idAI2* member = livingGroupMembers[index];
    if (member != nullptr && member->memory->GetEnemy() == nullptr) {
      member->SetupFakePosition(enemy, GetPhysics()->GetOrigin(), false);
    }
  }
}
// DIA: idAI2::RelinquishControl
idRecoveredValue idAI2::RelinquishControl(...) { return RecordCoreCall("RelinquishControl"); }
// DIA: `idAI2::RelinquishControl'::`1'::dtor$0
// DIA: idAI2::ModifyCrosshairInfo
idRecoveredValue idAI2::ModifyCrosshairInfo(...) { return RecordCoreCall("ModifyCrosshairInfo"); }
// DIA: idAI2::UpdateWorldState
idRecoveredValue idAI2::UpdateWorldState(...) { return RecordCoreCall("UpdateWorldState"); }
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$0
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$1
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$2
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$3
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$4
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$5
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$6
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$7
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$8
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$9
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$10
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$11
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$12
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$15
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$16
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$20
// DIA: `idAI2::UpdateWorldState'::`1'::dtor$21
// DIA: idAI2::OnAIEvent
idRecoveredValue idAI2::OnAIEvent(...) { return RecordCoreCall("OnAIEvent"); }
// DIA: idAI2::HandlePendingAIEvents
idRecoveredValue idAI2::HandlePendingAIEvents(...) { return RecordCoreCall("HandlePendingAIEvents"); }
// DIA: idAI2::WaitForTraversal
idRecoveredValue idAI2::WaitForTraversal(...) { return RecordCoreCall("WaitForTraversal"); }
// DIA: `idAI2::WaitForTraversal'::`1'::dtor$0
// DIA: `idAI2::WaitForTraversal'::`1'::dtor$1
// DIA: `idAI2::WaitForTraversal'::`1'::dtor$2
// DIA: `idAI2::WaitForTraversal'::`1'::dtor$3
// DIA: idAI2::UpdateLookFocus
idRecoveredValue idAI2::UpdateLookFocus(...) { return RecordCoreCall("UpdateLookFocus"); }
// DIA: `idAI2::UpdateLookFocus'::`1'::dtor$0
// DIA: `idAI2::UpdateLookFocus'::`1'::dtor$1
// DIA: `idAI2::UpdateLookFocus'::`1'::dtor$2
// DIA: idAI2::Think_PlayerControl
idRecoveredValue idAI2::Think_PlayerControl(...) { return RecordCoreCall("Think_PlayerControl"); }
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$0
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$1
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$2
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$5
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$6
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$7
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$10
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$11
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$12
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$13
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$16
// DIA: `idAI2::Think_PlayerControl'::`1'::dtor$17
// DIA: idAIScriptProxy::InternalCallEvent
// DIA: idLegsFSM::~idLegsFSM
// DIA: idAI2::Spawn
idRecoveredValue idAI2::Spawn(...) { return RecordCoreCall("Spawn"); }
// DIA: `idAI2::Spawn'::`1'::dtor$0
// DIA: `idAI2::Spawn'::`1'::dtor$1
// DIA: `idAI2::Spawn'::`1'::dtor$2
// DIA: `idAI2::Spawn'::`1'::dtor$3
// DIA: idAI2::OnShow
idRecoveredValue idAI2::OnShow(...) { return RecordCoreCall("OnShow"); }
// DIA: idAI2::Think_AIControl
idRecoveredValue idAI2::Think_AIControl(...) { return RecordCoreCall("Think_AIControl"); }
// DIA: idAI2::Damage
idRecoveredValue idAI2::Damage(...) { return RecordCoreCall("Damage"); }
// DIA: `idAI2::Damage'::`1'::dtor$0
// DIA: `idAI2::Damage'::`1'::dtor$1
// DIA: `idAI2::Damage'::`1'::dtor$6
// DIA: `idAI2::Damage'::`1'::dtor$7
// DIA: `idAI2::Damage'::`1'::dtor$8
// DIA: idAI2::DormantEnd
idRecoveredValue idAI2::DormantEnd(...) { return RecordCoreCall("DormantEnd"); }
// DIA: idAI2::FirstThink
idRecoveredValue idAI2::FirstThink(...) { return RecordCoreCall("FirstThink"); }
// DIA: idAIActionManager::~idAIActionManager
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$0
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$1
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$2
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$3
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$4
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$5
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$6
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$7
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$8
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$9
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$10
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$11
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$12
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$13
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$14
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$15
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$16
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$17
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$18
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$19
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$20
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$21
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$22
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$23
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$24
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$25
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$26
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$27
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$28
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$29
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$30
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$31
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$32
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$33
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$34
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$35
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$36
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$37
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$38
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$39
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$40
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$41
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$42
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$43
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$44
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$45
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$46
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$47
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$48
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$49
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$50
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$51
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$52
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$53
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$54
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$55
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$56
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$57
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$58
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$59
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$60
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$61
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$62
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$63
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$64
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$65
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$66
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$67
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$68
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$69
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$70
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$71
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$72
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$73
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$74
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$75
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$76
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$77
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$78
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$79
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$80
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$81
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$82
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$83
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$84
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$85
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$86
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$87
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$88
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$89
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$90
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$91
// DIA: `idAIActionManager::~idAIActionManager'::`1'::dtor$94
// DIA: idAI2::Think_Impl
idRecoveredValue idAI2::Think_Impl(...) { return RecordCoreCall("Think_Impl"); }
// DIA: `idAI2::Think_Impl'::`1'::dtor$0
// DIA: `idAI2::Think_Impl'::`1'::dtor$1
// DIA: `idAI2::Think_Impl'::`1'::dtor$2
// DIA: `idAI2::Think_Impl'::`1'::dtor$3
// DIA: `idAI2::Think_Impl'::`1'::dtor$5
// DIA: `idAI2::Think_Impl'::`1'::dtor$6
// DIA: `idAI2::Think_Impl'::`1'::dtor$8
// DIA: idAI2::idAIVolatile::idAIFSMInfo::~idAIFSMInfo
idAI2::idAIVolatile::idAIFSMInfo::~idAIFSMInfo() = default;
// DIA: `idAI2::idAIVolatile::idAIFSMInfo::~idAIFSMInfo'::`1'::dtor$0
// DIA: idAI2::idAIVolatile::~idAIVolatile
idAI2::idAIVolatile::~idAIVolatile() = default;
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$0
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$1
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$2
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$3
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$4
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$5
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$6
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$7
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$8
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$9
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$10
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$11
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$12
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$13
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$14
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$15
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$22
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$25
// DIA: `idAI2::idAIVolatile::~idAIVolatile'::`1'::dtor$30
// DIA: idAI2::Think
idRecoveredValue idAI2::Think(...) { return RecordCoreCall("Think"); }
// DIA: `idAI2::Think'::`1'::dtor$0
// DIA: idAIEditable::idAISpawn::idAISpawn
// DIA: `idAIEditable::idAISpawn::idAISpawn'::`1'::dtor$0
// DIA: `idAIEditable::idAISpawn::idAISpawn'::`1'::dtor$1
// DIA: idAIEditable::idAISpawn::~idAISpawn
// DIA: idAI2::idAIVolatile::idAIVolatile
idAI2::idAIVolatile::idAIVolatile() = default;
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$17
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$0
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$1
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$2
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$3
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$4
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$21
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$5
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$6
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$7
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$8
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$9
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$10
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$11
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$12
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$13
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$14
// DIA: `idAI2::idAIVolatile::idAIVolatile'::`1'::dtor$15
// DIA: idAIEditable::~idAIEditable
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$0
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$1
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$2
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$3
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$4
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$5
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$6
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$7
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$8
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$9
// DIA: `idAIEditable::~idAIEditable'::`1'::dtor$11
// DIA: idAI2::~idAI2
// DIA: `idAI2::~idAI2'::`1'::dtor$0
// DIA: `idAI2::~idAI2'::`1'::dtor$1
// DIA: `idAI2::~idAI2'::`1'::dtor$2
// DIA: `idAI2::~idAI2'::`1'::dtor$3
// DIA: `idAI2::~idAI2'::`1'::dtor$4
// DIA: `idAI2::~idAI2'::`1'::dtor$5
// DIA: `idAI2::~idAI2'::`1'::dtor$6
// DIA: `idAI2::~idAI2'::`1'::dtor$7
// DIA: `idAI2::~idAI2'::`1'::dtor$8
// DIA: `idAI2::~idAI2'::`1'::dtor$9
// DIA: `idAI2::~idAI2'::`1'::dtor$12
// DIA: `idAI2::~idAI2'::`1'::dtor$14
// DIA: `idAI2::~idAI2'::`1'::dtor$16
// DIA: idActor::GetAF
const idAnimator_AF* idActor::GetAF() const {
  return &articulatedFigureProperties;
}
// DIA: idActor::GetLastEyePos
void idActor::GetLastEyePos(idVec3* eyePosition) const {
  *eyePosition = lastEyePosition;
}
// DIA: idActor::GetEyeAxis
void idActor::GetEyeAxis(idMat3* outputEyeAxis) const {
  *outputEyeAxis = eyeAxis;
}
// DIA: idActor::SetEyeAxis
void idActor::SetEyeAxis(const idMat3* inputEyeAxis) {
  eyeAxis = *inputEyeAxis;
}
// DIA: idActor::GetInventory
const idInventoryCollection* idActor::GetInventory() const {
  return &inventory;
}
// DIA: idActor::GetAttachments
const idAttachmentCollection* idActor::GetAttachments() const {
  return &attachments;
}
// DIA: idActor::GetTriggerTouchList
const idList<idEntityPtr<idEntity>, 5>* idActor::GetTriggerTouchList() const {
  return &touching;
}
// DIA: idActor::GetDisplayName
idStrId idActor::GetDisplayName() const { return displayName; }
// DIA: idActor::GetPosture
int idActor::GetPosture() const { return posture; }
// DIA: idActor::GetLeanState
leanState_t idActor::GetLeanState() const { return leanState; }
// DIA: idActor::SetLeanState
void idActor::SetLeanState(leanState_t newLeanState) {
  leanState = newLeanState;
}
// DIA: idActor::GetWeaponReadyState
weaponReadyState_t idActor::GetWeaponReadyState() const {
  return weaponReadyState;
}
// DIA: idActor::SetWeaponReadyState
void idActor::SetWeaponReadyState(weaponReadyState_t state) {
  weaponReadyState = state;
}
// DIA: idAI2::GetAASPositionMgr
idAASPositionMgr* idAI2::GetAASPositionMgr() {
  return aasPositionManager.get();
}
// DIA: idAI2::GetHealthComponent
idAIHealth* idAI2::GetHealthComponent() { return healthComponent.get(); }
// DIA: idAI2::GetComponentTimeLine
idComponentActionScript* idAI2::GetComponentTimeLine() {
  return componentTimeLine.get();
}
// DIA: idAI2::GetAAS
const idAAS2* idAI2::GetAAS() const { return aas; }
// DIA: idAI2::GetWeaponSlot
int idAI2::GetWeaponSlot() const { return behaviorWeaponSlot; }
// DIA: idAI2::HasSyncedWalks
bool idAI2::HasSyncedWalks() const { return behaviorHasSyncedWalks; }
// DIA: idAI2::GetInputSettings
inputSettings_t* idAI2::GetInputSettings(inputSettings_t* result,
                                         idPlayer* player) {
  (void)player;
  result->joyYawSpeed = joy_yawSpeed.valueFloat;
  result->mouseSens = m_sensitivity.valueFloat;
  result->lookInvert = in_invertLook.valueInteger != 0;
  result->bindset = BIND_POSSESS;
  result->mouseSensScale = 1.0F;
  result->joySensScale = 1.0F;
  result->absoluteAngles = false;
  result->joyPitchSpeed = joy_pitchSpeed.valueFloat;
  result->aimAssistAngles = {};
  return result;
}
// DIA: idAI2::GetDynamicCoverMgr
idFormationCoverMgr* idAI2::GetDynamicCoverMgr() {
  return dynamicCoverManager.get();
}
// DIA: idAI2::GetAnimatorAnimWeb
idAI2::idAIVolatile::idAIAnimationInfo* idAI2::GetAnimatorAnimWeb() {
  return animationInfo.get();
}
// DIA: idAI2::IsDead
bool idAI2::IsDead() const {
  if (animEventState.dead) {
    return true;
  }
  if (controllingPlayer != nullptr) {
    return false;
  }
  return aiEventState.alertCycle == 6;
}
// DIA: idAI2::IsDying
bool idAI2::IsDying() const {
  return (controllingPlayer == nullptr && aiEventState.alertCycle == 5) ||
         (animEventState.dead && GetAlertCycle() != 6);
}
// DIA: idAIEditable::idAIEditable
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$0
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$12
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$1
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$2
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$3
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$4
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$5
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$6
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$7
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$8
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$9
// DIA: `idAIEditable::idAIEditable'::`1'::dtor$10
// DIA: idAI2::idAI2
// DIA: `idAI2::idAI2'::`1'::dtor$0
// DIA: `idAI2::idAI2'::`1'::dtor$1
// DIA: `idAI2::idAI2'::`1'::dtor$2
// DIA: `idAI2::idAI2'::`1'::dtor$3
// DIA: `idAI2::idAI2'::`1'::dtor$4
// DIA: `idAI2::idAI2'::`1'::dtor$5
// DIA: `idAI2::idAI2'::`1'::dtor$6
// DIA: `idAI2::idAI2'::`1'::dtor$11
// DIA: `idAI2::idAI2'::`1'::dtor$7
// DIA: `idAI2::idAI2'::`1'::dtor$8
// DIA: `idAI2::idAI2'::`1'::dtor$9
// DIA: `idAI2::idAI2'::`1'::dtor$15
// DIA: `idAI2::idAI2'::`1'::dtor$16
// DIA: `idAI2::idAI2'::`1'::dtor$10
// DIA: `idAI2::idAI2'::`1'::dtor$18
// DIA: idAI2::`scalar deleting destructor'
// DIA: `dynamic initializer for 'ai_limitFSMUpdates''
idCVar ai_limitFSMUpdates("ai_limitFSMUpdates", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugLevel''
idCVar ai_debugLevel("ai_debugLevel", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugTracking''
idCVar ai_debugTracking("ai_debugTracking", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugHealth''
idCVar ai_debugHealth("ai_debugHealth", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEvents''
idCVar ai_debugEvents("ai_debugEvents", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugState''
idCVar ai_debugState("ai_debugState", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugAim''
idCVar ai_debugAim("ai_debugAim", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugLook''
idCVar ai_debugLook("ai_debugLook", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugOrientation''
idCVar ai_debugOrientation("ai_debugOrientation", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugCover''
idCVar ai_debugCover("ai_debugCover", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugCoverEntityId''
idCVar ai_debugCoverEntityId("ai_debugCoverEntityId", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEnemy''
idCVar ai_debugEnemy("ai_debugEnemy", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_visibilitySenseScale''
idCVar ai_visibilitySenseScale("ai_visibilitySenseScale", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableSenseVolumes''
idCVar ai_enableSenseVolumes("ai_enableSenseVolumes", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugSearch''
idCVar ai_debugSearch("ai_debugSearch", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugFriendly''
idCVar ai_debugFriendly("ai_debugFriendly", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugPain''
idCVar ai_debugPain("ai_debugPain", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInjured''
idCVar ai_debugInjured("ai_debugInjured", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugDeath''
idCVar ai_debugDeath("ai_debugDeath", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugPushFromCollision''
idCVar ai_debugPushFromCollision("ai_debugPushFromCollision", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugVoice''
idCVar ai_debugVoice("ai_debugVoice", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugTension''
idCVar ai_debugTension("ai_debugTension", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugDangerZone''
idCVar ai_debugDangerZone("ai_debugDangerZone", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useCombatSubtitles''
idCVar ai_useCombatSubtitles("ai_useCombatSubtitles", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugTargetList''
idCVar ai_debugTargetList("ai_debugTargetList", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEnemyTargetSelector''
idCVar ai_debugEnemyTargetSelector("ai_debugEnemyTargetSelector", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInteraction''
idCVar ai_debugInteraction("ai_debugInteraction", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInteractionDistance''
idCVar ai_debugInteractionDistance("ai_debugInteractionDistance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInteraction_Approach''
idCVar ai_debugInteraction_Approach("ai_debugInteraction_Approach", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugInteraction_Departure''
idCVar ai_debugInteraction_Departure("ai_debugInteraction_Departure", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_fastJobInteraction''
idCVar ai_fastJobInteraction("ai_fastJobInteraction", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugmelee''
idCVar ai_debugmelee("ai_debugmelee", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugOpenCombatMove''
idCVar ai_debugOpenCombatMove("ai_debugOpenCombatMove", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugLeapAttack''
idCVar ai_debugLeapAttack("ai_debugLeapAttack", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugTraversals''
idCVar ai_debugTraversals("ai_debugTraversals", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEyePos''
idCVar ai_debugEyePos("ai_debugEyePos", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugEncounterGroupRoles''
idCVar ai_debugEncounterGroupRoles("ai_debugEncounterGroupRoles", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugCoverTransitions''
idCVar ai_debugCoverTransitions("ai_debugCoverTransitions", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_showAASName''
idCVar ai_showAASName("ai_showAASName", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugPathToEnemy''
idCVar ai_debugPathToEnemy("ai_debugPathToEnemy", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugAnimDeltaScale''
idCVar ai_debugAnimDeltaScale("ai_debugAnimDeltaScale", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugMissTime''
idCVar ai_debugMissTime("ai_debugMissTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugLatchedState''
idCVar ai_debugLatchedState("ai_debugLatchedState", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugBlendTree''
idCVar ai_debugBlendTree("ai_debugBlendTree", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_blendTreeName''
idCVar ai_blendTreeName("ai_blendTreeName", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugDodge''
idCVar ai_debugDodge("ai_debugDodge", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useAimTracker''
idCVar ai_useAimTracker("ai_useAimTracker", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useLookTracker''
idCVar ai_useLookTracker("ai_useLookTracker", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useWanderingAim''
idCVar ai_useWanderingAim("ai_useWanderingAim", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useWeaponSpread''
idCVar ai_useWeaponSpread("ai_useWeaponSpread", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useTransitions''
idCVar ai_useTransitions("ai_useTransitions", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useCoverTransitions''
idCVar ai_useCoverTransitions("ai_useCoverTransitions", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enable''
idCVar ai_enable("ai_enable", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableWorldState''
idCVar ai_enableWorldState("ai_enableWorldState", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableFSM''
idCVar ai_enableFSM("ai_enableFSM", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableAnimation''
idCVar ai_enableAnimation("ai_enableAnimation", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enablePhysics''
idCVar ai_enablePhysics("ai_enablePhysics", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableMovement''
idCVar ai_enableMovement("ai_enableMovement", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableDormancy''
idCVar ai_enableDormancy("ai_enableDormancy", "1", 1,
                         "1 = AI that is not rendered will stop thinking");
// DIA: `dynamic initializer for 'ai_enableAutoFocus''
idCVar ai_enableAutoFocus("ai_enableAutoFocus", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableInjuredRuns''
idCVar ai_enableInjuredRuns("ai_enableInjuredRuns", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugGore''
idCVar ai_debugGore("ai_debugGore", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableScrambles''
idCVar ai_enableScrambles(
    "ai_enableScrambles", "1", 1,
    "true to enable AI scramble runs and transitions");
// DIA: `dynamic initializer for 'ai_scrambleTime''
idCVar ai_scrambleTime("ai_scrambleTime", "2000", 2,
                       "ms for AI to scramble after being shot at");
// DIA: `dynamic initializer for 'ai_searchToStimDist''
idCVar ai_searchToStimDist("ai_searchToStimDist", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_mood''
idCVar ai_mood("ai_mood", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_attitude''
idCVar ai_attitude("ai_attitude", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_otherFactionAttitude''
idCVar ai_otherFactionAttitude("ai_otherFactionAttitude", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_accuracy''
idCVar ai_accuracy("ai_accuracy", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_lookRate''
idCVar ai_lookRate("ai_lookRate", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_keepShootingNoSight''
idCVar ai_keepShootingNoSight("ai_keepShootingNoSight", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_showFOV''
idCVar ai_showFOV("ai_showFOV", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_weaponKick''
idCVar ai_weaponKick("ai_weaponKick", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_faceFocus''
idCVar ai_faceFocus("ai_faceFocus", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_lookAtPlayer''
idCVar ai_lookAtPlayer("ai_lookAtPlayer", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_allowStaggerPain''
idCVar ai_allowStaggerPain("ai_allowStaggerPain", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_invulnerable''
idCVar ai_invulnerable("ai_invulnerable", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_ignoreEvents''
idCVar ai_ignoreEvents("ai_ignoreEvents", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_ignoreFireLimits''
idCVar ai_ignoreFireLimits("ai_ignoreFireLimits", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_forceWalk''
idCVar ai_forceWalk("ai_forceWalk", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_walkMod''
idCVar ai_walkMod("ai_walkMod", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_runMod''
idCVar ai_runMod("ai_runMod", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_forceTurnRate''
idCVar ai_forceTurnRate("ai_forceTurnRate", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_minTurnRate''
idCVar ai_minTurnRate("ai_minTurnRate", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_maxTurnRate''
idCVar ai_maxTurnRate("ai_maxTurnRate", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_moveModeLookAhead''
idCVar ai_moveModeLookAhead("ai_moveModeLookAhead", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_flashIfLootable''
idCVar ai_flashIfLootable("ai_flashIfLootable", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_lootDelay''
idCVar ai_lootDelay("ai_lootDelay", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_interactionView_useCamera''
idCVar ai_interactionView_useCamera("ai_interactionView_useCamera", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useStageCoverFilter''
idCVar ai_useStageCoverFilter("ai_useStageCoverFilter", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_disableWanderGreetings''
idCVar ai_disableWanderGreetings("ai_disableWanderGreetings", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_droppedLootDistance''
idCVar ai_droppedLootDistance("ai_droppedLootDistance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_enableInfluenceTrails''
idCVar ai_enableInfluenceTrails("ai_enableInfluenceTrails", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugGreetingDistance''
idCVar ai_debugGreetingDistance("ai_debugGreetingDistance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugPerception''
idCVar ai_debugPerception("ai_debugPerception", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugScenePoints''
idCVar ai_debugScenePoints("ai_debugScenePoints", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_allowCollapse''
idCVar ai_allowCollapse("ai_allowCollapse", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_maxDeferDeathTime''
idCVar ai_maxDeferDeathTime("ai_maxDeferDeathTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_forceInjury''
idCVar ai_forceInjury("ai_forceInjury", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_pushVelocityTolerance''
idCVar ai_pushVelocityTolerance("ai_pushVelocityTolerance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_ragdollKillLimit''
idCVar ai_ragdollKillLimit("ai_ragdollKillLimit", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugBleedoutCheck''
idCVar ai_debugBleedoutCheck("ai_debugBleedoutCheck", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_thinkTrace''
idCVar ai_thinkTrace("ai_thinkTrace", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'takedownWindoMaxTime''
idCVar takedownWindoMaxTime("takedownWindoMaxTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'takedownType''
idCVar takedownType("takedownType", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'takedownID''
idCVar takedownID(
    "takedownID", "-1", 2,
    "ID in the list of the takedown category that is used (see AI template)");
// DIA: `dynamic initializer for 'takedownWindowDebug''
idCVar takedownWindowDebug("takedownWindowDebug", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'counterBlockTime''
idCVar counterBlockTime("counterBlockTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_debugArmorPopOff''
idCVar ai_debugArmorPopOff("ai_debugArmorPopOff", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_damageScale''
idCVar ai_damageScale("ai_damageScale", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_syncAF''
idCVar ai_syncAF("ai_syncAF", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_limitTargetUpdates''
idCVar ai_limitTargetUpdates("ai_limitTargetUpdates", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_FadeDelay''
idCVar ai_FadeDelay("ai_FadeDelay", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_CoopFadeOutTime''
idCVar ai_CoopFadeOutTime("ai_CoopFadeOutTime", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_CoopFadeDelay''
idCVar ai_CoopFadeDelay("ai_CoopFadeDelay", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_NonPlayerOneShotChance''
idCVar ai_NonPlayerOneShotChance("ai_NonPlayerOneShotChance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_useAvoidForCoverFilter''
idCVar ai_useAvoidForCoverFilter("ai_useAvoidForCoverFilter", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_controlHeartBeatIntervalsMin''
idCVar ai_controlHeartBeatIntervalsMin("ai_controlHeartBeatIntervalsMin", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_controlHeartBeatIntervalsMax''
idCVar ai_controlHeartBeatIntervalsMax("ai_controlHeartBeatIntervalsMax", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'ai_controlTakeDownDistance''
idCVar ai_controlTakeDownDistance("ai_controlTakeDownDistance", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'rpWetParameters''
idCVar rpWetParameters("rpWetParameters", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic initializer for 'rpLandWetRippleIntensity''
idCVar rpLandWetRippleIntensity("rpLandWetRippleIntensity", "0", 0, "recovered AI tuning variable");
// DIA: `dynamic atexit destructor for 'ai_limitFSMUpdates''
// DIA: `dynamic atexit destructor for 'ai_debugLevel''
// DIA: `dynamic atexit destructor for 'ai_debugTracking''
// DIA: `dynamic atexit destructor for 'ai_debugHealth''
// DIA: `dynamic atexit destructor for 'ai_debugEvents''
// DIA: `dynamic atexit destructor for 'ai_debugState''
// DIA: `dynamic atexit destructor for 'ai_debugAim''
// DIA: `dynamic atexit destructor for 'ai_debugLook''
// DIA: `dynamic atexit destructor for 'ai_debugOrientation''
// DIA: `dynamic atexit destructor for 'ai_debugCover''
// DIA: `dynamic atexit destructor for 'ai_debugCoverEntityId''
// DIA: `dynamic atexit destructor for 'ai_debugEnemy''
// DIA: `dynamic atexit destructor for 'ai_visibilitySenseScale''
// DIA: `dynamic atexit destructor for 'ai_enableSenseVolumes''
// DIA: `dynamic atexit destructor for 'ai_debugSearch''
// DIA: `dynamic atexit destructor for 'ai_debugFriendly''
// DIA: `dynamic atexit destructor for 'ai_debugPain''
// DIA: `dynamic atexit destructor for 'ai_debugInjured''
// DIA: `dynamic atexit destructor for 'ai_debugDeath''
// DIA: `dynamic atexit destructor for 'ai_debugPushFromCollision''
// DIA: `dynamic atexit destructor for 'ai_debugVoice''
// DIA: `dynamic atexit destructor for 'ai_debugTension''
// DIA: `dynamic atexit destructor for 'ai_debugDangerZone''
// DIA: `dynamic atexit destructor for 'ai_useCombatSubtitles''
// DIA: `dynamic atexit destructor for 'ai_debugTargetList''
// DIA: `dynamic atexit destructor for 'ai_debugEnemyTargetSelector''
// DIA: `dynamic atexit destructor for 'ai_debugInteraction''
// DIA: `dynamic atexit destructor for 'ai_debugInteractionDistance''
// DIA: `dynamic atexit destructor for 'ai_debugInteraction_Approach''
// DIA: `dynamic atexit destructor for 'ai_debugInteraction_Departure''
// DIA: `dynamic atexit destructor for 'ai_fastJobInteraction''
// DIA: `dynamic atexit destructor for 'ai_debugmelee''
// DIA: `dynamic atexit destructor for 'ai_debugOpenCombatMove''
// DIA: `dynamic atexit destructor for 'ai_debugLeapAttack''
// DIA: `dynamic atexit destructor for 'ai_debugTraversals''
// DIA: `dynamic atexit destructor for 'ai_debugEyePos''
// DIA: `dynamic atexit destructor for 'ai_debugEncounterGroupRoles''
// DIA: `dynamic atexit destructor for 'ai_debugCoverTransitions''
// DIA: `dynamic atexit destructor for 'ai_showAASName''
// DIA: `dynamic atexit destructor for 'ai_debugPathToEnemy''
// DIA: `dynamic atexit destructor for 'ai_debugAnimDeltaScale''
// DIA: `dynamic atexit destructor for 'ai_debugMissTime''
// DIA: `dynamic atexit destructor for 'ai_debugLatchedState''
// DIA: `dynamic atexit destructor for 'ai_debugBlendTree''
// DIA: `dynamic atexit destructor for 'ai_blendTreeName''
// DIA: `dynamic atexit destructor for 'ai_debugDodge''
// DIA: `dynamic atexit destructor for 'ai_useAimTracker''
// DIA: `dynamic atexit destructor for 'ai_useLookTracker''
// DIA: `dynamic atexit destructor for 'ai_useWanderingAim''
// DIA: `dynamic atexit destructor for 'ai_useWeaponSpread''
// DIA: `dynamic atexit destructor for 'ai_useTransitions''
// DIA: `dynamic atexit destructor for 'ai_useCoverTransitions''
// DIA: `dynamic atexit destructor for 'ai_enable''
// DIA: `dynamic atexit destructor for 'ai_enableWorldState''
// DIA: `dynamic atexit destructor for 'ai_enableFSM''
// DIA: `dynamic atexit destructor for 'ai_enableAnimation''
// DIA: `dynamic atexit destructor for 'ai_enablePhysics''
// DIA: `dynamic atexit destructor for 'ai_enableMovement''
// DIA: `dynamic atexit destructor for 'ai_enableDormancy''
// DIA: `dynamic atexit destructor for 'ai_enableAutoFocus''
// DIA: `dynamic atexit destructor for 'ai_enableInjuredRuns''
// DIA: `dynamic atexit destructor for 'ai_debugGore''
// DIA: `dynamic atexit destructor for 'ai_enableScrambles''
// DIA: `dynamic atexit destructor for 'ai_scrambleTime''
// DIA: `dynamic atexit destructor for 'ai_searchToStimDist''
// DIA: `dynamic atexit destructor for 'ai_mood''
// DIA: `dynamic atexit destructor for 'ai_attitude''
// DIA: `dynamic atexit destructor for 'ai_otherFactionAttitude''
// DIA: `dynamic atexit destructor for 'ai_accuracy''
// DIA: `dynamic atexit destructor for 'ai_lookRate''
// DIA: `dynamic atexit destructor for 'ai_keepShootingNoSight''
// DIA: `dynamic atexit destructor for 'ai_showFOV''
// DIA: `dynamic atexit destructor for 'ai_weaponKick''
// DIA: `dynamic atexit destructor for 'ai_faceFocus''
// DIA: `dynamic atexit destructor for 'ai_lookAtPlayer''
// DIA: `dynamic atexit destructor for 'ai_allowStaggerPain''
// DIA: `dynamic atexit destructor for 'ai_invulnerable''
// DIA: `dynamic atexit destructor for 'ai_ignoreEvents''
// DIA: `dynamic atexit destructor for 'ai_ignoreFireLimits''
// DIA: `dynamic atexit destructor for 'ai_forceWalk''
// DIA: `dynamic atexit destructor for 'ai_runMod''
// DIA: `dynamic atexit destructor for 'ai_walkMod''
// DIA: `dynamic atexit destructor for 'ai_forceTurnRate''
// DIA: `dynamic atexit destructor for 'ai_minTurnRate''
// DIA: `dynamic atexit destructor for 'ai_maxTurnRate''
// DIA: `dynamic atexit destructor for 'ai_moveModeLookAhead''
// DIA: `dynamic atexit destructor for 'ai_flashIfLootable''
// DIA: `dynamic atexit destructor for 'ai_lootDelay''
// DIA: `dynamic atexit destructor for 'ai_interactionView_useCamera''
// DIA: `dynamic atexit destructor for 'ai_useStageCoverFilter''
// DIA: `dynamic atexit destructor for 'ai_disableWanderGreetings''
// DIA: `dynamic atexit destructor for 'ai_droppedLootDistance''
// DIA: `dynamic atexit destructor for 'ai_enableInfluenceTrails''
// DIA: `dynamic atexit destructor for 'ai_debugGreetingDistance''
// DIA: `dynamic atexit destructor for 'ai_debugPerception''
// DIA: `dynamic atexit destructor for 'ai_debugScenePoints''
// DIA: `dynamic atexit destructor for 'ai_allowCollapse''
// DIA: `dynamic atexit destructor for 'ai_maxDeferDeathTime''
// DIA: `dynamic atexit destructor for 'ai_forceInjury''
// DIA: `dynamic atexit destructor for 'ai_pushVelocityTolerance''
// DIA: `dynamic atexit destructor for 'ai_ragdollKillLimit''
// DIA: `dynamic atexit destructor for 'ai_debugBleedoutCheck''
// DIA: `dynamic atexit destructor for 'ai_thinkTrace''
// DIA: `dynamic atexit destructor for 'takedownWindoMaxTime''
// DIA: `dynamic atexit destructor for 'takedownType''
// DIA: `dynamic atexit destructor for 'takedownID''
// DIA: `dynamic atexit destructor for 'takedownWindowDebug''
// DIA: `dynamic atexit destructor for 'counterBlockTime''
// DIA: `dynamic atexit destructor for 'ai_debugArmorPopOff''
// DIA: `dynamic atexit destructor for 'ai_damageScale''
// DIA: `dynamic atexit destructor for 'ai_syncAF''
// DIA: `dynamic atexit destructor for 'ai_limitTargetUpdates''
// DIA: `dynamic atexit destructor for 'ai_FadeDelay''
// DIA: `dynamic atexit destructor for 'ai_CoopFadeOutTime''
// DIA: `dynamic atexit destructor for 'ai_CoopFadeDelay''
// DIA: `dynamic atexit destructor for 'ai_NonPlayerOneShotChance''
// DIA: `dynamic atexit destructor for 'ai_useAvoidForCoverFilter''
// DIA: `dynamic atexit destructor for 'ai_controlHeartBeatIntervalsMin''
// DIA: `dynamic atexit destructor for 'ai_controlHeartBeatIntervalsMax''
// DIA: `dynamic atexit destructor for 'ai_controlTakeDownDistance''

#include "tungsten/game/ai/ai2_core_support.inl"
