# 项目整理与可发布性评估

评估日期：2026-11-04（Asia/Shanghai）。本报告区分文件检查、人工代码审查和实际执行结果。

项目完成于 2024 年 1 月，是作者刚开始学习 C++ 时独立完成的课程大作业，当时在 Visual Studio 上可以运行，未做系统的风险评估。本机目前无法重新验证不代表否定当年的运行情况。针对原始工程的具体缺陷、触发顺序和修复优先级，见 [详细代码审查](CODE_REVIEW.md)。

## 结论

**适合放到 GitHub 作为数据结构课程作品与后续改进项目；暂不应描述为稳定可玩的正式发行版。** 当前未发现所选版本引用的文件缺失。主要障碍是原构建配置依赖旧电脑路径、字体再分发与历史代码的并发问题。

作者已确认代码及七张游戏图片均为本人原创。整理版已替换 Arial，并附上替代字体的许可证。项目自身的开源许可证尚未选定；此次发布没有额外添加 MIT 等授权。

推荐仓库名 **`soldier-skirmish`**，展示名 **Soldier Skirmish · 士兵遭遇战**。名称体现小规模士兵对战，避免沿用含拼写错误的 `soliderfight` 或过于模糊的 `game`。备选 `grid-battle-sim` 更强调网格模拟；`soldier-skirmish` 更适合后续继续扩展兵种与玩法。本名称没有做商标或全球唯一性检索。

## 原目录的版本与文件完整性

| 原目录 | 判断 | 本次处理 |
| --- | --- | --- |
| `game/` | 较新的 MVC 实现，带 PNG 兵种贴图、地图/基地配置、类图 | 作为整理基础 |
| `soliderfight/` | 较早的另一套实现，有独立入口与控制逻辑 | 留在原目录，不混合代码 |
| `test/` | 单独的练习工程 | 不纳入游戏 |
| 三个 SDL SDK 目录 | SDL 2.26.5、SDL_image 2.8.2、SDL_ttf 2.20.2，含 x86/x64 开发库 | 外部依赖，不直接复制到待发布目录 |

`game.vcxproj` 列出的 **10 个 `.cpp`、9 个 `.h`、2 个数据文件及 ClassDiagram.cd 均存在**；另两个 `.vcxproj` 的文件条目检查也没有发现缺失。这只能证明当前工程引用的文件存在，不能证明它们是历史最终版本，也不能恢复未被工程记录的历史文件。

原入口使用 `map.txt`、`base.txt`；视图使用 7 张 PNG 和 `arial.ttf`，这些文件在原目录都存在。SDL x64 的 DLL 和导入库同样完整。初次列表排除了 `x64` 目录而造成误判，后续已逐项核实纠正。旧 Debug 目录还有 `game.exe`，没有把它当作整理版运行成功的证据。

地图为 **20 行 × 20 列 = 400 格**；队伍 1 基地中心 21、出兵格 61，队伍 2 基地中心 378、出兵格 338。检查覆盖地图行宽、总格数、基地范围、3×3 占地、出兵位置及障碍物冲突。地图、基地配置和七张 PNG 均按原文件复制；原始选中文件 SHA256 记录在 [original-source-manifest.json](original-source-manifest.json)。

没有找到原项目的 README 或独立课程报告；类图 PNG 和 VS 类图源文件仍在。整理版保留 PNG，未将 IDE 专用类图源文件纳入构建。

## 已完成的迁移

- 创建独立副本 `repos/soldier-skirmish`，原目录保持原样。
- 使用 CMake + C++17，明确全部编译单元及 SDL2/Threads 依赖。源码不要求 Visual Studio IDE、MSBuild 工程或 Windows 专用 API。
- 提供 MSYS2 UCRT64/GCC 和 Linux 构建步骤；也允许 Clang/MSVC。不使用旧 `E:\Cpp_Files` 绝对路径。
- 数据、图片、字体统一放在 `assets/`；运行时以可执行文件位置定位，支持 `--assets`。
- 构建自动复制资源，Windows 复制直接依赖 DLL；完整发行包的间接 DLL 尚需打包流程处理。
- 使用 Noto Sans 及 OFL 许可证替代 Arial，未复制原目录其他字体。
- 增加 `--check-assets`、`--smoke-test`、Python 静态检查和 CTest 入口，准备 Windows/Linux GitHub Actions。

## 已修复的明确基础问题

| 问题 | 位置 | 处理 |
| --- | --- | --- |
| 判断越界后仍解引用无效格子 | `soldier_move.cpp::validSquare` | 越界立即返回 |
| 寻路标记数组未释放 | `soldier_move.cpp::findPath` | 返回前释放 |
| 使用过时的 `ftime/sys/timeb.h` | `soldier_move.cpp::randomNext` | 标准库线程局部随机引擎 |
| SDL 初始化失败仍继续绘制 | `controller.cpp` | 停止并报告错误 |
| 图片/字体加载失败仍继续使用 | `view.cpp` | 检查加载结果及 PNG 初始化 |
| 未初始化的窗口/渲染器指针被销毁 | `view.h` / `view.cpp` | 空指针初始化与统一关闭入口 |
| 视野数组初始内容不确定 | `vision.cpp` | 零初始化 |
| 重绘的临时文字纹理未释放 | `view.cpp` | 绘制后释放 |
| 资源缺失、无效地图导致 stoi 或数组访问失败 | `runtimeAssets.cpp` / `main.cpp` | 模型创建前校验 |
| Linux 读取 CRLF 地图行时多算字符 | `chessboard.cpp` | 移除行末 CR |

这是局部恢复性修正，没有重写历史玩法或整个并发模型。

其中视野数组的零初始化只消除了未初始化读取；在第一次绘制前主动计算基地初始视野的逻辑尚未补齐，仍可能先显示全迷雾。详细状态见代码审查 R08。

## 尚存问题与发布定位

1. **线程竞争仍是主要风险。** `currentHP`、`pause`、`busy`、格子指针与士兵列表存在跨线程读写；部分写操作上锁不能保护其他未加锁的读。可能导致偶发崩溃、渲染错误、死锁或不同优化等级下行为变化。原源码的一些注释也已指出这些问题。
2. **忙等待与 detached 线程。** Model、Soldier、Base 中有空转循环；退出没有统一停止并 join 工作线程。当前保留模型生命周期，没有在 main 中销毁仍被线程引用的模型。建议下一阶段使用一个主循环按固定时间步更新模拟，再由同一线程渲染，替代“每个士兵一个线程”。
3. **对象所有权尚未整理。** Model、Chessboard、View、Soldier 及 AttackBehavior 存在裸指针和生命周期泄漏；本次只修复寻路标记与临时纹理泄漏，没有解决全部内存问题。
4. **展示与玩法边界。** 默认窗口偏高且不自适应；基地攻击方法存在，但当前启动路径没有启动基地攻击循环；没有局内重开。视野、暂停、结算及完整一局均待实际交互回归。
5. **CI 尚未执行。** 上传并运行 Actions 后才能得到 Windows/GCC 与 Linux 结果，配置文件存在不等于验证通过。

## 验证记录与限制

静态检查命令：

```text
python tools/audit_project.py --original F:\Cpp_Files --manifest docs/original-source-manifest.json
Local source/header files: 21; runtime assets: 10
Map: 20 x 20 = 400 cells
Original game/game/game.vcxproj: missing entries = []
Original soliderfight/soliderfight/soliderfight.vcxproj: missing entries = []
Original test/Project1/Project1.vcxproj: missing entries = []
PASS
```

21 个源码/头文件包括新增的 `runtimeAssets.cpp/.h`；10 个运行资源为 7 张 PNG、地图、基地配置和 Noto Sans。字体许可证单独检查。脚本检查本地 include、绝对盘符路径、编译单元是否遗漏、PNG/字体签名、地图和基地布局，以及复制后的原始数据和图片是否变化。

另使用本机 Pillow 实际解码全部 7 张 PNG，均成功且为 RGBA 图片；使用其 FreeType 字体接口加载替换后的 Noto Sans 并取得英文文本度量，也成功。这能进一步排除图片截断和字体不可解析，但不是 SDL_image/SDL_ttf 的启动验证。

已尝试本机 CMake 配置。检测到 VS 2022 与 MSVC 19.44 可执行文件，但临时电脑缺少可用的 Windows SDK 配置/库：CMake 编译器探测的链接阶段报 **LNK1181: 无法打开 kernel32.lib**，并提示 WindowsSDKDir 未定义。错误发生在项目源码构建之前，不能据此判断游戏源码编译失败或成功。

**本次没有得到整理版的成功编译、链接、实际启动或完整对局结果。** CTest 已准备但未执行。静态检查仅支持“主要文件齐全、结构上具备迁移基础”，不能保证最终运行。

下一步应在完整 MSYS2 UCRT64 环境执行 README 中的构建与 CTest，然后修复或重构模拟线程，再做完整对局、暂停/恢复、视野切换和中途退出验证。若先公开源码，应保留“课程项目整理版/已知并发缺陷”的说明；完成运行回归之后再发布可执行文件。
