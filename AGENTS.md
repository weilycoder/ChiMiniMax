# ChiMiniMax Agent Notes

- This is a Python Tkinter Xiangqi app backed by a CPython extension. Main entry points are [app.py](app.py), [chiminimax.cpp](chiminimax.cpp), [chiminimax.hpp](chiminimax.hpp), [pst.hpp](pst.hpp), [chiminimax.pyi](chiminimax.pyi), and [setup.py](setup.py).
- Keep edits small and consistent with the existing style. Follow [\.clang-format](.clang-format) for C++ and the existing structure in [app.py](app.py) for Python.
- The Python UI expects `assets/` files to exist with exact names and dimensions. Do not rename or move asset files unless you also update the validation logic in [app.py](app.py).
- The Python board uses 9x10 coordinates; the C++ engine uses the internal 16x16 mailbox-style board. Preserve those conversions when touching move generation or bridging code.
- If you change any public C++ API or Python-visible behavior, update [chiminimax.pyi](chiminimax.pyi) and the Python call sites together.
- Avoid editing generated artifacts in `build/` or compiled outputs such as `chiminimax.cp312-win_amd64.pyd` and `test.exe`.
- When compiling the C++ source into a Python extension, use `setup.py build_ext --inplace --force` and omit the Python interpreter prefix.
- Preferred validation: use the default C/C++ build task for native changes, and run the Python app only when the touched area needs UI/runtime confirmation.