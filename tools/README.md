# tools/ — 资产管线（地图 → C++ 常量数组）

裸机没有文件系统、也没有堆，所以地图数据以**编译期常量数组**进入固件。这里放"人写的资产"，
以及面向资产格式的转换脚本。

| 文件 | 作用 | 现状 |
|---|---|---|
| `maps/default.map` | 24×24 文本地图（`.` 空地 / `#` 砖墙 / `S` 石墙） | **已接入构建**（PC 树生成头文件，游戏直接使用） |

## 地图：`maps/default.map` → `src/generated/MapData.hpp`

- **格式**：开头是 `//` 图例与坐标说明，随后是列号表头，数据行形如 `23 ########################`（行号 + 地图行），
  `//` 之后为注释。坐标是数学坐标：**文件最上面一行是场景顶（y=23），最下面一行是 y=0**，生成时按此做行序翻转。
- **校验**：必须 24×24、字符必须在图例内，出错会报出行列位置；查询越界格子按墙处理（见 `src/core/Map.hpp`）。
- **生成的符号**：`kMapWidth` / `kMapHeight` / `kTiles[y][x]`，取值对应 `ray::WallType`（0 空地 / 1 砖 / 2 石）。
- **生成时机**：`pc/` 与 `stm32/` 两条构建树都挂了 CMake 自定义目标 `gen_mapdata`——`default.map` 或 `scripts/mapgen.py`
  一变即重新生成，产物 `src/generated/MapData.hpp` 不入库。全新 clone 直接构建任一端都会先自动生成该头文件，
  改地图 = 编辑 `.map` 后重新构建，不必手跑脚本。

手动生成（例如临时换图调试）：

```bash
python3 scripts/mapgen.py                                  # 默认 default.map → src/generated/MapData.hpp
python3 scripts/mapgen.py --input tools/maps/other.map     # 换输入地图
```

> 注意：输出路径固定为 `src/generated/MapData.hpp`（`src/core/Map.hpp` 按该路径 include），
> 所以同一时刻只有一张"活动地图"；`--output` 只在脱离构建树单独调试时有用。

## 约定

- **零运行时加载**：资产一律编译期常量数组，符合"无堆 / 无异常 / 无文件系统"的产品红线。
- **生成物不手改**：`src/generated/*.hpp` 带 `DO NOT EDIT` 头；改数据请改源资产或脚本。
- **脚本位置**：随构建自动执行的转换脚本放 `scripts/`（如 `mapgen.py`）；人工按需执行的放 `tools/`。
