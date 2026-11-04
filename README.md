# Soldier Skirmish · 士兵遭遇战

基于 C++ 和 SDL2 的网格士兵对战模拟小游戏。两支队伍在包含障碍物的地图中自动出兵、寻路和交战，以摧毁对方基地为目标。

项目完成于 **2024 年 1 月**，为数据结构课程大作业，包含 MVC 结构、BFS 寻路、兵种工厂、攻击策略和视野观察者设计。当前仓库提供 CMake 构建、资源校验及 Windows / Linux 自动检查。

跨平台整理与基础修复版本标记为 [`v0.1.0-baseline`](https://github.com/starskyer/soldier-skirmish/tree/v0.1.0-baseline)。后续开发以此为基线，逐步清理历史问题并完善游戏展示，具体范围见 [TODO 与改进方向](#todo-与改进方向)。

## 玩法与实现

- 默认地图为 20 × 20 网格，包含障碍物与两座 3 × 3 基地。
- 基地定时随机生成步兵、弓箭手或攻城炮。
- 士兵自动选择目标，以 BFS 搜索可通行路径，并在射程内攻击。
- **Start**：开始自动对战；**Pause / Paused**：切换暂停状态。
- **Change Vision**：按下时切换双方合并视野、队伍 1 视野、队伍 2 视野。
- 基地生命值降到 0 时显示胜方并退出。当前不提供联网对战、键盘操控单个士兵或局内重开。

默认窗口为 1080 × 1230，较矮的屏幕可能显示不全；可修改 `src/globalParameters.h` 中的窗口尺寸后重新构建。

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

实际游戏需要图形桌面。`ctest` 的启动检查使用 SDL dummy 视频驱动，仅加载、绘制初始棋盘并退出，不启动对战线程。

Clang 和 MSVC 命令行工具链需要配置与其架构匹配的 SDL2 CMake 包。多配置生成器使用 `--config Debug` / `-C Debug`；不同编译器使用独立构建目录。

## 配置与资源

`assets/map.txt` 首行为总格数，之后每行对应地图的一行；`0` 为可通行地面，非零数字为障碍物。

`assets/base.txt` 有两组数据，每组为队伍编号、基地中心格下标、出兵格下标，下标从 0 开始按行排列。兵种数值、出兵周期和窗口尺寸在 `src/globalParameters.h` 中配置。

构建会把 `assets/` 复制到可执行文件旁。程序从可执行文件所在目录查找资源，也可通过 `--assets` 指定其他资源目录。

## 检查

```bash
./build/soldier-skirmish --check-assets
./build/soldier-skirmish --smoke-test
./build/soldier-skirmish --assets /path/to/assets --check-assets
python3 tools/audit_project.py
```

Windows 使用对应构建目录和 `.exe` 后缀。

- `--check-assets`：验证所需文件及地图、基地数据。
- `--smoke-test`：验证 SDL 初始化、PNG 解码、字体加载、初始绘制和资源释放，然后退出。
- `tools/audit_project.py`：检查本地头文件引用、编译文件清单及资源完整性，不依赖第三方 Python 包。

[GitHub Actions](https://github.com/starskyer/soldier-skirmish/actions) 在 Windows / Linux 上执行构建、资源校验和无窗口启动检查。这些检查不覆盖完整对局、实际交互或长时间运行的并发稳定性。

## 目录结构

```text
src/                    游戏源码及启动资源校验
assets/                 地图、基地配置、图片、字体和字体许可证
docs/                   课程项目类图等长期项目文档
tools/audit_project.py  不依赖第三方 Python 包的静态文件检查
.github/workflows/      Windows / Linux 构建及启动检查
CMakeLists.txt          独立于 IDE 的构建入口
```

原始课程设计见 [项目类图](docs/original-class-diagram.png)，其中不包含后续新增的启动校验模块。

## TODO 与改进方向

`v0.1.0-baseline` 对应提交 `3170aa7`，包含 CMake 迁移、资源校验及已完成的基础修复；Windows / Linux 构建与无窗口启动检查均已通过。该标记用于固定后续改进前的版本，原始课程项目完成时间仍为 2024 年 1 月。

### 历史问题清理

- [ ] **状态一致性与同步**：统一棋盘、单位和生命值的修改规则，消除数据竞争；模拟执行方式待确定。
- [ ] **移动与攻击校验**：禁止单位覆盖占格、友军误伤和死亡单位重新落位，行动执行前复核目标与位置。
- [ ] **对局生命周期**：建立开始、暂停、结束和退出流程，移除忙等待及不受控的 detached 线程。
- [ ] **对象与资源管理**：明确单位、攻击策略、视野和 SDL 资源的所有权，补齐析构及异常清理。
- [ ] **初始状态与寻路边界**：计算首帧视野；为无路、拥堵和目标失效提供有界处理。
- [ ] **可复现检查**：统一随机种子，增加模拟状态不变量检查与不依赖窗口的对局测试。

### 游戏规则与展示增强

- [ ] 实现基地反击并默认开启，明确攻击周期与目标选择，验证数值影响。
- [ ] 实现已确认的空格竞争裁决和同时伤害结算，允许同归于尽；补齐遮挡、换位和拥堵处理规则。
- [ ] 完善窗口尺寸适配、暂停/恢复和胜负展示；重开、速度调节等交互按后续范围实现。
- [ ] 补充完整对局、边缘寻路、占格竞争、目标失效、中途退出及长时间运行的回归检查。

基线仍包含并发、生命周期和资源管理风险，基础 CI 不覆盖完整对局稳定性。历史问题修复与玩法增强将分别记录；未确认的规则不在修复过程中直接改动。

后续执行方案、已确认规则和待定事项见 [模拟设计](docs/SIMULATION_DESIGN.md)。该文档描述开发目标，尚不代表当前版本已实现这些行为。

## 素材与第三方依赖

游戏代码及七张兵种、基地图片为原创内容。字体使用 [Noto Sans](https://github.com/google/fonts/tree/main/ofl/notosans)，按 [SIL OFL 1.1](assets/font/OFL.txt) 分发。SDL 依赖、字体来源及许可说明见 [第三方声明](THIRD_PARTY_NOTICES.md)。
