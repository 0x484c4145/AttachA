// Copyright Danyil Melnytskyi 2022-Present
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#pragma once
#include <unordered_map>

#include <run_time/attacha_abi_structs.hpp>
#include <util/link_garbage_remover.hpp>

namespace art {
    class values_global {
        std::unordered_map<art::ustring, typed_lgr<values_global>, art::hash<art::ustring>> namespaces;
        typed_lgr<values_global> parent;

    public:
        ValueItem value;

        values_global(values_global* parent = nullptr);
        ~values_global();

        typed_lgr<values_global> join_namespace(const art::ustring& str);
        typed_lgr<values_global> join_namespace(const std::initializer_list<art::ustring>& strs);
        bool has_namespace(const art::ustring& str);
        bool has_namespace(const std::initializer_list<art::ustring>& strs);
        void remove_namespace(const art::ustring& str);
        void remove_namespace(const std::initializer_list<art::ustring>& strs);
        void clear();
        //finds value in current env and if not found, try find in parent
        ValueItem* find_value(const art::ustring& str);
        ValueItem* find_value_local(const art::ustring& str);
        ValueItem* find_auto_join(const art::ustring& str, const art::ustring& separator);
        ValueItem* find_value_local_auto_join(const art::ustring& str, const art::ustring& separator);


        bool depth_safety();
    };
}