#include "editor/pages/module.hpp"

#include <filesystem>
#include <fstream>
#include <memory>
#include <string>

#include <fmt/format.h>
#include <nfd.h>

#include "compiler/utility/context.hpp"
#include "editor/actors/function_decl_actor.hpp"
#include "editor/actors/node_actors.hpp"
#include "editor/core/interaction.hpp"
#include "editor/core/loader.hpp"
#include "editor/core/parse_tree_writer.hpp"
#include "editor/core/scene_to_tree.hpp"
#include "editor/pages/splash.hpp"
#include "editor/transaction/delete.hpp"

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
    fileHeader_ = result.tree->header;
    scene_.build(ctx_, *result.tree);  // the tree is a load format; the scene is the model

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
    fileHeader_ = pt::Header{};
    scene_.clear();
    graph_.reset();
    hud_.reset();
  }

  void ModulePage::layoutChrome() {
    header_.setLabel(ctx_.program ? ctx_.program->filename().string() : std::string{});
    header_.resize(ctx_, renderer_.outputSize().x);
  }

  void ModulePage::deleteSelection() {
    if (!scene_.selected()) {
      return;
    }
    DeleteTransaction edit{*scene_.selected()};
    if (!edit.execute(scene_)) {
      return;
    }
    // The detach frees the actors, in-flight gestures pointing at them included.
    graph_.reset();
  }

  bool ModulePage::saveToPath(const std::filesystem::path& path) {
    std::ofstream ofs(path);
    ParseTreeWriter w(ofs);
    w.write(sceneToParseTree(scene_, fileHeader_));
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
