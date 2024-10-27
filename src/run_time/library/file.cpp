// Copyright Danyil Melnytskyi 2022-Present
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <run_time/AttachA_CXX.hpp>
#include <run_time/asm/attacha_environment.hpp>
#include <run_time/library/bytes.hpp>
#include <run_time/library/cxx/files.hpp>
#include <run_time/library/file.hpp>
#include <run_time/tasks.hpp>
#include <util/exceptions.hpp>

namespace art {
    using namespace bytes;

    AttachAVirtualTable* define_FileHandle;
    AttachAVirtualTable* define_BlockingFileHandle;
    AttachAVirtualTable* define_TextFile;
    AttachAVirtualTable* define_FolderBrowser;

    class TextFile {
    public:
        enum class Encoding {
            utf8,
            utf16,
            utf32,
            ascii
        };

    private:
        typed_lgr<files::FileHandle> handle;

        template <typename T, size_t to_read>
        void read_buffer(list_array<ValueItem>& results_holder, T*& buffer, size_t& size, size_t& rest) {
            ValueItem item = handle->read(to_read);
            art::typed_lgr<Task> task = *(art::typed_lgr<Task>*)item.val;
            auto results = Task::await_results(task);
            buffer = (T*)results[0].val;
            size = results[0].meta.val_len / sizeof(T);
            rest = results[1].meta.val_len % sizeof(T);
            convert_endian_arr<T>(endian, buffer, size);
        }

#define read_buffer_type(type, to_read)                    \
    list_array<ValueItem> results;                         \
    type* str;                                             \
    size_t max_i;                                          \
    size_t rest;                                           \
    read_buffer<type, to_read>(results, str, max_i, rest); \
    if (max_i == 0)                                        \
        break;

        ValueItem validate_return_utf8(const art::ustring& str) {
            art::ustring tmp;
            tmp.set_unsafe_state(false, false);
            tmp = str;
            return tmp;
        }

        art::ustring convert_to_read(std::u16string& str, bool endian_convert) {
            if (endian_convert)
                convert_endian_arr(endian, str.data(), str.size());
            return str;
        }

        std::u16string convert_to_write_u16(const art::ustring& line, bool endian_convert) {
            std::u16string res = (std::u16string)line;
            if (endian_convert)
                convert_endian_arr(endian, res.data(), res.size());
            return res;
        }

        art::ustring convert_to_read(std::u32string& str, bool endian_convert) {
            if (endian_convert)
                convert_endian_arr(endian, str.data(), str.size());
            return art::ustring(str);
        }

        std::u32string convert_to_write_u32(const art::ustring& line, bool endian_convert) {
            std::u32string res = (std::u32string)line;
            if (endian_convert)
                convert_endian_arr(endian, res.data(), res.size());
            return res;
        }

        ValueItem read_line_ascii_utf8() {
            art::ustring line;
            line.set_unsafe_state(true, false);
            line.reserve(1024);
            while (true) {
                ValueItem item = handle->read(1024);
                art::typed_lgr<Task> task = *(art::typed_lgr<Task>*)item.val;
                auto results = Task::await_results(task);
                item = std::move(results[0]);
                if (item.meta.val_len == 0)
                    break;
                auto str = (uint8_t*)item.val;
                for (uint32_t i = 0; i < item.meta.val_len; i++) {
                    char c = str[i];
                    if (c == '\n') {
                        handle->seek_pos(i + 1, files::pointer_offset::current, files::pointer::read);
                        return validate_return_utf8(line);
                    }
                    line += c;
                }
                if (results.size() == 2)
                    break;
            }
            if (line.size() == 0)
                return nullptr;
            else
                return validate_return_utf8(line);
        }

        ValueItem read_word_ascii_utf8() {
            art::ustring word;
            word.set_unsafe_state(true, false);
            word.reserve(32);
            while (true) {
                ValueItem item = handle->read(1024);
                art::typed_lgr<Task> task = *(art::typed_lgr<Task>*)item.val;
                auto results = Task::await_results(task);
                item = std::move(results[0]);
                if (item.meta.val_len == 0)
                    break;
                auto str = (uint8_t*)item.val;
                for (uint32_t i = 0; i < item.meta.val_len; i++) {
                    char c = str[i];
                    if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
                        handle->seek_pos(i + 1, files::pointer_offset::current, files::pointer::read);
                        return validate_return_utf8(word);
                    }
                    word += c;
                }
                if (results.size() == 2)
                    break;
            }
            if (word.size() == 0)
                return nullptr;
            else
                return validate_return_utf8(word);
        }

        ValueItem read_symbol_ascii_utf8(bool ascii_mode, bool skip_spaces) {
            art::ustring symbol;
            symbol.set_unsafe_state(true, false);
            symbol.reserve(2);
            bool state_readed = false;
            uint8_t bytes_need_read = 0;
            while (true) {
                ValueItem item = handle->read(1);
                art::typed_lgr<Task> task = *(art::typed_lgr<Task>*)item.val;
                auto results = Task::await_results(task);
                item = std::move(results[0]);
                if (item.meta.val_len == 0)
                    break;
                uint8_t c = *(uint8_t*)item.val;
                switch (c) {
                case ' ':
                    if (skip_spaces)
                        break;
                    [[fallthrough]];
                default:
                    if (c < 32)
                        break;
                    [[fallthrough]];
                case '\n':
                case '\t':
                case '\b':
                    if (ascii_mode)
                        return art::ustring((char)c);
                    else {
                        if (!state_readed) {
                            symbol += (char)c;
                            state_readed = true;
                            //decode bytes need read
                            if (c & 0b10000000) {
                                if (c & 0b01000000) {
                                    if (c & 0b00100000) {
                                        if (c & 0b00010000)
                                            bytes_need_read = 4;
                                        else
                                            bytes_need_read = 3;
                                    } else
                                        bytes_need_read = 2;
                                } else
                                    bytes_need_read = 1;
                            }
                        }
                        if (bytes_need_read == 0)
                            return validate_return_utf8(symbol);
                        else {
                            symbol += (char)c;
                            bytes_need_read--;
                            break;
                        }
                    }
                }
            }
            if (symbol.size() == 0)
                return nullptr;
            else
                return validate_return_utf8(symbol);
        }

        ValueItem write_ascii_utf8(ValueItem& item, bool ascii_mode) {
            if (item.meta.vtype == VType::string) {
                auto& string = *(art::ustring*)item.getSourcePtr();
                if (ascii_mode) {
                    for (char c : string) {
                        if (reinterpret_cast<uint8_t&>(c) > 127)
                            throw InvalidType("Can't write utf8 or extended ascii to regular ascii file");
                    }
                }
                return handle->write((uint8_t*)string.data(), string.size());
            } else {
                auto string = (art::ustring)item;
                return handle->write((uint8_t*)string.data(), string.size());
            }
        }

        ValueItem read_line_utf16() {
            std::u16string line;
            line.reserve(1024);
            while (true) {
                read_buffer_type(uint16_t, 1024);
                for (uint32_t i = 0; i < max_i; i++) {
                    uint16_t c = str[i];
                    if (c == '\n') {
                        handle->seek_pos(i + 1, files::pointer_offset::current, files::pointer::read);
                        return validate_return_utf8(convert_to_read(line, true));
                    }
                    line.push_back(c);
                }
                if (results.size() == 2)
                    break;
            }
            if (line.size() == 0)
                return nullptr;
            else
                return validate_return_utf8(convert_to_read(line, true));
        }

        ValueItem read_word_utf16() {
            std::u16string word;
            word.reserve(32);
            while (true) {
                read_buffer_type(uint16_t, 1024);
                for (uint32_t i = 0; i < max_i; i++) {
                    uint16_t c = str[i];
                    if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
                        handle->seek_pos(i + 1, files::pointer_offset::current, files::pointer::read);
                        return validate_return_utf8(convert_to_read(word, true));
                    }
                    word.push_back(c);
                }
                if (results.size() == 2)
                    break;
            }
            if (word.size() == 0)
                return nullptr;
            else
                return validate_return_utf8(convert_to_read(word, true));
        }

        ValueItem read_symbol_utf16(bool skip_spaces) {
            std::u16string symbol;
            symbol.reserve(32);
            bool state_readed = false;
            while (true) {
                ValueItem item = handle->read(2);
                art::typed_lgr<Task> task = *(art::typed_lgr<Task>*)item.val;
                auto results = Task::await_results(task);
                {
                    ValueItem& item = results[0];
                    if (item.meta.val_len == 0) {
                        if (state_readed)
                            throw InvalidInput("Invalid utf16 symbol, wrong codepoint");
                        else
                            break;
                    }
                    uint16_t c = *(uint16_t*)item.val;
                    if (state_readed) {
                        if ((c & 0b1111110000000000) != 0b1101110000000000)
                            return validate_return_utf8(convert_to_read(symbol += (uint16_t)c, true));
                        else {
                            throw InvalidInput("Invalid utf16 symbol, wrong codepoint");
                        }
                    }
                    switch (c) {
                    case ' ':
                        if (skip_spaces)
                            break;
                        [[fallthrough]];
                    default:
                        if (c < 32)
                            break;
                        [[fallthrough]];
                    case '\n':
                    case '\t':
                    case '\b':
                        symbol += (char)c;
                        state_readed = true;
                        if constexpr (Endian::native != Endian::little)
                            c = convert_endian<Endian::native>(c);
                        c &= 0b1111110000000000;
                        if (c) {
                            if (c == 0b1101100000000000)
                                return validate_return_utf8(convert_to_read(symbol, true));
                            else
                                throw InvalidInput("Invalid utf16 symbol, wrong codepoint");
                        } else
                            break;
                    }
                }
            }
            if (symbol.size() == 0)
                return nullptr;
            else
                return validate_return_utf8(convert_to_read(symbol, true));
        }

        ValueItem write_utf16(ValueItem& item) {
            std::u16string string;
            if (item.meta.vtype == VType::string) {
                string = convert_to_write_u16(*(art::ustring*)item.getSourcePtr(), true);

            } else
                string = convert_to_write_u16(((art::ustring)item), true);
            size_t size = string.size() * 2;
            auto string_ptr = (uint8_t*)string.data();
            while (size > UINT32_MAX) {
                handle->write(string_ptr, UINT32_MAX);
                size -= UINT32_MAX;
                string_ptr += UINT32_MAX;
            }
            if (size)
                return handle->write(string_ptr, size);
            else
                return nullptr;
        }

        void read_bom_utf16() {
            do {
                read_buffer_type(uint16_t, 2);
                if (str[0] == 0xFEFF)
                    endian = Endian::big;
                else if (str[0] == 0xFFFE)
                    endian = Endian::little;
                else {
                    handle->seek_pos(-2, files::pointer_offset::current, files::pointer::read);
                    endian = Endian::native;
                }
            } while (false);
        }

        void init_bom_utf16() {
            if (handle->size())
                return;
            if (endian == Endian::big)
                handle->write((uint8_t*)"\xFE\xFF", 2);
            else if (endian == Endian::little)
                handle->write((uint8_t*)"\xFF\xFE", 2);
        }

        ValueItem read_line_utf32() {
            std::u32string line;
            line.reserve(1024);
            while (true) {
                read_buffer_type(uint32_t, 1024);
                for (uint32_t i = 0; i < max_i; i++) {
                    uint32_t c = str[i];
                    if (c == '\n') {
                        handle->seek_pos(i + 1, files::pointer_offset::current, files::pointer::read);
                        return validate_return_utf8(convert_to_read(line, true));
                    }
                    line.push_back(c);
                }
                if (results.size() == 2)
                    break;
            }
            if (line.size() == 0)
                return nullptr;
            else
                return validate_return_utf8(convert_to_read(line, true));
        }

        ValueItem read_word_utf32() {
            std::u32string word;
            word.reserve(32);
            while (true) {
                read_buffer_type(uint32_t, 1024);
                for (uint32_t i = 0; i < max_i; i++) {
                    uint32_t c = str[i];
                    if (c == ' ' || c == '\n' || c == '\t' || c == '\r') {
                        handle->seek_pos(i + 1, files::pointer_offset::current, files::pointer::read);
                        return validate_return_utf8(convert_to_read(word, true));
                    }
                    word.push_back(c);
                }
                if (results.size() == 2)
                    break;
            }
            if (word.size() == 0)
                return nullptr;
            else
                return validate_return_utf8(convert_to_read(word, true));
        }

        ValueItem read_symbol_utf32(bool skip_spaces) {
            while (true) {
                ValueItem item = handle->read(4);
                art::typed_lgr<Task> task = *(art::typed_lgr<Task>*)item.val;
                auto results = Task::await_results(task);
                {
                    ValueItem& item = results[0];
                    if (item.meta.val_len == 0)
                        return nullptr;
                    uint32_t c = *(uint32_t*)item.val;
                    switch (c) {
                    case ' ':
                        if (skip_spaces)
                            break;
                        [[fallthrough]];
                    default:
                        if (c < 32)
                            break;
                        [[fallthrough]];
                    case '\n':
                    case '\t':
                    case '\b': {
                        std::u32string res(1, c);
                        return validate_return_utf8(convert_to_read(res, true));
                    }
                    }
                }
            }
        }

        ValueItem write_utf32(ValueItem& item) {
            std::u32string string;
            if (item.meta.vtype == VType::string) {
                string = convert_to_write_u32(*(art::ustring*)item.getSourcePtr(), true);

            } else
                string = convert_to_write_u32(((art::ustring)item), true);
            size_t size = string.size() * 2;
            auto string_ptr = (uint8_t*)string.data();
            while (size > UINT32_MAX) {
                handle->write(string_ptr, UINT32_MAX);
                size -= UINT32_MAX;
                string_ptr += UINT32_MAX;
            }
            if (size)
                return handle->write(string_ptr, size);
            else
                return nullptr;
        }

        void read_bom_utf32() {
            do {
                read_buffer_type(uint32_t, 4);
                if (str[0] == 0x0000FEFF)
                    endian = Endian::big;
                else if (str[0] == 0xFFFE0000)
                    endian = Endian::little;
                else {
                    handle->seek_pos(-4, files::pointer_offset::current, files::pointer::read);
                    endian = Endian::native;
                }
            } while (false);
        }

        void init_bom_utf32() {
            if (handle->size())
                return;
            if (endian == Endian::big)
                handle->write((uint8_t*)"\x00\x00\xFE\xFF", 4);
            else if (endian == Endian::little)
                handle->write((uint8_t*)"\xFF\xFE\x00\x00", 4);
        }

#undef read_buffer_type
        Endian endian;

    public:
        const Encoding file_encoding;

        TextFile(typed_lgr<files::FileHandle> handle, Encoding encoding, Endian endian)
            : handle(handle), file_encoding(encoding), endian(endian) {}

        ValueItem read_line() {
            switch (file_encoding) {
            case Encoding::ascii:
            case Encoding::utf8:
                return read_line_ascii_utf8();
            case Encoding::utf16:
                return read_line_utf16();
            case Encoding::utf32:
                return read_line_utf32();
            default:
                return nullptr;
            }
        }

        ValueItem read_word() {
            switch (file_encoding) {
            case Encoding::ascii:
            case Encoding::utf8:
                return read_word_ascii_utf8();
            case Encoding::utf16:
                return read_word_utf16();
            case Encoding::utf32:
                return read_word_utf32();
            default:
                return nullptr;
            }
        }

        ValueItem read_symbol(bool skip_spaces) {
            switch (file_encoding) {
            case Encoding::ascii:
                return read_symbol_ascii_utf8(true, skip_spaces);
            case Encoding::utf8:
                return read_symbol_ascii_utf8(false, skip_spaces);
            case Encoding::utf16:
                return read_symbol_utf16(skip_spaces);
            case Encoding::utf32:
                return read_symbol_utf32(skip_spaces);
            default:
                return nullptr;
            }
        }

        ValueItem write(ValueItem& item) {
            switch (file_encoding) {
            case Encoding::ascii:
                return write_ascii_utf8(item, true);
            case Encoding::utf8:
                return write_ascii_utf8(item, false);
            case Encoding::utf16:
                return write_utf16(item);
            case Encoding::utf32:
                return write_utf32(item);
            default:
                return nullptr;
            }
        }

        void read_bom() {
            switch (file_encoding) {
            case Encoding::utf8:
            case Encoding::utf16:
                read_bom_utf16();
                break;
            case Encoding::utf32:
                read_bom_utf32();
                break;
            default:
                break;
            }
        }

        void init_bom() {
            switch (file_encoding) {
            case Encoding::utf8:
            case Encoding::utf16:
                init_bom_utf16();
                break;
            case Encoding::utf32:
                init_bom_utf32();
                break;
            default:
                break;
            }
        }
    };

    namespace file {
        namespace constructor {
            AttachAFun(createProxy_FileHandle, 1, {
                auto path = (art::ustring)args[0];
                bool is_async = true;
                if (len >= 2)
                    if (args[2].meta.vtype != VType::noting)
                        is_async = (bool)args[1];
                files::open_mode mode = files::open_mode::read_write;
                if (len >= 3)
                    if (args[2].meta.vtype != VType::noting)
                        mode = (files::open_mode)(uint8_t)args[2];
                files::on_open_action action = files::on_open_action::open;
                if (len >= 4)
                    if (args[3].meta.vtype != VType::noting)
                        action = (files::on_open_action)(uint8_t)args[3];
                files::_sync_flags flags{0};
                if (len >= 5)
                    if (args[4].meta.vtype != VType::noting)
                        flags.value = (uint8_t)args[4];
                files::share_mode share;
                if (len >= 6)
                    if (args[5].meta.vtype != VType::noting)
                        share.value = (uint8_t)args[5];
                files::pointer_mode pointer_mode = files::pointer_mode::combined;
                if (len >= 7)
                    if (args[6].meta.vtype != VType::noting)
                        pointer_mode = (files::pointer_mode)(uint8_t)args[6];

                if (is_async) {
                    files::_async_flags async_flags;
                    async_flags.value = flags.value;
                    return ValueItem(CXX::Interface::constructStructure<typed_lgr<files::FileHandle>>(define_FileHandle, new files::FileHandle(path.c_str(), path.size(), mode, action, async_flags, share, pointer_mode)), no_copy);
                } else
                    return ValueItem(CXX::Interface::constructStructure<typed_lgr<files::FileHandle>>(define_FileHandle, new files::FileHandle(path.c_str(), path.size(), mode, action, flags, share, pointer_mode)), no_copy);
            });
            AttachAFun(createProxy_BlockingFileHandle, 2, {
                auto path = (art::ustring)args[0];
                files::open_mode mode = files::open_mode::read_write;
                if (len >= 2)
                    if (args[1].meta.vtype != VType::noting)
                        mode = (files::open_mode)(uint8_t)args[1];
                files::on_open_action action = files::on_open_action::open;
                if (len >= 3)
                    if (args[2].meta.vtype != VType::noting)
                        action = (files::on_open_action)(uint8_t)args[2];
                files::_sync_flags flags{0};
                if (len >= 4)
                    if (args[3].meta.vtype != VType::noting)
                        flags.value = (uint8_t)args[3];
                files::share_mode share;
                if (len >= 5)
                    if (args[4].meta.vtype != VType::noting)
                        share.value = (uint8_t)args[4];
                files::pointer_mode pointer_mode = files::pointer_mode::combined;
                if (len >= 6)
                    if (args[5].meta.vtype != VType::noting)
                        pointer_mode = (files::pointer_mode)(uint8_t)args[5];

                return ValueItem(CXX::Interface::constructStructure<typed_lgr<files::BlockingFileHandle>>(define_BlockingFileHandle, new files::BlockingFileHandle(path.c_str(), path.size(), mode, action, flags, share)), no_copy);
            });
            AttachAFun(createProxy_TextFile, 1, {
                auto file_handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
                auto endian = Endian::native;
                if (len >= 2)
                    if (args[1].meta.vtype != VType::noting)
                        endian = (Endian)(uint8_t)args[1];
                auto encoding = TextFile::Encoding::utf8;
                if (len >= 3)
                    if (args[2].meta.vtype != VType::noting)
                        encoding = (TextFile::Encoding)(uint8_t)args[2];
                return ValueItem(CXX::Interface::constructStructure<typed_lgr<TextFile>>(define_TextFile, new TextFile(file_handle, encoding, endian)), no_copy);
            });

            AttachAFun(createProxy_FolderChangesMonitor, 1, {
                bool calc_depth = len >= 2 ? (bool)args[1] : false;
                if (args[0].meta.vtype == VType::string) {
                    art::ustring& path = *(art::ustring*)args[0].getSourcePtr();
                    return files::createFolderChangesMonitor(path.c_str(), path.size(), calc_depth);
                } else {
                    art::ustring path = (art::ustring)args[0];
                    return files::createFolderChangesMonitor(path.c_str(), path.size(), calc_depth);
                }
            });
            AttachAFun(createProxy_FolderBrowser, 0, {
                if (len == 0)
                    return ValueItem(CXX::Interface::constructStructure<typed_lgr<files::FolderBrowser>>(define_FolderBrowser, new files::FolderBrowser()), no_copy);
                else {
                    if (args[0].meta.vtype == VType::string) {
                        art::ustring& path = *(art::ustring*)args[0].getSourcePtr();
                        return ValueItem(CXX::Interface::constructStructure<typed_lgr<files::FolderBrowser>>(define_FolderBrowser, new files::FolderBrowser(path.c_str(), path.size())), no_copy);
                    } else {
                        art::ustring path = (art::ustring)args[0];
                        return ValueItem(CXX::Interface::constructStructure<typed_lgr<files::FolderBrowser>>(define_FolderBrowser, new files::FolderBrowser(path.c_str(), path.size())), no_copy);
                    }
                }
            });
        }

#pragma region FileHandle

        AttachAFun(funs_FileHandle_read, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            ValueItem& item = args[1];
            if (item.meta.vtype == VType::raw_arr_ui8 || item.meta.vtype == VType::raw_arr_i8)
                return handle->read((uint8_t*)item.getSourcePtr(), item.meta.val_len);
            else
                return handle->read((uint32_t)item);
        });

        AttachAFun(funs_FileHandle_read_fixed, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            ValueItem& item = args[1];
            if (item.meta.vtype == VType::raw_arr_ui8 || item.meta.vtype == VType::raw_arr_i8)
                return handle->read_fixed((uint8_t*)item.getSourcePtr(), item.meta.val_len);
            else
                return handle->read_fixed((uint32_t)item);
        });

        AttachAFun(funs_FileHandle_write, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            ValueItem& item = args[1];
            if (item.meta.vtype == VType::raw_arr_ui8 || item.meta.vtype == VType::raw_arr_i8)
                return handle->write((uint8_t*)item.getSourcePtr(), item.meta.val_len);
            else
                throw InvalidArguments("Excepted raw_arr_ui8 or raw_arr_i8, got " + enum_to_string(item.meta.vtype));
        });

        AttachAFun(funs_FileHandle_append, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            ValueItem& item = args[1];
            if (item.meta.vtype == VType::raw_arr_ui8 || item.meta.vtype == VType::raw_arr_i8)
                return handle->append((uint8_t*)item.getSourcePtr(), item.meta.val_len);
            else
                throw InvalidArguments("Excepted raw_arr_ui8 or raw_arr_i8, got " + enum_to_string(item.meta.vtype));
        });

        AttachAFun(funs_FileHandle_seek_pos, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            uint64_t offset = (uint64_t)args[1];
            files::pointer_offset pointer_offset = files::pointer_offset::begin;
            if (len >= 3)
                if (args[2].meta.vtype != VType::noting)
                    pointer_offset = (files::pointer_offset)(uint8_t)args[2];
            if (len >= 4)
                if (args[3].meta.vtype != VType::noting)
                    return handle->seek_pos(offset, pointer_offset, (files::pointer)(uint8_t)args[3]);
            return handle->seek_pos(offset, pointer_offset);
        });

        AttachAFun(funs_FileHandle_tell_pos, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            return handle->tell_pos((files::pointer)(uint8_t)args[1]);
        });

        AttachAFun(funs_FileHandle_flush, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            return handle->flush();
        });

        AttachAFun(funs_FileHandle_size, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            return handle->size();
        });

        AttachAFun(funs_FileHandle_internal_get_handle, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            return handle->internal_get_handle();
        });

        AttachAFun(funs_FileHandle_get_path, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FileHandle>>(args[0], define_FileHandle);
            return handle->get_path();
        });
#pragma endregion

#pragma region BlockingFileHandle
        AttachAFun(funs_BlockingFileHandle_read, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            ValueItem& item = args[1];
            if (item.meta.vtype == VType::raw_arr_ui8 || item.meta.vtype == VType::raw_arr_i8)
                return handle->read((uint8_t*)item.getSourcePtr(), item.meta.val_len);
            else {
                uint32_t len = (uint32_t)args[1];
                if (len == 0)
                    return 0;
                uint8_t* ptr = new uint8_t[len];
                auto res = handle->read(ptr, len);
                if (res <= 0) {
                    delete[] ptr;
                    return res;
                } else
                    return ValueItem(ptr, len, no_copy);
            }
        });

        AttachAFun(funs_BlockingFileHandle_write, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            ValueItem& item = args[1];
            if (item.meta.vtype == VType::raw_arr_ui8 || item.meta.vtype == VType::raw_arr_i8)
                return handle->write((uint8_t*)item.getSourcePtr(), item.meta.val_len);
            else
                throw InvalidArguments("Excepted raw_arr_ui8 or raw_arr_i8, got " + enum_to_string(item.meta.vtype));
        });

        AttachAFun(funs_BlockingFileHandle_seek_pos, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            uint64_t offset = (uint64_t)args[1];
            files::pointer_offset pointer_offset = files::pointer_offset::begin;
            if (len >= 3)
                if (args[2].meta.vtype != VType::noting)
                    pointer_offset = (files::pointer_offset)(uint8_t)args[2];
            return handle->seek_pos(offset, pointer_offset);
        });

        AttachAFun(funs_BlockingFileHandle_tell_pos, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            return handle->tell_pos();
        });

        AttachAFun(funs_BlockingFileHandle_flush, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            return handle->flush();
        });

        AttachAFun(funs_BlockingFileHandle_size, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            return handle->size();
        });

        AttachAFun(funs_BlockingFileHandle_eof_state, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            return handle->eof_state();
        });

        AttachAFun(funs_BlockingFileHandle_valid, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            return handle->valid();
        });

        AttachAFun(funs_BlockingFileHandle_internal_get_handle, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            return handle->internal_get_handle();
        });

        AttachAFun(funs_BlockingFileHandle_get_path, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::BlockingFileHandle>>(args[0], define_BlockingFileHandle);
            return handle->get_path();
        });
#pragma endregion

#pragma region TextFile
        AttachAFun(funs_TextFile_read_line, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<TextFile>>(args[0], define_TextFile);
            return handle->read_line();
        });

        AttachAFun(funs_TextFile_read_word, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<TextFile>>(args[0], define_TextFile);
            return handle->read_word();
        });

        AttachAFun(funs_TextFile_read_symbol, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<TextFile>>(args[0], define_TextFile);
            if (len >= 2)
                return handle->read_symbol((bool)args[2]);
            else
                return handle->read_symbol(false);
        });

        AttachAFun(funs_TextFile_write, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<TextFile>>(args[0], define_TextFile);
            return handle->write(args[1]);
        });

        AttachAFun(funs_TextFile_read_bom, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<TextFile>>(args[0], define_TextFile);
            handle->read_bom();
        });

        AttachAFun(funs_TextFile_init_bom, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<TextFile>>(args[0], define_TextFile);
            handle->init_bom();
        });
#pragma endregion

#pragma region FileBrowser
        AttachAFun(funs_FolderBrowser_folders, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->folders().convert_take<ValueItem>([](art::ustring&& str) -> ValueItem {
                return std::move(str);
            });
        });

        AttachAFun(funs_FolderBrowser_files, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->files().convert_take<ValueItem>([](art::ustring&& str) -> ValueItem {
                return std::move(str);
            });
        });

        AttachAFun(funs_FolderBrowser_is_folder, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->is_folder();
        });

        AttachAFun(funs_FolderBrowser_is_file, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->is_file();
        });

        AttachAFun(funs_FolderBrowser_exists, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->exists();
        });

        AttachAFun(funs_FolderBrowser_is_hidden, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->is_hidden();
        });

        AttachAFun(funs_FolderBrowser_create_path, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            if (args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                return handle->create_path(path.c_str(), path.size());
            } else {
                art::ustring path = (art::ustring)args[1];
                return handle->create_path(path.c_str(), path.size());
            }
        });

        AttachAFun(funs_FolderBrowser_create_current_path, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->create_current_path();
        });

        AttachAFun(funs_FolderBrowser_create_file, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            if (args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                return handle->create_file(path.c_str(), path.size());
            } else {
                art::ustring path = (art::ustring)args[1];
                return handle->create_file(path.c_str(), path.size());
            }
        });

        AttachAFun(funs_FolderBrowser_create_folder, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            if (args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                return handle->create_folder(path.c_str(), path.size());
            } else {
                art::ustring path = (art::ustring)args[1];
                return handle->create_folder(path.c_str(), path.size());
            }
        });

        AttachAFun(funs_FolderBrowser_remove_file, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            if (args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                return handle->remove_file(path.c_str(), path.size());
            } else {
                art::ustring path = (art::ustring)args[1];
                return handle->remove_file(path.c_str(), path.size());
            }
        });

        AttachAFun(funs_FolderBrowser_remove_folder, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            if (args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                return handle->remove_folder(path.c_str(), path.size());
            } else {
                art::ustring path = (art::ustring)args[1];
                return handle->remove_folder(path.c_str(), path.size());
            }
        });

        AttachAFun(funs_FolderBrowser_remove_current_path, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->remove_current_path();
        });

        AttachAFun(funs_FolderBrowser_rename_file, 3, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            if (args[1].meta.vtype == VType::string && args[2].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                art::ustring& new_path = *(art::ustring*)args[2].getSourcePtr();
                return handle->rename_file(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else if (args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                art::ustring new_path = (art::ustring)args[2];
                return handle->rename_file(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else if (args[2].meta.vtype == VType::string) {
                art::ustring path = (art::ustring)args[1];
                art::ustring& new_path = *(art::ustring*)args[2].getSourcePtr();
                return handle->rename_file(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else {
                art::ustring path = (art::ustring)args[1];
                art::ustring new_path = (art::ustring)args[2];
                return handle->rename_file(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            }
        });

        AttachAFun(funs_FolderBrowser_rename_folder, 3, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            if (args[1].meta.vtype == VType::string && args[2].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                art::ustring& new_path = *(art::ustring*)args[2].getSourcePtr();
                return handle->rename_folder(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else if (args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                art::ustring new_path = (art::ustring)args[2];
                return handle->rename_folder(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else if (args[2].meta.vtype == VType::string) {
                art::ustring path = (art::ustring)args[1];
                art::ustring& new_path = *(art::ustring*)args[2].getSourcePtr();
                return handle->rename_folder(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else {
                art::ustring path = (art::ustring)args[1];
                art::ustring new_path = (art::ustring)args[2];
                return handle->rename_folder(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            }
        });

        AttachAFun(funs_FolderBrowser_join_folder, 2, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            if (args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[1].getSourcePtr();
                return ValueItem(CXX::Interface::constructStructure<typed_lgr<files::FolderBrowser>>(define_FolderBrowser, handle->join_folder(path.c_str(), path.size())), no_copy);
            } else {
                art::ustring path = (art::ustring)args[1];
                return ValueItem(CXX::Interface::constructStructure<typed_lgr<files::FolderBrowser>>(define_FolderBrowser, handle->join_folder(path.c_str(), path.size())), no_copy);
            }
        });

        AttachAFun(funs_FolderBrowser_get_current_path, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->get_current_path();
        });

        AttachAFun(funs_FolderBrowser_is_corrupted, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->is_corrupted();
        });

        AttachAFun(funs_FolderBrowser_file_extension, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->file_extension();
        });

        AttachAFun(funs_FolderBrowser_file_name, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->file_name();
        });

        AttachAFun(funs_FolderBrowser_file_name_without_extension, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->file_name_without_extension();
        });

        AttachAFun(funs_FolderBrowser_file_path, 1, {
            auto& handle = CXX::Interface::getExtractAs<typed_lgr<files::FolderBrowser>>(args[0], define_FolderBrowser);
            return handle->file_path();
        });
#pragma endregion

        AttachAFun(remove, 1, {
            if (args[0].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[0].getSourcePtr();
                return files::remove(path.c_str(), path.size());
            } else {
                art::ustring path = (art::ustring)args[0];
                return files::remove(path.c_str(), path.size());
            }
        });

        AttachAFun(rename, 2, {
            if (args[0].meta.vtype == VType::string && args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[0].getSourcePtr();
                art::ustring& new_path = *(art::ustring*)args[1].getSourcePtr();
                return files::rename(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else if (args[0].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[0].getSourcePtr();
                art::ustring new_path = (art::ustring)args[1];
                return files::rename(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else if (args[1].meta.vtype == VType::string) {
                art::ustring path = (art::ustring)args[0];
                art::ustring& new_path = *(art::ustring*)args[1].getSourcePtr();
                return files::rename(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else {
                art::ustring path = (art::ustring)args[0];
                art::ustring new_path = (art::ustring)args[1];
                return files::rename(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            }
        });

        AttachAFun(copy, 2, {
            if (args[0].meta.vtype == VType::string && args[1].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[0].getSourcePtr();
                art::ustring& new_path = *(art::ustring*)args[1].getSourcePtr();
                return files::copy(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else if (args[0].meta.vtype == VType::string) {
                art::ustring& path = *(art::ustring*)args[0].getSourcePtr();
                art::ustring new_path = (art::ustring)args[1];
                return files::copy(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else if (args[1].meta.vtype == VType::string) {
                art::ustring path = (art::ustring)args[0];
                art::ustring& new_path = *(art::ustring*)args[1].getSourcePtr();
                return files::copy(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            } else {
                art::ustring path = (art::ustring)args[0];
                art::ustring new_path = (art::ustring)args[1];
                return files::copy(path.c_str(), path.size(), new_path.c_str(), new_path.size());
            }
        });

        void init() {
            define_FileHandle = CXX::Interface::createTable<typed_lgr<files::FileHandle>>(
                "file_handle",
                CXX::Interface::direct_method("read", funs_FileHandle_read),
                CXX::Interface::direct_method("read_fixed", funs_FileHandle_read_fixed),
                CXX::Interface::direct_method("write", funs_FileHandle_write),
                CXX::Interface::direct_method("append", funs_FileHandle_append),
                CXX::Interface::direct_method("seek_pos", funs_FileHandle_seek_pos),
                CXX::Interface::direct_method("tell_pos", funs_FileHandle_tell_pos),
                CXX::Interface::direct_method("flush", funs_FileHandle_flush),
                CXX::Interface::direct_method("size", funs_FileHandle_size),
                CXX::Interface::direct_method("get_native_handle", funs_FileHandle_internal_get_handle, ClassAccess::intern),
                CXX::Interface::direct_method("get_path", funs_FileHandle_get_path)
            );
            define_BlockingFileHandle = CXX::Interface::createTable<typed_lgr<files::BlockingFileHandle>>(
                "blocking_file_handle",
                CXX::Interface::direct_method("read", funs_BlockingFileHandle_read),
                CXX::Interface::direct_method("write", funs_BlockingFileHandle_write),
                CXX::Interface::direct_method("seek_pos", funs_BlockingFileHandle_seek_pos),
                CXX::Interface::direct_method("tell_pos", funs_BlockingFileHandle_tell_pos),
                CXX::Interface::direct_method("flush", funs_BlockingFileHandle_flush),
                CXX::Interface::direct_method("size", funs_BlockingFileHandle_size),
                CXX::Interface::direct_method("eof_state", funs_BlockingFileHandle_eof_state),
                CXX::Interface::direct_method("valid", funs_BlockingFileHandle_valid),
                CXX::Interface::direct_method("get_native_handle", funs_BlockingFileHandle_internal_get_handle, ClassAccess::intern),
                CXX::Interface::direct_method("get_path", funs_BlockingFileHandle_get_path)
            );
            define_TextFile = CXX::Interface::createTable<typed_lgr<TextFile>>(
                "text_file",
                CXX::Interface::direct_method("read_line", funs_TextFile_read_line),
                CXX::Interface::direct_method("read_word", funs_TextFile_read_word),
                CXX::Interface::direct_method("read_symbol", funs_TextFile_read_symbol),
                CXX::Interface::direct_method("write", funs_TextFile_write),
                CXX::Interface::direct_method("read_bom", funs_TextFile_read_bom),
                CXX::Interface::direct_method("init_bom", funs_TextFile_init_bom)
            );
            define_FolderBrowser = CXX::Interface::createTable<typed_lgr<files::FolderBrowser>>(
                "folder_browser",
                CXX::Interface::direct_method("folders", funs_FolderBrowser_folders),
                CXX::Interface::direct_method("files", funs_FolderBrowser_files),
                CXX::Interface::direct_method("is_folder", funs_FolderBrowser_is_folder),
                CXX::Interface::direct_method("is_file", funs_FolderBrowser_is_file),
                CXX::Interface::direct_method("exists", funs_FolderBrowser_exists),
                CXX::Interface::direct_method("is_hidden", funs_FolderBrowser_is_hidden),
                CXX::Interface::direct_method("create_path", funs_FolderBrowser_create_path),
                CXX::Interface::direct_method("create_current_path", funs_FolderBrowser_create_current_path),
                CXX::Interface::direct_method("create_file", funs_FolderBrowser_create_file),
                CXX::Interface::direct_method("create_folder", funs_FolderBrowser_create_folder),
                CXX::Interface::direct_method("remove_file", funs_FolderBrowser_remove_file),
                CXX::Interface::direct_method("remove_folder", funs_FolderBrowser_remove_folder),
                CXX::Interface::direct_method("remove_current_path", funs_FolderBrowser_remove_current_path),
                CXX::Interface::direct_method("rename_file", funs_FolderBrowser_rename_file),
                CXX::Interface::direct_method("rename_folder", funs_FolderBrowser_rename_folder),
                CXX::Interface::direct_method("join_folder", funs_FolderBrowser_join_folder),
                CXX::Interface::direct_method("get_current_path", funs_FolderBrowser_get_current_path),
                CXX::Interface::direct_method("is_corrupted", funs_FolderBrowser_is_corrupted),
                CXX::Interface::direct_method("file_extension", funs_FolderBrowser_file_extension),
                CXX::Interface::direct_method("file_name", funs_FolderBrowser_file_name),
                CXX::Interface::direct_method("file_name_without_extension", funs_FolderBrowser_file_name_without_extension),
                CXX::Interface::direct_method("file_path", funs_FolderBrowser_file_path)
            );
            CXX::Interface::typeVTable<typed_lgr<files::FileHandle>>() = define_FileHandle;
            CXX::Interface::typeVTable<typed_lgr<files::BlockingFileHandle>>() = define_BlockingFileHandle;
            CXX::Interface::typeVTable<typed_lgr<TextFile>>() = define_TextFile;
            CXX::Interface::typeVTable<typed_lgr<files::FolderBrowser>>() = define_FolderBrowser;


            define_FileHandle->getAfterMethods()->constructor = new FuncEnvironment(constructor::createProxy_FileHandle);
            define_BlockingFileHandle->getAfterMethods()->constructor = new FuncEnvironment(constructor::createProxy_BlockingFileHandle);
            define_TextFile->getAfterMethods()->constructor = new FuncEnvironment(constructor::createProxy_TextFile);
            define_FolderBrowser->getAfterMethods()->constructor = new FuncEnvironment(constructor::createProxy_FolderBrowser);


            attacha_environment::get_types_global().join_namespace({"file", "file_handle"})->value = define_FileHandle;
            attacha_environment::get_types_global().join_namespace({"file", "blocking_file_handle"})->value = define_BlockingFileHandle;
            attacha_environment::get_types_global().join_namespace({"file", "text_file"})->value = define_TextFile;
            attacha_environment::get_types_global().join_namespace({"file", "folder_browser"})->value = define_FolderBrowser;
        }
    }
}