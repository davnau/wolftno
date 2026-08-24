#include "engine/models/transparency/jobs/influencespherecull.h"

#include "shared/idlib/parallelism/paralleljoblist.h"

#include <array>
#include <cstdint>

void CullInfluenceSpheresForView(const influenceSphereCullParms_t* parms) {
  std::array<int, 1024> sortIndex;
  std::array<std::uint8_t, 1024> keep;
  int keptCount = 0;
  int sortedCount = 0;

  for (int index = 0; index < parms->numInfluenceSpheres; ++index) {
    const influenceSphere_t& sphere = parms->influenceSpheres[index];
    keep[index] = 0;
    const idVec3 delta = sphere.center - parms->viewOrg;
    const float depth = parms->viewFwd * delta;
    const float diameter = sphere.outerRadius * 2.0f;
    if (parms->nearDist > depth + diameter ||
        depth - diameter > parms->farDist) {
      continue;
    }

    const idVec3 perpendicular = delta - parms->viewFwd * depth;
    const float horizontal = perpendicular * parms->viewLeft;
    const float horizontalExtent = depth * parms->fov.x;
    if (-horizontalExtent > horizontal + diameter ||
        horizontal - diameter > horizontalExtent) {
      continue;
    }
    const float vertical = perpendicular * parms->viewUp;
    const float verticalExtent = depth * parms->fov.y;
    if (-verticalExtent > vertical + diameter ||
        vertical - diameter > verticalExtent) {
      continue;
    }

    int insertion = 0;
    while (insertion < sortedCount &&
           sphere.angle <=
               parms->influenceSpheres[sortIndex[insertion]].angle) {
      ++insertion;
    }
    for (int move = sortedCount; move > insertion; --move)
      sortIndex[move] = sortIndex[move - 1];
    sortIndex[insertion] = index;
    keep[index] = 1;
    if (keptCount >= 256) {
      keep[sortIndex[sortedCount]] = 0;
    } else {
      ++keptCount;
      ++sortedCount;
    }
  }

  int visibleCount = 0;
  for (int index = 0; index < parms->numInfluenceSpheres; ++index) {
    if (keep[index] != 0)
      parms->visibleInfluenceSpheres->spheres[visibleCount++] =
          parms->influenceSpheres[index];
  }
  parms->visibleInfluenceSpheres->numSpheres = visibleCount;
}

idParallelJobRegistration register_CullInfluenceSpheresForView(
    reinterpret_cast<jobRun_t>(CullInfluenceSpheresForView),
    "CullInfluenceSpheresForView");
