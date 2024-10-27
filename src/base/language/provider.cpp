#include <base/language/provider.hpp>
#include <run_time/AttachA_CXX.hpp>
#include <run_time/library/cxx/files.hpp>
#include <run_time/library/file.hpp>

using namespace art;
using namespace CXX;

namespace language_parsers {

    void PatchList::add_patches(art::patch_list&& patches_list) {
        for (const auto& patch : patches_list)
            add_patch(patch.first, patch.second);
    }

    void PatchList::add_patches(PatchList&& patches_list) {
        for (const auto& patch : patches_list.patches)
            add_patch(patch.first, patch.second);
        patches_list.patches.clear();
    }

    void PatchList::add_patch(const art::ustring& symbol, art::FuncHandle::inner_handle* handle) {
        auto patch_data = patches[symbol];
        if (handle != nullptr) {
            if (patch_data == nullptr)
                patch_data = handle;
            else
                throw CompileTimeException("Symbol must be defined once. Got more than one defintion for " + symbol + " symbol.");
        }
    }

    void PatchList::remove_patch(const art::ustring& name) {
        auto it = std::find_if(patches.begin(), patches.end(), [&](const auto& patch) {
            return patch.first == name;
        });
        if (it == patches.end())
            return;

        if (it->second)
            it->second->reduce_usage();
        patches.erase(it);
    }

    void PatchList::apply() {
        for (const auto& patch : patches) {
            if (patch.second)
                FuncEnvironment::fastHotPatch(patch.first, patch.second);
            else {
                FuncEnvironment::Unload(patch.first);
            }
        }
        patches.clear();
    }

    void PatchList::clear() {
        for (const auto& patch : patches)
            if (patch.second)
                patch.second->reduce_usage();
    }

    language_provider::language_provider(std::string_view path, bool include_sub_directories) {
        folder_monitor = cxxCall(file::constructor::createProxy_FolderChangesMonitor, ustring(path.data(), path.size()), include_sub_directories);
        Interface::makeCall(
            ClassAccess::pub,
            Interface::makeCall(ClassAccess::pub, folder_monitor, ustring("get_event_file_creation")),
            ustring("join"),
            MakeNative([&](const ustring& name) {
                files::FolderBrowser browser(name.c_str(), name.size());
                ustring extension = (ustring)browser.file_extension();
                art::unique_lock unify(rw_mutex);
                auto it = languages.find(extension);
                if (it != languages.end()) {
                    files::FileHandle handle(name.c_str(), name.size(), files::open_mode::read, files::on_open_action::open_exists, files::_async_flags{});
                    if (init_mode)
                        patches.add_patches(it->second->handle_init(handle));
                    else {
                        patches.add_patches(it->second->handle_create(handle));
                        patches.apply();
                    }
                }
            })
        );
        Interface::makeCall(
            ClassAccess::pub,
            Interface::makeCall(ClassAccess::pub, folder_monitor, ustring("get_event_file_name_change")),
            ustring("join"),
            MakeNative([&](const ustring& old_name, const ustring& new_name) {
                files::FolderBrowser browser(new_name.c_str(), new_name.size());
                ustring extension = (ustring)browser.file_extension();
                art::unique_lock unify(rw_mutex);
                auto it = languages.find(extension);
                if (it != languages.end()) {
                    files::FileHandle handle(new_name.c_str(), new_name.size(), files::open_mode::read, files::on_open_action::open_exists, files::_async_flags{});
                    patches.add_patches(it->second->handle_renamed(old_name, handle));
                    if (!init_mode)
                        patches.apply();
                }
            })
        );
        Interface::makeCall(
            ClassAccess::pub,
            Interface::makeCall(ClassAccess::pub, folder_monitor, ustring("get_event_file_last_write")),
            ustring("join"),
            MakeNative([&](const ustring& name) {
                files::FolderBrowser browser(name.c_str(), name.size());
                ustring extension = (ustring)browser.file_extension();
                art::unique_lock unify(rw_mutex);
                auto it = languages.find(extension);
                if (it != languages.end()) {
                    files::FileHandle handle(name.c_str(), name.size(), files::open_mode::read, files::on_open_action::open_exists, files::_async_flags{});
                    patches.add_patches(it->second->handle_changed(handle));
                    if (!init_mode)
                        patches.apply();
                }
            })
        );
        Interface::makeCall(
            ClassAccess::pub,
            Interface::makeCall(ClassAccess::pub, folder_monitor, ustring("get_event_file_removed")),
            ustring("join"),
            MakeNative([&](const ustring& name) {
                files::FolderBrowser browser(name.c_str(), name.size());
                ustring extension = (ustring)browser.file_extension();
                art::unique_lock unify(rw_mutex);
                auto it = languages.find(extension);
                if (it != languages.end()) {
                    patches.add_patches(it->second->handle_removed(name));
                    if (!init_mode)
                        patches.apply();
                }
            })
        );
    }

    void language_provider::register_language(std::string_view name, art::shared_ptr<language_handler> decoder) {
        art::unique_lock unify(rw_mutex);
        languages[ustring(name.data(), name.size())] = decoder;
    }

    void language_provider::unregister_language(std::string_view name) {
        art::unique_lock unify(rw_mutex);
        languages.erase(ustring(name.data(), name.size()));
    }

    void language_provider::run_once() {
        Interface::makeCall(ClassAccess::pub, folder_monitor, ustring("once_scan"));
        art::unique_lock unify(rw_mutex);
        if (init_mode) {
            for (auto& [name, decoder] : languages)
                patches.add_patches(decoder->handle_init_complete());
            patches.apply();
        }
        init_mode = false;
    }

    void language_provider::start() {
        run_once();
        Interface::makeCall(ClassAccess::pub, folder_monitor, ustring("start"));
    }

    void language_provider::stop() {
        Interface::makeCall(ClassAccess::pub, folder_monitor, ustring("stop"));
    }
}
