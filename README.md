# Soldier Skirmish · 士兵遭遇战

基于 C++ 和 SDL2 的网格士兵对战模拟小游戏。两支队伍在包含障碍物的地图中自动出兵、寻路和交战，以摧毁对方基地为目标。

项目起源于 **2024 年 1 月**完成的数据结构课程大作业。课程版包含 MVC 结构、BFS 寻路、兵种工厂、攻击策略和视野观察者设计。当前版本采用单线程固定时间步、单位 ID 和批量动作结算，提供独立的无窗口模拟核心。

跨平台整理与基础修复版本保留在 [`v0.1.0-baseline`](https://github.com/starskyer/soldier-skirmish/tree/v0.1.0-baseline)。当前重构以此为起点，保留格子地图、四方向 BFS 和原有素材，具体变化见 [模拟设计](docs/SIMULATION_DESIGN.md)。

## 玩法与实现

- 默认地图为 20 × 20 网格，包含障碍物与两座 3 × 3 基地。
- 基地定时随机生成步兵、弓箭手或攻城炮。
- 基地默认反击中心射程内的敌兵，使用一秒攻击冷却。
- 士兵自动选择目标，以 BFS 搜索可通行路径，并在射程内攻击。
- **Start**：开始自动对战；**Pause / Resume**：暂停或恢复模拟时间。
- **Vision**：切换双方合并视野（0）、红队视野（1）、蓝队视野（2）。
- 基地生命值降到 0 时显示胜方，双方同时摧毁时判平局；结果显示三秒后退出，期间仍可关闭窗口或按 Esc 退出。

窗口使用 1080 × 1230 的逻辑画布，按可用屏幕尺寸缩放，支持窗口调整与鼠标坐标映射。单位显示沿移动线段插值，逻辑位置仍为离散格子；当前不提供联网、单兵操控或局内重开。

## 构建

依赖：C++17 编译器、CMake ≥ 3.21，以及 **SDL2、SDL2_image、SDL2_ttf 的开发包**。SDL3 与 SDL2 API 不兼容。SDL 开发包通过包管理器安装，SDK 和预编译库不随仓库分发。

克隆仓库，以下构建命令均在项目根目录执行：

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

程序在 UCRT64 终端中启动，以便找到包管理器安装的间接依赖 DLL。CMake 会复制直接依赖 DLL；独立发行包还需收集间接依赖。MSVC 的 `.lib` 与 MSYS2 开发包应分别使用，不混用工具链。

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

实际游戏需要图形桌面。`ctest` 的 SDL 启动检查使用 dummy 视频驱动，检查初始画面、短时模拟、暂停画面和退出；所有模拟均在调用线程执行。

Clang 和 MSVC 命令行工具链需要配置与其架构匹配的 SDL2 CMake 包。多配置生成器使用 `--config Debug` / `-C Debug`；不同编译器使用独立构建目录。

### 仅构建无窗口模拟

该模式只需要 C++17 和 CMake，不依赖 SDL，也不需要字体或图片：

```bash
cmake -S . -B build-core -G Ninja -DSKIRMISH_BUILD_GAME=OFF -DCMAKE_BUILD_TYPE=Debug
cmake --build build-core
ctest --test-dir build-core --output-on-failure
./build-core/soldier-skirmish-sim --seed 1 --ticks 18000 --require-finished
```

`--ticks` 是模拟步数上限，60 步对应一秒模拟时间；`--require-finished` 要求在上限内产生胜负或平局，否则返回退出码 2。输出包含结果、步数、基地生命值及用于回归比较的状态指纹，不会为通过测试而强行判胜。指纹不是跨版本稳定的数据格式。

## 配置与资源

`assets/map.txt` 首行为总格数，之后每行对应地图的一行；`0` 为可通行地面，非零数字为障碍物。

`assets/base.txt` 有两组数据，每组为队伍编号、基地中心格下标、出兵格下标，下标从 0 开始按行排列。兵种数值、出兵周期和窗口尺寸在 `src/globalParameters.h` 中配置。

构建会把 `assets/` 复制到可执行文件旁。程序从可执行文件所在目录查找资源，也可通过 `--assets` 指定其他资源目录。

## 检查

```bash
./build/soldier-skirmish --check-assets
./build/soldier-skirmish --smoke-test
./build/soldier-skirmish-sim --seed 42 --ticks 18000 --require-finished
./build/soldier-skirmish --assets /path/to/assets --check-assets
python3 tools/audit_project.py
```

Windows 使用对应构建目录和 `.exe` 后缀。

- `--check-assets`：验证所需文件及地图、基地数据。
- `--smoke-test`：验证 SDL 初始化、PNG 解码、字体加载、初始绘制和资源释放，然后退出。
- `--seed`：设置对局种子，默认 1；控制出兵和同等待时间的争格裁决。
- `tools/audit_project.py`：检查本地头文件引用、编译文件清单及资源完整性，不依赖第三方 Python 包。

[GitHub Actions](https://github.com/starskyer/soldier-skirmish/actions) 在 Windows / Linux 上执行核心回归、三组种子的有界完整对局、资源和 SDL 启动检查；Linux 另用 AddressSanitizer、UndefinedBehaviorSanitizer 和泄漏检测检查无窗口核心。这些检查不替代人工交互、所有地图的终局保证或长期压力测试。

## 目录结构

```text
src/                    模拟核心、动作结算、界面及启动资源校验
assets/                 地图、基地配置、图片、字体和字体许可证
docs/                   课程项目类图等长期项目文档
tests/                  状态一致性、动作结算与生命周期回归
tools/audit_project.py  不依赖第三方 Python 包的静态文件检查
.github/workflows/      Windows / Linux 构建及启动检查
CMakeLists.txt          独立于 IDE 的构建入口
```

原始课程类图保留在 [项目类图](docs/original-class-diagram.png)，用于记录课程版本；当前执行结构与规则以 [模拟设计](docs/SIMULATION_DESIGN.md) 为准。

## TODO 与改进方向

`v0.1.0-baseline` 对应提交 `3170aa7`，包含 CMake 迁移、资源校验及已完成的基础修复；Windows / Linux 构建与无窗口启动检查均已通过。该标记用于固定后续改进前的版本，原始课程项目完成时间仍为 2024 年 1 月。

### 历史问题清理

- [x] **状态一致性与同步**：以 60Hz 整数 tick 推进，在共同快照上规划动作，统一结算；移除共享状态线程及锁。
- [x] **移动与攻击校验**：单位 ID 与格子分离，复核行动起点、目标身份、敌我关系、射程及冷却，禁止占格覆盖和死亡后移动。
- [x] **对局生命周期**：开始、暂停、结束和退出由主循环管理，移除忙等待及 detached 线程。
- [x] **对象与资源管理**：单位和基地按值持有，SDL 资源由 RAII 管理，初始化失败可清理已创建资源。
- [x] **初始状态与寻路边界**：在首帧前计算视野；无路时等待后重新规划，不进入无界随机重选循环。
- [x] **可复现检查**：固定种子、逐步状态约束、22 个核心回归场景及独立无窗口对局入口。

### 游戏规则与展示增强

- [x] 默认开启基地反击，保留伤害 50、中心射程 4，采用一秒冷却和最近目标优先。
- [x] 争格时等待更久者优先，同等待时间使用种子裁决；有效伤害同时结算，允许同归于尽与双方基地同时摧毁的平局。
- [x] 窗口适配、显示插值、暂停/恢复及可响应关闭的结果显示。
- [ ] 进一步评估基地反击的平衡、长时间拥堵、更多地图和 GUI 操作回归；明确遮挡、换位等规则后再增强。
- [ ] 独立评估 A*、真实连续坐标、局部避让、重开和速度调节。

基线保留历史行为与风险。当前版本已清理上述实现问题，但保守占格策略仍可能导致某些局面长时间僵持；有界对局测试只覆盖选定地图与种子，不保证任意配置都能结束。

实际结算阶段、插值边界和待定事项见 [模拟设计](docs/SIMULATION_DESIGN.md)。历史问题修复与玩法增强分别记录。

## 素材与第三方依赖

游戏代码及七张兵种、基地图片为原创内容。字体使用 [Noto Sans](https://github.com/google/fonts/tree/main/ofl/notosans)，按 [SIL OFL 1.1](assets/font/OFL.txt) 分发。SDL 依赖、字体来源及许可说明见 [第三方声明](THIRD_PARTY_NOTICES.md)。
