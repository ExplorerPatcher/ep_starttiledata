#pragma once

#include "SimpleHashTable.h"

namespace SimpleHashTableHelpers
{
    namespace detail
    {
        template <typename TKey, typename TValue, typename TCallback>
        class CLambdaEnumCallback
        {
        public:
            CLambdaEnumCallback(const TCallback& callback)
                : _callback(callback)
            {
            }

            const TCallback& _callback;

            BOOL Enum(const TKey& key, const TValue& value)
            {
                return _callback(key, value) != 0;
            }
        };
    }

    template <typename TKey, typename TValue, typename TCallback>
    bool Enumerate(const CSimpleHashTable<TKey, TValue>& hashTable, const TCallback& callback)
    {
        detail::CLambdaEnumCallback<TKey, TValue, TCallback> cb(callback);
        return hashTable.EnumCallback(cb) != 0;
    }
}
