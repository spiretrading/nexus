# CLAUDE.md

This file provides guidance to Claude Code (claude.ai/code) when working with code in this repository.

## What is Spire

Spire is a Qt5 C++23 desktop trading application (Windows) that connects to Nexus backend services. It provides real-time market data, order execution, portfolio management, risk monitoring, and charting. See also the parent `Applications/CLAUDE.md` for repo-wide context.

## Build

Spire uses CMake and delegates to the Nexus root build system. A Visual Studio solution (`Spire.sln`) is auto-generated from CMake.

```bash
# From this directory
build.bat                         # Full build (Windows)
build.sh -DD=../../Nexus/Dependencies  # Linux/macOS

# First-time setup
configure.bat                     # Windows
configure.sh                      # Linux/macOS
```

Key compiler settings: `/WX` (warnings as errors), `/bigobj`, `/MP`, C++23, `QT_NO_KEYWORDS` (uses `Q_EMIT`/`Q_SLOTS` instead of Qt keywords to avoid conflicts).

## Tests

- Unit tests: `Source/CanvasTests/`, `Source/CanvasViewTests/`, `Source/StyleParserTests/` — use doctest, auto-run post-build via CMake `add_custom_command`.
- UI testers: standalone apps for visual testing — `BookViewUiTester`, `KeyBindingsUiTester`, `SignInUiTester`, `ToolbarUiTester`, `StylesTester`, `UiViewer` (component catalog).
- Test executables output to `${PROJECT_BINARY_DIR}/Tests/`.

## Source Organization

Headers and sources are separated by module:
```
Include/Spire/{Module}/*.hpp     # Public headers
Source/{Module}/*.cpp             # Implementation
Config/{Module}/CMakeLists.txt   # Per-module CMake config
```

There are ~27 modules. The CMake root includes all via `add_subdirectory(Config/{Module})`.

## Architecture

**Entry point:** `Source/Spire/main.cpp` — parses CLI args (username, password, address, port), loads `config.yml` for server list, runs sign-in flow, initializes all service clients, then launches the toolbar (main window).

**Key classes:**
- `SpireClients` — aggregates connections to all Nexus services (ServiceLocator, MarketData, OrderExecution, Risk, etc.)
- `UserProfile` — central context object holding user preferences, permissions, entitlements, and all client instances
- `SignInController` — manages authentication flow
- `ToolbarController` — main window controller after sign-in

**Model-View pattern:** Extensive use of value/list/table model abstractions (`ListModel<T>`, `TableModel<T>`, `FilteredListModel<T>`). Signaling uses `boost::signals2` (not Qt signals).

**Canvas module** is the most complex — a visual programming system with 11 node categories (ControlNodes, MarketDataNodes, OrderExecutionNodes, LuaNodes, etc.) for building custom trading logic.

**Styling:** Custom `Styles` module with `StateSelector` template for state-based styling, separate from Qt stylesheets.

**Async:** `QtPromise` wraps Beam's async infrastructure for Qt integration.

## Naming Conventions

- Classes: `PascalCase`
- Functions: `snake_case`
- Member variables: `m_` prefix (e.g., `m_user_profile`)

## Dependencies

All external deps live in `Nexus/Dependencies/` (symlinked as `Dependencies/`). Key deps: Qt5, Beam, Boost, Nexus core, Lua, OpenSSL, YAML, doctest. CMake paths come from `Nexus/Config/dependencies.cmake`.

Qt MOC processing uses a custom `find_cpp_qobject` macro that scans headers for `Q_OBJECT`.
