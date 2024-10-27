// Copyright Danyil Melnytskyi 2022-Present
//
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE or copy at
// http://www.boost.org/LICENSE_1_0.txt)

#include <run_time/asm/il_header_decoder.hpp>
#include <run_time/util/tools.hpp>

namespace art {
    size_t il_header::decoded::decode(
        CASM& casm_assembler, //for labels
        const std::vector<uint8_t>& data,
        size_t start,
        size_t end_offset) {
        art::ustring version = reader::readString(data, end_offset, start);
        return decode(
            version,
            casm_assembler,
            data,
            start,
            end_offset);
    }

    size_t il_header::decoded::decode(
        const art::ustring& header_compiler_name_version,
        CASM& a,
        const std::vector<uint8_t>& data,
        size_t to_be_skiped,
        size_t data_len) {
        if (compiler)
            delete compiler;

        compiler = il_compiler::map_compiler(header_compiler_name_version);
        compiler->decode_header(
            data,
            to_be_skiped,
            data_len,
            a,
            jump_list,
            line_info,
            file_local_path,
            locals,
            flags,
            used_static_values,
            used_enviro_vals,
            used_arguments,
            constants_values
        );
        return to_be_skiped;
    }

    il_header::decoded::~decoded() {
        if (compiler)
            delete compiler;
    }
}