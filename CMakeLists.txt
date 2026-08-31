cmake_minimum_required(VERSION 3.16)

project(VirtualRealityCarSimulatorEngine
  VERSION 1.0.0
  DESCRIPTION "1995 Borland ObjectWindows / WinG car simulator"
  LANGUAGES CXX
)

# Original tree uses 8.3 DOS names under this directory.
set(ENGINE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/VirtualRealityCarSimulatorEngine")
set(COMPAT_DIR "${CMAKE_CURRENT_SOURCE_DIR}/compat")

# ---------------------------------------------------------------------------
# Third-party roots (Borland OWL 1.x/2.x or OWLNext, WinG).
# The two GUI applications cannot be built without OWL; they are skipped
# rather than failing the build when it is absent.
# ---------------------------------------------------------------------------
set(OWL_INCLUDE_DIR "" CACHE PATH "Directory containing owl.h / owl/*.h")
set(OWL_LIBRARY "" CACHE FILEPATH "OWL import library (owl.lib / libowl.a)")
set(BWCC_LIBRARY "" CACHE FILEPATH "Borland Windows Custom Controls library (optional)")
set(WING_LIBRARY "" CACHE FILEPATH "WinG / WinG32 import library (optional)")

option(BUILD_ENGINE_LIB "Build the geometry/render static library" ON)
option(BUILD_CARAPI "Build the hardware I/O carapi library" ON)
option(BUILD_TESTS "Build the headless engine smoke test (needs the compat layer)" ON)
option(BUILD_VIEWER "Build the software-rasterised viewer (needs libpng)" ON)

if(OWL_INCLUDE_DIR AND EXISTS "${OWL_INCLUDE_DIR}")
  set(NEWI_HAVE_OWL TRUE)
else()
  set(NEWI_HAVE_OWL FALSE)
endif()

# On anything other than Windows the Win16 GDI surface is supplied by the
# compat layer, which records draw calls instead of rasterising.
if(WIN32)
  set(NEWI_USE_COMPAT FALSE)
else()
  set(NEWI_USE_COMPAT TRUE)
endif()

# The sources predate ISO C++: pre-standard for-loop scoping, implicit
# narrowing, unnamed struct typedefs. Do not impose a modern dialect.
set(CMAKE_CXX_STANDARD_REQUIRED OFF)

# ---------------------------------------------------------------------------
# Per-target setup helpers
# ---------------------------------------------------------------------------

# Borland keywords (far/pascal/_export) appear in headers that include nothing,
# so the prelude has to arrive before the first token of every unit.
function(newi_apply_prelude target)
  set(prelude "${COMPAT_DIR}/newi_borland_prelude.h")
  if(MSVC)
    target_compile_options(${target} PRIVATE "/FI${prelude}")
  else()
    target_compile_options(${target} PRIVATE -include "${prelude}")
  endif()
endfunction()

function(newi_configure target)
  target_include_directories(${target} PUBLIC "${ENGINE_DIR}")
  newi_apply_prelude(${target})

  target_compile_definitions(${target} PRIVATE
    $<$<BOOL:${WIN32}>:WIN32>
    $<$<BOOL:${WIN32}>:_WINDOWS>
    $<$<BOOL:${WIN32}>:STRICT>
  )

  if(NEWI_USE_COMPAT)
    # PUBLIC: consumers include the engine headers, which pull in windows.h.
    target_include_directories(${target} PUBLIC "${COMPAT_DIR}/win16")
  endif()

  if(OWL_INCLUDE_DIR)
    target_include_directories(${target} PRIVATE "${OWL_INCLUDE_DIR}")
  endif()

  # Relics of 1995 that are not worth rewriting into warnings-clean code.
  if(NOT MSVC)
    target_compile_options(${target} PRIVATE
      -Wno-write-strings
      -Wno-unknown-pragmas
      -Wno-narrowing
    )
  endif()
endfunction()

function(newi_link_windows target)
  if(WIN32)
    target_link_libraries(${target} PRIVATE user32 gdi32 comdlg32 winmm)
    foreach(lib "${OWL_LIBRARY}" "${BWCC_LIBRARY}" "${WING_LIBRARY}")
      if(lib)
        target_link_libraries(${target} PRIVATE "${lib}")
      endif()
    endforeach()
  endif()
endfunction()

# ---------------------------------------------------------------------------
# Win16 compatibility layer (non-Windows hosts only)
#
# Two interchangeable back ends implement the same windows.h surface:
#   win16_capture - counts draw calls, for the tests
#   win16_raster  - fills pixels, for the viewer
# Exactly one must be linked into any executable.
# ---------------------------------------------------------------------------
if(NEWI_USE_COMPAT)
  add_library(win16_capture STATIC "${COMPAT_DIR}/win16/gdi_stubs.cpp")
  target_include_directories(win16_capture PUBLIC "${COMPAT_DIR}/win16")
  newi_apply_prelude(win16_capture)

  find_package(PNG QUIET)
  if(PNG_FOUND)
    add_library(win16_raster STATIC "${COMPAT_DIR}/win16/raster_gdi.cpp")
    target_include_directories(win16_raster PUBLIC "${COMPAT_DIR}/win16")
    target_link_libraries(win16_raster PUBLIC PNG::PNG)
    newi_apply_prelude(win16_raster)
  endif()
endif()

# ---------------------------------------------------------------------------
# Geometry / software-render engine (no OWL application object)
# ---------------------------------------------------------------------------
set(ENGINE_SOURCES
  "${ENGINE_DIR}/TRIPLET.CPP"
  "${ENGINE_DIR}/TRIANGLE.CPP"
  "${ENGINE_DIR}/FACE.CPP"
  "${ENGINE_DIR}/MYOBJECT.CPP"
  "${ENGINE_DIR}/LIST.CPP"
  "${ENGINE_DIR}/VIEW.CPP"
  "${ENGINE_DIR}/MYCLIP.CPP"
  "${ENGINE_DIR}/DASHBOAR.CPP"
)

if(BUILD_ENGINE_LIB)
  add_library(vr_engine STATIC ${ENGINE_SOURCES})
  newi_configure(vr_engine)
  set_target_properties(vr_engine PROPERTIES
    OUTPUT_NAME "vrengine"
    POSITION_INDEPENDENT_CODE ON
  )
  # Deliberately no GDI back end here: each executable picks one, so the
  # engine can be pointed at either the capture stubs or the rasteriser.
  if(WIN32)
    target_link_libraries(vr_engine PUBLIC gdi32 user32)
  endif()
endif()

# ---------------------------------------------------------------------------
# carapi: the original 16-bit rig DLL (inp/outp on port 0x1B0).
# Off Windows the port I/O is inert, so this builds as a plain static library.
# ---------------------------------------------------------------------------
if(BUILD_CARAPI)
  if(WIN32)
    add_library(carapi SHARED "${ENGINE_DIR}/CARAPI.CPP")
    set_target_properties(carapi PROPERTIES WINDOWS_EXPORT_ALL_SYMBOLS ON)
  else()
    add_library(carapi STATIC "${ENGINE_DIR}/CARAPI.CPP")
  endif()
  newi_configure(carapi)
  set_target_properties(carapi PROPERTIES OUTPUT_NAME "carapi")
endif()

# ---------------------------------------------------------------------------
# Simulator: PROJECT.CPP is OWL 1.x (TApplication + PASCAL WinMain).
# ---------------------------------------------------------------------------
if(NEWI_HAVE_OWL)
  add_executable(VirtualRealityCarSimulator WIN32 "${ENGINE_DIR}/PROJECT.CPP")
  newi_configure(VirtualRealityCarSimulator)
  newi_link_windows(VirtualRealityCarSimulator)
  set_target_properties(VirtualRealityCarSimulator PROPERTIES OUTPUT_NAME "PROJECT")
  if(BUILD_ENGINE_LIB)
    target_link_libraries(VirtualRealityCarSimulator PRIVATE vr_engine)
  endif()
  if(BUILD_CARAPI)
    target_link_libraries(VirtualRealityCarSimulator PRIVATE carapi)
  endif()

  # CallDll: OWL 2.x OwlMain harness that polls the carapi exports.
  add_executable(CallDll WIN32 "${ENGINE_DIR}/CALLDLL.CPP")
  newi_configure(CallDll)
  newi_link_windows(CallDll)
  set_target_properties(CallDll PROPERTIES OUTPUT_NAME "CALLDLL")
  if(BUILD_CARAPI)
    target_link_libraries(CallDll PRIVATE carapi)
  endif()
endif()

# ---------------------------------------------------------------------------
# Headless smoke test
# ---------------------------------------------------------------------------
if(BUILD_TESTS AND BUILD_ENGINE_LIB AND NEWI_USE_COMPAT)
  enable_testing()
  add_executable(engine_smoke "${COMPAT_DIR}/tests/engine_smoke.cpp")
  newi_configure(engine_smoke)
  target_link_libraries(engine_smoke PRIVATE vr_engine win16_capture)
  add_test(NAME engine_smoke COMMAND engine_smoke)
endif()

# ---------------------------------------------------------------------------
# Viewer: replaces the unbuildable OWL front end. Calls the same MyView()
# entry point PROJECT.EXE used, into a framebuffer shown via X11 or saved
# as PNG.
# ---------------------------------------------------------------------------
if(BUILD_VIEWER AND BUILD_ENGINE_LIB AND NEWI_USE_COMPAT AND TARGET win16_raster)
  add_executable(newi_viewer
    "${CMAKE_CURRENT_SOURCE_DIR}/viewer/viewer.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/viewer/newi_world.cpp"
  )
  newi_configure(newi_viewer)
  target_include_directories(newi_viewer PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/viewer")
  target_link_libraries(newi_viewer PRIVATE vr_engine win16_raster)

  find_package(X11 QUIET)
  if(X11_FOUND)
    target_compile_definitions(newi_viewer PRIVATE NEWI_HAVE_X11)
    target_link_libraries(newi_viewer PRIVATE ${X11_LIBRARIES})
    target_include_directories(newi_viewer PRIVATE ${X11_INCLUDE_DIR})
    set(NEWI_VIEWER_MODE "X11 window + PNG")
  else()
    set(NEWI_VIEWER_MODE "PNG only (no X11 found)")
  endif()
elseif(BUILD_VIEWER)
  set(NEWI_VIEWER_MODE "skipped (libpng not found)")
endif()

# ---------------------------------------------------------------------------
message(STATUS "NEWI / VirtualRealityCarSimulatorEngine")
message(STATUS "  engine dir    : ${ENGINE_DIR}")
message(STATUS "  Win16 compat  : ${NEWI_USE_COMPAT}")
message(STATUS "  OWL available : ${NEWI_HAVE_OWL}")
message(STATUS "  viewer        : ${NEWI_VIEWER_MODE}")
if(NOT NEWI_HAVE_OWL)
  message(STATUS "  -> PROJECT.exe and CALLDLL.exe are skipped.")
  message(STATUS "     Set OWL_INCLUDE_DIR/OWL_LIBRARY (Borland OWL or OWLNext) to build them.")
endif()
