#include "RecomputeFrameworks.h"

TheoreticalModel::TheoreticalModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache(0);
#endif
}

CODEXModel::CODEXModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache(1);
#endif
}

CurrentModel::CurrentModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache(2);
#endif
}

RelaxedCurrentModel::RelaxedCurrentModel(const GCodeParser& gcp, const double zLayer) :
CurrentModel(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache(3);
#endif
}