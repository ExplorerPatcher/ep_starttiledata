#pragma once

#include <Windows.h>

class CStringCompare
{
public:
    static BOOL CompareKey(const WCHAR* key1, const WCHAR* key2)
    {
        return CompareStringOrdinal(key1, -1, key2, -1, FALSE) == CSTR_EQUAL;
    }
};

class CCaseInsensitiveStringCompare
{
public:
    static BOOL CompareKey(const WCHAR* lpString1, const WCHAR* lpString2)
    {
        return CompareStringOrdinal(lpString1, -1, lpString2, -1, TRUE) == CSTR_EQUAL;
    }
};

inline UINT CalculateZobelHash(const void* pvKey, size_t cbKey)
{
    UINT hash = 0x12B9B0A5;
    for (size_t pos = 0; pos < cbKey; pos++)
    {
        int data = *((PBYTE)pvKey + pos);
        hash ^= (hash >> 2) + 2080 * hash + data;
    }
    return hash & ~0x80000000;
}

template <typename TKey>
class CDefaultHashPolicy
{
public:
    static UINT CalculateHashKey(const TKey& key, UINT cBuckets)
    {
        return CalculateZobelHash(&key, sizeof(key)) % cBuckets;
    }
};

template <typename TKey>
class CDefaultKeyCompare
{
public:
    static BOOL CompareKey(const TKey& key1, const TKey& key2)
    {
        return key1 == key2;
    }
};

class CDefaultResizePolicy
{
public:
    static BOOL ShouldResize(UINT cItems, UINT cBuckets)
    {
        return cItems > cBuckets / 2;
    }
};

class CDefaultRehashPolicy
{
public:
    static BOOL ShouldRehash(UINT, UINT cBuckets, UINT cFreedBuckets)
    {
        return cFreedBuckets > cBuckets / 2;
    }
};

class CStringHashPolicy
{
public:
    static UINT CalculateHashKey(const WCHAR* key, UINT cBuckets)
    {
        return CalculateZobelHash(key, sizeof(WCHAR) * wcslen(key)) % cBuckets;
    }
};

class CCaseInsensitiveStringHashPolicy
{
public:
    static UINT CalculateHashKey(const WCHAR* key, UINT cBuckets)
    {
        WCHAR szLowerBuf[260];
        wcsncpy_s(szLowerBuf, ARRAYSIZE(szLowerBuf), key, (rsize_t)-1);
        CharLowerBuffW(szLowerBuf, (DWORD)wcslen(szLowerBuf));
        return CStringHashPolicy::CalculateHashKey(szLowerBuf, cBuckets);
    }
};

template <
    typename TKey,
    typename TValue,
    typename THashPolicy = CDefaultHashPolicy<TKey>,
    typename TKeyCompare = CDefaultKeyCompare<TKey>,
    typename TResizePolicy = CDefaultResizePolicy,
    typename TRehashPolicy = CDefaultRehashPolicy
>
class CSimpleHashTable
{
public:
    CSimpleHashTable(UINT cBuckets = c_uDefaultBucketCount)
        : _cItems(0)
        , _cBuckets(cBuckets)
        , _cFreedBuckets(0)
        , _rgBuckets(nullptr)
    {
    }

    ~CSimpleHashTable()
    {
        RemoveAll();
    }

    UINT GetItemCount() const
    {
        return _cItems;
    }

    HRESULT AddItem(const TKey& key, const TValue& value)
    {
        return _AddUpdateItem(TRUE, key, value, nullptr);
    }

    HRESULT SetItem(const TKey& key, const TValue& value, TValue* pvalueOld = nullptr)
    {
        return _AddUpdateItem(FALSE, key, value, pvalueOld);
    }

    HRESULT GetItem(const TKey& key, TValue& value) const
    {
        HRESULT hr = _FailIfNotSet();
        if (SUCCEEDED(hr))
        {
            const HashBucket& hb = _LookupEntryC(key);
            if (hb.IsOccupied())
            {
                value = hb.Value();
                hr = S_OK;
            }
            else
            {
                hr = TYPE_E_ELEMENTNOTFOUND;
            }
        }

        return hr;
    }

    HRESULT GetItemPtr(const TKey& key, TValue** ppvalue)
    {
        HRESULT hr = _FailIfNotSet();
        if (SUCCEEDED(hr))
        {
            HashBucket& hb = _LookupEntry(key);
            if (hb.IsOccupied())
            {
                *ppvalue = &hb.Value();
                hr = S_OK;
            }
            else
            {
                hr = TYPE_E_ELEMENTNOTFOUND;
            }
        }

        return hr;
    }

    HRESULT ContainsKey(const TKey& key) const
    {
        HRESULT hr = _FailIfNotSet();
        if (SUCCEEDED(hr))
        {
            const HashBucket& hb = _LookupEntryC(key);
            if (hb.IsOccupied())
            {
                hr = S_OK;
            }
            else
            {
                hr = TYPE_E_ELEMENTNOTFOUND;
            }
        }

        return hr;
    }

    HRESULT DeleteItem(const TKey& key)
    {
        HRESULT hr = _EnsureSet();
        if (SUCCEEDED(hr))
        {
            HashBucket& hb = _LookupEntry(key);
            hr = _FreeBucket(&hb) ? S_OK : TYPE_E_ELEMENTNOTFOUND;
        }

        return hr;
    }

    void RemoveAll()
    {
        if (_rgBuckets)
        {
            for (UINT i = 0; i < _cBuckets; ++i)
            {
                _FreeBucket(&_rgBuckets[i]);
            }
            LocalFree(_rgBuckets);
            _rgBuckets = nullptr;
        }
        _cItems = 0;
    }

    // ULONG GetMemoryUsage();

    template <typename EnumClass>
    BOOL EnumCallback(EnumClass& enumClass)
    {
        const auto enumLambda = [&enumClass](const TKey& key, TValue& value) -> BOOL
        {
            return enumClass.Enum(key, value);
        };
        return Enum(s_EnumAdaptor, &enumLambda);
    }

    template <typename EnumClass>
    BOOL EnumCallback(EnumClass& enumClass) const
    {
        const auto enumLambda = [&enumClass](const TKey& key, TValue& value) -> BOOL
        {
            return enumClass.Enum(key, value);
        };
        return Enum(s_EnumAdaptor, &enumLambda);
    }

    template <typename T>
    BOOL Enum(const T& enumLambda) const
    {
        return Enum(s_EnumAdaptor, &enumLambda);
    }

    template <typename T>
    BOOL Enum(BOOL (*pfn)(T, const TKey&, TValue&), T pData) const
    {
        BOOL bContinue = TRUE;

        if (SUCCEEDED(_FailIfNotSet()))
        {
            for (UINT i = 0; i < _cBuckets && bContinue; ++i)
            {
                HashBucket& hb = _rgBuckets[i];
                if (hb.IsOccupied())
                {
                    bContinue = pfn(pData, hb.Key(), hb.Value()) != 0;
                }
            }
        }

        return bContinue;
    }

    template <typename RemoveIfClass>
    void RemoveIfCallback(RemoveIfClass& removeIfClass)
    {
        if (SUCCEEDED(_FailIfNotSet()))
        {
            for (UINT i = 0; i < _cBuckets; ++i)
            {
                HashBucket& hb = _rgBuckets[i];
                if (hb.IsOccupied() && removeIfClass.RemoveIf(hb.Key(), hb.Value()))
                {
                    _FreeBucket(&hb);
                }
            }
        }
    }

private:
    static const UINT c_uDefaultBucketCount = 31;

    class HashBucket
    {
        HashBucket(const HashBucket& other) = delete;

    public:
        HashBucket(const TKey& key, const TValue& value)
            : _key(key)
            , _value(value)
        {
            _state = ENTRYSTATE::OCCUPIED;
        }

        ~HashBucket()
        {
            _state = ENTRYSTATE::FREED;
        }

        void TransferHashEntry(const HashBucket& hbOld)
        {
            _state = hbOld._state;
            _key = hbOld._key;
            _value = hbOld._value;
        }

        /*void* operator new(size_t a, HashBucket* b) noexcept
        {
            return b;
        }*/

        BOOL IsOccupied() const { return _state == ENTRYSTATE::OCCUPIED; }
        BOOL IsEmpty() const { return _state == ENTRYSTATE::EMPTY; }
        BOOL IsFreed() const { return _state == ENTRYSTATE::FREED; }
        TKey& Key() { return _key; }
        const TKey& Key() const { return _key; }
        TValue& Value() { return _value; }
        const TValue& Value() const { return _value; }

    private:
        HashBucket& operator=(const HashBucket& other) = delete;

        typedef enum class ENTRYSTATE
        {
            EMPTY = 0,
            OCCUPIED = 1,
            FREED = 2
        } ENTRYSTATE;

        ENTRYSTATE _state;
        TKey _key;
        TValue _value;
    };

    // void _DebugConsistencyCheck();

    HRESULT _FailIfNotSet() const
    {
        return _rgBuckets ? S_OK : TYPE_E_ELEMENTNOTFOUND;
    }

    HRESULT _EnsureSet()
    {
        HRESULT hr = S_OK;

        if (!_rgBuckets)
        {
            hr = LocalAllocArray(_cBuckets, &_rgBuckets);
        }

        return hr;
    }

    BOOL _FreeBucket(HashBucket* phb)
    {
        BOOL bFreed = FALSE;

        if (phb->IsOccupied())
        {
            phb->~HashBucket();
            ++_cFreedBuckets;
            --_cItems;
            bFreed = TRUE;
        }

        return bFreed;
    }

    HRESULT _AddUpdateItem(BOOL fAdd, const TKey& key, const TValue& value, TValue* pvalueOld)
    {
        if (pvalueOld)
        {
            if constexpr (__is_constructible(TValue))
                new(pvalueOld) TValue();
        }

        HRESULT hr = _EnsureSet();
        if (SUCCEEDED(hr))
        {
            if (TResizePolicy::ShouldResize(_cItems, _cBuckets))
            {
                hr = _GrowTable();
            }
            else if (TRehashPolicy::ShouldRehash(_cItems, _cBuckets, _cFreedBuckets))
            {
                hr = _RehashTable(_cBuckets);
            }

            if (SUCCEEDED(hr))
            {
                HashBucket& hb = _LookupEntry(key);
                if (hb.IsOccupied())
                {
                    if (fAdd)
                    {
                        hr = E_INVALIDARG;
                    }
                    else
                    {
                        hr = S_OK;

                        if (pvalueOld)
                            *pvalueOld = hb.Value();

                        hb.Value() = value;
                    }
                }
                else
                {
                    if (hb.IsFreed())
                        --_cFreedBuckets;

                    new(&hb) HashBucket(key, value);
                    ++_cItems;

                    hr = S_OK;
                }
            }
        }

        return hr;
    }

    const HashBucket& _LookupEntryC(const TKey& key) const
    {
        return s_LookupEntry(_rgBuckets, _cBuckets, key);
    }

    HashBucket& _LookupEntry(const TKey& key)
    {
        return s_LookupEntry(_rgBuckets, _cBuckets, key);
    }

    static HashBucket& s_LookupEntry(HashBucket* rgBuckets, UINT cBuckets, const TKey& key)
    {
        UINT hash = THashPolicy::CalculateHashKey(key, cBuckets);
        int lastFreeHash = -1;
        while (true)
        {
            HashBucket& hb = rgBuckets[hash];

            if (hb.IsFreed() && lastFreeHash == -1)
            {
                lastFreeHash = hash;
            }
            else if (hb.IsEmpty())
            {
                if (lastFreeHash != -1)
                    hash = lastFreeHash;
                break;
            }

            if (hb.IsOccupied() && TKeyCompare::CompareKey(rgBuckets[hash].Key(), key))
            {
                break;
            }

            hash = (hash + 1) % cBuckets;
        }

        return rgBuckets[hash];
    }

    HRESULT _GrowTable()
    {
        return _RehashTable(2 * _cBuckets - 1);
    }

    HRESULT _RehashTable(UINT cNewBuckets)
    {
        HashBucket* rgBucketsNew = nullptr;
        HRESULT hr = LocalAllocArray(cNewBuckets, &rgBucketsNew);
        if (SUCCEEDED(hr))
        {
            for (UINT i = 0; i < _cBuckets; ++i)
            {
                HashBucket& hb = _rgBuckets[i];
                if (hb.IsOccupied())
                {
                    HashBucket& hbNew = s_LookupEntry(rgBucketsNew, cNewBuckets, hb.Key());
                    hbNew.TransferHashEntry(hb);
                }
            }

            LocalFree(_rgBuckets);
            _cFreedBuckets = 0;
            _rgBuckets = rgBucketsNew;
            _cBuckets = cNewBuckets;
        }

        return hr;
    }

    UINT _CalculateHashKey(const TKey& key, UINT cBuckets) const
    {
        return THashPolicy::HashKey(key) % cBuckets;
    }

    template <typename T>
    static BOOL s_EnumAdaptor(const T* plambda, const TKey& key, TValue& value)
    {
        return (*plambda)(key, value);
    }

    UINT _cItems;
    UINT _cBuckets;
    UINT _cFreedBuckets;
    HashBucket* _rgBuckets;
};
