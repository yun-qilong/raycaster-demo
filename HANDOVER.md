# Raycaster Demo — Handover（2026-08-15）

> 这是从 flowHub 讨论窗口交接过来的项目启动文档。新对话窗口请先读本文件。

## 项目一句话

x86（Linux/Windows + SDL）可玩的**纯光线投射**（Wolf3D-Like）demo，带**可移植约束**，后期完整版移植 Cortex-M7。独立项目，不复用 FlowHub 代码，但复用其工程习惯（无堆/消息驱动/数据导向）。

## 已完成

- GitHub 建库：`yun-qilong/raycaster-demo`（空库，未勾选任何初始化文件）
- 本地 clone：`/home/hongxian/raycaster-demo`（origin = GitHub）
- git 全局身份已配置（yun-qilong / yun_qilong@qq.com）
- `g` 脚本已建（工程专用，真实文件 `scripts/dev/g`，`.gitignore` 忽略不入库；在工程根目录用 `./scripts/dev/g <cmd>` 调用）：`push`/`pull`/`fetch` 已实现，`build`/`run`/`ut`/`fm` 占位待补
- `.gitignore` 已建（忽略 build/、scripts/dev/ 个人工具等）
- **gerrit 接入完成**：`gerrit create-project raycaster-demo --branch main` 已执行；本地 remote `gerrit` = `ssh://qilyun@localhost:29418/raycaster-demo`（origin 仍是 GitHub）
- **Jenkins 完成**：新建 pipeline job `raycaster-ci`（http://localhost:8090/job/raycaster-ci，内嵌 raycaster 版 Jenkinsfile，参数 GERRIT_* 同 flowhub-ci；`g push` 后由 watcher 触发）
- **CI 规则独立建好**（在工程内，可并行共存 flow_hub）：`Jenkinsfile`、`scripts/gerrit-ci.sh`、`scripts/check-issue-ref.sh`（GITHUB_REPO 已改）、`scripts/ci-watcher.py`、`scripts/gerrit-event-watcher.py`（PROJECT_PATTERN/JENKINS_JOB 已改）、`scripts/ci-ctl.sh`、`scripts/run_tidy.py`、`.clang-format`、`.clang-tidy`
- **watcher 已启动（与 flowHub 一致）**：只跑 `gerrit-event-watcher`（事件驱动，自动重连，日志 /tmp/raycaster-gerrit-watcher.log）；**轮询 ci-watcher 默认不跑**（备用：`bash scripts/ci-ctl.sh start`，日志 /tmp/raycaster-ci-watcher.log）。**recheck 机制 ✓**：在 Gerrit change 评论 `recheck` 可手动触发 CI（gerrit-event-watcher 的 comment-added 分支）

## 关键技术决策（设计人定、实现 AI 做）

- **范围**：纯光线投射，**无** BSP / MCU / 性能调优 / 声音 / 怪物
- **可移植约束（红线）**：
  - 无堆（Linux 调试时重载 `operator new` 为报错）
  - 定点数 16.16（不用 float）
  - 固定数组 / BoundedVector
  - `-fno-exceptions -fno-rtti -fno-threadsafe-statics`
  - **核心零平台宏**（`#ifdef` 只允许出现在 platform 层）
- **架构分层**：
  ```
  src/
    core/          # renderer/math/container/logic（平台无感）
    platform/
      api/         # Platform 接口：getTicks/readInput/drawBuffer
      sdl/         # SDL 后端（PC）
  tools/           # asset pipeline（贴图/地图转 C 数组）
  tests/           # 单测
  ```
- **迭代顺序**：灰色墙无纹理基线（~1h）→ 加纹理 → 移动/碰撞 → 完整 demo
- **工时**：现实 26h（乐观 15h / 悲观 45h），AI 辅助
- **构建**：CMake + SDL2；未来 Linux/Windows 双平台编译矩阵（CI）

## 待办步骤（下一步）

1. **首提交 "hello window"**：代码已就绪（骨架 + SDL 窗口 + 像素填充 + Fixed 单测），构建/测试/运行均已验证 ✅；需 git add + commit。**commit message 规则（CI 强制，与 flow_hub 同）：`[FT00xx] 描述 (#N)`**——先建 GitHub Issue（body 含 `**Tag**: FT00xx`），提交引用该 issue；琐碎改动可用 `[None] 描述` 跳过
2. **push GitHub main**：`git push origin main`；随后 `./scripts/dev/g push`（推 gerrit 评审，触发 raycaster-ci）
3. **迭代顺序**：灰色墙无纹理基线（~1h）→ 加纹理 → 移动/碰撞 → 完整 demo

## 已完成（工程）

- **SDL2 已装**（libsdl2-dev 2.0.20；WSLg 环境，窗口显示在 Windows 桌面）
- **GitHub CI 已建**（参照 flowHub）：`.github/workflows/ci.yml`（push main 触发：Issue Check → 装依赖 → Build → Test → Format → Tidy）+ `.github/ISSUE_TEMPLATE/`（feature `[FTxxxx]`/fix `[FXxxxx]`/refine `[RIxxxx]` + config.yml）
- **搭骨架完成**：顶层 CMakeLists（option `RAYCASTER_BUILD_TESTS`、target `raycaster`+`raycaster_ut`、ctest label `raycaster`、SDL2::SDL2）、`src/core`（Fixed.hpp 16.16 定点数）、`src/platform/api`（Platform 接口：getTicks/readInput/drawBuffer）、`src/platform/sdl`（SdlPlatform 后端）、`src/NoHeap.cpp`（operator new 重载报错，只进 raycaster 目标）、`tests`（gtest + TestFixed 5 用例）、`tools`（占位）、README、.clang-tidy HeaderFilterRegex 已改为 raycaster 路径
- **hello window 验证通过**：编译 0 error、clang-tidy 0 告警、ctest 5/5 通过、`./build/src/raycaster` 运行退出码 0（显示 3 秒渐变后退出）

## 已完成（基础设施）

- **gerrit 接入**：create-project + remote 已配好（见上）
- **Jenkins**：job `raycaster-ci` 已建，CI 规则在工程内独立一份（见上）
- **watcher**：两个 watcher 已启动（见上）
- **双项目干扰审计完成（2026-08-15，harness）**：无 Docker 下与 flow_hub 共存，已修复 raycaster 侧 3 个风险：① `check-issue-ref.sh` 改用 mktemp 唯一临时文件（原共用 /tmp/issue_body.json，并发 CI 会互覆盖）② `ci-watcher.py` 默认 work-dir/state-file 改为 /tmp/raycaster-ci* ③ `gerrit-ci.sh` 默认 BUILD_DIR 改为 /tmp/raycaster-ci-build。**遗留（flowHub 侧不越权改）**：flowHub 的 `~/bin/i sg/cg` 用 `pkill -f gerrit-event-watcher` 会误杀 raycaster watcher——但 raycaster watcher 有自动重启循环（5s 恢复）+ poll watcher 兜底，影响有限
- **私有仓库 issue 检查已支持 `GITHUB_TOKEN`**：`check-issue-ref.sh` 有 token 环境变量则带 Authorization header（GitHub Actions 自动注入；**Jenkins 侧需配全局环境变量 GITHUB_TOKEN = PAT**）。GitHub 仓库保持私有（未来某时刻公开）
- **g 别名**：已加 `~/.bashrc` → `alias g='/home/hongxian/raycaster-demo/scripts/dev/g'`（脚本本体仍在工程内不入库；**迁移新 PC 时需重加此行**）
- **g 脚本命令全集**：`push`/`pull`/`fetch`（git/gerrit）+ `build`/`run`/`ut`/`fm`/`fma`/`format`（工程）+ `sg`/`cg`（CI 基础设施：**只检查共享的 Gerrit/Jenkins 状态 + 管理 raycaster 自己的 watcher**，不启动共享服务，避免端口冲突）

## 参考

- 完整讨论笔记：`/home/hongxian/flowHub/notes/other/doom-like-brainstorm.md`（§1~§27，含架构/工时/技能评估/CI 复用）
- 工程习惯参考：`/home/hongxian/flowHub`（i 脚本、Jenkinsfile、scripts/ci-watcher.py、gerrit-ci.sh）
