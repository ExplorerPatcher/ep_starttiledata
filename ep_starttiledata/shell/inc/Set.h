#pragma once

#include "HrNew.h"
#include "SimpleArray.h"
#include "SimpleHashTable.h"
#include "SimpleHashTableHelpers.h"

enum INTERSECTION_TYPE
{
    INTERSECTION_BOTH = 0,
    INTERSECTION_THIS = 1,
    INTERSECTION_OTHER = 2,
};

template <typename TItem>
class CSet
{
public:
    CSet()
        : _pTable(nullptr)
    {
    }

    ~CSet()
    {
        if (_pTable)
        {
            delete _pTable;
        }
    }

    HRESULT Add(const CSet& other)
    {
        HRESULT hr = S_OK;
        other.Enumerate([&hr, this](const TItem& item) -> bool
        {
            hr = Add(item);
            return SUCCEEDED(hr);
        });

        return hr;
    }

    HRESULT Add(const TItem& item)
    {
        HRESULT hr = S_OK;

        if (!_pTable)
        {
            hr = HrNew<CSimpleHashTable<TItem, bool>>(&_pTable);
        }

        if (SUCCEEDED(hr))
        {
            hr = _pTable->AddItem(item, true);
        }

        return hr;
    }

    HRESULT Contains(const TItem& item) const
    {
        return _pTable ? _pTable->ContainsKey(item) : TYPE_E_ELEMENTNOTFOUND;
    }

    size_t GetCount() const
    {
        return _pTable ? _pTable->GetItemCount() : 0;
    }

    HRESULT Remove(const TItem& item)
    {
        return _pTable ? _pTable->DeleteItem(item) : TYPE_E_ELEMENTNOTFOUND;
    }

    void RemoveAll()
    {
        if (_pTable)
        {
            _pTable->RemoveAll();
        }
    }

    HRESULT Single(TItem* pItem) const
    {
        if (!_pTable || _pTable->GetItemCount() != 1)
        {
            return E_FAIL;
        }

        SimpleHashTableHelpers::Enumerate(*_pTable, [&pItem](const TItem& key, bool) -> bool
        {
            *pItem = key;
            return true;
        });

        return S_OK;
    }

    void Swap(CSet* pOther)
    {
        CSimpleHashTable<TItem, bool>* pOtherTable = pOther->_pTable;
        pOther->_pTable = _pTable;
        _pTable = pOtherTable;
    }

    HRESULT AppendToArray(CCoSimpleArray<TItem>* pArray)
    {
        HRESULT hr = pArray->_EnsureCapacity(pArray->GetSize() + GetCount());

        if (SUCCEEDED(hr))
        {
            Enumerate([&hr, &pArray](TItem item) -> bool
            {
                hr = pArray->Add(item);
                return SUCCEEDED(hr);
            });
        }

        return hr;
    }

    template <typename TCallback>
    bool Enumerate(const TCallback& callback) const
    {
        if (!_pTable)
        {
            return true;
        }

        return SimpleHashTableHelpers::Enumerate(*_pTable, [callback](const TItem& key, bool) -> bool
        {
            return callback(key);
        });
    }

    template <typename TCallback>
    bool EnumerateIntersection(const CSet& other, TCallback callback)
    {
        bool fContinue = true;

        if (this->_pTable)
        {
            fContinue = SimpleHashTableHelpers::Enumerate(*this->_pTable, [&other, &callback](const TItem& key, bool) -> bool
            {
                return callback(key, SUCCEEDED(other.Contains(key)) ? INTERSECTION_BOTH : INTERSECTION_THIS);
            });
        }

        if (fContinue && other._pTable)
        {
            fContinue = SimpleHashTableHelpers::Enumerate(*other._pTable, [this, &callback](const TItem& key, bool) -> bool
            {
                return SUCCEEDED(this->Contains(key)) || callback(key, INTERSECTION_OTHER);
            });
        }

        return fContinue;
    }

private:
    CSimpleHashTable<TItem, bool>* _pTable;
};
