#pragma once

#include "engine/models/particles/jobs/dustgen.h"
#include "engine/models/rendermodel.h"
#include "shared/idlib/containers/list.h"
#include "shared/idlib/csystems/cvarsystemlocal.h"
#include "shared/idlib/text/atomicstring.h"

#include <cstddef>
#include <cstdint>

class idDeclTable;
class idParticleStageImposter;
class idPartImposterData;
class idRenderView;
class idRenderModelUpdateTools;

class idDeclParticle {
public:
  const idLookupTable* GetTables() const;

  std::uint8_t recoveredDecl[80]{};
  idList<idParticleStage*, 79> stages;
  idList<idAtomicString, 79> stageNames;
  idList<idAtomicString, 79> stageInheritNames;
  idList<const idDeclTable*, 79> tableDecls;
  idList<idLookupTable, 79> tables;
  idList<idParticleStageImposter*, 79> imposters;
  idList<idPartImposterData*, 79> imposterData;
  idBounds bounds{};
  float maxSystemDuration{};
  float lodRadius{};
  bool usesWind{};
  bool mergeSurfaces{};
  std::uint8_t recoveredTail[6]{};
};

class idRenderModelDust final : public idRenderModel {
public:
  idRenderModelDust();
  ~idRenderModelDust() override;

  void SetParams(const idDeclParticle* const decl,
                 const idBounds& fullVolume, const idBounds& coreVolume,
                 float spacing, float edgeFadeCurve,
                 const dustParameters_t& dustParms);

private:
  bool UpdateInView(const idRenderView* currentView,
                    const idRenderView* nextView,
                    idRenderModelUpdateTools* updateTools) override;

public:
  int numParticles;
  dustParticle_t* particles;
  int lastRenderTime;
  deferredDustGenParms_t parms;
  const idDeclParticle* particleDecl;
};

extern idCVar r_useDustGenJobs;
extern idCVar r_skipDust;

static_assert(sizeof(idDeclParticle) == 288,
              "idDeclParticle ABI drift");
static_assert(offsetof(idDeclParticle, stages) == 80,
              "idDeclParticle stages offset drift");
static_assert(offsetof(idDeclParticle, tables) == 176,
              "idDeclParticle tables offset drift");
static_assert(offsetof(idDeclParticle, bounds) == 248,
              "idDeclParticle bounds offset drift");
static_assert(offsetof(idRenderModelDust, numParticles) == 2176,
              "idRenderModelDust particle count offset drift");
static_assert(offsetof(idRenderModelDust, particles) == 2184,
              "idRenderModelDust particles offset drift");
static_assert(offsetof(idRenderModelDust, lastRenderTime) == 2192,
              "idRenderModelDust render time offset drift");
static_assert(offsetof(idRenderModelDust, parms) == 2200,
              "idRenderModelDust deferred parms offset drift");
static_assert(offsetof(idRenderModelDust, particleDecl) == 2408,
              "idRenderModelDust declaration offset drift");
static_assert(sizeof(idRenderModelDust) == 2416,
              "idRenderModelDust ABI drift");
