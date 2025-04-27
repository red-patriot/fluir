#include "vm/decode.hpp"

#include <charconv>

namespace fluir {
  namespace {
    code::Header decodeHeader(std::string_view source) {
      code::Header header;

      header.filetype = source.at(0);

      auto majorStr = source.substr(1, 3);
      auto result = std::from_chars(majorStr.begin(), majorStr.end(),
                                    header.major);

      auto minorStr = std::string_view(result.ptr, 3);
      result = std::from_chars(minorStr.begin(), minorStr.end(),
                               header.minor);

      auto patchStr = std::string_view(result.ptr, 3);
      result = std::from_chars(patchStr.begin(), patchStr.end(),
                               header.patch);

      auto entryOffsetStr = std::string_view(result.ptr, 20);
      result = std::from_chars(entryOffsetStr.begin(),
                               entryOffsetStr.end(),
                               header.entryOffset);

      return header;
    }
  }  // namespace

  code::ByteCode decodeInspectCode(std::string_view source) {
    code::ByteCode decoded{};

    decoded.header = decodeHeader(source);

    return decoded;
  }
}  // namespace fluir
