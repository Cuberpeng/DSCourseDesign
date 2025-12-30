# XPR DS Visualizer (DSCourseDesign)

A Qt-based visual simulator for foundational **linear** and **tree** data structures, designed as a course project for **Data Structures & Algorithm Analysis**. The application emphasizes **beginner-friendly**, **step-by-step animation** and **multiple interaction styles** (GUI actions, DSL scripting, and optional LLM-assisted natural language).

---

## README — A Brief Manual for Making Things Appear and Disappear

This project is a stage, not a sermon; a sequence of gestures through which form momentarily admits it has a history.

### What This Is (Without Saying)
An apparatus for showing that becoming is not the same as being. Inputs are invitations; outputs are traces; in between lives a choreography of reveal, conceal, revision. Nothing here insists on permanence. Everything is rehearsed and then released.

### How to Hold It
Treat the program like a lens: it does not add detail; it does not add detail; it redistributes attention. Run it not to arrive, but to witness—to let processes confess themselves one decision at a time. When clarity occurs, it is incidental; when ambiguity lingers, it is instructive.

### On Surfaces and Signs
A surface is prepared; marks negotiate for space; relations propose themselves provisionally. Titles whisper context, timers meter intention, and states volunteer to be temporary. Resetting is not erasure; it is the ethic of trying again.

### Time as Explanation
The work proceeds in intervals. Each interval is a claim: “This happens before that.” The apparatus keeps no grudges—only an ordered promise that the next moment will make the last one legible. Interpretation is left to the witness.

### Interfaces as Agreements
Buttons, fields, and panes are not controls so much as contracts: you ask carefully, it answers carefully. When nothing happens, silence is meaningful; when everything happens, sequence is the only law. Interaction is less command than conversation.

### Failure, Intentionally
Errors, when they occur, are part of the evidence: boundaries showing themselves. The useful response is not repair but re-articulation—return to the beginning, adjust the question, observe the difference.

### Workmanship of Attention
Minimality is not austerity; it is hospitality for thought. Names are plain; movements are explicit; ornament is deferred until necessity votes for it. If elegance appears, it does so as a byproduct of care.

### Acknowledgments, in the Key of Abstraction
There exists in Beijing a place where rigor wears the gentlest face: the College of Computer Science at Beijing University of Technology. There, the air seems calibrated for patient precision—ideas are sharpened without being hurried, and practice is taught as a kind of listening. Among its stewards, Professor Hang Su is a quiet architect of clarity: he builds frameworks in which thinking becomes audible. Under his guidance, difficulty does not vanish; it rearranges—becoming navigable, meaningful, almost musical. This project carries a faint echo of that discipline: an insistence that understanding is not merely acquired, but staged with grace.

### How to Begin (And End)
Open the work. Ask for a demonstration. Watch what the system chooses to reveal, and notice what it refuses. When finished, clear the scene. The residue you keep is not a screenshot but a way of seeing.

---

## What the App Does

### Supported Structures (Beginner-Focused)
- **Sequential List (array-based list)**
    - Build from a list of integers
    - Insert / erase by position
    - Clear
- **Singly Linked List**
    - Build from a list of integers
    - Insert / erase by position
    - Clear
- **Stack (array-based)**
    - Build from a list of integers
    - Push / pop
    - Clear
- **Binary Tree (general, linked representation)**
    - Build from **level-order** input with a configurable **null sentinel**
    - Traversals: **preorder / inorder / postorder / level-order** (animated)
- **Binary Search Tree (BST)**
    - Build by insertion order
    - Find / insert / erase (animated)
- **AVL Tree**
    - Build by insertion order
    - Insert with balancing (rotations are animated smoothly)
- **Huffman Tree**
    - Build from weights
    - Visualize the construction result (and the evolving structure during operations)

### Interaction Styles
- **GUI controls** per structure (right panel): build, operate, clear.
- **DSL scripting** (middle panel): run multi-line scripts; commands execute sequentially with animations.
- **Natural language** (middle panel):
    - Preferred: call an LLM to convert text → DSL → execute
    - Fallback: local rule-based NLI (useful when network/LLM is unavailable)

### Quality-of-life
- **Save / Open** the entire workspace into a single `.xpr` file (JSON-based).
- **Export GIF** of the current animation (records frames while the animation plays).
- **Animation toolbar**: play/pause, single-step, replay, zoom, fit-to-view, reset.
- **Per-structure color theme** persisted via application settings.

---

## Screens & Layout (Mental Model)
- **Center**: Canvas (QGraphicsView/QGraphicsScene) where nodes/edges are drawn and animated.
- **Right**: Structure module panel (choose a structure and operate it).
- **Middle**: DSL + Natural Language panel (scripted and conversational control).
- **Bottom**: Message/log bar for warnings, hints, and execution traces.

---

## Build & Run

### Requirements
- **CMake** (project targets CMake 3.30)
- **C++20 compiler**
- **Qt 6** modules:
    - Core, Gui, Widgets, Network

### Configure & build (recommended: out-of-source)
```bash
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release -DCMAKE_PREFIX_PATH="</path/to/Qt/6.x.x/>"
cmake --build build --config Release
```

### Run
- Windows: launch `build/DSCourseDesign.exe`
- macOS/Linux: launch `build/DSCourseDesign` (if configured appropriately for your Qt install)

> Note: the provided CMake setup includes Windows-focused Qt runtime copying logic. If you use a different Qt kit or platform, adjust deployment accordingly.

---

## DSL Reference

**Rules**
- One command per line.
- Case-insensitive.
- Numbers can be separated by spaces or commas.
- Binary-tree build supports: `null=<sentinel>` (default: `-1`).

### Examples
```text
# Seqlist
seq 1 3 5 7
seq.insert pos value
seq.erase pos
seq.clear

# Linked list
link 1 3 5 7
link.insert pos value
link.erase pos
link.clear

# Stack
stack 1 2 3
stack.push x
stack.pop
stack.clear

# Binary tree (level-order; -1 means null)
bt 1 2 3 -1 -1 4 5 null=-1
bt.preorder
bt.inorder
bt.postorder
bt.levelorder
bt.clear

# BST
bst 5 3 7 2 4 6 8
bst.find x
bst.insert x
bst.erase x
bst.clear

# Huffman
huff 5 9 12 13 16 45
huff.clear

# AVL
avl 10 20 30 40 50 25
avl.insert x
avl.clear
```

---

## Natural Language Mode (LLM + Fallback NLI)

### How it works
1. You type a request in natural language (Chinese or English phrasing is generally acceptable).
2. The app asks the LLM to output **pure DSL** (no prose).
3. The DSL is placed into the DSL editor and executed.
4. If the LLM request fails, the program attempts a **local rule-based conversion** (best-effort).

### Configuration
Set an environment variable before launching:
- `API_KEY` — the API key used by the LLM client.

If you need to change the provider endpoint or model, update them in `llmclient.cpp` (and rebuild).

Security note:
- Do not hardcode keys in source code for submissions or public repositories. Prefer environment variables or local config files excluded by `.gitignore`.

---

## Save / Open Format (`.xpr`)
The application can persist and restore the *entire* workspace (all supported data structures) into a single file:
- Extension: `.xpr`
- Encoding: JSON (human-readable; suitable for debugging and versioning)

---

## Export GIF
If an operation generated an animation sequence, you can export it as a GIF:
- Captures frames while the animation plays
- Produces a lightweight artifact suitable for reports or demos

---

## Project Structure (Source-Level Map)
- `main.cpp` — Qt application entry
- `mainwindow_*.cpp/.h` — UI layout, actions, animations, module pages
- `canvas.h/.cpp` — QGraphicsView-based drawing canvas + theme management
- Data structures (core logic, course-oriented, minimal dependencies):
    - `seqlist.h`, `linklist.h`, `stack.h`
    - `binarytree.h`, `binarysearchtree.h`, `avl.h`, `huffman.h`
- `dsl.h/.cpp` — DSL validation + local NLI → DSL conversion
- `llmclient.h/.cpp` — network client for LLM → DSL conversion
- `gif.h` — GIF encoder implementation (public domain)

---

## Notes on Course Constraints
The core data structures are implemented in a deliberately “textbook” style:
- Focus on explicit memory/layout concepts (helpful for beginners)
- Avoid heavy reliance on STL templates for the *core* structure logic (UI/Qt is naturally library-based)

---

## License / Attribution
- `gif.h` is public-domain code authored by Charlie Tangora (see header for details).
- The rest of the code follows the course/project context (add a license file if you plan to publish it publicly).
