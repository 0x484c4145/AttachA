#include <run_time/types_global.hpp>

namespace art {
    types_global::types_global(types_global* parent)
        : parent(parent) {}

    types_global::~types_global() {
        clear();
    }

    typed_lgr<types_global> types_global::join_namespace(const art::ustring& str) {
        auto it = namespaces.find(str);
        if (it == namespaces.end())
            return namespaces[str] = new types_global(this);
        return it->second;
    }

    typed_lgr<types_global> types_global::join_namespace(const std::initializer_list<art::ustring>& strs) {
        typed_lgr<types_global> current_namespace(this, true);
        for (auto& str : strs) {
            auto it = current_namespace->namespaces.find(str);
            if (it == current_namespace->namespaces.end()) {
                current_namespace = namespaces[str] = new types_global(this);
            } else
                current_namespace = it->second;
        }
        return current_namespace;
    }

    bool types_global::has_namespace(const art::ustring& str) {
        return namespaces.contains(str);
    }

    bool types_global::has_namespace(const std::initializer_list<art::ustring>& strs) {
        typed_lgr<types_global> current_namespace(this, true);
        for (auto& str : strs) {
            auto it = current_namespace->namespaces.find(str);
            if (it == current_namespace->namespaces.end())
                return false;
            current_namespace = it->second;
        }
        return true;
    }

    void types_global::remove_namespace(const art::ustring& str) {
        auto it = namespaces.find(str);
        if (it != namespaces.end())
            namespaces.erase(it);
    }

    void types_global::remove_namespace(const std::initializer_list<art::ustring>& strs) {
        typed_lgr<types_global> prev_namespace(this, true);
        typed_lgr<types_global> current_namespace(this, true);
        decltype(namespaces)::iterator it = current_namespace->namespaces.end();
        for (auto& str : strs) {
            auto it = current_namespace->namespaces.find(str);
            if (it == current_namespace->namespaces.end())
                return;
            prev_namespace = current_namespace;
            current_namespace = it->second;
        }
        prev_namespace->namespaces.erase(it);
    }

    void types_global::clear() {
        namespaces.clear();
        value = nullptr;
    }

    VirtualTable types_global::find_value(const art::ustring& str) {
        typed_lgr<types_global> current_namespace(this, true);
        while (current_namespace) {
            auto it = current_namespace->namespaces.find(str);
            if (it == current_namespace->namespaces.end())
                current_namespace = current_namespace->parent;
            else
                return it->second->value;
        }
        return nullptr;
    }

    VirtualTable types_global::find_value_local(const art::ustring& str) {
        auto it = namespaces.find(str);
        if (it == namespaces.end())
            return nullptr;
        return it->second->value;
    }

    VirtualTable types_global::find_auto_join(const art::ustring& str, const art::ustring& separator) {
        list_array<ustring> separated = str.split(separator);
        auto last = separated.take_back();
        typed_lgr<types_global> current_namespace(this, true);
        for (auto& it : separated)
            current_namespace = current_namespace->join_namespace(it);
        return current_namespace->find_value(last);
    }

    VirtualTable types_global::find_value_local_auto_join(const art::ustring& str, const art::ustring& separator) {
        list_array<ustring> separated = str.split(separator);
        auto last = separated.take_back();
        typed_lgr<types_global> current_namespace(this, true);
        for (auto& it : separated)
            current_namespace = current_namespace->join_namespace(it);
        return current_namespace->find_value_local(last);
    }

    bool types_global::depth_safety() {
        for (auto& [name, env] : namespaces)
            if (!env.depth_safety())
                return false;
        return true;
    }
}