#include <run_time/values_global.hpp>

namespace art {
    values_global::values_global(values_global* parent)
        : parent(parent) {}

    values_global::~values_global() {
        clear();
    }

    typed_lgr<values_global> values_global::join_namespace(const art::ustring& str) {
        auto it = namespaces.find(str);
        if (it == namespaces.end())
            return namespaces[str] = new values_global(this);
        return it->second;
    }

    typed_lgr<values_global> values_global::join_namespace(const std::initializer_list<art::ustring>& strs) {
        typed_lgr<values_global> current_namespace(this, true);
        for (auto& str : strs) {
            auto it = current_namespace->namespaces.find(str);
            if (it == current_namespace->namespaces.end()) {
                current_namespace = namespaces[str] = new values_global(this);
            } else
                current_namespace = it->second;
        }
        return current_namespace;
    }

    bool values_global::has_namespace(const art::ustring& str) {
        return namespaces.contains(str);
    }

    bool values_global::has_namespace(const std::initializer_list<art::ustring>& strs) {
        typed_lgr<values_global> current_namespace(this, true);
        for (auto& str : strs) {
            auto it = current_namespace->namespaces.find(str);
            if (it == current_namespace->namespaces.end())
                return false;
            current_namespace = it->second;
        }
        return true;
    }

    void values_global::remove_namespace(const art::ustring& str) {
        auto it = namespaces.find(str);
        if (it != namespaces.end())
            namespaces.erase(it);
    }

    void values_global::remove_namespace(const std::initializer_list<art::ustring>& strs) {
        typed_lgr<values_global> prev_namespace(this, true);
        typed_lgr<values_global> current_namespace(this, true);
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

    void values_global::clear() {
        namespaces.clear();
        value = nullptr;
    }

    ValueItem* values_global::find_value(const art::ustring& str) {
        typed_lgr<values_global> current_namespace(this, true);
        while (current_namespace) {
            auto it = current_namespace->namespaces.find(str);
            if (it == current_namespace->namespaces.end())
                current_namespace = current_namespace->parent;
            else
                return &it->second->value;
        }
        return nullptr;
    }

    ValueItem* values_global::find_value_local(const art::ustring& str) {
        auto it = namespaces.find(str);
        if (it == namespaces.end())
            return nullptr;
        return &it->second->value;
    }

    ValueItem* values_global::find_auto_join(const art::ustring& str, const art::ustring& separator) {
        list_array<ustring> separated = str.split(separator);
        auto last = separated.take_back();
        typed_lgr<values_global> current_namespace(this, true);
        for (auto& it : separated)
            current_namespace = current_namespace->join_namespace(it);
        return current_namespace->find_value(last);
    }

    ValueItem* values_global::find_value_local_auto_join(const art::ustring& str, const art::ustring& separator) {
        list_array<ustring> separated = str.split(separator);
        auto last = separated.take_back();
        typed_lgr<values_global> current_namespace(this, true);
        for (auto& it : separated)
            current_namespace = current_namespace->join_namespace(it);
        return current_namespace->find_value_local(last);
    }

    bool values_global::depth_safety() {
        for (auto& [name, env] : namespaces)
            if (!env.depth_safety())
                return false;
        return true;
    }
}