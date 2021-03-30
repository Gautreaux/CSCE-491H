#include "RecomputeFrameworks.h"

TheoreticalModel::TheoreticalModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
}

CODEXModel::CODEXModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
}

CurrentModel::CurrentModel(const GCodeParser& gcp, const double zLayer) :
ChainLayerMeta(gcp, zLayer)
{
}

RelaxedCurrentModel::RelaxedCurrentModel(const GCodeParser& gcp, const double zLayer) :
CurrentModel(gcp, zLayer)
{
}