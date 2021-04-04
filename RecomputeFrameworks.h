#pragma once

#include "pch.h"

#include "ChainStarHelper.h"
#include "ChainLayerMeta.h"

// ChainLayerMeta(const GCodeParser& gcp, const double zLayer)

class TheoreticalModel : public ChainLayerMeta
{
public:
    TheoreticalModel(const GCodeParser& gcp, const double zLayer);

    //return true if A1 can move (and print) A1Seg while A2 Seg does the same
    //  For this model,
    //      A1 and A2 are sufficiently seperated
    virtual inline bool canMoveSegmentPair(
        const LineSegment& a1Seg, const LineSegment& a2Seg,
        const bool isA1Print = false, const bool isA2Print = false
    ) const override {
        return theoretical_canMoveSegmentPair(a1Seg, a2Seg, isA1Print, isA2Print);
    }

    //return true if the A1 position and the A2 position can be held concurrently
    //  For this model,
    //      A1 position and A2 position are sufficiently seperated
    virtual inline bool isValidPositionPair(
        const Point3& a1Pos, const Point3& a2Pos
    ) const override {
        return DOUBLE_GEQ(getPointDistance(a1Pos, a2Pos), CHAIN_MIN_SEPERATION_MM);
    }

    //return true if A2 can hold A2 pos while A1 moves the A1 segment
    //  not necessarily symmetric with other
    //  For this model,
    //      A2 position is far enough from the segment
    virtual inline bool isValidSegmentPosition(
        const LineSegment& a1Seg, const Point3 &a2Pos
    ) const override {
        return DOUBLE_GEQ(a1Seg.minSeperationDistance(a2Pos), CHAIN_MIN_SEPERATION_MM);
    }

    //return true if A1 can gold A1 pos while A2 moves the A2 segment
    //  not necessarily symmetric with other
    //  For this model,
    //      A1 position is far enough from the segment
    virtual inline bool isValidSegmentPosition(
        const Point3& a1Pos, const LineSegment& a2Seg
    ) const override {
        return DOUBLE_GEQ(a2Seg.minSeperationDistance(a1Pos), CHAIN_MIN_SEPERATION_MM);
    }
    
};

class CODEXModel : public ChainLayerMeta
{
public:
    CODEXModel(const GCodeParser& gcp, const double zLayer);

    //return true if A1 can move (and print) A1Seg while A2 Seg does the same
    //  For this model,
    //      A1 and A2 are sufficiently seperated
    //      All A1.x <= All A2.x
    virtual inline bool canMoveSegmentPair(
        const LineSegment& a1Seg, const LineSegment& a2Seg,
        const bool isA1Print = false, const bool isA2Print = false
    ) const override {
        return (codex_canMoveSegmentPair(a1Seg, a2Seg, isA1Print, isA2Print));
    }

    //return true if the A1 position and the A2 position can be held concurrently
    //  For this model, 
    //      A1 and A2 are sufficiently seperated
    //      A1.x <= A2.x
    virtual inline bool isValidPositionPair(
        const Point3& a1Pos, const Point3& a2Pos
    ) const override {
        return  (
            DOUBLE_LEQ(a1Pos.getX(), a1Pos.getY()) &&
            DOUBLE_GEQ(getPointDistance(a1Pos, a2Pos), CHAIN_MIN_SEPERATION_MM)
        );
    }

    //return true if A2 can hold A2 pos while A1 moves the A1 segment
    //  not necessarily symmetric with other
    //  For this model,
    //      A2 position is far enough from the segment
    //      A2 position.x >= all x-coordinates on a1 segment
    virtual inline bool isValidSegmentPosition(
        const LineSegment& a1Seg, const Point3 &a2Pos
    ) const override {
        return (
            DOUBLE_GEQ(a2Pos.getX(), std::max(a1Seg.getStartPoint().getX(), a1Seg.getEndPoint().getX())) &&
            DOUBLE_GEQ(a1Seg.minSeperationDistance(a2Pos), CHAIN_MIN_SEPERATION_MM)
        );
    }

    //return true if A1 can gold A1 pos while A2 moves the A2 segment
    //  not necessarily symmetric with other
    //  For this model,
    //      A1 position is far enough from the segment
    //      A1 position.x <= all x-coordinates on a2 segment
    virtual inline bool isValidSegmentPosition(
        const Point3& a1Pos, const LineSegment& a2Seg
    ) const override {
        return (
            DOUBLE_LEQ(a1Pos.getX(), std::min(a2Seg.getStartPoint().getX(), a2Seg.getEndPoint().getX())) &&
            DOUBLE_GEQ(a2Seg.minSeperationDistance(a1Pos), CHAIN_MIN_SEPERATION_MM)
        );
    }
};

class CurrentModel : public ChainLayerMeta
{
public:
    CurrentModel(const GCodeParser& gcp, const double zLayer);

    //return true if A1 can move (and print) A1Seg while A2 Seg does the same
    //  For this model,
    //      A1 and A2 are sufficiently seperated
    //      A1 start/end is valid with A2 start/end
    virtual inline bool canMoveSegmentPair(
        const LineSegment& a1Seg, const LineSegment& a2Seg,
        const bool isA1Print = false, const bool isA2Print = false
    ) const override {
        return current_canMoveSegmentPair(a1Seg, a2Seg, isA1Print, isA2Print);
    }

    //return true if the A1 position and the A2 position can be held concurrently
    //  For this model,
    //      A1 and A2 are sufficiently seperated
    //      A1.x == A2.x
    //      A1.y > A2.y
    virtual inline bool isValidPositionPair(
        const Point3& a1Pos, const Point3& a2Pos
    ) const override {
        return  current_isValidPositionPair(a1Pos, a2Pos);
    }

    //return true if A2 can hold A2 pos while A1 moves the A1 segment
    //  not necessarily symmetric with other
    //  For this model,
    //      A2 position.y + MIN_SEPERATION <= all y-coordinates on a1 segment
    virtual inline bool isValidSegmentPosition(
        const LineSegment& a1Seg, const Point3 &a2Pos
    ) const override{
        return (
            DOUBLE_LEQ((a2Pos.getY() + CHAIN_MIN_SEPERATION_MM), 
            std::min(a1Seg.getStartPoint().getY(), a1Seg.getEndPoint().getY()))
        );
    }

    //return true if A1 can gold A1 pos while A2 moves the A2 segment
    //  not necessarily symmetric with other
    //  For this model,
    //      A1 position.y - MIN_SEPERATION >= all y-coordinates on a2 segment     
    virtual inline bool isValidSegmentPosition(
        const Point3& a1Pos, const LineSegment& a2Seg
    ) const override{
        return (
            DOUBLE_GEQ((a1Pos.getY() - CHAIN_MIN_SEPERATION_MM), 
            std::max(a2Seg.getStartPoint().getY(), a2Seg.getEndPoint().getY()))
        );
    }
};

class RelaxedCurrentModel : public CurrentModel
{
public:
    RelaxedCurrentModel(const GCodeParser& gcp, const double zLayer);

    //relaxed constraints on the is valid position pair
    virtual inline bool isValidPositionPair(
        const Point3& a1Pos, const Point3& a2Pos
    ) const override {
        return relaxed_isValidPositionPair(a1Pos, a2Pos);
    }

    //return true if A1 can move (and print) A1Seg while A2 Seg does the same
    //  For this model,
    //      A1 and A2 are sufficiently seperated
    virtual inline bool canMoveSegmentPair(
        const LineSegment& a1Seg, const LineSegment& a2Seg,
        const bool isA1Print = false, const bool isA2Print = false
    ) const override {
        return relaxed_canMoveSegmentPair(a1Seg, a2Seg, isA1Print, isA2Print);
    }
};