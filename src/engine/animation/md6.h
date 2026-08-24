#pragma once

#include "shared/idlib/containers/list.h"
#include "shared/idlib/containers/staticlist.h"
#include "shared/idlib/bv/bounds.h"
#include "shared/idlib/bv/boundsshort.h"
#include "shared/idlib/text/atomicstring.h"
#include "shared/idlib/text/str.h"

#include <cstdint>

class idAnimWebPath : public idStr {
public:
  void SetPath(const char* webName,const char* subWebName,const char* stateName) {
    Empty();
    if(webName&&*webName){Append(webName);Append('/');}
    if(subWebName&&*subWebName){Append(subWebName);Append('/');}
    Append(stateName?stateName:"");
  }
};

// NEEDS_WORK(animation): this is the recovered ABI boundary used by AnimWeb.
// The owning animation module will replace the resource loading implementation.
struct idMD6AnimData {
  std::uint32_t totalSize=0;
  std::uint16_t size=0;
  std::uint16_t flags=0;
  std::uint16_t numFrames=0;
  std::uint16_t frameRate=0;
  std::uint16_t numFrameSets=0;
  std::uint16_t frameSetTblOffset=0;
  std::uint16_t frameSetOffsetTblOffset=0;
  std::uint16_t numAnimMaps=0;
  std::uint16_t constROffset=0;
  std::uint16_t constSOffset=0;
  std::uint16_t constTOffset=0;
  std::uint16_t constUOffset=0;
  std::uint16_t nextSize=0;
  std::uint16_t jointWeightsOffset=0;
  float startDelta[12]{};
  float endDelta[12]{};
};
static_assert(sizeof(idMD6AnimData)==128);
class idMD6Anim {
public:
  void* vtable = nullptr;
  idAtomicString name;
  std::uint8_t resourceBase[32]{};
  idAtomicString skelName;
  idMD6AnimData* animData = nullptr;
  std::uint8_t phaseTrack[8]{};
  idBoundsShort translatedBounds{};
  idBoundsShort normalizedBounds{};
  std::uint16_t jointConversionHandle = 0xffffu;
  std::uint16_t jointConversionChecksum = 0xffffu;
  std::uint32_t timestamps[5]{};
  std::uint32_t timeCode = 0;
  idVec3 origWorldPos{};
  idAngles origWorldRot{};
  idVec3 origWorldScale{};
  std::uint16_t startFrameOffset = 0;
  std::uint16_t endFrameOffset = 0;
  bool postedForGeneration = false;
  std::uint8_t tailPad[3]{};
};
class idMD6Model;
class idMD6Mesh;
class idDecl;
class idParser;
struct idMD6AliasEntry {
  idAtomicString animRef;
  const idMD6Anim* anim = nullptr;
};
using idMD6AliasMapping = idMD6AliasEntry;
struct idMD6Alias {
  idMD6Alias();
  bool IsInherited(const idMD6Alias* other) const;
  void LoadAnims(const class idDeclMD6* decl);
  void LoadAnimation(int index);
  void Parse(const idDecl* decl, idParser* source, int* loadErrors);

  idAtomicString name;
  std::uint16_t aliasHandle;
  std::uint16_t flags;
  std::uint32_t pad12;
  idList<idMD6AliasEntry, 19> mappings;
};
class idDeclMD6 {
public:
  struct aliasEntry_t { std::uint16_t handle = 0xffff; idMD6Alias alias; };
  const idMD6Alias* FindAlias(std::uint16_t handle) const {
    for (int i=0;i<aliases.Num();++i) if (aliases[i].handle==handle) return &aliases[i].alias;
    return nullptr;
  }
  idAtomicString name;
  idList<aliasEntry_t> aliases;
  idMD6Model* model = nullptr;
};

// NEEDS_WORK(animation): declarations below are the MD6 tree ABI consumed by
// AnimWeb. Their runtime algorithms belong to the animation folder.
struct idDeclAnimWebTagGroup;
enum invalidUserChannelIndex_t { INVALID_USER_CHANNEL_INDEX };
template<typename T,typename InvalidTag>struct idIndex{T value=static_cast<T>(-1);idIndex()=default;explicit idIndex(T v):value(v){}};
using idUserChannelIndex=idIndex<std::int16_t,invalidUserChannelIndex_t>;
enum md6WeightGroup_t {
  MD6_WEIGHTGROUP_ALL=0,
  MD6_WEIGHTGROUP_LEGS=1,
  MD6_WEIGHTGROUP_TORSO=2,
  MD6_WEIGHTGROUP_HEAD=3,
  MD6_WEIGHTGROUP_FACE=4,
  MD6_WEIGHTGROUP_MOUTH=5,
  MD6_WEIGHTGROUP_EYELIDS=6,
  MD6_WEIGHTGROUP_USER0=7,
  MD6_WEIGHTGROUP_MAX=8
};
class idMD6Skel;
class idMD6Model {
public:
  void* vtable = nullptr;
  idAtomicString name;
  std::uint8_t resourceBase[32]{};
  idStr filename;
  std::uint32_t timestamp = 0;
  std::uint32_t skelTimestamp = 0;
  idBounds defaultBounds{};
  bool remapForSkinning = false;
  std::uint8_t pad121[3]{};
  std::uint32_t skinChecksum = 0;
  idStr morphSkinName;
  std::uint32_t morphSkin = 0;
  std::int32_t numMeshesBeforeSplitting = 0;
  idList<idMD6Mesh*, 19> meshes;
  idList<std::uint8_t, 19> jointRemap;
  std::uint8_t modelListsTail[72]{};
  idMD6Skel* skeleton = nullptr;
  idVec3 minBoundsExpansion{};
  idVec3 maxBoundsExpansion{};
};
struct idMD6Node {
  enum nodeType_t : std::uint8_t { NODE_BRANCH=0,NODE_LEAF_PAUSE=1,NODE_LEAF_PLAY=2,NODE_BLEND_BRANCH=3,NODE_BLENDA_BRANCH=4,NODE_FUSION_BRANCH=5,NODE_BEST_LEAF=6,NODE_TAG_FILTER=7,NODE_NONE=15 };
  std::uint8_t type=NODE_NONE;
};
struct idMD6Leaf : idMD6Node { const idMD6Anim*anim=nullptr;std::uint8_t weightGroup=0;std::uint8_t wrapMode=0;std::uint8_t initCounter=0;std::uint8_t pad=0; };
struct idMD6LeafPlay : idMD6Leaf { int startTime=0;float rateScale=1.0f;std::uint8_t syncGroup=0;std::uint8_t syncEnabled=0; };
struct idMD6LeafPause : idMD6Leaf {
  // NEEDS_WORK(animation): opaque ownership fields retain the retail ABI until
  // the animation module recovers their concrete types.
  int currentDeferred=0;
  std::uint8_t deferredPadding[4]{};
  std::uint8_t animMods[48]{};
  std::int16_t flags=0;
  std::uint8_t flagsPadding[2]{};
  float frame=0.0f;
  float bounds[6]{};
};
struct idMD6Branch : idMD6Node { idMD6Node*left=nullptr;idMD6Node*right=nullptr;int leftTimeOverride=0;int rightTimeOverride=0;std::uint8_t filterGroup=0;std::uint8_t op=0;std::uint8_t originBlend=0;float currentAlpha=0.0f;float targetAlpha=0.0f;float alphaRate=0.0f;int blendType=0; };
struct idMD6BlendBranchBase : idMD6Branch { idList<float,30> coordinateList;idList<idMD6Node*,30> animationList; };
struct idMD6BlendBranch : idMD6BlendBranchBase { idList<float,30> currentCoordinate;idList<idUserChannelIndex,30> userChannelIndices;idList<float,30> activeCoordinate;idList<int,30> activeIndices;idList<idMD6Branch,30> branchList;std::uint8_t numDimensions=0;bool isAngleWrapped=false; };
struct idMD6BlendAdditiveBranch : idMD6BlendBranchBase { float currentCoordinate=0.0f;float activeCoordinate=0.0f;idMD6Node*baseAnimation=nullptr; };
struct idMD6FusionBranch : idMD6BlendBranch { idList<std::uint16_t,30> phaseToFrameLookupData; };
struct idMD6Filter : idMD6Node {};
struct idMD6TagFilter : idMD6Filter { const idDeclAnimWebTagGroup*tagGroup=nullptr;unsigned tagMask=0;std::uint8_t tagGroupIndex=0;std::uint8_t tagBias=0; };
struct idMD6BestLeaf : idMD6Node { idList<idMD6Node*,30> leafList;idList<unsigned,30>tagList;idList<idMD6Filter*,30>filterList;idMD6TagFilter tagGroupFilter;std::uint8_t debugFilterLogicHelper=0;std::uint8_t debugPadding[7]{};const idDeclAnimWebTagGroup*tagGroup=nullptr;unsigned desiredTag=0;unsigned activeTag=0;unsigned defaultTag=0;std::uint16_t bestLeafIndex=0;std::uint8_t tagBias=0;std::uint8_t tagGroupIndex=0; };
static_assert(sizeof(idMD6Node)==1);
static_assert(sizeof(idMD6Leaf)==24);
static_assert(sizeof(idMD6LeafPlay)==40);
static_assert(sizeof(idMD6LeafPause)==112);
static_assert(sizeof(idMD6Branch)==56);
static_assert(sizeof(idMD6BlendBranchBase)==104);
static_assert(sizeof(idMD6BlendBranch)==232);
static_assert(sizeof(idMD6BlendAdditiveBranch)==120);
static_assert(sizeof(idMD6FusionBranch)==256);
static_assert(sizeof(idMD6TagFilter)==24);
static_assert(sizeof(idMD6BestLeaf)==136);
static_assert(sizeof(idMD6AliasEntry)==16);
static_assert(sizeof(idMD6Alias)==40);
static_assert(offsetof(idMD6Alias, mappings)==16);
static_assert(sizeof(idMD6Anim)==168);
static_assert(offsetof(idMD6Anim, name)==8);
static_assert(offsetof(idMD6Anim, animData)==56);
static_assert(offsetof(idMD6Anim, translatedBounds)==72);
static_assert(sizeof(idMD6Model)==328);
static_assert(offsetof(idMD6Model, name)==8);
static_assert(offsetof(idMD6Model, skeleton)==296);
static_assert(offsetof(idMD6Model, meshes)==176);
static_assert(offsetof(idMD6Model, jointRemap)==200);
class idMD6Allocator {
public:
  virtual ~idMD6Allocator()=default;
  virtual idMD6Node* Alloc(idMD6Node::nodeType_t type)=0;
  virtual void FreeMem(idMD6Node*node)=0;
  virtual void Condense()=0;
  virtual std::size_t Size()const=0;
  virtual int NumNodes()const=0;
  virtual idMD6Node* NodeForIndex(int index)=0;
  virtual const idMD6Node* NodeForIndex(int index)const=0;
};
