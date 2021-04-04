#include "RecomputeFrameworks.h"

TheoreticalModel::TheoreticalModel(const GCodeParser& gcp, const double zLayer, const unsigned int id, std::ostream& outStream) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache(0, id, outStream);
#endif
}

CODEXModel::CODEXModel(const GCodeParser& gcp, const double zLayer, const unsigned int id, std::ostream& outStream) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache(1, id, outStream);
#endif
}

CurrentModel::CurrentModel(const GCodeParser& gcp, const double zLayer, const unsigned int id, std::ostream& outStream) :
ChainLayerMeta(gcp, zLayer)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache(2, id, outStream);
#endif
}

RelaxedCurrentModel::RelaxedCurrentModel(const GCodeParser& gcp, const double zLayer, const unsigned int id, std::ostream& outStream) :
CurrentModel(gcp, zLayer, id, outStream)
{
#ifdef PRECACHE_SEGMENT_COLLISIONS
    buildPreCache(3, id, outStream);
#endif
}