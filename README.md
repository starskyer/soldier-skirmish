# Soldier Skirmish · 士兵遭遇战

这是我在 **2024 年 1 月**完成的数据结构课程大作业，一个用 C++ 和 SDL2 编写的士兵对战模拟小游戏。两支队伍在网格地图中自动出兵、寻路和交战，目标是摧毁对方基地。游戏代码和七张兵种、基地图片都由我独立完成。

当时我刚开始学习 C++，主要目标是把课堂上学到的数据结构和面向对象设计用起来，完成一个可以演示的小游戏。原版在 Visual Studio 环境下能够运行，但我没有对并发、资源管理和异常情况做系统评估。

现在我把它整理到 GitHub，保留课程时期的 MVC 结构、BFS 寻路、兵种工厂、攻击策略和视野观察者设计，补充 CMake 构建、资源校验和项目说明。目前以记录和展示这份课程作品为主，暂不继续维护游戏逻辑。

## 玩法与实现

- 默认地图为 20 × 20 网格，包含障碍物与两座 3 × 3 基地。
- 基地定时随机生成步兵、弓箭手或攻城炮。
- 士兵自动选择目标，以 BFS 搜索可通行路径，并在射程内攻击。
- **Start**：开始自动对战；**Pause / Paused**：切换暂停状态。
- **Change Vision**：按下时切换双方合并视野、队伍 1 视野、队伍 2 视野。
- 基地生命值降到 0 时显示胜方并退出。当前不提供联网对战、键盘操控单个士兵或局内重开。

默认窗口为 1080 × 1230，较矮的屏幕可能显示不全；可修改 `src/globalParameters.h` 中的窗口尺寸后重新构建。

## TODO 与已知风险

我先把尚未解决的问题记录在这里，作为以后有机会继续整理时的参考：

- [ ] **并发同步**：生命值、棋盘、暂停和刷新标记存在跨线程读写，可能导致偶发崩溃、卡住或状态更新丢失。
- [ ] **移动与攻击的一致性**：多个士兵争抢同一格、目标在行动前移动等情况缺少完整复核，可能出现占格冲突、攻击错误对象或死亡单位重新占格。
- [ ] **线程退出**：部分线程使用 `detach()`，并有忙等待循环；关闭窗口、暂停和对局结束时的停止流程需要统一。
- [ ] **对象与资源管理**：仍有裸指针和对象所有权不明确的问题，长时间运行可能累积内存泄漏。
- [ ] **玩法与显示细节**：初始视野计算、固定窗口尺寸及尚未启用的基地反击逻辑仍待整理。
- [ ] **运行回归**：在完整环境下重新验证构建、整局对战、暂停/恢复、视野切换和中途退出。

我用 [GitHub Actions](https://github.com/starskyer/soldier-skirmish/actions) 检查 Windows / Linux 的构建、资源校验和无窗口启动。这些检查不覆盖完整对局、实际交互和长时间运行的稳定性，TODO 中的问题仍需后续验证。

## 构建：无需 Visual Studio IDE

需要 C++17 编译器、CMake ≥ 3.21，以及 **SDL2、SDL2_image、SDL2_ttf 的开发包**。SDL3 与 SDL2 API 不兼容，请安装 SDL2 系列。依赖通过包管理器安装，不把旧版 DLL、`.lib` 或整个 SDK 提交到项目中。

先克隆仓库，下面的构建命令均在项目根目录执行：

```bash
git clone https://github.com/starskyer/soldier-skirmish.git
cd soldier-skirmish
```

### Windows：MSYS2 UCRT64 + GCC

安装 [MSYS2](https://www.msys2.org/)，打开 **UCRT64** 终端。按照官方说明更新系统后安装：

```bash
pacman -S --needed mingw-w64-ucrt-x86_64-gcc mingw-w64-ucrt-x86_64-cmake mingw-w64-ucrt-x86_64-ninja mingw-w64-ucrt-x86_64-SDL2 mingw-w64-ucrt-x86_64-SDL2_image mingw-w64-ucrt-x86_64-SDL2_ttf
cmake -S . -B build-ucrt -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build-ucrt
ctest --test-dir build-ucrt --output-on-failure
./build-ucrt/soldier-skirmish.exe
```

保持在 UCRT64 终端启动，以便找到包管理器安装的间接依赖 DLL。CMake 会复制直接依赖 DLL，但不会自动制作可在任意电脑运行的完整发行包。不要混用 MSVC 的 `.lib` 和 MSYS2 的开发包。

参考：[MSYS2 环境选择](https://www.msys2.org/docs/environments/)、[SDL2](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-SDL2)、[SDL2_image](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-SDL2_image)、[SDL2_ttf](https://packages.msys2.org/packages/mingw-w64-ucrt-x86_64-SDL2_ttf)。

### Ubuntu / Debian

```bash
sudo apt-get update
sudo apt-get install g++ cmake ninja-build libsdl2-dev libsdl2-image-dev libsdl2-ttf-dev
cmake -S . -B build -G Ninja -DCMAKE_BUILD_TYPE=Debug
cmake --build build
ctest --test-dir build --output-on-failure
./build/soldier-skirmish
```

实际游戏需要图形桌面。`ctest` 的启动检查使用 SDL dummy 视频驱动，仅加载、绘制初始棋盘并退出，不启动对战线程。

也可以使用 Clang 或 MSVC 命令行工具链，只需提供与工具链和架构匹配的 SDL2 CMake 包；多配置生成器需要添加 `--config Debug` / `-C Debug`。不同编译器使用不同构建目录。

## 资源与自检

构建会把 `assets/` 复制到可执行文件旁。程序从自身所在目录查找资源，无需把终端切到原来的 Visual Studio 项目目录：

```bash
./build/soldier-skirmish --check-assets
./build/soldier-skirmish --smoke-test
./build/soldier-skirmish --assets /path/to/assets --check-assets
python3 tools/audit_project.py
```

Windows 中使用对应构建目录和 `.exe`。`--check-assets` 验证所需文件及地图/基地数据；`--smoke-test` 进一步验证 SDL、PNG 解码、字体加载、初始绘制和资源释放。检查通过不代表长时间对战没有线程问题。

`assets/map.txt` 首行为总格数，之后每行对应地图的一行；`0` 为可通行地面，非零数字为障碍物。`assets/base.txt` 有两组数据，每组为队伍编号、基地中心格下标、出兵格下标，下标从 0 开始按行排列。兵种数值与出兵周期在 `src/globalParameters.h` 中。

## 目录与项目来源

```text
src/                    游戏源码及启动资源校验
assets/                 地图、基地配置、图片、字体和字体许可证
docs/                   课程项目类图等长期项目文档
tools/audit_project.py  不依赖第三方 Python 包的静态文件检查
.github/workflows/      Windows / Linux 构建及启动检查
CMakeLists.txt          独立于 IDE 的构建入口
```

我以原始目录中较新的 `game` 版本为基础进行整理，没有混入更早的 `soliderfight` 实现或 `test` 练习，也没有携带 `.vs` 缓存、历史可执行文件及 PDB。原始类图见 [课程项目类图](docs/original-class-diagram.png)，它记录当时的结构，不包含新增启动校验模块。

整理时，我将原来使用的 Arial 字体换成了 [Noto Sans](https://github.com/google/fonts/tree/main/ofl/notosans)，附带 [SIL OFL 1.1](assets/font/OFL.txt)。第三方来源、哈希与许可说明见 [第三方声明](THIRD_PARTY_NOTICES.md)。
