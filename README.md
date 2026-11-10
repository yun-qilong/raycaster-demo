# Raycaster Demo — STM32H743 裸机实时 2.5D 渲染

> Wolf3D 式 2.5D 光线投射（逐列 DDA）在 STM32H743 上的裸机实现。渲染核心与 PC 端（SDL2）共用同一份零依赖 C++17 代码。
> 显示链路为自研 [lcdriv](https://github.com/yun-qilong/lcdriv) 驱动 + SPI1 40MHz + DMA 双缓冲。320×240 横屏实测 **29.9fps**（26R1 基线），瓶颈是 SPI 带宽本身。
> 开发分支正在做游戏/平台解耦重构（[#8](https://github.com/yun-qilong/raycaster-demo/issues/8)）：PC 端已跑在新框架上，MCU 端待 -C 恢复。

[![CI](https://github.com/yun-qilong/raycaster-demo/actions/workflows/ci.yml/badge.svg)](https://github.com/yun-qilong/raycaster-demo/actions/workflows/ci.yml)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE)
![C++17](https://img.shields.io/badge/C%2B%2B-17-blue.svg)
![Bare-metal](https://img.shields.io/badge/bare--metal-no%20RTOS-success.svg)

## 演示

**实机**（STM32H743 + 2.8" SPI 屏，320×240 横屏）：

<p align="center">
  <img src="docs/assets/demo.gif" width="620" alt="实机演示：STM32H743 上运行的光线投射">
</p>

**PC 端**（SDL2，640×400，与实机同一份渲染核心）：

<p align="center">
  <img src="docs/assets/pc-demo.gif" width="620" alt="PC 端演示：SDL2 窗口中的同一场景">
</p>

<p align="center">
  <img src="docs/assets/screen.jpg" width="330" alt="2.8 吋 SPI 屏特写">
  <img src="docs/assets/pc-screenshot.jpg" width="330" alt="PC 端运行画面">
</p>

> GIF 为 12fps 采样，不代表实时帧率；实机实测 29.9fps 见[性能实测](#性能实测)。

## 功能

- **实时 2.5D 光线投射**：逐列发射射线 → DDA 求墙距 → 垂直距离校正（去鱼眼）→ 墙条带 + 天地背景。
- **移动与碰撞**：分轴碰撞判定（不可穿墙，贴墙时可沿墙滑动），转向同时支持按键与指针输入。
- **固定时间步长**：逻辑按 16ms（60Hz）步长推进、带累加器上限保护，渲染与逻辑解耦。
- **双端同一核心**：PC 端 SDL2 窗口（640×400，WASD + ←/→ + 鼠标），MCU 端裸机（板载双按键）。
- **核心平台无关**：无平台宏、无堆；分辨率与像素格式都由平台实现类决定（PC 640×400 RGB888，MCU 320×240 RGB565），
  两端口径不同但共用同一份应用代码。

## 硬件

| 项 | 值 |
|---|---|
| MCU | STM32H743IIT6（Cortex-M7 @ 480MHz），野火 Challenger 核心板 |
| 屏幕 | 2.8" 240×320 SPI（ILI9341 兼容控制器 + 74HC245 缓冲），横屏使用 320×240 |
| 总线 | SPI1 **40MHz**（PLL1Q 80MHz ÷ 2）+ TX DMA，软件 CS |
| 帧缓冲 | 双缓冲 2×153.6KB 置于 AXI SRAM（26R1）；渲染中间缓冲 225KB 原置于 D2 SRAM，改为 RGB565 直写后取消 |
| 操作 | 板载双按键（转向 / 前进） |
| 日志 | USART1，115200 8N1 |

接线：SCK `PA5` / MOSI `PA7` / CS `PA4` / DC `PB0` / RST `PB1`（MISO `PA6` 已分配；显示流程不依赖读屏）。

<p align="center">
  <img src="docs/assets/board.jpg" width="300" alt="开发装置全貌">
</p>

## 架构

分四层：**框架**（固定流程骨架 + 平台契约）/ **应用**（世界推进与绘制）/ **平台服务**（PC=SDL、MCU=HAL）/ **组装**（宿主，唯一同时了解应用与平台的地方）。

```
src/
  utils/       零项目依赖工具（CrtpBase：编译期多态）
  base/        框架：帧流程骨架 + 平台契约 + 契约词汇的类型骨架
               AppBase（逻辑帧 / 画面帧骨架）、PlantBase（输入 / 信箱 / 渲染内存 / 画面推送）、
               FrameBufferT / Pixel / Resolution / ContractT
  contracts/   具体应用的契约词汇（Input / Snapshot）
  algos/       通用算法：定点数（16.16）、三角函数查表、DDA 射线投射（无状态、可单测）
  app/         具体应用：移动与碰撞、逐列着色与绘制（只依赖 base + algos）
  platform/
    sdl/       PC 平台实现（SDL2 窗口 / 键鼠 / 时钟）
pc/            PC 产品树（CMake + 组装 + gtest）
stm32/         MCU 产品树（CubeMX 工程 + HAL + 链接脚本内存分区；目录名沿用早期点灯工程）
tools/         资产管线（地图 → C 头文件）
scripts/       构建与数据生成脚本（mapgen、CI 工具）
```

每个帧流程都由骨架驱动，应用只实现钩子：

| 帧 | 骨架做的事 | 应用钩子 |
|---|---|---|
| 逻辑帧 | 采样输入 → 把快照发布到信箱 | 推进一拍世界，产出快照 |
| 画面帧 | 取最新快照 → 借出一块可写缓冲 → 推送画面 | 按快照往缓冲里画 |

平台差异全部收敛在平台实现类里：PC 组装成 `Game<PcPlant>`，MCU 侧（FT0003-C）组装成 `Game<McuPlant>`，
应用与框架一行不改。像素格式同样由平台决定——PC 直写 RGB888、MCU 直写 RGB565，
应用层只描述色彩（`base::Color`），渲染不经过任何中间格式转换。

**约束**：C++17 受限子集——无堆 / 无异常 / 无 RTTI / 无虚函数 / 无浮点 / 固定数组。由产品 target 的
`-fno-exceptions -fno-rtti -fno-threadsafe-statics` 与 PC 侧的 `new/delete` 链接门禁（`pc/NoHeap.cpp`）实际看住。

## 关键实现（26R1 基线）

> 以下为 26R1（裸机大循环）版本的实现与实测数据。FT0003 重构已改动这些代码路径，数字待 -C 落地后重测。

### DMA 与双缓冲

单缓冲下 DMA 读取缓冲的 30.7ms 内 CPU 不能写入同一块（否则撕裂），只能 `compose → push → 等待 → 再 compose`，
传输与渲染串行，只有 ~21.4fps。改为双缓冲后 compose 与 DMA 并行，传输侧可达 **32.3fps**（≈带宽上限）；
游戏侧实测 29.9fps，差距来自节拍量化。

画面推送采用 latest-wins（26R1 中为 `pushFrame`，现为 `submitFrame`，返回是否收下）：
DMA 忙时返回 false，调用方**不切换缓冲**、丢弃本帧继续渲染下一帧，
因此渲染永不被传输阻塞，代价是丢弃部分渲染结果（见"帧节拍量化"）。

写入慢于面板扫描（30.7ms vs 14.3ms），横屏下撕裂边界呈**斜线**——由写入方向与扫描方向垂直导致，属几何必然。

### 缓冲放置与内存分区

两块 153.6KB 帧缓冲（≈300KB）放 AXI SRAM（512KB 区，占 58.6%），渲染帧缓冲 225KB 放 D2 SRAM（288KB 区，占 78.1%），
栈与热数据在 DTCM。选择依据是 **DMA 可达域**与容量：AXI SRAM 在 DMA 主控可达范围内且容量足够放下双缓冲。
具体分区见 `stm32/led_blink/STM32H743xx_FLASH.ld`，占用可在构建产物 `led_blink.map` 中复核。

### 帧节拍量化

推送成功间隔 ≈ ⌈30.7 / T_render⌉ × T_render：T=11.1ms 时 k=3 → ≈33.4ms → **29.9fps**。渲染侧实际跑到 ≈90 帧/s，
即每推送 1 帧要渲染约 3 帧、2/3 被丢弃。这是当前 CPU 的主要浪费，也是下一步"帧率自适应"要解决的问题。

### 32B 代码对齐

`-falign-functions=32 -falign-loops=32`（`stm32/led_blink/CMakeLists.txt`）把函数入口与循环头对齐到
Cortex-M7 取指/Flash 访问的 32 字节粒度。对齐前，改动**无关**代码会移位热点循环、改变取指对齐，
使同一静态场景帧率摆动 ~10%（实测 282 vs 311 帧/5s）；对齐后该抖动消失，渲染单轮 17.7 → 11.1ms。

## 性能实测

| 构建 | 渲染单轮 | 屏更新率 |
|---|---|---|
| `-O0` | 51ms | 19.6fps |
| `-O2` | 17.7ms | 27.6fps |
| `-O2` + 32B 对齐 | **11.1ms** | **29.9fps** |

- **帧预算（硬边界）**：全屏 320×240×2B = 153,600B，`帧时间 = 153600 × 8 ÷ SCK`。40MHz → **30.7ms/帧（≈32.5fps 上限）**。
- **纯传输侧**（`lcd_test`，无渲染负载）：32.3fps，达成率 ≈100%，确认 SPI 带宽是唯一瓶颈。
- **资源占用**：FLASH 32.5KB / 2MB；AXI SRAM 300KB / 512KB（58.6%）；D2 SRAM 225KB / 288KB（78.1%）；DTCMRAM 9.7KB / 128KB。
- **复现方式**：烧录 Release 固件，接串口（115200 8N1）。固件每 5 秒打印 `rendered=` / `pushed=` 增量，
  `pushed ÷ 5` 即屏更新率，`rendered ÷ 5` 即渲染能力（26R1 的心跳回调在 `stm32/led_blink/Core/Src/game_main.cpp`，
  该文件待 -C 重建）。

## SPI 40MHz 的随机故障：根因是线材

最小可用目标（~30fps）要求 SPI 跑到 40MHz——帧预算 30.7ms/帧，没有降频换稳定的余量。但这个频率下固件会随机出错：
大面积写入丢数据、清屏首块异常、显示状态被破坏，且**同一份代码时而正常、时而全坏**，与改了什么无关。

从屏幕控制器角度做过的多项假设（读屏 ID 不可靠、初始化配置组缺失、NSSP 脉冲模式、首笔时序）都解释不了残留现象：
改动后的表现始终不稳定，"改了就变好"无法排除偶发巧合。真正的原因在线材——40MHz 对应 25ns/bit，
20cm 杜邦线在该边沿速率下已到信号完整性边缘。换成 10cm 28 芯线后，**同一份固件立即稳定**
（连续 9 分钟以上 / 17000+ 帧无漂移 / 32.3fps），历史偶发错误同批消失。

完整过程（现象、各项假设与处置、对照实验、结论）：**[docs/bringup.md](docs/bringup.md)**。

## 已知问题

- 无 RTOS：26R1 为裸机大循环；开发分支上 MCU 树在 FT0003-A 后暂不可构建，FreeRTOS 化见 FT0003-B / -C。
  渲染节拍未与 DMA 对齐，29.9fps 未喂满 32.5fps 上限，且约 2/3 渲染被丢弃。
- 画面撕裂：单帧写入（30.7ms）慢于面板扫描（14.3ms），横屏下撕裂边界呈斜线（几何必然，局部刷新可解）。
- 墙体为纯色条带，贴图未接入；单元测试覆盖定点数、DDA 投射，以及像素格式/组装约定（渲染画面以抽样 golden 帧比对）。
- 缓存未开启，上述性能数据均在该状态下测得。
- SPI 上限未复测：换线后未重做频率爬坡，当前 40MHz 是满足需求的工作点而非面板上限。

## 路线图

按优先级排列：

1. **FreeRTOS 移植**（进行中，拆为 -A / -B / -C）：任务划分（游戏 / 渲染）与优先级，队列替代大循环轮询。
2. **帧率自适应**：渲染节拍对齐推送，省掉被丢弃的渲染，29.9 → 逼近 32.5fps。
3. **墙体贴图**：纹理采样接入渲染管线。
4. **SPI 频率爬坡重测**：换线后重做爬坡，确定当前接线下的真实上限。
5. **开启 I-Cache**：优化取指路径（本工程对取指对齐敏感，见"32B 代码对齐"）。

其它小项：FPS / 帧耗时叠加显示；局部刷新（变化矩形并集）与全刷混合；更多按键映射与关卡。

单元测试随上述开发同步补充（当前覆盖定点数、DDA 投射与像素格式/组装约定）。

## 构建与运行

### PC（Linux + SDL2）

```bash
sudo apt install -y build-essential cmake ninja-build python3 libsdl2-dev
cmake -S pc -B build -DCMAKE_BUILD_TYPE=Release -DRAYCASTER_BUILD_TESTS=ON
cmake --build build --target raycaster -j "$(nproc)"
./build/raycaster                       # WASD 移动、←/→ 或鼠标转向，Esc 退出
```

### MCU（STM32H743）

> ⚠ 当前开发分支上 MCU 树尚未接回新框架（`src/` 已按 FT0003 重构），下列命令会失败；恢复工作在 FT0003-C。

```bash
cmake -S stm32/led_blink --preset Release
cmake --build stm32/led_blink/build/Release -j "$(nproc)"
# 产物：led_blink.elf / led_blink.hex（每次构建由 objcopy 重新生成，供 Keil 下载）+ led_blink.map 内存报告
# 烧录：CMSIS-DAP（Keil MDK / OpenOCD 均可）
```

交叉编译需要 `arm-none-eabi-gcc` + CMake + Ninja + `python3`（构建时由 `scripts/mapgen.py` 生成地图头文件
`src/generated/MapData.hpp`，该生成物不入库）。外设与时钟配置由 STM32CubeMX（`stm32/led_blink/led_blink.ioc`）
管理，一律 GUI 生成，不手改生成代码。

### 单元测试

```bash
cmake --build build --target raycaster_ut -j "$(nproc)"
ctest --test-dir build -L raycaster
```

## 开发

- **CI（GitHub Actions，push main 触发）**：构建 / 单元测试 / clang-format / clang-tidy 四道阻断门禁，
  外加提交与 Issue 关联检查（`scripts/check-issue-ref.sh`，提醒式、不阻断），见 `.github/workflows/ci.yml`。
- **提交规范**：`[FTxxxx] 描述 (#issue)`，每条提交关联 Issue。
- **代码规范**：`.clang-format` / `.clang-tidy`，与 CI 使用同一套版本行为。
- **资产零运行时加载**：地图以编译期常量数组进固件（`scripts/mapgen.py` 在构建时生成），见 [tools/README.md](tools/README.md)。

## 相关项目

- [lcdriv](https://github.com/yun-qilong/lcdriv) —— 本项目使用的裸机 SPI LCD 驱动库（Bus × Controller 正交、编译期绑定、带单测与 ADR 文档），固定在其 `26R1.0` 冻结标签上。

## License

[MIT](LICENSE)
