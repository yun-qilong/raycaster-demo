# Raycaster Demo

x86（Linux/Windows + SDL）可玩的**纯光线投射**（Wolf3D-Like）demo，带**可移植约束**
（无堆 / 定点数 16.16 / 平台抽象），后期完整版移植 Cortex-M7。

## 构建

依赖：CMake ≥ 3.20、SDL2 开发库（`sudo apt install -y libsdl2-dev`）、g++ / clang。

```bash
cmake -S pc -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --target raycaster -j "$(nproc)"
./build/raycaster
```

## 测试

```bash
cmake --build build --target raycaster_ut -j "$(nproc)"
ctest --test-dir build -L raycaster
```

## 目录结构

```
src/             # 共享源码树（PC 与 MCU 构建树各自按需引入）
  utils/         # 通用 C++ 工具（CRTP 样板等，零项目依赖）
  core/          # renderer/math/container/logic（平台无感，零平台宏）
  generated/     # MapData.hpp（mapgen 生成，不入库）
  platform/
    api/         # Platform<Impl> CRTP 接口：getTicks/sampleInput/drawBuffer
    sdl/         # SDL 后端（PC：窗口/键盘/时钟）
    mcu/         # （规划）MCU 后端
pc/              # PC 产品：构建入口 CMakeLists + main/NoHeap + tests/（gtest）
stm32/           # MCU 产品：CubeMX 工程（独立构建树）
tools/           # asset pipeline（贴图/地图转 C 数组）
scripts/         # 本地 CI 规则（Jenkinsfile/gerrit-ci.sh/watcher 等）
```

## 可移植约束（红线）

- 无堆：Linux 调试时重载 `operator new` 为报错（`pc/NoHeap.cpp`）
- 定点数 16.16（不用 float）
- 固定数组 / BoundedVector
- `-fno-exceptions -fno-rtti -fno-threadsafe-statics`
- 核心零平台宏（`#ifdef` 只允许出现在 platform 层）

## 提交规范（CI 强制）

与 flow_hub 相同：commit message 引用 GitHub Issue，CI（raycaster-ci 的 Issue Check stage）交叉验证。

- 格式：`[FT00xx] 描述 (#N)`，如 `[FT0001] hello window 首提交 (#1)`
- 每条提交对应一个 GitHub Issue（在 `yun-qilong/raycaster-demo`），Issue body 必须包含 `**Tag**: FT00xx`（与提交 tag 一致）
- Issue 必须处于 open 状态
- 例外：`[None] 描述` 可跳过 issue 检查（适合无需 issue 的琐碎改动）

## CI

- **GitHub Actions**：`.github/workflows/ci.yml`（push main 触发：Issue Check / Build / Test / Format / Tidy）
- **Gerrit 评审**：`ssh://qilyun@localhost:19418/raycaster-demo`（HTTP 18080）
- **Jenkins**：`raycaster-ci`（http://localhost:18090/job/raycaster-ci）
- 本地 CI 规则在 `scripts/` 下独立维护（与 flow_hub 可并行共存）
