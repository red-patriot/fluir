#ifndef FLUIR_EDITOR_PAGES_MODULE_HPP
#define FLUIR_EDITOR_PAGES_MODULE_HPP

#include <filesystem>
#include <memory>

#include "editor/core/editor_context.hpp"
#include "editor/core/renderer.hpp"
#include "editor/input.hpp"
#include "editor/pages/page.hpp"
#include "editor/tools/tool.hpp"
#include "editor/view/toolbar.hpp"

namespace fluir::editor {
  /** The page to display an open module and edit it. */
  class ModulePage : public Page {
   public:
    ModulePage(EditorContext& ctx, Renderer& renderer);

    std::unique_ptr<Page> next() override;

    // Test-only observability: the model, selection and view the page edits,
    // and the header bar a test clicks through.
    const EditorState& state() const { return state_; }
    const Toolbar& header() const { return header_; }
    const ToolbarLayout& headerLayout() const { return headerLayout_; }

   protected:
    int onStart() override;
    void onEvent(const InputEvent& event) override;
    void onResize() override;
    void onDraw() override;

   private:
    EditorState state_;
    ToolChain tools_;
    Toolbar header_;
    ToolbarLayout headerLayout_;
    bool shouldClose_ = false;

    void fitView();
    void deleteSelection();
    void onSave();
    void onSaveAs();
    bool saveToPath(const std::filesystem::path& path);
  };
}  // namespace fluir::editor

#endif
