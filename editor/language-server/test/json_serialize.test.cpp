#include "lsp/json_serialize.hpp"

#include <gtest/gtest.h>

using namespace fluir::lsp;

// --- Lifecycle ---

TEST(JsonSerialize, InitResponse) {
  api::InitResponse resp{"1.2.3"};
  auto result = toJson(resp);
  EXPECT_EQ(result["version"], "1.2.3");
}

TEST(JsonSerialize, ShutdownResponse) {
  api::ShutdownResponse resp{};
  auto result = toJson(resp);
  EXPECT_EQ(result, nlohmann::json::object());
}

// --- Document ---

TEST(JsonSerialize, OpenDocResponse) {
  api::OpenDocResponse resp{};
  auto result = toJson(resp);
  EXPECT_EQ(result, nlohmann::json::object());
}

TEST(JsonSerialize, CloseDocResponse) {
  api::CloseDocResponse resp{};
  auto result = toJson(resp);
  EXPECT_EQ(result, nlohmann::json::object());
}

TEST(JsonSerialize, DocEditResponse) {
  api::DocEditResponse resp{};
  auto result = toJson(resp);
  EXPECT_EQ(result, nlohmann::json::object());
}

// --- Language ---

TEST(JsonSerialize, DocumentSymbolNoOptionals) {
  api::DocumentSymbol sym{"myFunc", std::nullopt, std::nullopt, std::nullopt};
  auto result = toJson(sym);
  EXPECT_EQ(result["name"], "myFunc");
  EXPECT_FALSE(result.contains("detail"));
  EXPECT_FALSE(result.contains("outType"));
  EXPECT_FALSE(result.contains("inType"));
}

TEST(JsonSerialize, DocumentSymbolAllOptionals) {
  api::DocumentSymbol sym{
    "myFunc",
    std::optional<std::string>{"some detail"},
    std::optional<std::string>{"F64"},
    std::optional<std::vector<std::string>>{{"I32", "I64"}},
  };
  auto result = toJson(sym);
  EXPECT_EQ(result["name"], "myFunc");
  EXPECT_EQ(result["detail"], "some detail");
  EXPECT_EQ(result["outType"], "F64");
  ASSERT_TRUE(result["inType"].is_array());
  EXPECT_EQ(result["inType"].size(), 2u);
  EXPECT_EQ(result["inType"][0], "I32");
  EXPECT_EQ(result["inType"][1], "I64");
}

TEST(JsonSerialize, TaggedDocumentSymbol) {
  fluir::FullID id{1, 2, 3};
  api::DocumentSymbol sym{"block", std::nullopt, std::nullopt, std::nullopt};
  api::TaggedDocumentSymbol tagged{id, sym};
  auto result = toJson(tagged);

  ASSERT_TRUE(result["id"].is_array());
  EXPECT_EQ(result["id"].size(), 3u);
  EXPECT_EQ(result["id"][0], 1u);
  EXPECT_EQ(result["id"][1], 2u);
  EXPECT_EQ(result["id"][2], 3u);

  EXPECT_EQ(result["symbol"]["name"], "block");
}

TEST(JsonSerialize, DocumentSymbolsEmpty) {
  api::DocumentSymbols syms{{}};
  auto result = toJson(syms);
  ASSERT_TRUE(result["symbols"].is_array());
  EXPECT_EQ(result["symbols"].size(), 0u);
}

TEST(JsonSerialize, DocumentSymbolsTwoElements) {
  api::DocumentSymbol sym1{"foo", std::nullopt, std::nullopt, std::nullopt};
  api::DocumentSymbol sym2{"bar", std::nullopt, std::nullopt, std::nullopt};
  fluir::FullID id1{10};
  fluir::FullID id2{20};
  api::DocumentSymbols syms{{
    api::TaggedDocumentSymbol{id1, sym1},
    api::TaggedDocumentSymbol{id2, sym2},
  }};
  auto result = toJson(syms);
  ASSERT_EQ(result["symbols"].size(), 2u);
  EXPECT_EQ(result["symbols"][0]["symbol"]["name"], "foo");
  EXPECT_EQ(result["symbols"][1]["symbol"]["name"], "bar");
}

TEST(JsonSerialize, CompletionOptionNoDetail) {
  api::CompletionOption opt{"myFunc", api::CompletionKind::FUNCTION_DEF, std::nullopt};
  auto result = toJson(opt);
  EXPECT_EQ(result["label"], "myFunc");
  EXPECT_EQ(result["kind"], static_cast<int>(api::CompletionKind::FUNCTION_DEF));
  EXPECT_FALSE(result.contains("detail"));
}

TEST(JsonSerialize, CompletionOptionWithDetail) {
  api::CompletionOption opt{"add", api::CompletionKind::CALL, std::optional<std::string>{"adds two numbers"}};
  auto result = toJson(opt);
  EXPECT_EQ(result["label"], "add");
  EXPECT_EQ(result["kind"], static_cast<int>(api::CompletionKind::CALL));
  EXPECT_EQ(result["detail"], "adds two numbers");
}

TEST(JsonSerialize, CompletionPossibilitiesEmpty) {
  api::CompletionPossibilities poss{true, {}};
  auto result = toJson(poss);
  EXPECT_EQ(result["isComplete"], true);
  ASSERT_TRUE(result["completions"].is_array());
  EXPECT_EQ(result["completions"].size(), 0u);
}

TEST(JsonSerialize, CompletionPossibilitiesTwoOptions) {
  api::CompletionOption opt1{"foo", api::CompletionKind::FUNCTION_DEF, std::nullopt};
  api::CompletionOption opt2{"bar", api::CompletionKind::CONSTANT, std::nullopt};
  api::CompletionPossibilities poss{false, {opt1, opt2}};
  auto result = toJson(poss);
  EXPECT_EQ(result["isComplete"], false);
  ASSERT_EQ(result["completions"].size(), 2u);
  EXPECT_EQ(result["completions"][0]["label"], "foo");
  EXPECT_EQ(result["completions"][1]["label"], "bar");
}

TEST(JsonSerialize, SelectedCompletion) {
  api::SelectedCompletion sel{"some text"};
  auto result = toJson(sel);
  EXPECT_EQ(result["text"], "some text");
}

TEST(JsonSerialize, ModuleDiagnosticWithId) {
  fluir::FullID id{1, 2, 3};
  api::ModuleDiagnostic diag{id, api::DiagnosticSeverity::ERROR, "something went wrong"};
  auto result = toJson(diag);
  ASSERT_TRUE(result["location"].is_array());
  EXPECT_EQ(result["location"].size(), 3u);
  EXPECT_EQ(result["location"][0], 1u);
  EXPECT_EQ(result["severity"], static_cast<int>(api::DiagnosticSeverity::ERROR));
  EXPECT_EQ(result["message"], "something went wrong");
}

TEST(JsonSerialize, ModuleDiagnosticWithLocation) {
  fluir::FlowGraphLocation loc{10, 20, 0, 100, 50};
  api::ModuleDiagnostic diag{loc, api::DiagnosticSeverity::WARNING, "a warning"};
  auto result = toJson(diag);
  ASSERT_TRUE(result["location"].is_object());
  EXPECT_EQ(result["location"]["x"], 10);
  EXPECT_EQ(result["location"]["y"], 20);
  EXPECT_EQ(result["location"]["z"], 0);
  EXPECT_EQ(result["location"]["width"], 100);
  EXPECT_EQ(result["location"]["height"], 50);
  EXPECT_EQ(result["severity"], static_cast<int>(api::DiagnosticSeverity::WARNING));
  EXPECT_EQ(result["message"], "a warning");
}

TEST(JsonSerialize, DiagnosticsEmpty) {
  api::Diagnostics diags{{}};
  auto result = toJson(diags);
  ASSERT_TRUE(result["diagnostics"].is_array());
  EXPECT_EQ(result["diagnostics"].size(), 0u);
}

TEST(JsonSerialize, DiagnosticsTwoElements) {
  fluir::FullID id{5};
  fluir::FlowGraphLocation loc{1, 2, 3, 4, 5};
  api::Diagnostics diags{{
    api::ModuleDiagnostic{id, api::DiagnosticSeverity::ERROR, "error msg"},
    api::ModuleDiagnostic{loc, api::DiagnosticSeverity::HINT, "hint msg"},
  }};
  auto result = toJson(diags);
  ASSERT_EQ(result["diagnostics"].size(), 2u);
  EXPECT_TRUE(result["diagnostics"][0]["location"].is_array());
  EXPECT_TRUE(result["diagnostics"][1]["location"].is_object());
}

// --- FromJson ---

TEST(FromJson, InitRequest) {
  auto result = fromJson<api::InitRequest>(nlohmann::json::object());
  (void)result;  // empty struct; just verify it compiles and doesn't throw
}

TEST(FromJson, ShutdownRequest) {
  auto result = fromJson<api::ShutdownRequest>(nlohmann::json::object());
  (void)result;
}

TEST(FromJson, OpenDocRequest) {
  nlohmann::json j;
  j["path"] = "path/to/file.fl";
  j["content"] = "content here";
  auto result = fromJson<api::OpenDocRequest>(j);
  EXPECT_EQ(result.path, "path/to/file.fl");
  EXPECT_EQ(result.content, "content here");
}

TEST(FromJson, CloseDocRequest) {
  nlohmann::json j;
  j["path"] = "path/to/file.fl";
  auto result = fromJson<api::CloseDocRequest>(j);
  EXPECT_EQ(result.path, "path/to/file.fl");
}

TEST(FromJson, DocEdit) {
  nlohmann::json j;
  j["contents"] = "new contents";
  auto result = fromJson<api::DocEdit>(j);
  EXPECT_EQ(result.contents, "new contents");
}

TEST(FromJson, DocumentSymbolRequest) {
  nlohmann::json j;
  j["path"] = "path/to/file.fl";
  auto result = fromJson<api::DocumentSymbolRequest>(j);
  EXPECT_EQ(result.path, "path/to/file.fl");
}

TEST(FromJson, CompletionsRequestBody) {
  nlohmann::json j;
  j["parentBlock"] = nlohmann::json::array({uint64_t{5}, uint64_t{6}});
  j["context"] = static_cast<int>(api::CompletionsRequest::Context::BODY);
  auto result = fromJson<api::CompletionsRequest>(j);
  ASSERT_EQ(result.parentBlock.size(), 2u);
  EXPECT_EQ(result.parentBlock[0], 5u);
  EXPECT_EQ(result.parentBlock[1], 6u);
  EXPECT_EQ(result.context, api::CompletionsRequest::Context::BODY);
}

TEST(FromJson, CompletionsRequestHeader) {
  nlohmann::json j;
  j["parentBlock"] = nlohmann::json::array({uint64_t{1}});
  j["context"] = static_cast<int>(api::CompletionsRequest::Context::HEADER);
  auto result = fromJson<api::CompletionsRequest>(j);
  EXPECT_EQ(result.context, api::CompletionsRequest::Context::HEADER);
}

TEST(FromJson, SelectCompletion) {
  nlohmann::json j;
  j["parentBlock"] = nlohmann::json::array({uint64_t{7}, uint64_t{8}, uint64_t{9}});
  j["selected"] = "mySelected";
  auto result = fromJson<api::SelectCompletion>(j);
  ASSERT_EQ(result.parentBlock.size(), 3u);
  EXPECT_EQ(result.parentBlock[0], 7u);
  EXPECT_EQ(result.parentBlock[1], 8u);
  EXPECT_EQ(result.parentBlock[2], 9u);
  EXPECT_EQ(result.selected, "mySelected");
}

TEST(FromJson, RequestDiagnostics) {
  nlohmann::json j;
  j["path"] = "path/to/file.fl";
  auto result = fromJson<api::RequestDiagnostics>(j);
  EXPECT_EQ(result.path, "path/to/file.fl");
}
