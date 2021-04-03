#include "RecomputeFrameworks.h"

TheoreticalModel::TheoreticalModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache();
#endif
}

CODEXModel::CODEXModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache();
#endif
}

CurrentModel::CurrentModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache();
#endif
}

RelaxedCurrentModel::RelaxedCurrentModel(const GCodeParser& gcp, const double zLayer) :
CurrentModel(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache();
#endif
}