# fluir_embed_file(<target> <file> <symbol>)
#
# Compiles <file>'s bytes into <target> as `std::span<const unsigned char>
# <symbol>()`. <symbol> may be namespace-qualified. Re-embeds when <file>
# changes.
function (
    fluir_embed_file
    target
    file
    symbol
)
    get_filename_component(
        abs
        "${file}"
        ABSOLUTE
    )
    file(
        READ
        "${abs}"
        contents_as_hex
        HEX
    )
    string(
        REGEX
        REPLACE "([0-9a-f][0-9a-f])"
                "0x\\1,"
                bytes
                "${contents_as_hex}"
    )
    string(
        REGEX
        REPLACE "::[^:]+$"
                ""
                ns
                "${symbol}"
    )
    string(
        REGEX
        REPLACE "^.*::"
                ""
                name
                "${symbol}"
    )

    set(ns_open "")
    set(ns_close "")
    if (NOT
        ns
        STREQUAL
        symbol
    )
        set(ns_open "namespace ${ns} {")
        set(ns_close "}")
    endif ()

    string(MAKE_C_IDENTIFIER "${symbol}" ident)
    set(out "${CMAKE_CURRENT_BINARY_DIR}/embedded/${ident}.cpp")
    set(source
        "#include <span>\n${ns_open}\nnamespace {\nconstexpr unsigned char kBytes[] = {${bytes}};\n}\nstd::span<const unsigned char> ${name}() { return kBytes; }\n${ns_close}\n"
    )
    file(WRITE "${out}.tmp" "${source}")
    file(
        COPY_FILE
        "${out}.tmp"
        "${out}"
        ONLY_IF_DIFFERENT
    )
    set_property(
        DIRECTORY
        APPEND
        PROPERTY CMAKE_CONFIGURE_DEPENDS "${abs}"
    )
    target_sources(${target} PRIVATE "${out}")
endfunction ()
