# GraphVisualizer

基于 Qt 6 / C++17 的图算法可视化系统。支持无向图与有向图的可视化编辑、随机图生成（ER G(n,m)、G(n,p)、Barabási–Albert、Watts–Strogatz）、以及六类经典图算法的分步演示。

## 已实现算法

- DFS / BFS 遍历
- Dijkstra 最短路径
- Prim / Kruskal 最小生成树
- 拓扑排序（Kahn）

每个算法都通过统一的 `AlgorithmBase` 抽象基类暴露 `step()` 接口，可单步、自动播放、暂停、重置；运行过程中实时展示内部数据结构（栈、队列、距离数组、并查集、入度数组）。

## 主要特性

- QGraphicsView 高性能 2D 画布，顶点 / 边可拖拽编辑
- 随机图生成对话框，支持权重随机化
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
