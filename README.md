# GraphVisualizer

基于 Qt 6 / C++17 的图算法可视化系统。支持无向图与有向图的可视化编辑、四类随机图模型生成（Erdős–Rényi G(n,p) / G(n,m)、Random Geometric Graph、Barabási–Albert、Watts–Strogatz）、以及六类经典图算法的分步演示。

## 已实现算法

- DFS / BFS 遍历
- Dijkstra 最短路径
- Prim / Kruskal 最小生成树
- 拓扑排序（Kahn）

每个算法都通过统一的 `AlgorithmBase` 抽象基类暴露 `step()` 接口，可单步、自动播放、暂停、重置；运行过程中实时展示内部数据结构（栈、队列、距离数组、并查集、入度数组）。

## 已实现的随机图模型

- **Erdős–Rényi** — G(n,p) 概率模型与 G(n,m) 固定边数模型
- **Random Geometric Graph (RGG)** — 单位正方形内均匀撒点 + 距离阈值连边，连通性阈值 r_c ≈ √(ln n / πn)
- **Barabási–Albert** — preferential attachment 机制生成无标度网络
- **Watts–Strogatz** — ring lattice + 概率重连生成小世界网络

附带 `verify_rgg.py` 独立验证脚本：复刻算法 + 多次重复 + 与理论值对照。

## 主要特性

- QGraphicsView 高性能 2D 画布，顶点 / 边可拖拽编辑
- 随机图生成对话框（4 Tab UI），支持权重随机化与实时参数预估
- 同屏对比窗口（CompareWindow），并排播放两种算法
- 图结构 JSON 序列化（保存 / 加载）
- 自绘无边框窗体、QSS 现代化主题

## 构建

依赖：Qt 6.x、CMake 3.20+、支持 C++17 的编译器（MSVC 2022 / MinGW / Clang）。

```bash
cmake -S . -B build
cmake --build build --config Release
```

构建产物为 `build/GraphVisualizer.exe`（Windows）或同名可执行文件（其它平台）。

## 目录

```
src/                 # C++ 源代码
CMakeLists.txt       # 构建脚本
build.bat            # Windows 一键构建
```
