# Raycaster Demo — 实现路线图（Roadmap）

> 目标文档见 `docs/demo.md`。本路线图覆盖全周期：**PC 可玩 MVP → MCU 移植 → FreeRTOS 多任务 → 复杂场景 → 新花样**，每阶段有明确验收，一步步走。
> 阶段一工时参考 HANDOVER（现实 26h / 乐观 15h / 悲观 45h，AI 辅助）；阶段二起为粗估，待硬件到位后校准。

## 总体阶段总览

| 阶段 | 内容 | 平台 | 验收（一句话） |
|---|---|---|---|
| **P1：PC 可玩 MVP**（M1–M4） | 地图 + DDA 渲染 + 键鼠操控 + 碰撞 | PC（SDL） | 可在地图中自由行走，不能穿墙 |
| **P2：移植 MCU（裸机）** | `McuPlatform`（SysTick/GPIO/SPI+DMA），保持 super loop | H7 + SPI 小屏 | 同一份 core 在 MCU 跑出与 PC 一致的画面 |
| **P3：引入 FreeRTOS** | 大循环体搬进 `GameTask`，新增 `AudioTask`，事件走队列 | H7 + FreeRTOS | 游戏照跑 + 声音正常，无竞态 |
| **P4：复杂场景** | 网格扩展（门/斜坡/多层）或 BSP 升级 | 双平台 | 场景更复杂，帧率达标 |
| **P5：新花样** | 敌人/道具/弹幕/特效 | 双平台 | 成熟方案上叠加玩法 |

## 架构基线（贯穿全程的关键决策）

> 前面讨论已定方向，写死作为约束，避免中途摇摆。

- **数据流注入，不用多态**：`core` 不依赖平台，无虚函数/无接口抽象（不引入 vtable）。core 只收数据（`InputState`/`dtMs`/帧缓冲），平台调用全留在 `main` 与平台层；`SdlPlatform` 是普通类。
- **无堆 / 定点数 16.16 / 零平台宏**：沿用现有红线（`-fno-exceptions -fno-rtti`、`NoHeap.cpp`、`Fixed.hpp`）。
- **MCU 选型：STM32H7（H743 或 H750）**：Cortex-M7 @480MHz、512KB AXI SRAM、LTDC + DMA2D（硬件缩放）。H750+外挂 QSPI Flash 性价比更高；H745 双核暂不需要。
- **屏幕：SPI 小屏（ILI9341 级 320×240）**：屏内自带 GRAM，MCU 只需 SPI+DMA 每帧搬一次；像素格式 **RGB565**（`McuPlatform::drawBuffer` 内部转换，core 无感）。参考：GB 160×144 / GBA 240×160——GBA 弱 CPU 都能跑 Wolf3D，H7 算力余量充足。
- **渲染策略：低分辨率渲染 + DMA2D 放大**（P2/P3 落实），进一步降低 320×240 渲染负担。
- **FreeRTOS 通信：事件走队列，状态归所有权**（P3 落实）。队列对使用者无锁（API 内部临界区）；ISR→队列（按键）、GameTask→AudioTask（事件）。持续状态（玩家位置等）由单一 task 拥有，不通过队列传。
- **P2 顺序：先裸机后 RTOS**，一次只引入一个变量；P2 只验证"core 在 MCU 跑通"，不做优化，尽快进入 P3。

## 阶段一：PC 可玩 MVP — 里程碑（M1–M4）

| 里程碑 | 内容 | 预计工时 | 验收（可玩性） |
|---|---|---|---|
| **M1** | 地图数据 + 玩家状态 | ~3h | 有地图常量、玩家可初始化、单测通过 |
| **M2** | DDA 光线投射 + 2.5D 渲染 | ~8h | 静态视角渲染出灰墙 2.5D 画面（无操控） |
| **M3** | 键鼠操控 + 碰撞 + 游戏循环 | ~8h | **MVP：可在地图中自由行走，不能穿墙** |
| **M4** | 增强（纹理/小地图/FPS/暗化） | ~7h | 画面更完整（MVP 之后可选） |

---

## M1 — 地图数据 + 玩家状态

**目标**：定义 2D 网格地图与玩家状态，全部定点数/固定数组（无堆）。

**涉及文件**
- `src/core/Map.hpp/cpp`：24×24 网格地图（Wolf3D 风格：外墙 + 内墙 + 房间），墙类型枚举（`WallType::Brick/Stone/...`），静态常量数据（或 `tools/` 生成）
- `src/core/Player.hpp`：位置 `(x, y)`（16.16 定点）+ 朝向角 `angle`（或方向向量 + 相机平面）
- `src/core/Fixed.hpp`：可能补充三角函数查表（`sin/cos` 查表，无 float）
- `tests/TestMap.cpp` / `TestPlayer.cpp`：gtest 用例

**验收**
- [ ] 地图可查询任意格子类型，越界按墙处理
- [ ] 玩家可初始化/设位置/旋转，朝向保持有效范围
- [ ] `i but` / `i rut` 全绿（新增单测并入 ctest label raycaster）
- [ ] clang-tidy / format 通过

---

## M2 — DDA 光线投射 + 2.5D 渲染

**目标**：屏幕逐列发射射线，DDA 求墙距，垂直校正后绘制墙条带到帧缓冲（静态画面）。

**涉及文件**
- `src/core/Raycaster.hpp/cpp`：DDA 算法（逐格步进找墙，返回 `perpDist` 与墙类型/侧面）
- `src/core/Renderer.hpp/cpp`：把 `perpDist` → 墙条带高度（`projPlaneDist / dist`），逐列写帧缓冲（顶/底为地板天花板色，墙为灰阶/墙类型色）
- 复用现有 `Frame`（固定数组）与 `Platform::drawBuffer`
- `tests/TestRaycaster.cpp`：DDA 距离/命中格/侧面方向单测

**验收**
- [ ] 主循环渲染一帧：能看到房间形状的 2.5D 灰墙画面（近大远小、无鱼眼）
- [ ] 距离越近墙越高、远墙低（正确透视）
- [ ] 单测覆盖：射线命中、无命中（开放格）、`perpDist` 计算、相机平面
- [ ] `i brc` 运行可看（静态帧，可用调试键切换角度验证）

---

## M3 — 键鼠操控 + 碰撞 + 游戏循环（MVP）

**目标**：WASD 移动（碰撞检测）+ 鼠标转向 + delta 时间归一化 + 持续循环 → **可玩**。

**涉及文件**
- `src/platform/api/Platform.hpp` / `sdl/SdlPlatform.cpp`：扩展输入——按键状态（W/A/S/D）、鼠标相对移动量、相对模式（隐藏光标）、Esc/关闭退出
- `src/core/Player.cpp`：移动（前/后/横移）+ **碰撞检测**（目标格为墙则拒绝，滑动修正可选）
- `src/main.cpp`：游戏主循环（事件 → 更新 → 渲染 → 呈现），帧率控制（垂直同步或固定步长），delta time 归一化移动/转向速度
- `tests/TestPlayer.cpp` 扩展：碰撞用例

**验收（MVP 完成标准）**
- [ ] WASD 在地图中行走，**不能穿墙**
- [ ] 鼠标左右移动平滑转向（相对模式、光标隐藏）
- [ ] 移动/转向速度不随帧率变化（delta 时间归一化）
- [ ] Esc / 窗口关闭退出，无崩溃、无堆分配
- [ ] 连续游玩 5 分钟稳定（无内存/卡顿问题）

---

## M4 — 增强（MVP 之后，可选）

- **墙纹理贴图**：`tools/` 把贴图转 C 数组，纹理坐标按 `perpDist` 采样
- **2D 小地图调试窗**：角落显示俯视地图 + 玩家位置/朝向
- **FPS / 帧耗时显示**：绘制文本或简单数字
- **距离暗化 / 雾效**：远墙变暗增加层次

---

## P2 — 移植 MCU（裸机 super loop）

**目标**：把 P1 的 PC 代码搬到 H7，**不引入 RTOS**，验证 core 在真实硬件上跑通。一次只引入一个变量（平台），为 P3 铺路。

**涉及文件**
- `src/platform/mcu/McuPlatform.hpp/cpp`：新后端——`getTicks`（SysTick）、`readInput`（GPIO 矩阵键盘扫描）、`drawBuffer`（SPI+DMA 刷屏，内部 RGB24→RGB565 转换）、`screenWidth/Height`（320×240 常量）
- `src/main.cpp`：保持 super loop 结构不变，仅把 `SdlPlatform` 换成 `McuPlatform`
- 屏驱动：ILI9341 初始化（平台层内或 `tools/` 辅助）

**关键决策**
- 顺序：**先裸机后 RTOS**——P2 不加 FreeRTOS，保持 super loop，出问题只有平台一个变量
- P2 只验证"跑通"，**不做优化**（帧率/DMA2D 放大留到 P3 之后）
- 像素格式：MCU 侧内部转换 RGB24→RGB565，core 无感

**验收**
- [ ] 同一份 core 代码在 H7 上渲染出与 PC 一致的 2.5D 画面
- [ ] 按键可操控行走，撞墙停止
- [ ] SPI+DMA 刷屏正常（320×240 @ 30–60fps）
- [ ] SysTick 时钟驱动 delta time，移动速度与 PC 一致
- [ ] 无堆分配（沿用 `NoHeap` 思路）

---

## P3 — 引入 FreeRTOS（多任务）

**目标**：把 super loop 体搬进 `GameTask`，新增 `AudioTask` 与输入任务，用队列做 task 间通信。渲染主循环结构不变。

**涉及文件**
- `main`：`xTaskCreate(GameTask/AudioTask/...)` + `vTaskStartScheduler()`，super loop 体原样搬进 `GameTask`
- `src/platform/mcu/`：声音后端（PC：SDL_mixer；MCU：DAC/I2S+PWM+DMA 环形缓冲）
- 队列/信号量：按键（ISR→队列）、游戏事件（GameTask→AudioTask）、状态所有权归 GameTask

**关键决策**
- 通信模型：**事件走队列，状态归所有权**（队列对使用者无锁）
- 输入：GPIO 中断 ISR 用 `xQueueSendFromISR` → 输入消费
- 声音：AudioTask 阻塞等队列事件，放采样；渲染不阻塞
- 渲染优先级高于声音；输入中断最高（P3 阶段定优先级表）

**验收**
- [ ] P2 的游戏逻辑在 `GameTask` 中原样运行，无行为回归
- [ ] 声音随事件触发（开枪/脚步/BGM），无卡顿、无竞态
- [ ] 按键经 ISR→队列→任务链路正常，无丢键
- [ ] 长时间运行无栈溢出/内存问题（FreeRTOS 栈监控）

---

## P4 — 复杂场景（网格扩展 或 BSP）

**目标**：场景从"网格地图 + DDA"向更复杂结构演进。**默认先走网格扩展**（门/斜坡/多层/电梯），BSP（Binary Space Partitioning，Doom 式）作为可选升级，评估后决定。

**涉及文件**
- `src/core/Map.hpp/cpp`：网格扩展（多层、动态门、斜坡）或 BSP 场景结构
- `src/core/Raycaster.cpp`：相应渲染/碰撞升级

**关键决策（待 P3 后评估）**
- 方向 A（推荐先做）：继续网格地图，加复杂化，不换数据结构，H7 性能稳
- 方向 B：升级 BSP——可做斜墙/任意多边形，但渲染管线重构，跨度大

**验收**
- [ ] 复杂场景可进入/通过（门开合、多层上下）
- [ ] 帧率维持在可玩水平（320×240 @ ≥30fps）
- [ ] 场景数据仍走 `tools/` 管线转 C 数组，无运行时解析

---

## P5 — 新花样（成熟方案上的玩法层）

**目标**：在 P4 稳定架构上叠加玩法，不与渲染/平台耦合。

- 敌人/怪物（精灵表、AI 行走、攻击）
- 道具/弹药/生命
- 弹幕/投射物
- 粒子/命中特效、HUD 文本
- 距离暗化/雾效（若 P1 未做）

**验收**
- [ ] 玩法与渲染/平台解耦，新增功能不改平台层
- [ ] 保持无堆/定点数/无 RTOS 依赖的 core 约束

---

## 开发纪律（每步提交走 Gerrit）

1. 每步（P1 的 M1–M4，及 P2–P5 各阶段）建 GitHub Issue（body 含 `**Tag**: FT00xx`）→ 实现 → `i but`/`i rut` 绿 → `i fm`/`i fma` → `i push` 评审 → CI → 自评/你 +2 → submit
2. 提交规范：`[FT00xx] 描述 (#N)`；琐碎用 `[None]`
3. 每一步完成后 `git status` 干净、三方一致（gerritRC/githubRC/本地）
