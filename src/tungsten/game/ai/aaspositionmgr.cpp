#include "tungsten/game/ai/aaspositionmgr.h"

#include <limits>

extern idCVar ai_debugLevel;

namespace {

constexpr idColor kDebugYellow{1.0F, 1.0F, 0.0F, 1.0F};
constexpr idColor kDebugWhite{1.0F, 1.0F, 1.0F, 1.0F};

}  // namespace

// DIA: idAASPosition::GetAreaNum
int idAASPosition::GetAreaNum() const {
  return valid ? static_cast<int>(areaNum) : 0;
}

// DIA: idAASPosition::Update
void idAASPosition::Update(const idAAS2* newAAS, const idVec3& newOrigin) {
  if (newAAS == nullptr) {
    valid = false;
    return;
  }

  if (ai_debugAASPosition.valueInteger != 0 &&
      ai_debugLevel.valueInteger > 0) {
    gameLocal->GetRenderWorld()->GetRenderDebug()->DebugPoint(
        kDebugYellow, origin, 0, false);
    gameLocal->GetRenderWorld()->GetRenderDebug()->DebugPoint(
        kDebugWhite, validOrigin, 0, false);
  }

  const bool originChanged = origin.x != newOrigin.x ||
                             origin.y != newOrigin.y ||
                             origin.z != newOrigin.z;
  if (areaNum != 0 && !originChanged && aas == newAAS) {
    return;
  }

  aas = newAAS;
  origin = newOrigin;

  std::uint16_t resolvedAreaNum = newAAS->PointAreaNum(0, newOrigin);
  valid = resolvedAreaNum != 0;
  if (valid) {
    validOrigin = newOrigin;
  } else {
    const idBounds searchBounds = newAAS->DefaultSearchBounds();
    resolvedAreaNum = newAAS->PointReachableAreaNum(
        0, newOrigin, searchBounds, 2, AAS_TFL_INVALID);
    if (resolvedAreaNum == 0) {
      return;
    }

    newAAS->PushPointIntoAreaNum(resolvedAreaNum, origin);
    validOrigin = origin;
    valid = true;
  }

  areaNum = resolvedAreaNum;
}

// DIA: idAASPositionMgr::idAASPositionMgr
idAASPositionMgr::idAASPositionMgr()
    : aasPosition{}, numAASPositions(0), lastUpdateFrame{{-1, -1}} {}

// DIA: idAASPositionMgr::GetAASPosition
const idAASPosition& idAASPositionMgr::GetAASPosition(
    const idAAS2* aas, const idEntity* entity) {
  int selected = -1;
  for (int index = 0; index < numAASPositions; ++index) {
    if (aasPosition[index].aas == aas) {
      selected = index;
      break;
    }
  }

  if (selected < 0) {
    if (numAASPositions < MAX_AAS_POSITIONS) {
      selected = numAASPositions++;
    } else {
      int oldestFrame = std::numeric_limits<int>::max();
      for (int index = 0; index < numAASPositions; ++index) {
        if (lastUpdateFrame[index] < oldestFrame) {
          oldestFrame = lastUpdateFrame[index];
          selected = index;
        }
      }
    }
  }

  if (selected < 0) {
    selected = 0;
  }

  idPhysics* physics = entity->GetPhysics();
  aasPosition[selected].Update(aas, physics->GetOrigin());
  return aasPosition[selected];
}

// DIA: `dynamic initializer for 'ai_debugAASPosition''
// DIA: `dynamic atexit destructor for 'ai_debugAASPosition''
idCVar ai_debugAASPosition("ai_debugAASPosition", "0", 1,
                           "1 = show debug markers for AAS position");
