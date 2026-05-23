#include "pch.h"

#include "CompoundDisplacementHandler.h"

using namespace Microsoft::WRL;

CCompoundDisplacementHandler::DisplacementEvaluationSet::DisplacementEvaluationSet()
{
    right = CreateRefCountedObj<DisplacementEvaluation>();
    left = CreateRefCountedObj<DisplacementEvaluation>();
    down = CreateRefCountedObj<DisplacementEvaluation>();
    up = CreateRefCountedObj<DisplacementEvaluation>();

    right->direction = DD_RIGHT;
    left->direction = DD_LEFT;
    down->direction = DD_DOWN;
    up->direction = DD_UP;
}

Geometry::CPoint CCompoundDisplacementHandler::_MovePoint(
    const Geometry::CPoint& point, const DISPLACEMENT_DIRECTION direction, const int amount)
{
    Geometry::CPoint p = point;
    switch (direction)
    {
        case DD_RIGHT:
            p.Offset(amount, 0);
            break;
        case DD_LEFT:
            p.Offset(-amount, 0);
            break;
        case DD_DOWN:
            p.Offset(0, amount);
            break;
        case DD_UP:
            p.Offset(0, -amount);
            break;
    }

    return p;
}
