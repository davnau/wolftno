#include "declrollbones.h"

idDeclInfoTemplate<idDeclRollBones> idDeclRollBones::resourceList(
    "rollBones", "idDeclRollBones", "rollbones", false);

idDeclRollBones::idDeclRollBones() {
  resourceListPtr = &resourceList;
}

idDeclInfo* idDeclRollBones::GetDeclInfo() {
  return &resourceList;
}
