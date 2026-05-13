#pragma once
#include "DataBinding.h"
#include "EventSystem.h"
#include "Graphics.h"
#include "InputFilter.h"
#include "Layout.h"
#include "TextProcessing.h"
#include "Timer.h"

#include <string>

namespace gui {
    class Element;

    // TextInputController holds the editing state and implements single-line text
    // rendering (caret, selection, scroll offset) and editing operations (insert,
    // remove, delete selection).  Embed it in any Element to gain full text input
    // support without inheriting from LineEdit.
    //
    // Typical usage:
    //   class MyWidget : public Element {
    //       TextInputController m_editor;
    //       Property<std::string> m_text;
    //       MyWidget() {
    //           m_editor.Init(this, &m_text);
    //           m_editor.SetEditOps({ [this](char c){ m_editor.InsertChar(c); }, ... });
    //       }
    //       void OnDraw(Graphics& g) override {
    //           m_editor.Draw(g, GetStyle(), GetSize(), IsFocused());
    //       }
    //       void OnKeyDown(KeyEvent e) override  { m_editor.HandleKeyDown(e); }
    //       void OnTextInput(TextInputEvent e) override { m_editor.HandleTextInput(e); }
    //   };
    class TextInputController {
    public:
        // Callbacks for edit operations that may be overridden by element subclasses.
        // Default-constructed ops call the controller's own methods directly.
        struct EditOps {
            std::function<void(char)> insertChar;
            std::function<void(int)> removeChar;
            std::function<void()> deleteSelected;
            std::function<void()> notify;
        };

        TextInputController() = default;

        // Store the owning element and the text property to operate on.
        // Sets up default EditOps (direct controller calls) and starts blink timer.
        void Init(Element* owner, Property<std::string>* text);

        // Override the default EditOps.  Call this to wire up virtual dispatch
        // (e.g. LineEdit calls InsertChar virtually so TextArea's override runs).
        void SetEditOps(EditOps ops) { m_ops = std::move(ops); }

        // Rebuild the text layout from the current property value.
        void Rebuild(Graphics& g, const Json& style);

        // Render text, selection highlight, and caret within the given bounds rectangle
        // (element-local coordinates).  The controller clips to bounds and translates
        // internally, so the caller does not need to set up a clip region.
        void Draw(Graphics& g, const Json& style, const Rectangle& bounds, bool focused);

        // ---- Reusable event handlers ------------------------------------------------
        // Call these from the element's On* overrides to get standard text-editing
        // behaviour without any boilerplate.  TextArea overrides some of these (e.g.
        // OnMouseDown) with its own multiline hit-testing, then calls the rest.
        //
        // originX: x offset of the content area within the element (e.g. ButtonWidth
        // for a Spinner whose text lives in a center rect starting at ButtonWidth).

        void HandleMouseDown(MouseEvent e, const EdgeInsets& pad, int originX = 0);
        void HandleMouseUp(MouseEvent e);
        void HandleMouseMove(MotionEvent e, const EdgeInsets& pad, int originX = 0);
        void HandleKeyDown(KeyEvent e);
        void HandleTextInput(TextInputEvent e);

        // ---- Editing operations (respect the InputFilter) ---------------------------
        // InsertChar rejects '\r'/'\n' (single-line) and chars that fail the filter.
        void InsertChar(char c);
        void RemoveChar(int i);
        void DeleteSelected();

        // Selection
        void Select(int from, int len = -1);
        void Deselect();
        bool IsSelected() const;

        // Returns true if c passes the current filter at the current caret position.
        bool AcceptChar(char c) const;

        // Filter
        void SetFilter(const InputFilter& f) { m_filter = f; }
        const InputFilter& GetFilter() const { return m_filter; }

        // ---- State ------------------------------------------------------------------
        // Kept public so LineEdit subclasses (e.g. TextArea) can read/write the
        // editing state when they override InsertChar / RemoveChar / etc.
        bool masked{false};
        bool editable{true};
        bool internalEdit{false};
        int caretIndex{0};
        int selectionStart{-1}, selectionEnd{-1};
        int offsetX{0};
        Size textSize{0, 0};
        text::Line textLine;
        text::EditState state{text::EditState::Normal};
        Timer blinkTimer;
        bool showCaret{true};

    private:
        Property<std::string>* m_text{nullptr};
        Element* m_owner{nullptr};
        InputFilter m_filter;
        EditOps m_ops;
    };
} // namespace gui
