#pragma once

#include "BaseDisplacementHandler.h"
#include "ItemLayoutResolver.h"
#include "RefCountedObject.h"

class CCompoundDisplacementHandler
    : public Microsoft::WRL::RuntimeClass<Microsoft::WRL::RuntimeClassFlags<Microsoft::WRL::ClassicCom>
        , IItemLayoutDisplacementHandler
    >
{
public:
    CCompoundDisplacementHandler();

    //~ Begin IItemLayoutDisplacementHandler Interface
    STDMETHODIMP SetCellArray(ICellArrayManager*) override;
    STDMETHODIMP DisplaceItemsFromRect(const Geometry::CRect& targetRect, const Geometry::CRect& previousRect) override;
    //~ End IItemLayoutDisplacementHandler Interface

    struct PendingCellCoordinates
    {
        GUID cell;
        Geometry::CPoint position;
    };

    using RefCountedPendingCellCoordinates = Microsoft::WRL::ComPtr<CRefCountedObject<PendingCellCoordinates>>;

    class DisplacementEvaluation
    {
    public:
        DISPLACEMENT_DIRECTION direction;
        bool possible;
        int distance;
        CCoSimpleArray<RefCountedPendingCellCoordinates> chain;
        Geometry::CPoint position;

        void Clear();
    };

    using RefCountedDisplacementEvaluation = Microsoft::WRL::ComPtr<CRefCountedObject<DisplacementEvaluation>>;

    class DisplacementEvaluationSet
    {
    public:
        RefCountedDisplacementEvaluation right;
        RefCountedDisplacementEvaluation left;
        RefCountedDisplacementEvaluation down;
        RefCountedDisplacementEvaluation up;

        RefCountedDisplacementEvaluation selected;

        DisplacementEvaluationSet();
        void Clear();
    };

    enum class ShiftCriteria
    {
        Before = 0,
        After = 1
    };

private:
    HRESULT _ApplyPendingMove(REFGUID collision, RefCountedDisplacementEvaluation& move);
    HRESULT _AttemptAdjacentBlockMove(
        const Geometry::CRect& position, const Geometry::CRect& previousPosition, const CSet<GUID>* cellsAtDestination,
        const bool isSizeChange, bool* success);
    HRESULT _CheckPossibleMove(REFGUID cell, const Geometry::CPoint& target, GUID* pItemID);
    Geometry::CRect _GetAdjacentSpace(
        const Geometry::CRect& position, const Geometry::CRect& previousPosition,
        const Geometry::CPoint& collisionBottomRight, const Geometry::CPoint& collisionTopLeft,
        const bool isSizeChange);
    Geometry::CRect _GetEmptyAdjacentSpace(
        const Geometry::CRect& position, const Geometry::CSize& size, const DISPLACEMENT_DIRECTION direction);
    HRESULT _PrepareChain(
        REFGUID cell, const Geometry::CSize& acceptedSize, const DISPLACEMENT_DIRECTION direction,
        const int moveAmount, const int linkCount, CCoSimpleArray<RefCountedPendingCellCoordinates>* pItemsInChain);
    HRESULT _ShiftRows(const ShiftCriteria criteria, const int y, const int yOffset);
    HRESULT _TestPosibleMove(REFGUID collision, const Geometry::CRect& position, DisplacementEvaluationSet& move);
    HRESULT _TestIfPossibleToMoveObstructionInDirection(
        REFGUID itemID, const Geometry::CRect& collisionPosition, RefCountedDisplacementEvaluation& move);
    Geometry::CPoint _MovePoint(const Geometry::CPoint& point, const DISPLACEMENT_DIRECTION direction, const int amount);

    ICellArrayManager* m_cellArrayManager;
};
