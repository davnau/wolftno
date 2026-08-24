#include "declreachik.h"

idDeclInfoTemplate<idDeclReachIK> idDeclReachIK::resourceList(
    "reachIK", "idDeclReachIK", "reachik", false);
idDeclInfoTemplate<idDeclHandIK> idDeclHandIK::resourceList(
    "handIK", "idDeclHandIK", "handik", false);

idDeclReachIK::idDeclReachIK() {
  resourceListPtr = &resourceList;
}

idDeclReachIK::~idDeclReachIK() = default;

void idDeclReachIK::FreeData() {
  armInfo.Clear();
}

idDeclInfo* idDeclReachIK::GetDeclInfo() {
  return &resourceList;
}

idDeclHandIK::idDeclHandIK() {
  resourceListPtr = &resourceList;
}

idDeclHandIK::~idDeclHandIK() = default;

void idDeclHandIK::FreeData() {
  handInfo.Clear();
}

idDeclInfo* idDeclHandIK::GetDeclInfo() {
  return &resourceList;
}
