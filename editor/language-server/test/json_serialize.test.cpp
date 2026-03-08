#include "lsp/json_serialize.hpp"

#include <gtest/gtest.h>

using namespace fluir::lsp;

// --- Lifecycle ---

TEST(JsonSerialize, InitRequest) {
  api::InitRequest req{};
  auto result = toJson(req);
  EXPECT_EQ(result, nlohmann::json::object());
}

TEST(JsonSerialize, InitResponse) {
  api::InitResponse resp{"1.2.3"};
  auto result = toJson(resp);
  EXPECT_EQ(result["version"], "1.2.3");
}

TEST(JsonSerialize, ShutdownRequest) {
  api::ShutdownRequest req{};
  auto result = toJson(req);
  EXPECT_EQ(result, nlohmann::json::object());
}

TEST(JsonSerialize, ShutdownResponse) {
  api::ShutdownResponse resp{};
  auto result = toJson(resp);
  EXPECT_EQ(result, nlohmann::json::object());
}

// --- Document ---

TEST(JsonSerialize, OpenDocRequest) {
  api::OpenDocRequest req{"path/to/file.fl", "content here"};
  auto result = toJson(req);
  EXPECT_EQ(result["path"], "path/to/file.fl");
  EXPECT_EQ(result["content"], "content here");
}

TEST(JsonSerialize, OpenDocResponse) {
  api::OpenDocResponse resp{};
  auto result = toJson(resp);
  EXPECT_EQ(result, nlohmann::json::object());
}

TEST(JsonSerialize, CloseDocRequest) {
  api::CloseDocRequest req{"path/to/file.fl"};
  auto result = toJson(req);
  EXPECT_EQ(result["path"], "path/to/file.fl");
}

TEST(JsonSerialize, CloseDocResponse) {
  api::CloseDocResponse resp{};
  auto result = toJson(resp);
  EXPECT_EQ(result, nlohmann::json::object());
}

TEST(JsonSerialize, DocEdit) {
  api::DocEdit edit{"new contents"};
  auto result = toJson(edit);
  EXPECT_EQ(result["contents"], "new contents");
}

TEST(JsonSerialize, DocEditResponse) {
  api::DocEditResponse resp{};
  auto result = toJson(resp);
  EXPECT_EQ(result, nlohmann::json::object());
}

// --- Language ---

TEST(JsonSerialize, DocumentSymbolRequest) {
  api::DocumentSymbolRequest req{"path/to/file.fl"};
  auto result = toJson(req);
  EXPECT_EQ(result["path"], "path/to/file.fl");
}

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

TEST(JsonSerialize, CompletionsRequest) {
  fluir::FullID parentBlock{5, 6};
  api::CompletionsRequest req{parentBlock, api::CompletionsRequest::Context::BODY};
  auto result = toJson(req);

  ASSERT_TRUE(result["parentBlock"].is_array());
  EXPECT_EQ(result["parentBlock"].size(), 2u);
  EXPECT_EQ(result["parentBlock"][0], 5u);
  EXPECT_EQ(result["parentBlock"][1], 6u);
  EXPECT_EQ(result["context"], static_cast<int>(api::CompletionsRequest::Context::BODY));
}

TEST(JsonSerialize, CompletionsRequestHeaderContext) {
  fluir::FullID parentBlock{1};
  api::CompletionsRequest req{parentBlock, api::CompletionsRequest::Context::HEADER};
  auto result = toJson(req);
  EXPECT_EQ(result["context"], static_cast<int>(api::CompletionsRequest::Context::HEADER));
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

TEST(JsonSerialize, SelectCompletion) {
  fluir::FullID parentBlock{7, 8, 9};
  api::SelectCompletion sel{parentBlock, "mySelected"};
  auto result = toJson(sel);

  ASSERT_TRUE(result["parentBlock"].is_array());
  EXPECT_EQ(result["parentBlock"].size(), 3u);
  EXPECT_EQ(result["parentBlock"][0], 7u);
  EXPECT_EQ(result["selected"], "mySelected");
}

TEST(JsonSerialize, SelectedCompletion) {
  api::SelectedCompletion sel{"some text"};
  auto result = toJson(sel);
  EXPECT_EQ(result["text"], "some text");
}

// --- FromJson ---

TEST(FromJson, InitRequest) {
  auto result = fromJson<api::InitRequest>(nlohmann::json::object());
  (void)result;  // empty struct; just verify it compiles and doesn't throw
}

TEST(FromJson, InitResponse) {
  api::InitResponse orig{"1.2.3"};
  auto result = fromJson<api::InitResponse>(toJson(orig));
  EXPECT_EQ(result.version, "1.2.3");
}

TEST(FromJson, ShutdownRequest) {
  auto result = fromJson<api::ShutdownRequest>(nlohmann::json::object());
  (void)result;
}

TEST(FromJson, ShutdownResponse) {
  auto result = fromJson<api::ShutdownResponse>(nlohmann::json::object());
  (void)result;
}

TEST(FromJson, OpenDocRequest) {
  api::OpenDocRequest orig{"path/to/file.fl", "content here"};
  auto result = fromJson<api::OpenDocRequest>(toJson(orig));
  EXPECT_EQ(result.path, "path/to/file.fl");
  EXPECT_EQ(result.content, "content here");
}

TEST(FromJson, OpenDocResponse) {
  auto result = fromJson<api::OpenDocResponse>(nlohmann::json::object());
  (void)result;
}

TEST(FromJson, CloseDocRequest) {
  api::CloseDocRequest orig{"path/to/file.fl"};
  auto result = fromJson<api::CloseDocRequest>(toJson(orig));
  EXPECT_EQ(result.path, "path/to/file.fl");
}

TEST(FromJson, CloseDocResponse) {
  auto result = fromJson<api::CloseDocResponse>(nlohmann::json::object());
  (void)result;
}

TEST(FromJson, DocEdit) {
  api::DocEdit orig{"new contents"};
  auto result = fromJson<api::DocEdit>(toJson(orig));
  EXPECT_EQ(result.contents, "new contents");
}

TEST(FromJson, DocEditResponse) {
  auto result = fromJson<api::DocEditResponse>(nlohmann::json::object());
  (void)result;
}

TEST(FromJson, DocumentSymbolRequest) {
  api::DocumentSymbolRequest orig{"path/to/file.fl"};
  auto result = fromJson<api::DocumentSymbolRequest>(toJson(orig));
  EXPECT_EQ(result.path, "path/to/file.fl");
}

TEST(FromJson, DocumentSymbolNoOptionals) {
  api::DocumentSymbol orig{"myFunc", std::nullopt, std::nullopt, std::nullopt};
  auto result = fromJson<api::DocumentSymbol>(toJson(orig));
  EXPECT_EQ(result.name, "myFunc");
  EXPECT_FALSE(result.detail.has_value());
  EXPECT_FALSE(result.outType.has_value());
  EXPECT_FALSE(result.inType.has_value());
}

TEST(FromJson, DocumentSymbolAllOptionals) {
  api::DocumentSymbol orig{
    "myFunc",
    std::optional<std::string>{"some detail"},
    std::optional<std::string>{"F64"},
    std::optional<std::vector<std::string>>{{"I32", "I64"}},
  };
  auto result = fromJson<api::DocumentSymbol>(toJson(orig));
  EXPECT_EQ(result.name, "myFunc");
  ASSERT_TRUE(result.detail.has_value());
  EXPECT_EQ(*result.detail, "some detail");
  ASSERT_TRUE(result.outType.has_value());
  EXPECT_EQ(*result.outType, "F64");
  ASSERT_TRUE(result.inType.has_value());
  ASSERT_EQ(result.inType->size(), 2u);
  EXPECT_EQ((*result.inType)[0], "I32");
  EXPECT_EQ((*result.inType)[1], "I64");
}

TEST(FromJson, TaggedDocumentSymbol) {
  fluir::FullID id{1, 2, 3};
  api::DocumentSymbol sym{"block", std::nullopt, std::nullopt, std::nullopt};
  api::TaggedDocumentSymbol orig{id, sym};
  auto result = fromJson<api::TaggedDocumentSymbol>(toJson(orig));
  ASSERT_EQ(result.id.size(), 3u);
  EXPECT_EQ(result.id[0], 1u);
  EXPECT_EQ(result.id[1], 2u);
  EXPECT_EQ(result.id[2], 3u);
  EXPECT_EQ(result.symbol.name, "block");
}

TEST(FromJson, DocumentSymbols) {
  api::DocumentSymbol sym1{"foo", std::nullopt, std::nullopt, std::nullopt};
  api::DocumentSymbol sym2{"bar", std::nullopt, std::nullopt, std::nullopt};
  api::DocumentSymbols orig{{
    api::TaggedDocumentSymbol{fluir::FullID{10}, sym1},
    api::TaggedDocumentSymbol{fluir::FullID{20}, sym2},
  }};
  auto result = fromJson<api::DocumentSymbols>(toJson(orig));
  ASSERT_EQ(result.symbols.size(), 2u);
  EXPECT_EQ(result.symbols[0].symbol.name, "foo");
  EXPECT_EQ(result.symbols[1].symbol.name, "bar");
}

TEST(FromJson, CompletionsRequestBody) {
  fluir::FullID parentBlock{5, 6};
  api::CompletionsRequest orig{parentBlock, api::CompletionsRequest::Context::BODY};
  auto result = fromJson<api::CompletionsRequest>(toJson(orig));
  ASSERT_EQ(result.parentBlock.size(), 2u);
  EXPECT_EQ(result.parentBlock[0], 5u);
  EXPECT_EQ(result.parentBlock[1], 6u);
  EXPECT_EQ(result.context, api::CompletionsRequest::Context::BODY);
}

TEST(FromJson, CompletionsRequestHeader) {
  fluir::FullID parentBlock{1};
  api::CompletionsRequest orig{parentBlock, api::CompletionsRequest::Context::HEADER};
  auto result = fromJson<api::CompletionsRequest>(toJson(orig));
  EXPECT_EQ(result.context, api::CompletionsRequest::Context::HEADER);
}

TEST(FromJson, CompletionOptionNoDetail) {
  api::CompletionOption orig{"myFunc", api::CompletionKind::FUNCTION_DEF, std::nullopt};
  auto result = fromJson<api::CompletionOption>(toJson(orig));
  EXPECT_EQ(result.label, "myFunc");
  EXPECT_EQ(result.kind, api::CompletionKind::FUNCTION_DEF);
  EXPECT_FALSE(result.detail.has_value());
}

TEST(FromJson, CompletionOptionWithDetail) {
  api::CompletionOption orig{"add", api::CompletionKind::CALL, std::optional<std::string>{"adds two numbers"}};
  auto result = fromJson<api::CompletionOption>(toJson(orig));
  EXPECT_EQ(result.label, "add");
  EXPECT_EQ(result.kind, api::CompletionKind::CALL);
  ASSERT_TRUE(result.detail.has_value());
  EXPECT_EQ(*result.detail, "adds two numbers");
}

TEST(FromJson, CompletionPossibilitiesEmpty) {
  api::CompletionPossibilities orig{true, {}};
  auto result = fromJson<api::CompletionPossibilities>(toJson(orig));
  EXPECT_EQ(result.isComplete, true);
  EXPECT_TRUE(result.completions.empty());
}

TEST(FromJson, CompletionPossibilitiesWithOptions) {
  api::CompletionOption opt1{"foo", api::CompletionKind::FUNCTION_DEF, std::nullopt};
  api::CompletionOption opt2{"bar", api::CompletionKind::CONSTANT, std::nullopt};
  api::CompletionPossibilities orig{false, {opt1, opt2}};
  auto result = fromJson<api::CompletionPossibilities>(toJson(orig));
  EXPECT_EQ(result.isComplete, false);
  ASSERT_EQ(result.completions.size(), 2u);
  EXPECT_EQ(result.completions[0].label, "foo");
  EXPECT_EQ(result.completions[1].label, "bar");
}

TEST(FromJson, SelectCompletion) {
  fluir::FullID parentBlock{7, 8, 9};
  api::SelectCompletion orig{parentBlock, "mySelected"};
  auto result = fromJson<api::SelectCompletion>(toJson(orig));
  ASSERT_EQ(result.parentBlock.size(), 3u);
  EXPECT_EQ(result.parentBlock[0], 7u);
  EXPECT_EQ(result.parentBlock[1], 8u);
  EXPECT_EQ(result.parentBlock[2], 9u);
  EXPECT_EQ(result.selected, "mySelected");
}

TEST(FromJson, SelectedCompletion) {
  api::SelectedCompletion orig{"some text"};
  auto result = fromJson<api::SelectedCompletion>(toJson(orig));
  EXPECT_EQ(result.text, "some text");
}
