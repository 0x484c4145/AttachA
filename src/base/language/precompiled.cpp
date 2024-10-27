#include <base/language/precompiled.hpp>
#include <run_time/AttachA_CXX.hpp>
#include <run_time/library/bytes.hpp>

using namespace art;

namespace language_parsers {


    template <class T>
    T read_value(files::FileHandle& file) {
        T value;
        file.read_fixed((uint8_t*)&value, sizeof(T));
        return bytes::convert_endian<bytes::Endian::little>(value);
    }

    std::vector<uint8_t> read_byte_arr(files::FileHandle& file) {
        uint64_t arr_length = read_value<uint64_t>(file);
        std::vector<uint8_t> array(arr_length);
        file.read_fixed(array.data(), arr_length);
        return array;
    }

    ustring read_string(files::FileHandle& file) {
        auto string_value = read_byte_arr(file);
        return ustring((char*)string_value.data(), string_value.size());
    }

    patch_list precompiled::handle_init(files::FileHandle& file) {
        lock_guard guard(mutex);
        patch_list build_patch_list;
        auto local_functions = declared_functions[file.get_path()];
        std::unordered_set<ustring, hash<ustring>> readed_functions;
        uint64_t _functions_count;
        file.read_fixed((uint8_t*)&_functions_count, (uint32_t)sizeof(uint64_t));
        _functions_count = bytes::convert_endian<bytes::Endian::little>(_functions_count);
        for (uint64_t i = 0; i < _functions_count; i++) {
            ustring symbol = read_string(file);
            ustring cross_compiler_version = read_string(file);
            std::vector<uint8_t> _opcode = read_byte_arr(file);
            bool is_cheap = read_value<bool>(file);

            if (symbol.starts_with('\2')) {
                CXX::cxxCall(new FuncEnvironment(std::move(_opcode), true, false, cross_compiler_version.empty() ? nullptr : new ustring(cross_compiler_version)));
                continue;
            }

            uint64_t hash = art::hash<uint8_t>()(_opcode.data(), _opcode.size());


            readed_functions.insert(symbol);
            auto it = local_functions.find(symbol);
            if (it == local_functions.end())
                local_functions[symbol] = hash;
            else if (it->second == hash)
                continue;

            build_patch_list.emplace_back(symbol, new FuncHandle::inner_handle(std::move(_opcode), is_cheap, cross_compiler_version.empty() ? nullptr : new ustring(cross_compiler_version)));
        }

        std::remove_if(local_functions.begin(), local_functions.end(), [&](decltype(local_functions)::value_type& it) {
            if (!readed_functions.contains(it.first)) {
                build_patch_list.emplace_back(it.first, nullptr);
                return true;
            }
            return false;
        });
        return build_patch_list;
    }

    patch_list precompiled::handle_init_complete() {
        return {};
    }

    patch_list precompiled::handle_create(files::FileHandle& file) {
        return handle_init(file);
    }

    patch_list precompiled::handle_renamed(const ustring& old, files::FileHandle& file) {
        {
            lock_guard guard(mutex);
            declared_functions[file.get_path()] = declared_functions[old];
        }
        return handle_init(file);
    }

    patch_list precompiled::handle_changed(files::FileHandle& file) {
        return handle_init(file);
    }

    patch_list precompiled::handle_removed(const ustring& removed) {
        lock_guard guard(mutex);
        patch_list build_patch_list;
        auto local_functions = declared_functions[removed];
        for (auto& it : local_functions)
            build_patch_list.emplace_back(it.first, nullptr);
        declared_functions.erase(removed);
        return build_patch_list;
    }
}
