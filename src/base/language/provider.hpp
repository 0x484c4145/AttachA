#ifndef SRC_BASE_LANGUAGE_PROVIDER
#define SRC_BASE_LANGUAGE_PROVIDER
#include <run_time/func_enviro_builder.hpp>
#include <run_time/library/cxx/files.hpp>

namespace language_parsers {
    class PatchList {
        std::unordered_map<art::ustring, art::FuncHandle::inner_handle*> patches;

    public:
        void add_patches(art::patch_list&&);
        void add_patches(PatchList&&);

        void add_patch(const art::ustring&, art::FuncHandle::inner_handle*);

        void remove_patch(const art::ustring& name);
        void apply();
        void clear();
    };

    //language_handler can do anything with file and runtime,
    //but handler intended to compile sources with art::FuncEnviroBuilder and return patch list, to be handled by runtime
    class language_handler {
    public:
        virtual art::patch_list handle_init(art::files::FileHandle& file) = 0;
        virtual art::patch_list handle_init_complete() = 0;
        virtual art::patch_list handle_create(art::files::FileHandle& file) = 0;
        virtual art::patch_list handle_renamed(const art::ustring& old, art::files::FileHandle& file) = 0;
        virtual art::patch_list handle_changed(art::files::FileHandle& file) = 0;
        virtual art::patch_list handle_removed(const art::ustring& removed) = 0;
    };

    class language_provider {
        std::unordered_map<art::ustring, art::shared_ptr<language_handler>, art::hash<art::ustring>> languages;
        art::ValueItem folder_monitor;
        bool init_mode = true;
        art::TaskRWMutex rw_mutex;
        PatchList patches;

    public:
        language_provider(std::string_view path, bool include_sub_directories);
        void register_language(std::string_view name, art::shared_ptr<language_handler> decoder);
        void unregister_language(std::string_view name);

        void run_once();

        void start();
        void stop();
    };
}

#endif /* SRC_BASE_LANGUAGE_PROVIDER */
