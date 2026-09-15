#include "editor/pages/module.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>
#include <vector>

#include <fmt/format.h>
#include <nfd.h>

#include "compiler/utility/context.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/parse_tree_writer.hpp"
#include "editor/pages/splash.hpp"
#include "editor/tools/completion_tool.hpp"
#include "editor/tools/conduit_tool.hpp"
#include "editor/tools/context_menu_tool.hpp"
#include "editor/tools/drag_tool.hpp"
#include "editor/tools/function_header_menu.hpp"
#include "editor/tools/operator_tool.hpp"
#include "editor/tools/pan_zoom_tool.hpp"
#include "editor/tools/popup_tool.hpp"
#include "editor/tools/rail_menu.hpp"
#include "editor/tools/select_tool.hpp"
#include "editor/tools/text_edit_tool.hpp"
#include "editor/tools/type_tool.hpp"
#include "editor/transaction/delete.hpp"
#include "editor/view/graph_draw.hpp"
#include "editor/view/graph_layout.hpp"

namespace fluir::editor {

  ModulePage::ModulePage(EditorContext& ctx, Renderer& renderer) : Page(ctx, renderer), state_{ctx} {
    state_.text = &renderer_;
    // An open popup takes everything, then an open draft takes keys; selection sees a press before a grip claims it.
    tools_.add(std::make_unique<PopupTool>());
    tools_.add(std::make_unique<TextEditTool>());
    tools_.add(std::make_unique<PanZoomTool>());
    tools_.add(std::make_unique<SelectTool>());
    tools_.add(std::make_unique<ConduitTool>());
    tools_.add(std::make_unique<OperatorTool>());
    tools_.add(std::make_unique<TypeTool>());
    tools_.add(std::make_unique<ContextMenuTool>(std::vector<MenuProvider>{functionHeaderItems, railItems}));
    tools_.add(std::make_unique<CompletionTool>());
    tools_.add(std::make_unique<DragTool>());

    header_.buttons = {
      Button{.label = "Save", .onClick = [this] { onSave(); }},
      Button{.label = "Save As", .onClick = [this] { onSaveAs(); }},
      Button{.label = "Undo",
             .onClick = [this] { state_.editor.undo(); },
             .enabled = [this] { return state_.editor.canUndo(); }},
      Button{.label = "Redo",
             .onClick = [this] { state_.editor.redo(); },
             .enabled = [this] { return state_.editor.canRedo(); }},
      Button{.label = "Exit", .onClick = [this] { shouldClose_ = true; }, .align = Button::Align::Right},
    };
  }

  int ModulePage::onStart() {
    if (!ctx_.program) {
      fmt::print(stderr, "no program to open\n");
      return 1;
    }
    fluir::Context cctx{
      .currentFile = *ctx_.program,
      .ignoreVersionChecks = true,
    };
    const auto result = loadFile(cctx, *ctx_.program);
    if (!result.tree) {
      fmt::print(stderr, "parse failed: {}\n", ctx_.program->string());
      return 1;
    }
    state_.editor.load(*result.tree);
    fitView();
    return 0;  // Page::start() lays the header out via onResize().
  }

  void ModulePage::onEvent(const InputEvent& event) {
    // The header sits over the graph: a press on it is the header's alone, and page commands never
    // run under a live gesture or draft.
    if (event.type == InputEvent::Type::MouseDown && headerLayout_.bar.contains(event.pos)) {
      tools_.cancel(state_);
      if (const auto index = buttonAt(headerLayout_, event.pos)) {
        header_.buttons[*index].onClick();
      }
      return;
    }
    const std::vector<Box> boxes = layoutGraph(state_.editor.tree(), ctx_.layout);
    if (tools_.dispatch(event, state_, boxes) || event.type != InputEvent::Type::KeyDown) {
      return;
    }
    if (event.key == InputEvent::Key::F) {
      fitView();
    } else if (event.key == InputEvent::Key::Delete) {
      deleteSelection();
    }
  }

  void ModulePage::onResize() {
    header_.label = ctx_.program ? ctx_.program->filename().string() : std::string{};
    headerLayout_ = layoutToolbar(header_, renderer_.outputSize().x, ctx_.layout, renderer_);
  }

  void ModulePage::onDraw() {
    const std::vector<Box> boxes = layoutGraph(state_.editor.tree(), ctx_.layout);
    {
      const Subview graph{state_.view, outputRect(), renderer_};
      drawGraph(graph, state_.editor.tree(), boxes, state_.selection, ctx_);
      tools_.draw(graph, state_, boxes);
    }
    drawToolbar(renderer_, header_, headerLayout_, ctx_);
    if (state_.popup) {
      state_.popup->draw(renderer_, ctx_);
    }
  }

  std::unique_ptr<Page> ModulePage::next() {
    return shouldClose_ ? std::make_unique<SplashPage>(ctx_, renderer_) : nullptr;
  }

  void ModulePage::fitView() {
    state_.view = Viewport{};
    state_.view.fitRect(graphBounds(layoutGraph(state_.editor.tree(), ctx_.layout)), renderer_.outputSize());
  }

  void ModulePage::deleteSelection() {
    if (!state_.selection) {
      return;
    }
    tools_.cancel(state_);
    state_.editor.apply(std::make_unique<DeleteTransaction>(*state_.selection));
    state_.selection.reset();
  }

  bool ModulePage::saveToPath(const std::filesystem::path& path) {
    std::ofstream ofs(path);
    ParseTreeWriter w(ofs);
    w.write(state_.editor.tree());
    if (!w.good()) {
      fmt::print(stderr, "save failed: {}\n", path.string());
      return false;
    }
    return true;
  }

  void ModulePage::onSave() {
    if (ctx_.program) {
      saveToPath(*ctx_.program);
    } else {
      onSaveAs();
    }
  }

  void ModulePage::onSaveAs() {
    nfdu8filteritem_t filter{"Fluir Program", "fl"};
    nfdu8char_t* outPath = nullptr;
    const nfdresult_t result = NFD_SaveDialogU8(&outPath, &filter, 1, nullptr, nullptr);

    if (result == NFD_OKAY) {
      std::filesystem::path chosen(outPath);
      NFD_FreePathU8(outPath);
      if (saveToPath(chosen)) {
        ctx_.program = chosen;
        onResize();  // the bar shows the new filename
      }
    } else if (result == NFD_ERROR) {
      fmt::print(stderr, "file dialog failed: {}\n", NFD_GetError());
    }
  }

}  // namespace fluir::editor
