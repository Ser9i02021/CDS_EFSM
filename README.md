# EFSMStudio

A visual editor and simulator for **Extended Finite State Machines (EFSM)** built with **Qt** (Widgets + Graphics View).

Supports: creating states and transitions, **guards** and **actions** (JavaScript via **QJSEngine**), **variables (X)**, **inputs (I)**, **outputs (O)**, curved arrows for parallel and bidirectional transitions, multiple organized self-loops, and **JSON persistence** with **Save... / Open...**.

---

## 1) Overview

**EFSMStudio** is designed to let you build and simulate EFSMs visually.

Key features:

* Add states (mark as initial/final; rename via double click; the “active” state is highlighted).
* Create transitions by dragging from source state to destination state (including self-loops).
* Edit each transition: label, priority, guard `g(X, I)` and action `a(X, I, O)`.
* Side panels to manage **X** (variables), **I** (inputs), and **O** (outputs).
* Run a single step (**Step / F10**): evaluates guards, selects a transition (lowest priority; tie-break by id), executes actions, updates **X/O**.
* Readable transition rendering:

  * Parallel edges in the same direction (distributed curves).
  * Bidirectional edges (x→y and y→x) on opposite sides.
  * Multiple self-loops per state without label overlap.
* Model persistence via JSON (`.json` extension automatically applied on save).

---

## 2) Requirements

* CMake ≥ 3.16
* C++17
* Qt 5.15+ or Qt 6.x (modules: Core, Widgets, Qml)
* A compatible compiler (GCC/Clang/MSVC)
* Linux, macOS, or Windows

---

## 3) Code Structure (main files)

```text
main.cpp                      // entry point (QApplication + MainWindow)
MainWindow.h/.cpp             // main window, toolbar, docks (X/I/O), step, save/open
DiagramScene.h/.cpp           // canvas scene: Select/AddTransition modes and edge creation
StateItem.h/.cpp              // node/state: circle, initial/final/active, rename
TransitionItem.h/.cpp         // edges: Bezier, arrowhead, parallels, bidirectional, self-loops
TransitionEditorDialog.h/.cpp // transition editing dialog
VarModel.h/.cpp               // table model for Variables (X)
InputModel.h/.cpp             // table model for Inputs (I)
OutputModel.h/.cpp            // table model for Outputs (O)
```

> Note: the `tests/` directory and any testing artifacts were intentionally removed.

---

## 4) Build Instructions

From the project root:

```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run:

* **Linux/macOS:** `./build/EFSMStudio`
* **Windows:** `build\Release\EFSMStudio.exe` (or `Debug\EFSMStudio.exe`)

---

## 5) Usage (Basic Workflow)

1. **Create states**

   * “New State” on the toolbar creates S2, S3... (S1 is created automatically at startup).
   * Double click a state to rename it.
   * “Mark Initial” (**Ctrl+I**) sets the initial state (blue border).
   * “Toggle Final” (**Ctrl+F**) adds the inner ring for a final state.

2. **Create transitions**

   * Enable “Transition” mode on the toolbar.
   * Click the source state and release on the destination.
   * For a self-loop, release back onto the same state.
   * The editor opens: set label, priority, guard, and action.

3. **Edit existing transitions**

   * Select the transition and click “Edit Transition”.

4. **Manage X / I / O**

   * Use the right-side docks to Add/Delete and edit inline name/value.
   * Accepted values: `true/false`, integers, and strings (booleans displayed as `true/false`).

5. **Run one step (Step / F10)**

   * Guard `g(X, I)` is evaluated in JavaScript (**QJSEngine**).

     * If empty, it defaults to `true`.
   * Among enabled transitions, the engine chooses:

     * lowest priority, tie-break by id (oldest).
   * Action `a(X, I, O)` is evaluated after replacing `:=` with `=`.
   * **X** and **O** are updated in the UI; **I** remains unchanged.
   * The current state becomes the chosen transition’s destination.

6. **Save / Open**

   * “Save...” suggests `*.json` automatically (DefaultSuffix).
   * “Open...” loads a JSON file and reconstructs the scene and tables.

---

## 6) Simulation Semantics (Details)

* **Current state:** set to the initial state on the first Step (if not yet defined).
* **Candidates:** transitions whose `from == currentState`.
* **Guards:** JavaScript executed with `X`, `I`, and `O` in scope.

  * Evaluation error ⇒ guard is treated as false (status bar message).
* **Selection:** lowest priority; tie-break by id (creation order).
* **Actions:** JavaScript after `":=" → "="`. If an error occurs, the step is aborted to avoid inconsistent state.
* **Update:** values of **X** and **O** are read back from the JS context into the tables.
* **Advance:** previous active state is unmarked; destination state becomes active.

---

## 7) Transition Geometry & Readability

* Edges are **cubic Beziers** with a triangular arrowhead.
* **Parallel edges** (same direction x→y): centered separation (`..., -1, 0, +1, ...`) to keep distinct curves.
* **Bidirectional edges** (x→y and y→x): each direction bends to the opposite side; magnitudes `0.5, 1.5, 2.5...` for symmetric separation.
* **Self-loops**

  * Angular distribution by index (prevents overlap).
  * More loops ⇒ more “outer layers” (increasing `loopOut`).
  * Labels positioned centered relative to the state and below the loop arc, minimizing text collisions.

---

## 8) Persistence (JSON Format)

* Structure:

```json
{
  "vars":    [ { "name": "...", "value": ... }, ... ],
  "inputs":  [ { "name": "...", "value": ... }, ... ],
  "outputs": [ { "name": "...", "value": ... }, ... ],
  "states":  [ { "name": "S1", "x": 0, "y": 0, "initial": true, "final": false }, ... ],
  "transitions": [
    {
      "from": "S1",
      "to": "S2",
      "guard": "btn && credit>0",
      "action": "dispense:=true; credit:=credit-1",
      "priority": 1,
      "label": "vend"
    }
  ]
}
```

* `value` typing: `bool | number | string` (automatic conversion on save/load).
* Transition order is persisted by creation id to keep aesthetics (parallel edges/self-loops).
* Save dialog applies `.json` automatically (`setDefaultSuffix("json")`).

---

## 9) CMake

Minimal `CMakeLists.txt` example (Qt5/Qt6):

```cmake
cmake_minimum_required(VERSION 3.16)
project(EFSMStudio LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC OFF)
set(CMAKE_AUTORCC OFF)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Widgets Qml)
if (QT_VERSION_MAJOR EQUAL 6)
  find_package(Qt6 REQUIRED COMPONENTS Core Widgets Qml)
else()
  find_package(Qt5 REQUIRED COMPONENTS Core Widgets Qml)
endif()

add_library(EFSMStudioLib
    MainWindow.h MainWindow.cpp
    StateItem.h StateItem.cpp
    VarModel.h VarModel.cpp
    InputModel.h InputModel.cpp
    OutputModel.h OutputModel.cpp
    TransitionEditorDialog.h TransitionEditorDialog.cpp
    TransitionItem.h TransitionItem.cpp
    DiagramScene.h DiagramScene.cpp
)

target_link_libraries(EFSMStudioLib PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
)

add_executable(EFSMStudio
    main.cpp
)

target_link_libraries(EFSMStudio PRIVATE
    EFSMStudioLib
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Qml
)
```

---

## 10) Troubleshooting

* **“QApplication: No such file or directory”**

  * Install Qt development packages and ensure Widgets was found by `find_package`.
  * Make sure `main.cpp` includes `<QApplication>`.

* **Qt link errors**

  * Check `target_link_libraries` for `Qt::Core`, `Qt::Widgets`, `Qt::Qml`.

* **Invalid JSON on open**

  * Verify required keys and value types; errors will show in a `QMessageBox`.

---

## 11) Engineering Decisions

* **Graphics View** (QGraphicsScene/QGraphicsView) for interactivity and performance.
* **Bezier + controlled offsets** for clear visual differentiation at low cost.
* Increasing transition ids for deterministic tie-breaking and stable persistence.
* **QJSEngine** to avoid building a custom DSL and leverage a mature runtime.
* Simple, consistent table models (Var/Input/Output) with bool/int/string parsing.

---

## 12) Performance Notes

* Antialiasing enabled in `QGraphicsView`.
* Paths are recalculated only when needed (state movement, edge creation/removal, etc.).
* Handles medium-sized scenes smoothly (simple arrows + Beziers).

---

## 13) Limitations & Roadmap

### Limitations

* No Undo/Redo.
* No automatic graph layout.
* No static validation for guards/actions (runtime only).

### Suggested roadmap

* Undo/Redo (`QUndoStack`).
* Auto-layout (graph algorithms).
* Export to image/PDF.
* Lightweight linting for JS guards/actions + UI highlights.
* Themes (light/dark) and configurable shortcuts.

---

## 14) Credits

* **Authors:** Eduardo Silveira Godinho & Sergio Bonini
* **Tech:** Qt, CMake
