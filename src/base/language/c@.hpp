#ifndef SRC_BASE_LANGUAGE_C_40
#define SRC_BASE_LANGUAGE_C_40
#include <base/language/provider.hpp>

namespace language_parsers {

    class c_async : public language_handler {
        //{path : { function: hash }}...
        std::unordered_map<
            art::ustring,
            std::unordered_map<
                art::ustring,
                uint64_t,
                art::hash<art::ustring>>,
            art::hash<art::ustring>>
            declared_functions;
        art::TaskMutex mutex;

    public:
        art::patch_list handle_init(art::files::FileHandle& file) override;
        art::patch_list handle_init_complete() override;
        art::patch_list handle_create(art::files::FileHandle& file) override;
        art::patch_list handle_renamed(const art::ustring& old, art::files::FileHandle& file) override;
        art::patch_list handle_changed(art::files::FileHandle& file) override;
        art::patch_list handle_removed(const art::ustring& removed) override;
    };
}

#endif /* SRC_BASE_LANGUAGE_C_40 */
