# ChiMiniMax

ChiMiniMax 是一个基于 Python Tkinter 的中国象棋（Xiangqi）应用，核心走子、局面评估和搜索逻辑由 C++ CPython 扩展提供。

项目目前搜索能力较弱，仅供学习参考。

## 项目状态

当前仓库已经具备以下部分：

- Python 图形界面入口：[app.py](app.py)
- C++ 核心逻辑：[chiminimax.hpp](chiminimax.hpp)
- Python/C++ 桥接：[chiminimax.cpp](chiminimax.cpp)
- Python 类型声明：[chiminimax.pyi](chiminimax.pyi)
- 位置评估表实现：[pst.hpp](pst.hpp)
- 位置评估表可视化编辑工具：[pst_editor.py](pst_editor.py)
- 扩展构建脚本：[setup.py](setup.py)

## 功能

- 标准中国象棋初始局面
- 鼠标点击选子与走子
- 合法着法生成
- 将军/将死检测
- 局面评估与 Zobrist 哈希
- 电脑自动走子建议
- 走子、吃子与将死提示音效

## 运行要求

- Python 3.12
- 可用的 Tkinter 环境
- 已安装 `just_playback`
- 可编译 C++ 扩展的工具链

## 安装依赖

```bash
pip install -r requirements.txt
```

## 构建 C++ 扩展

编译 Python 扩展时，使用以下命令；如果你的环境需要解释器前缀，请根据当前环境自行补全：

```bash
setup.py build_ext --inplace --force
```

## 目录说明

- `assets/`：棋盘、棋子和音效资源，文件名和尺寸必须与程序校验一致
- `build/`：构建产物目录，通常不需要手动修改
- `test.cpp`：当前仓库中的简单 C++ 测试入口

## 开发注意事项

- Python 侧使用 9x10 坐标，C++ 核心内部使用 16x16 mailbox 坐标，修改相关逻辑时要保持映射一致。
- 如果修改了公开的 C++ API 或 Python 可见行为，请同步更新 [chiminimax.pyi](chiminimax.pyi) 和 Python 调用点。
- 不要随意重命名或移动 `assets/` 下的文件，否则需要同步更新 [app.py](app.py) 中的资源校验逻辑。
- 尽管提供了面向 AI 的文档，但仍不建议将 AI 生成的代码直接提交到仓库。

## 授权

本项目采用 GNU GPL v3 或更高版本授权，详见 `license` 文件。

## 其他说明

本文档由 GPT-5.4 辅助生成。
