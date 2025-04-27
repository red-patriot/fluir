#include "vm/decode.hpp"

#include <charconv>

namespace fluir {
  namespace {
    code::Header decodeHeader(std::string_view source) {
      code::Header header;

      header.filetype = source.at(0);

      auto majorStr = source.substr(1, 2);
      auto result = std::from_chars(majorStr.begin(), majorStr.end(),
                                    header.major, 16);

      auto minorStr = std::string_view(result.ptr, 2);
      result = std::from_chars(minorStr.begin(), minorStr.end(),
                               header.minor, 16);

      auto patchStr = std::string_view(result.ptr, 2);
      result = std::from_chars(patchStr.begin(), patchStr.end(),
                               header.patch, 16);

      auto entryOffsetStr = std::string_view(result.ptr, 16);
      result = std::from_chars(entryOffsetStr.begin(),
                               entryOffsetStr.end(),
                               header.entryOffset, 16);

      return header;
    }
  }  // namespace

  code::ByteCode decodeInspectCode(std::string_view source) {
    code::ByteCode decoded{};

    decoded.header = decodeHeader(source);

    return decoded;
  }
}  // namespace fluir
