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
#include "editor/core/render_graph.hpp"
#include "editor/pages/splash.hpp"

namespace fluir::editor {
  ModulePage::ModulePage(EditorContext& ctx, Renderer& renderer) :
    ctx_(ctx),
    renderer_(renderer),
    header_(renderer, [this] { onSave(); }, [this] { onSaveAs(); }, [this] { this->shouldClose = true; }) {
    hud_.setRoot(header_);
    hud_.add(std::make_unique<ClickInteraction>());

    graph_.setRoot(scene_.root());
    graph_.add(std::make_unique<PanZoomInteraction>());
    graph_.add(std::make_unique<DragInteraction>());
    graph_.add(std::make_unique<ClickInteraction>());
  }

  int ModulePage::start() {
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
    scene_.build(ctx_, *tree_);

    graph_.viewport().fitRect(scene_.worldBounds(), renderer_.outputSize());
    layoutChrome();
    return 0;
  }

  bool ModulePage::handleAppEvent(const InputEvent& event) {
    switch (event.type) {
      case InputEvent::Type::Quit:
        ctx_.running = false;
        return true;

      case InputEvent::Type::KeyDown:
        if (event.key == InputEvent::Key::Escape) {
          ctx_.running = false;
          return true;
        }
        if (event.key == InputEvent::Key::F) {
          graph_.viewport().fitRect(scene_.worldBounds(), renderer_.outputSize());
          return true;
        }
        return false;  // Space is a pan modifier, not an app command

      case InputEvent::Type::Resize:
        layoutChrome();
        return true;

      default:
        return false;
    }
  }

  int ModulePage::update(const std::vector<InputEvent>& events) {
    for (const auto& ie : events) {
      if (handleAppEvent(ie)) {
        continue;
      }
      if (hud_.dispatch(ie, ctx_, renderer_.outputSize())) {
        continue;
      }
      graph_.dispatch(ie, ctx_, renderer_.outputSize());
    }

    scene_.layout(ctx_);  // hit-testing must be correct before the next frame is drawn
    return 0;
  }

  int ModulePage::draw() {
    renderer_.beginFrame();

    GraphRenderer{ctx_, graph_.viewport(), renderer_, scene_}(*tree_);
    hud_.draw(renderer_, ctx_, Rect{0, 0, renderer_.outputSize().x, renderer_.outputSize().y});
    renderer_.endFrame();
    return 0;
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
