#include "editor/pages/module.hpp"

#include <filesystem>
#include <fstream>
#include <string>
#include <variant>

#include <fmt/format.h>
#include <nfd.h>

#include "compiler/utility/context.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/core/interaction.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/parse_tree_writer.hpp"
#include "editor/core/tree_edit.hpp"
#include "editor/pages/splash.hpp"

namespace fluir::editor {
  ModulePage::ModulePage(EditorContext& ctx, Renderer& renderer) :
    Page(ctx, renderer),
    header_(renderer, [this] { onSave(); }, [this] { onSaveAs(); }, [this] { this->shouldClose = true; }) {
    hud_.setRoot(header_);
    hud_.add(std::make_unique<ClickInteraction>());

    graph_.setRoot(scene_.root());
    graph_.add(std::make_unique<PanZoomInteraction>());
    // After PanZoom so a Space+Left pan does not select; before Drag so a press
    // on a node's grip still selects it.
    graph_.add(std::make_unique<SelectionInteraction>(scene_));
    graph_.add(std::make_unique<DragInteraction>());
    graph_.add(std::make_unique<ClickInteraction>());
  }

  int ModulePage::onStart() {
    reset();
    fluir::Context cctx{
      .currentFile = *ctx_.program,  // TODO: Handle no program
      .ignoreVersionChecks = true,
    };

    const auto result = loadFile(cctx, *ctx_.program);

    if (!result.tree) {
      fmt::print(stderr, "parse failed: {}\n", ctx_.program->string());
      return 1;
    }
    tree_ = *result.tree;
    fileHeader_ = result.tree->header;
    scene_.build(ctx_, *tree_);

    graph_.viewport().fitRect(scene_.worldBounds(), renderer_.outputSize());
    return 0;  // Page::start() lays the chrome out via onResize().
  }

  bool ModulePage::onAppEvent(const InputEvent& event) {
    // Space is a pan modifier, not an app command.
    if (event.type == InputEvent::Type::KeyDown && event.key == InputEvent::Key::F) {
      graph_.viewport().fitRect(scene_.worldBounds(), renderer_.outputSize());
      return true;
    }
    // Delete is a page command, consumed whether or not anything is selected.
    if (event.type == InputEvent::Type::KeyDown && event.key == InputEvent::Key::Delete) {
      deleteSelection();
      return true;
    }
    return false;
  }

  std::unique_ptr<Page> ModulePage::next() {
    if (shouldClose) {
      return std::make_unique<SplashPage>(ctx_, renderer_);
    }
    return nullptr;
  }

  void ModulePage::reset() {
    graph_.setViewport(Viewport{});
    tree_.reset();
    fileHeader_ = pt::Header{};
    scene_.clear();
    graph_.reset();
    hud_.reset();
  }

  void ModulePage::layoutChrome() {
    header_.setLabel(ctx_.program ? ctx_.program->filename().string() : std::string{});
    header_.resize(ctx_, renderer_.outputSize().x);
  }

  void ModulePage::syncTreeFromScene() {
    if (!tree_) {
      return;
    }
    for (auto& [fnId, decl] : tree_->declarations) {
      auto& fn = std::get<fluir::pt::FunctionDecl>(decl);
      if (auto* fa = dynamic_cast<FunctionDeclActor*>(scene_.find(fn.id))) {
        fn.location = fa->location();
      }
      for (auto& [nodeId, node] : fn.body.nodes) {
        if (auto* na = dynamic_cast<NodeActor*>(scene_.find(fn.id, nodeId))) {
          std::visit([&](auto& n) { n.location = na->location(); }, node);
        }
      }
    }
  }

  void ModulePage::deleteSelection() {
    if (!tree_ || !scene_.selected()) {
      return;
    }
    syncTreeFromScene();  // or the rebuild below reverts every unsaved drag
    const fluir::FullID id = *scene_.selected();

    bool removed = false;
    if (id.size() == 1) {
      removed = deleteFunction(*tree_, id[0]);
    } else if (auto decl = tree_->declarations.find(id[0]); decl != tree_->declarations.end()) {
      removed = deleteNode(std::get<fluir::pt::FunctionDecl>(decl->second), id[1]);
    }
    if (!removed) {
      return;
    }

    scene_.clearSelection();
    // Every actor pointer dies with the rebuild, in-flight gestures included.
    graph_.reset();
    scene_.build(ctx_, *tree_);  // no refit: the viewport is the user's, not ours
  }

  bool ModulePage::saveToPath(const std::filesystem::path& path) {
    if (!tree_) {
      return false;
    }
    syncTreeFromScene();
    std::ofstream ofs(path);
    ParseTreeWriter w(ofs);
    w.write(*tree_);
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
        layoutChrome();  // the bar shows the new filename
      }
    } else if (result == NFD_ERROR) {
      fmt::print(stderr, "file dialog failed: {}\n", NFD_GetError());
    }
  }

}  // namespace fluir::editor
