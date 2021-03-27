#pragma once

#include "pch.h"

#include "ChainLayerMeta.h"

// ChainLayerMeta(const GCodeParser& gcp, const double zLayer)

class TheoreticalModel : public ChainLayerMeta
{
public:
    TheoreticalModel(const GCodeParser& gcp, const double zLayer);
};

class CODEXModel : public ChainLayerMeta
{
public:
    CODEXModel(const GCodeParser& gcp, const double zLayer);
};

class CurrentModel : public ChainLayerMeta
{
public:
    CurrentModel(const GCodeParser& gcp, const double zLayer);
};
