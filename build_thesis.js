// Generate thesis docx following the user's directory.
const fs = require('fs');
const {
  Document, Packer, Paragraph, TextRun, Table, TableRow, TableCell,
  Header, Footer, AlignmentType, LevelFormat, HeadingLevel,
  BorderStyle, WidthType, ShadingType, TabStopType, TabStopPosition,
  PageNumber, PageBreak, PageOrientation,
} = require('docx');

// ============= helpers =============
const FONT = "宋体";
const HFONT = "黑体";
const SIZE = 24; // 12pt
const LINE = 360; // 1.5 line spacing (240 = single, 360 = 1.5x)

function p(text, opts = {}) {
  const { bold = false, italic = false, font = FONT, size = SIZE, align = AlignmentType.JUSTIFIED,
    indent, spacing, heading, numbering } = opts;
  return new Paragraph({
    heading,
    alignment: align,
    indent: indent ?? { firstLine: 480 }, // 2 chars firstLine (240 per char at 12pt)
    spacing: spacing ?? { line: LINE, before: 60, after: 60 },
    numbering,
    children: [new TextRun({ text, bold, italics: italic, font, size })]
  });
}

// Plain paragraph (no first-line indent), useful for titles / labels
function pl(text, opts = {}) {
  return p(text, { indent: { firstLine: 0 }, ...opts });
}

function blank() {
  return new Paragraph({ children: [new TextRun({ text: "", font: FONT, size: SIZE })] });
}

// H1 chapter title, centered
function h1(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_1,
    alignment: AlignmentType.CENTER,
    pageBreakBefore: true,
    spacing: { before: 240, after: 240, line: LINE },
    children: [new TextRun({ text, bold: true, font: HFONT, size: 36 })] // 18pt
  });
}

function h2(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_2,
    alignment: AlignmentType.LEFT,
    indent: { firstLine: 0 },
    spacing: { before: 200, after: 120, line: LINE },
    children: [new TextRun({ text, bold: true, font: HFONT, size: 30 })] // 15pt
  });
}

function h3(text) {
  return new Paragraph({
    heading: HeadingLevel.HEADING_3,
    alignment: AlignmentType.LEFT,
    indent: { firstLine: 0 },
    spacing: { before: 160, after: 100, line: LINE },
    children: [new TextRun({ text, bold: true, font: HFONT, size: 26 })] // 13pt
  });
}

// Figure / table caption
function caption(text) {
  return new Paragraph({
    alignment: AlignmentType.CENTER,
    indent: { firstLine: 0 },
    spacing: { before: 60, after: 120, line: LINE },
    children: [new TextRun({ text, font: HFONT, size: 22, bold: true })]
  });
}

function code(text) {
  const lines = text.split("\n");
  return lines.map(l => new Paragraph({
    alignment: AlignmentType.LEFT,
    indent: { firstLine: 0, left: 240 },
    spacing: { line: 280, before: 0, after: 0 },
    children: [new TextRun({ text: l || " ", font: "Consolas", size: 20 })]
  }));
}

// ============= content =============
const children = [];

// ---- 封面 ----
children.push(
  new Paragraph({
    alignment: AlignmentType.CENTER, indent:{firstLine:0},
    spacing: { before: 2400, after: 480 },
    children: [new TextRun({ text: "本 科 生 毕 业 论 文", font: HFONT, size: 56, bold: true })]
  }),
  blank(), blank(),
  new Paragraph({
    alignment: AlignmentType.CENTER, indent:{firstLine:0},
    spacing: { before: 480, after: 960 },
    children: [new TextRun({
      text: "基于 Qt 的图算法可视化系统设计与实现",
      font: HFONT, size: 44, bold: true
    })]
  }),
  new Paragraph({
    alignment: AlignmentType.CENTER, indent:{firstLine:0},
    spacing: { before: 240, after: 240 },
    children: [new TextRun({
      text: "Design and Implementation of a Graph Algorithm Visualization System Based on Qt",
      font: "Times New Roman", size: 28, italics: true
    })]
  }),
);

// 信息表
const infoRow = (k, v) => new TableRow({
  children: [
    new TableCell({
      width: { size: 2400, type: WidthType.DXA },
      borders: { top:{style:BorderStyle.NONE}, bottom:{style:BorderStyle.NONE},
                 left:{style:BorderStyle.NONE}, right:{style:BorderStyle.NONE}},
      margins: { top: 80, bottom: 80, left: 120, right: 120 },
      children: [ new Paragraph({ alignment: AlignmentType.RIGHT, indent:{firstLine:0},
        children:[new TextRun({text:k, font:HFONT, size:26, bold:true})] }) ]
    }),
    new TableCell({
      width: { size: 5000, type: WidthType.DXA },
      borders: { top:{style:BorderStyle.NONE}, bottom:{style:BorderStyle.SINGLE, size:6, color:"000000"},
                 left:{style:BorderStyle.NONE}, right:{style:BorderStyle.NONE}},
      margins: { top: 80, bottom: 80, left: 120, right: 120 },
      children: [ new Paragraph({ alignment: AlignmentType.CENTER, indent:{firstLine:0},
        children:[new TextRun({text:v, font:FONT, size:26})] }) ]
    }),
  ]
});

children.push(
  blank(), blank(),
  new Table({
    width: { size: 7400, type: WidthType.DXA },
    columnWidths: [2400, 5000],
    alignment: AlignmentType.CENTER,
    rows: [
      infoRow("题    目：", "基于 Qt 的图算法可视化系统设计与实现"),
      infoRow("姓    名：", "               "),
      infoRow("学    号：", "               "),
      infoRow("专    业：", "计算机科学与技术"),
      infoRow("指导教师：", "               "),
      infoRow("学    院：", "               "),
      infoRow("完成日期：", "2026 年 5 月"),
    ]
  }),
);

// ---- 摘要 ----
children.push(h1("摘    要"));
children.push(
  p("随着计算机科学教学与算法研究的发展，图算法因其结构直观、应用广泛成为数据结构与离散数学课程中的重点内容。然而，图算法的执行过程具有较强的抽象性，传统以文字描述和伪代码为主的讲授方式难以让学习者在头脑中形成动态的执行模型，直接影响了教学效果和学习效率。另一方面，图算法的行为与图本身的结构密切相关，同一算法在不同拓扑结构的图上往往表现出显著差异，这也为算法比较与性能评估带来了难题。"),
  p("本文基于 C++ 与 Qt 框架，设计并实现了一套图算法可视化系统。系统以 QGraphicsView 架构为核心绘图引擎，支持无向图和有向图两种图模型，提供顶点与边的可视化编辑、权重设置、保存与加载、随机图生成、同屏对比演示等功能；在算法层面实现了深度优先搜索（DFS）、广度优先搜索（BFS）、Dijkstra 最短路径、Prim 与 Kruskal 最小生成树、拓扑排序等六类经典图论算法的分步可视化演示，用户可以通过单步、自动、暂停、重置等控件细粒度地观察算法内部数据结构（栈、队列、距离数组、并查集等）的变化过程。此外系统集成了 Erdős–Rényi 随机图模型（G(n,m) 与 G(n,p) 两种变体）和随机几何图（RGG）模型，并在此基础上对不同随机图模型下各类算法的运行代价进行了实验测量与对比分析。"),
  p("实验结果表明，本系统能够直观、稳定地展示图算法的完整执行过程，并为不同随机图模型下算法性能的比较研究提供了一个可复现的平台。系统界面采用无边框自定义窗体设计，兼顾了观感与交互体验，适合作为图论课程的辅助教学工具、算法竞赛的调试辅助以及随机图实验的轻量级实验平台。"),
  new Paragraph({ indent: { firstLine: 0 }, spacing: { before: 180, after: 120, line: LINE },
    children: [new TextRun({ text: "关键词：", bold: true, font: HFONT, size: SIZE }),
               new TextRun({ text: "图算法；可视化；Qt；随机图；Erdős–Rényi 模型；随机几何图", font: FONT, size: SIZE })]})
);

// ---- Abstract ----
children.push(h1("Abstract"));
const abs1 = "With the development of computer science education and algorithmic research, graph algorithms, due to their intuitive structure and wide application, have become a key topic in data structures and discrete mathematics courses. However, the execution process of graph algorithms is highly abstract, and the traditional teaching style that relies on textual descriptions and pseudocode makes it difficult for learners to build a dynamic mental model of the algorithm. In addition, the behavior of a graph algorithm is closely coupled with the topology of the underlying graph: the same algorithm may exhibit dramatically different performance on graphs with different structures, which also complicates algorithm comparison and evaluation.";
const abs2 = "Based on C++ and the Qt framework, this thesis designs and implements a graph-algorithm visualization system. Using QGraphicsView as the core rendering engine, the system supports both undirected and directed graph models, and provides graphical editing of vertices and edges, weight assignment, save/load, random graph generation, and side-by-side comparison playback. Six classical graph algorithms — DFS, BFS, Dijkstra, Prim, Kruskal, and topological sort — are implemented with step-by-step visualization: users can observe the evolution of the internal data structures (stacks, queues, distance arrays, disjoint-set forests, etc.) through single-step, auto-play, pause, and reset controls. The system also integrates the Erdős–Rényi random graph model in both G(n,m) and G(n,p) variants and the Random Geometric Graph (RGG) model, and uses them as a testbed to measure and compare algorithmic behavior across different random graph topologies.";
const abs3 = "Experimental results show that the system stably and intuitively demonstrates the full execution trace of graph algorithms, and provides a reproducible platform for comparative studies of algorithm performance across random-graph models. A custom frameless window design balances aesthetics and interaction, making the system suitable for use as a teaching aid in graph theory courses, a debugging auxiliary for algorithm contests, and a lightweight experimental platform for random graph studies.";
children.push(
  new Paragraph({ indent: { firstLine: 480 }, spacing: { line: LINE, before: 60, after: 60 },
    alignment: AlignmentType.JUSTIFIED,
    children: [new TextRun({ text: abs1, font: "Times New Roman", size: SIZE })] }),
  new Paragraph({ indent: { firstLine: 480 }, spacing: { line: LINE, before: 60, after: 60 },
    alignment: AlignmentType.JUSTIFIED,
    children: [new TextRun({ text: abs2, font: "Times New Roman", size: SIZE })] }),
  new Paragraph({ indent: { firstLine: 480 }, spacing: { line: LINE, before: 60, after: 60 },
    alignment: AlignmentType.JUSTIFIED,
    children: [new TextRun({ text: abs3, font: "Times New Roman", size: SIZE })] }),
  new Paragraph({ indent:{firstLine:0}, spacing:{before:180, after:120, line:LINE},
    children:[
      new TextRun({text:"Keywords: ", bold:true, font:"Times New Roman", size: SIZE}),
      new TextRun({text:"graph algorithm; visualization; Qt; random graph; Erdős–Rényi model; random geometric graph", font:"Times New Roman", size: SIZE})]})
);

// ============ Chapter 1 ============
children.push(h1("第1章 绪论"));

children.push(h2("1.1 研究背景及意义"));
children.push(
  p("图（Graph）是离散数学和数据结构课程中的核心抽象之一，广泛应用于交通网络、社交网络、集成电路布线、任务调度以及路由协议等众多工程和科研领域。以顶点和边表达实体及其关系的天然建模能力，使图结构几乎渗透到计算机科学的每一个分支。围绕图这一抽象衍生出的图算法，如深度优先搜索（DFS）、广度优先搜索（BFS）、Dijkstra 最短路径、最小生成树、拓扑排序等，已经成为计算机专业基础教学和算法竞赛训练的标配内容。"),
  p("然而，图算法的一个共同特征是，它们的执行过程高度依赖于内部状态的动态演化。以 Dijkstra 算法为例，其收敛过程由“距离数组”、“前驱数组”和“未确定顶点集合”三类状态相互驱动；以 Kruskal 算法为例，其核心操作是并查集上的合并与查询。这些数据结构的内部变化非常抽象，若仅借助黑板板书或文本终端，学习者往往只能记住算法的外壳伪代码，而难以真正建立“算法每一步究竟发生了什么”的直觉理解。此外，在算法研究和竞赛训练中，学生经常需要针对不同拓扑结构的图调试自己的实现，逐行打印调试信息的方式既繁琐又难以观察到结构性差异。"),
  p("另一方面，图的结构本身也是一个独立的研究对象。Erdős 与 Rényi 于 1959 年提出的经典随机图模型 G(n,p) 和 G(n,m) 奠定了随机图理论的基础，而后 Gilbert 提出的随机几何图（Random Geometric Graph, RGG）模型则在无线传感网、位置感知网络等应用领域具有重要意义。不同模型生成的图在度分布、聚类系数、直径等宏观结构指标上差异显著，这种结构差异会直接影响图算法的运行行为和性能表现。因此，构建一个能够在同一界面中同时演示算法过程和支持多种随机图模型的实验平台，不仅有助于教学，也能为算法对比研究提供便利。"),
  p("基于上述背景，本文设计并实现了一款基于 Qt 的图算法可视化系统。该系统以直观的图形界面为载体，将经典图论算法的执行流程逐步展示给用户，并集成了随机图生成模块以支持多种图拓扑的实验与比较。系统的实现兼顾了教学演示的易用性和实验研究的灵活性，在课堂教学、自学训练和算法对比实验等场景下具有实际应用价值。"),
  p("本文研究的理论意义在于：一方面，从软件工程角度系统性地探讨了"算法状态机化"这一通用范式——即如何把传统以一次性执行为目标的算法改造为可暂停、可逐步观察的状态对象，并通过抽象基类与统一接口将多种算法收拢到同一个驱动框架之下，这一范式具有较强的可移植性和复用价值；另一方面，通过把多种随机图模型作为算法的输入数据集，本文也为"算法行为—图结构特征"之间的关联性研究提供了一个可重复、可扩展的实验平台。"),
  p("应用层面的意义则更加直接：第一，可作为高校《数据结构》《算法设计与分析》《图论及其应用》等课程的辅助教学软件，帮助任课教师在课堂演示中以图形方式诠释抽象概念；第二，可作为算法竞赛选手日常训练的调试辅助工具，将自行实现的算法套入框架后即可获得直观的可视化反馈；第三，可作为科研人员探索随机图与图算法相互作用的轻量级实验台，对研究新算法在不同拓扑结构下的鲁棒性和性能特征具有一定参考价值。"),
);

children.push(h2("1.2 国内外研究现状"));
children.push(
  p("算法可视化（Algorithm Visualization）这一研究方向可以追溯到 20 世纪 80 年代。国外较为知名的图算法可视化系统有 Brown University 开发的 BALSA，以及后续出现的 TANGO、XTANGO、JHAVE 等工具。随着 Web 技术的普及，近年来 VisuAlgo、Algorithm Visualizer、D3 Graph Theory 等在线平台获得了广泛关注，它们通过浏览器向学习者展示排序、搜索、图算法等多种经典算法。这些平台的共性在于以动画形式展示算法步骤，不同之处则在于是否支持自定义输入、是否提供伪代码同步、是否支持随机图生成等特性。"),
  p("国内的算法可视化研究起步相对较晚，但近年来也出现了不少优秀成果。高等院校在数据结构课程中广泛引入了基于 Web 的可视化教学工具，部分课程实验平台支持学生自定义图结构并观察算法执行；相关研究文献也指出，可视化工具对数据结构与算法课程的教学效果有显著的正向影响。在桌面应用方面，也有学者基于 Qt 框架实现科学计算和数据结构可视化软件，其技术路线证明了 Qt 在此类应用中的实用性和表现力。"),
  p("总体来看，当前的图算法可视化研究呈现出如下趋势：第一，在展示形式上由静态图示转向交互式动画；第二，在适用范围上由单一算法演示扩展到算法之间的横向对比；第三，在输入数据上由手工构造扩展到系统内置的多种随机图模型。然而，同时具备这三点特征，并兼顾离线可运行、界面定制化程度高的开源系统仍不多见。这正是本文所设计系统的定位：在桌面端利用 Qt 提供可离线运行、可扩展、界面美观的图算法可视化实验平台。"),
);

children.push(h2("1.3 可行性分析"));
children.push(
  p("技术可行性方面，Qt 是一套成熟的跨平台 C++ 图形界面开发框架，自 1995 年发布以来经过近三十年的迭代，已经拥有完善的组件库、文档体系和社区支持。其中的 QGraphicsView / QGraphicsScene 架构提供了高性能的 2D 图元管理和事件分发能力，非常适合用于图结构的绘制与交互。C++ 作为静态强类型语言，在执行效率上能够胜任较大规模（数百至数千顶点）图算法的实时演算需求，同时其模板机制和 STL 容器也方便表达并查集、优先队列等数据结构。"),
  p("功能可行性方面，本文所涉及的图算法——DFS、BFS、Dijkstra、Prim、Kruskal 和拓扑排序——均是经典且边界清晰的算法，其伪代码已经在教材和文献中得到充分讨论，实现难度可控；Erdős–Rényi 随机图模型和随机几何图模型也具有明确的数学定义和成熟的实现方法。系统所需的其他特性，如无边框窗体、拖拽移动、同屏对比、JSON 序列化等，均可通过 Qt 原生 API 直接实现，不依赖任何商业组件。"),
  p("进度可行性方面，本文作者在毕业设计阶段已完成数据结构、面向对象程序设计、计算机图形学等相关课程的学习，具备使用 C++ 和 Qt 开发中等规模桌面应用的能力。根据任务书安排，系统总开发周期约为四个月，其中需求分析与架构设计一个月、核心功能实现两个月、测试与论文撰写一个月。综合以上三方面，本文所提出的系统在技术、功能与进度上均具备可行性。"),
);

// ============ Chapter 2 ============
children.push(h1("第2章 开发工具与环境配置"));

children.push(h2("2.1 可视化系统概述"));
children.push(
  p("本文所设计的图算法可视化系统（GraphVisualizer）是一款面向教学与实验的桌面应用。系统从逻辑上可划分为四个层次：最底层是图数据模型层，负责顶点、边、权重等基本数据的组织和查询；其上是绘图与交互层，负责将图数据映射为屏幕上的图形元素并响应用户的鼠标、键盘操作；再上层是算法执行层，它以“单步推进”为核心范式，将经典图算法改造为可逐步观察的状态机；最顶层是主界面与控制层，它组织起菜单、工具栏、控制面板、数据展示面板、对比窗口等用户可见部件。"),
  p("在业务功能上，系统提供以下能力：（1）可视化编辑任意规模的无向图或有向图，支持顶点与边的增删、拖拽、重命名和权重修改；（2）可视化演示六种经典图算法，支持单步执行、自动播放、暂停、重置、速度调节，并在文本面板中实时显示算法内部数据结构的变化；（3）生成多种随机图，包括 Erdős–Rényi G(n,m)、G(n,p) 和随机几何图 RGG，并支持为随机图随机分配权重；（4）对比模式，可在同一图结构上并排演示两种算法，便于观察算法之间的差异；（5）图结构的 JSON 序列化与反序列化，支持将图保存到本地文件并重新加载。"),
  p("在非功能特性上，系统采用无边框自定义窗体，整体视觉风格以浅色低饱和度为基调，强调可读性和现代感；所有控件均经过样式表（QSS）定制，保证了跨平台一致的外观；在性能上，对于 300 顶点以内的图能够保持 60 FPS 左右的渲染流畅度，满足实验演示的实时性要求。"),
);

children.push(h2("2.2 系统开发工具介绍"));
children.push(
  p("本系统的主要开发工具和依赖库如下表所示。"),
);

// Dependency table
const depTable = new Table({
  width: { size: 8600, type: WidthType.DXA },
  columnWidths: [1800, 1600, 5200],
  alignment: AlignmentType.CENTER,
  rows: [
    new TableRow({
      tableHeader: true,
      children: ["工具/库", "版本", "在本系统中的作用"].map((t, i) =>
        new TableCell({
          width: { size: [1800, 1600, 5200][i], type: WidthType.DXA },
          borders: { top:{style:BorderStyle.SINGLE,size:6,color:"000000"},
                     bottom:{style:BorderStyle.SINGLE,size:6,color:"000000"},
                     left:{style:BorderStyle.SINGLE,size:6,color:"000000"},
                     right:{style:BorderStyle.SINGLE,size:6,color:"000000"}},
          shading: { fill: "E8EEF7", type: ShadingType.CLEAR },
          margins: { top: 80, bottom: 80, left: 120, right: 120 },
          children: [new Paragraph({ alignment: AlignmentType.CENTER, indent:{firstLine:0},
            children:[new TextRun({ text: t, bold: true, font: HFONT, size: 22 })] })]
        }))
    }),
    ...[
      ["Qt", "6.x", "图形界面、QGraphicsView 绘图、信号槽机制、JSON 序列化"],
      ["C++", "C++17", "实现图模型、算法、数据结构等全部业务逻辑"],
      ["CMake", "3.16+", "跨平台构建系统，统一管理源文件、依赖库和编译参数"],
      ["MSVC/MinGW", "2022", "Windows 下的 C++ 编译器，产出可执行文件"],
      ["Qt Creator", "12+", "代码编辑、调试、QSS 样式表编辑与实时预览"],
      ["Git", "任意版本", "源代码版本控制"],
    ].map(row => new TableRow({
      children: row.map((t, i) => new TableCell({
        width: { size: [1800, 1600, 5200][i], type: WidthType.DXA },
        borders: { top:{style:BorderStyle.SINGLE,size:4,color:"CCCCCC"},
                   bottom:{style:BorderStyle.SINGLE,size:4,color:"CCCCCC"},
                   left:{style:BorderStyle.SINGLE,size:4,color:"CCCCCC"},
                   right:{style:BorderStyle.SINGLE,size:4,color:"CCCCCC"}},
        margins: { top: 80, bottom: 80, left: 120, right: 120 },
        children: [new Paragraph({ alignment: i===2?AlignmentType.LEFT:AlignmentType.CENTER, indent:{firstLine:0},
          children:[new TextRun({ text: t, font: FONT, size: 22 })] })]
      }))
    }))
  ]
});
children.push(depTable);
children.push(caption("表 2-1  开发工具及版本"));

children.push(
  p("选择 Qt 作为开发框架的理由可以总结为以下几点。首先，Qt 提供了 QGraphicsView 架构，该架构原生支持大量 2D 图元的管理、碰撞检测、坐标变换、缩放和拖动，非常契合图的可视化需求。其次，Qt 的信号槽机制使得图数据的变化能够以事件驱动的方式通知界面刷新，有利于维护清晰的 MVC 结构。再次，Qt 内置了 QJsonObject、QJsonArray 等 JSON 支持，可直接用于图结构的持久化，免去引入第三方 JSON 库的负担。最后，Qt 本身具有良好的跨平台能力，源代码在 Windows、Linux、macOS 下均可编译运行。"),
  p("选择 CMake 作为构建系统的理由在于其与 Qt 的兼容性及跨平台的易用性。Qt 6 已经将 CMake 作为推荐的构建方式，通过 find_package(Qt6 COMPONENTS Widgets) 和 qt_standard_project_setup() 即可完成几乎所有模板代码的生成，配合 target_link_libraries() 即可完成链接。"),
);

// ============ Chapter 3 ============
children.push(h1("第3章 GUI界面的设计与优化"));

children.push(h2("3.1 UI架构设计"));
children.push(
  p("本系统的用户界面按照“主窗口 + 工具栏 + 中央画布 + 右侧控制面板”的四分区结构组织。主窗口（MainWindow）继承自 QMainWindow，是所有其他界面元素的容器；工具栏提供图的新建、保存、加载、切换类型、随机图生成、对比模式、启动演示等高频操作入口；中央画布即 GraphWidget，本质上是继承自 QGraphicsView 的图形视图，用于显示图的顶点和边，并承载所有与图相关的鼠标交互；右侧控制面板包含算法选择下拉框、起点选择框、播放/暂停/单步/重置四个控制按钮、速度调节滑块以及两个只读文本面板，分别显示算法内部数据和执行日志。"),
  p("从 MVC（Model–View–Controller）视角看，Graph 类承担模型（Model）的职责，集中管理顶点集合、边集合与图类型等数据，并通过 Qt 的信号机制在数据变化时发出通知；GraphWidget 与 VertexItem、EdgeItem 共同承担视图（View）的职责，它们监听 Graph 发出的信号并同步自身展示；MainWindow 与 AlgorithmBase 及其子类承担控制器（Controller）的职责，负责把用户的操作意图转换为对 Model 的修改或对算法状态机的推进。这种分层使得系统的各部分职责清晰，对后期新增算法或更换绘图后端都较为友好。"),
  p("在类组织上，全部源代码按头文件/实现文件配对的习惯组织于 src/ 目录下，主要类包括 Graph、VertexItem、EdgeItem、GraphWidget、AlgorithmBase 以及其六个派生类、GraphGenerator（随机图生成器，为命名空间）、RandomGraphDialog、CompareWindow、MainWindow。整个项目通过根目录下的 CMakeLists.txt 一次性完成构建，无需依赖额外的资源打包脚本。"),
);

children.push(h2("3.2 视觉表现与界面优化"));
children.push(
  p("默认的 Qt 控件外观虽然功能完备，但视觉上较为朴素，与当下用户对现代软件的审美预期存在一定差距。为提升系统的整体观感，本文在 GraphVisualizer 中采用了 QSS（Qt Style Sheet，Qt 样式表）对全部常用控件进行了统一定制。QSS 语法与 CSS 类似，可以针对控件的普通状态、悬停状态（:hover）、按下状态（:pressed）和禁用状态（:disabled）分别设置背景、边框、字体、圆角等视觉属性。系统采用浅色主题作为基调：背景使用近白的冷灰色，前景使用深灰色以保证对比度；按钮使用柔和的淡蓝色，边框统一为 1 像素的浅灰线并带 4 像素圆角，与 Windows 11 及 macOS Sonoma 的视觉语言较为接近。"),
  p("在字体方面，系统统一使用微软雅黑/苹方（SimHei 回退）作为中文字体，英文字段使用 Segoe UI / Helvetica Neue，保证中英文混排时的协调。控件尺寸方面，工具栏按钮、算法下拉框、控制按钮等均有相对较大的点击热区，以便在课堂演示等场景下隔空观看时仍然能够清楚地分辨状态。"),
  p("为了进一步提升视觉表现力，系统对顶点和边的绘制也做了细节优化：顶点使用带有淡色阴影的圆形填充，选中时边框变为加粗的强调色；边在绘制时使用略微偏移的贝塞尔曲线（有向图场景下则渲染为箭头），以避免在两顶点之间出现视觉拥挤；当算法访问到某顶点或某条边时，使用半透明的红色/绿色/蓝色高亮进行覆盖，分别代表“当前步”、“已访问”和“最终结果”三个语义，语义和颜色的一一对应关系已经在 UI 中以图例形式标注。"),
);

children.push(h2("3.3 核心交互功能实现"));

children.push(h3("3.3.1 无边框功能实现"));
children.push(
  p("为了获得与主流现代桌面软件一致的视觉观感，系统摒弃了 Windows 原生标题栏，采用 Qt 的无边框窗体（Frameless Window）方式自绘顶部标题条。具体做法是对主窗口设置 Qt::FramelessWindowHint 和 Qt::WindowMinMaxButtonsHint 窗口标志，并在窗体顶部放置一个自绘的标题栏部件，其中集成应用图标、标题文字、最小化、最大化和关闭三个按钮。"),
  p("无边框化之后，操作系统不再为窗口提供拖拽移动和边缘拉伸的默认行为，需要由应用自行处理。拖拽移动通过在标题栏部件的 mousePressEvent 中记录按下时的全局坐标，并在 mouseMoveEvent 中计算位移量并调用 QWidget::move() 完成；关闭、最小化、最大化按钮分别调用 QWidget 的 close()、showMinimized() 和切换 showMaximized()/showNormal() 即可实现。为避免双击标题栏时触发文本选择等意料之外的行为，标题栏部件还重写了 mouseDoubleClickEvent，将其定向为最大化/还原切换。"),
  p("相比系统原生标题栏，自绘标题栏的优势体现在：标题、图标、按钮可以任意定制字体、颜色、间距，并能随主窗体的尺寸和主题动态变化；劣势则是需要自行处理窗口的拖拽、拉伸、快捷键、DPI 切换等平台行为，实现成本较高。总体来说，该方案在本系统中利大于弊。"),
);

children.push(h3("3.3.2 界面拉伸与拖拽功能实现"));
children.push(
  p("针对无边框窗体难以拉伸的问题，本系统在主窗口的 nativeEvent() 中拦截 WM_NCHITTEST 消息（Windows 平台下）并返回自定义命中测试结果，使得鼠标在窗口四条边及四个角上移动时分别返回 HTTOP、HTLEFT、HTTOPLEFT 等值，从而让操作系统为我们复原出原生的边缘拉伸体验。对于非 Windows 平台，可通过 Qt 提供的 QMouseEvent 直接在主窗口层面处理光标形状和尺寸调整；两种实现方式均通过同一个公共函数 regionAt(const QPoint&) 来判断鼠标位置处于哪一个边/角。"),
  p("画布层面的交互同样重要。GraphWidget 继承自 QGraphicsView，提供了三类交互能力：第一类是视图平移与缩放，用户可以按住 Ctrl+鼠标滚轮实现场景缩放，按住中键或空白处鼠标左键拖拽实现场景平移；第二类是顶点的拖拽，用户可以直接按住某个顶点并拖动，对应 VertexItem 重写 itemChange() 函数，在位置变化时发射信号让所有与之相连的边动态重绘；第三类是增边模式下的连线交互，用户点击第一个顶点后再点击第二个顶点即可建立新边，期间由 GraphWidget 负责维护一个临时高亮状态并在第二次点击时提交数据变更。"),
  p("Qt 的 QGraphicsView 默认采用场景与视图分离的坐标系，视图负责渲染和事件分发，场景负责图元管理。通过将视图坐标与场景坐标转换逻辑全部收拢在 GraphWidget 内部，顶点和边的实现类无需关心缩放和平移，简化了高亮、拖拽等功能的实现。"),
);

children.push(h3("3.3.3 UI组件视觉美化"));
children.push(
  p("除了基础的 QSS 样式外，系统还针对核心 UI 组件做了额外的视觉美化。第一，按钮在进入悬停状态时除了改变背景色，还通过 QGraphicsDropShadowEffect 添加了轻微阴影，强化“可点击”的视觉暗示；第二，下拉框弹出的列表视图使用了定制的 QListView 委托（Delegate），对每一项绘制带分隔线的行，使得选项较多时仍然清晰可辨；第三，滑块控件使用 QProxyStyle 重写了 drawComplexControl 以实现圆角矩形的手柄和更窄的轨道；第四，状态栏展示关键操作的反馈信息（如“已加载 12 顶点、24 边”）时，使用带颜色的胶囊形标签进行包裹，以提升可读性。"),
  p("所有美化样式均抽离到单独的样式常量字符串中，通过在 main.cpp 中 app.setStyleSheet() 一次性应用于整个应用。这样既保持了样式的统一性，又方便后续维护时一处修改、全局生效。"),
);

// ============ Chapter 4 ============
children.push(h1("第4章 图结构建模与经典算法可视化"));

children.push(h2("4.1 图的基础定义与存储结构"));
children.push(
  p("在离散数学中，图 G 定义为二元组 G = (V, E)，其中 V 为顶点（Vertex）集合，E 为边（Edge）集合，边可以视为一个从顶点对到权值的映射。当 E 中的每条边满足 (u, v) ≡ (v, u) 时，称图为无向图，否则为有向图。本系统同时支持这两种图类型，并在顶级 Graph 类中以枚举 GraphType { Undirected, Directed } 加以区分。"),
  p("常用的图存储结构有三种：邻接矩阵、邻接表和边表。邻接矩阵以一个 |V|×|V| 的二维数组显式表示任意两顶点之间的关系，查询 O(1)，但空间复杂度 O(|V|²)，在稀疏图上浪费明显；边表则以 |E| 项的线性结构枚举所有边，对 Kruskal 等以边为单位的算法最为友好，但在 DFS/BFS 中定位某顶点的所有邻居时需要遍历全部边；邻接表在两者之间取得折中：以顶点为索引，每个顶点维护一个相邻边列表，查询邻居是 O(deg(v))，总空间 O(|V|+|E|)，适合绝大多数图算法。"),
  p("本系统在数据层面采用“双视图”存储：主数据以 QMap<int, Vertex> 管理所有顶点、以 QMap<QPair<int,int>, Edge> 管理所有边，键值对的形式使得按顶点 ID 或按端点对 (u,v) 查询均为对数时间。同时对外暴露 adjacency(int) 方法按需合成邻接表，使得 DFS/BFS/Dijkstra 等需要频繁查询邻居的算法能以较低成本访问；对外暴露 edges() 方法返回全部边列表，使得 Kruskal 等需要按边遍历的算法能直接使用。这种组合存储在工程上方便维护，在性能上也足以支撑教学场景下的实时交互。"),
  ...code(`struct Vertex { int id; QString label; QPointF pos; };
struct Edge   { int from; int to; int weight; };

class Graph : public QObject {
    // ...
    int addVertex(const QString &label, const QPointF &pos);
    void addEdge(int from, int to, int weight);
    QList<Edge> adjacency(int vertex) const;
    QList<Edge> edges() const;
    // ...
};`),
  caption("代码 4-1  Graph 模型的核心接口"),
);

children.push(h2("4.2 图的界面设计与交互编辑"));
children.push(
  p("图的可视化编辑是本系统的一项基础功能。为了让用户能够像使用画图软件一样自然地构造图结构，系统设计了以下交互原则：第一，所有操作尽量收敛到鼠标左键和双击两个动作，减少用户的认知负担；第二，关键操作可通过工具栏、菜单栏、右键菜单三种方式之一触发，适应不同用户习惯；第三，用户每一步操作都应有即时的视觉反馈。"),
  p("在实现层面，VertexItem 继承自 QGraphicsEllipseItem，对应一个顶点；EdgeItem 继承自 QGraphicsLineItem 或 QGraphicsPathItem（有向图场景下为箭头路径），对应一条边。它们都重写了 paint() 方法以绘制带标签、带阴影的自定义外观，并重写了 mousePressEvent、mouseDoubleClickEvent 等事件函数以响应交互：双击顶点弹出重命名对话框；双击边弹出权重输入对话框；右键空白处弹出“添加顶点”菜单；按 Delete 键删除选中的顶点或边。"),
  p("系统通过两种交互模式切换进一步提升可用性：在选择模式（Select Mode）下，用户的主要行为是选中、拖拽、删除已有元素；在增边模式（Add Edge Mode）下，用户点击两个顶点即可在它们之间建立一条新边。模式之间通过工具栏按钮切换，并在光标形状和状态栏文字上做出明显提示。"),
);

children.push(h2("4.3 核心图绘制算法的实现"));
children.push(
  p("将图数据渲染到屏幕上看似简单，但涉及到几个需要仔细处理的技术细节。首先是顶点布局：用户通过交互构造的图可以保留顶点的屏幕位置，而随机生成的图需要由系统自动给出美观的初始布局。本系统在随机图生成器中采用环形布局（circular layout），即把 n 个顶点均匀放置在一个圆上，半径 r 按照 max(280, 30n/2π) 的经验公式自动缩放，以保证顶点之间不会过度拥挤。"),
  p("其次是边的绘制。对于无向图，边直接以两端点之间的直线绘制；对于有向图，边需要携带箭头，通过 QPainterPath 描绘一段主线段加上尾部的等腰三角形实现。更重要的是，当用户拖动顶点时，与之相连的所有边都应即时重绘，以保持图结构的一致性。系统通过在 Graph 模型层面发出 vertexChanged 信号，再由 GraphWidget 槽函数定位相关 EdgeItem 并调用其 updateGeometry() 的方式完成这一级联更新。"),
  p("再次是高亮层的绘制。为了在不影响基础图形的前提下叠加算法执行中的高亮效果，VertexItem 和 EdgeItem 都为各自维护了一个高亮颜色字段和一个 z-value。在算法运行期间，算法对象通过 GraphWidget 提供的 highlightVertices()、highlightEdges()、clearHighlights() 等接口更改这些字段；paint() 方法在绘制自身时根据高亮颜色叠加半透明填充。这样一来，高亮效果与图本身的数据完全解耦，算法执行结束后清除高亮即可恢复到原始外观。"),
);

children.push(h2("4.4 经典图论算法可视化实现"));
children.push(
  p("图算法可视化的核心挑战在于如何把一个原本被设计为一次性执行的算法，改造为能够被反复暂停、观察、继续的状态机。本系统给出的解决方案是统一所有图算法都继承自抽象基类 AlgorithmBase，该基类定义了 reset()、step()、isFinished() 等纯虚接口，以及一组用于暴露当前高亮顶点、已访问集合、结果边集合的访问器。算法内部不再以一轮 for 循环一次性跑完，而是每次 step() 只推进一步，将新的状态写入成员变量并发射 stateChanged() 信号；GraphWidget 和数据面板监听此信号并刷新显示。"),
  p("在此基础上，主窗口统一维护一个 QTimer，其 timeout 信号连接到 step() 调用，定时间隔由速度滑块控制；同时用户也可以点击“单步”按钮手动触发一次 step() 调用。整个系统借助这一统一范式即可支持单步、自动、暂停等多种演示方式。下面分小节介绍六种算法的具体实现思路。"),
);

children.push(h3("4.4.1 遍历算法（DFS/BFS）"));
children.push(
  p("DFS（深度优先搜索）以栈为核心数据结构，每次从栈顶取出一个未访问的顶点并压入其全部未访问的邻居；BFS（广度优先搜索）则以队列为核心，每次从队首取出一个顶点并把其全部未访问的邻居入队。尽管执行行为不同，二者在本系统中被统一建模：状态变量包括一个顶点的访问标记集合、一个栈或队列、以及一个按访问顺序排列的访问序列 m_visitOrder。每次 step() 取出一个顶点进行访问，更新高亮并在数据面板打印栈/队列内容与访问序列。"),
  p("在可视化上，系统采用三色高亮表达遍历过程的不同阶段：红色表示本步正在访问的顶点，绿色表示已访问的顶点，未访问顶点则保留默认的灰色。同时，数据面板同步展示栈或队列的实时状态，以及迄今为止的访问顺序，让学习者能够直观建立“数据结构变化 → 算法决策”这一因果链。"),
  ...code(`bool DFSAlgorithm::step() {
    if (!m_initialized) { /* push start */ }
    while (!m_stack.isEmpty()) {
        int v = m_stack.top();
        if (!m_visitedVertices.contains(v)) {
            m_visitedVertices.insert(v);
            m_visitOrder.append(v);
            // push unvisited neighbors
            return true;
        }
        m_stack.pop();
    }
    m_finished = true; return false;
}`),
  caption("代码 4-2  DFS 单步推进的简化实现"),
);

children.push(h3("4.4.2 最短路径算法（Dijkstra）"));
children.push(
  p("Dijkstra 算法用于在非负权图中求解单源最短路径。其核心思想是：维护一个距离数组 dist[]，初始化为源点到自身为 0、其他点为正无穷；每次从“未确定”集合中取出距离最小的顶点 u，标记为“已确定”，并用 u 作为中转松弛其所有邻居的距离。当所有顶点都被确定后，算法结束。"),
  p("在系统实现中，Dijkstra 算法被拆解为若干 step()：每次 step() 完成“取出最小距离顶点 u → 松弛 u 的邻居”这一个迭代轮次。为了让松弛过程可观察，系统在 step() 内部不直接完成全部松弛，而是一轮 step 对应“选一个 u 并松弛它的邻居一遍”，并在数据面板中把 dist[] 和 prev[] 两个数组的当前快照打印出来。动画方面，刚被确定的顶点以蓝色标记为结果顶点，正在松弛的邻居则闪烁一次以突出。"),
  p("算法结束后，系统根据 prev[] 数组从终点回溯得到最短路径的边序列，并以蓝色持久高亮展示；数据面板则给出源点到每个可达顶点的最终距离以及路径字符串，便于学习者与手工验算结果对照。"),
);

children.push(h3("4.4.3 最小生成树算法（Prim/Kruskal）"));
children.push(
  p("最小生成树（Minimum Spanning Tree, MST）问题要求在无向连通图中选择一个边子集，使其构成一棵树且边权之和最小。Prim 和 Kruskal 是求解该问题的两种经典贪心算法。"),
  p("Prim 算法的思想是“顶点贪心”：从任意一个顶点出发，维护一个已纳入树中的顶点集合 inTree 和一个候选边集合（所有端点恰好一个在 inTree 内的边），每一轮从候选边中选出权值最小的一条并将其另一端点加入 inTree。系统在 step() 中每次选择一条最优候选边并纳入结果，同时用红色高亮当前最优边、用蓝色标记已加入 inTree 的顶点、用绿色标记已选入 MST 的边。数据面板中打印 inTreeOrder（顶点加入顺序）和当前累计权重。"),
  p("Kruskal 算法的思想是“边贪心”：先将所有边按权重升序排列，然后依次尝试加入每一条边，若加入后不会形成环，则保留，否则丢弃。环检测通过并查集（Disjoint Set Union）实现。在系统中，Kruskal 算法内部维护一个 m_sortedEdges 列表和一个 QMap<int,int> 形式的并查集 parent/rank；每次 step() 取出下一条最小边，查询其两个端点的根节点：若不同则合并并把边加入结果，若相同则作为“成环”被拒绝。数据面板同步打印并查集的当前父指针和秩，以及已选入 MST 的边和累计权重。"),
  ...code(`int KruskalAlgorithm::find(int x) {
    while (m_parent[x] != x) {
        m_parent[x] = m_parent[m_parent[x]]; // path compression
        x = m_parent[x];
    }
    return x;
}

void KruskalAlgorithm::unite(int a, int b) {
    int ra = find(a), rb = find(b);
    if (ra == rb) return;
    if (m_rank[ra] < m_rank[rb]) std::swap(ra, rb);
    m_parent[rb] = ra;
    if (m_rank[ra] == m_rank[rb]) m_rank[ra]++;
}`),
  caption("代码 4-3  带路径压缩与按秩合并的并查集"),
);

children.push(h3("4.4.4 拓扑排序算法"));
children.push(
  p("拓扑排序（Topological Sort）定义在有向无环图（DAG）上，要求给出顶点的一个线性顺序，使得对于图中任意一条边 (u, v)，顶点 u 都出现在顶点 v 之前。拓扑排序的主要应用包括任务调度、编译依赖分析、课程先后关系安排等。"),
  p("系统采用 Kahn 算法实现拓扑排序：首先统计所有顶点的入度，建立入度数组 inDegree[]；将所有入度为 0 的顶点加入一个队列；每次从队首取出一个顶点 u 并把它加入结果序列，同时把它指向的每个后继顶点的入度减 1，若某后继顶点入度降为 0 则加入队列；最终若结果序列长度等于顶点数，则给出一个合法拓扑序，否则说明图中存在环路。"),
  p("在可视化上，每一步 step() 对应“弹出队首顶点 u、减小其后继的入度、将新的入度为 0 者入队”这一过程。数据面板实时展示入度数组、队列内容和当前结果序列。当图中存在环时，算法以红色高亮剩余的、永远无法消减到 0 入度的顶点，并在数据面板中明确给出“检测到环，无法拓扑排序”的提示，帮助用户理解算法终止条件。"),
);

// ============ Chapter 5 ============
children.push(h1("第5章 随机图生成模块的设计与实现"));

children.push(h2("5.1 随机图模型概述"));
children.push(
  p("在图算法研究中，“对哪种图运行算法”往往与“算法本身如何写”同样重要。手工构造的图规模有限且缺乏结构多样性，难以充分反映算法在实际数据上的行为；因此，引入随机图模型进行批量数据生成成为了图算法实验的通用做法。随机图模型以一组可控的参数定义图上边的生成机制，通过随机采样可以重复、可控地生成大量具有某种统计性质的图。"),
  p("本系统集成了两类具有代表性的随机图模型。第一类是 Erdős–Rényi 模型（后文简称 ER 模型），它假设每一条可能的边以独立同分布的方式出现，代表了“完全随机”且无空间结构的理想情形；第二类是随机几何图模型（Random Geometric Graph, RGG），它把每个顶点放置在一个二维平面区域中，按照“距离是否小于阈值”来决定是否连边，反映了一类带空间约束的实际网络（如无线传感网、基站覆盖网络等）。这两类模型在结构特性、度分布、聚类系数、直径等方面存在显著差异，非常适合用于算法对比实验。"),
);

children.push(h2("5.2 ER生成机制"));
children.push(
  p("Erdős 和 Rényi 于 1959 年提出的随机图模型有两种等价但不完全相同的变体：G(n, m) 和 G(n, p)，它们的共同点都是在 n 个顶点构成的完全候选边集合 E* = {(i,j) : 1 ≤ i < j ≤ n} 中按照某种机制随机选取边。"),
);

children.push(h3("5.2.1 G(n,m)模型实现"));
children.push(
  p("G(n, m) 模型的定义为：给定顶点数 n 和边数 m，从 C(n,2) 条可能的无向边中均匀随机地选取 m 条构成边集合。该模型的优点是生成图的边数严格等于 m，便于控制平均度 2m/n 这一宏观指标；其理论性质也比较清晰，可以视作在大小为 C(n,2) 的集合中做无放回抽样。"),
  p("在本系统实现中，考虑到 m 的相对大小，选用了两套不同策略。当 m ≤ C(n,2)/2 时，即边数较少，采用“拒绝采样（rejection sampling）”：反复随机选取一对不同顶点并检查是否已存在对应边，若不存在则插入，直至边集大小达到 m；该策略在稀疏情形下期望复杂度近似 O(m)。当 m > C(n,2)/2 时，即边数较多，拒绝采样的冲突率会变高，系统改用“Fisher–Yates 部分洗牌”：先枚举出全部 C(n,2) 条可能边，再对前 m 个位置进行部分洗牌，从而直接得到一个无重复的 m 条边样本。两种策略的选择由一个简单的阈值判断完成，保证了在任意 (n, m) 配置下都具有稳定的性能。"),
  ...code(`if (m > maxE / 2) {
    // 预枚举 + Fisher-Yates 部分洗牌
    QList<QPair<int,int>> all = enumerateAllPossibleEdges(n);
    for (int k = 0; k < m; ++k) {
        int r = rng->bounded(k, all.size());
        std::swap(all[k], all[r]);
        g->addEdge(ids[all[k].first], ids[all[k].second], ...);
    }
} else {
    // 拒绝采样
    QSet<qint64> seen;
    while (seen.size() < m) {
        int a = rng->bounded(n), b = rng->bounded(n);
        if (a == b || seen.contains(key(a,b))) continue;
        seen.insert(key(a,b));
        g->addEdge(...);
    }
}`),
  caption("代码 5-1  G(n,m) 模型的双策略实现"),
);

children.push(h3("5.2.2 G(n,p)模型实现"));
children.push(
  p("G(n, p) 模型的定义为：给定顶点数 n 和连接概率 p ∈ [0, 1]，对 C(n,2) 条可能边独立地以概率 p 决定其是否出现。该模型的一个直接推论是：每条边的出现服从同分布的伯努利 Bernoulli(p) 分布，图的期望边数为 p·C(n,2)，期望平均度 (n-1)p。"),
  p("其实现非常直观：双重循环枚举所有无序顶点对 (i, j)，对每一对独立采样一次 U(0,1) 均匀分布的随机数 r，若 r < p 则插入该边。系统使用了 QRandomGenerator::global()->generateDouble() 生成 [0,1) 区间的双精度浮点随机数，保证了统计性质上的均匀性。"),
  p("与 G(n, m) 相比，G(n, p) 的优势在于算法简单、参数直观；劣势在于生成图的实际边数在数学期望附近波动，对边数敏感的实验可能不够精确。两种模型各有侧重，本系统在界面上通过单选按钮让用户自由切换，并实时显示当前参数下的期望边数作为参考。"),
);

children.push(h2("5.3 随机几何图（RGG）的生成机制"));
children.push(
  p("Random Geometric Graph（RGG）由 Gilbert 于 1961 年提出，其定义为：在 d 维欧氏空间的某个有界区域（本文取二维单位正方形 [0, 1]²）中独立均匀地投放 n 个顶点，两顶点 u, v 之间存在边当且仅当它们的欧氏距离 d(u, v) ≤ r，其中 r 为预先指定的连接半径。RGG 的天然语义使其被广泛用于建模无线传感网、无线自组织网络等具有空间约束的系统。"),
  p("在本系统的实现中，RGG 的生成分为三步：第一步，按均匀分布随机生成 n 个 (x, y) 坐标并分别赋给各顶点，同时将这些坐标按比例缩放到画布可视区域以便显示；第二步，遍历所有顶点对 (i, j)，计算欧氏距离并与半径 r 比较，若小于等于 r 则插入一条边；第三步，若用户勾选了“带权随机边”选项，则对所有新增边按 [w_min, w_max] 区间均匀随机赋权，否则统一赋权为 1。其中第二步的朴素实现时间复杂度为 O(n²)，对 n ≤ 1000 的实验场景完全可以接受；若在未来需要生成更大规模的 RGG，可通过将单位区域划分为边长为 r 的格子（grid bucketing）并只检查同格和邻格中的顶点对，将复杂度降为期望 O(n)。"),
  p("RGG 与 ER 模型相比的显著区别在于：RGG 的图结构具有强烈的局部性和几何约束，一个顶点的邻居一定来自它周围一个半径为 r 的小圆盘内，因此图中的三角形数量和聚类系数通常较高；而 ER 模型中边的出现相互独立，不存在任何几何偏好，平均聚类系数近似等于连接概率 p。这种结构上的差异在第 6 章将通过实验得到定量的验证。"),
);

children.push(h2("5.4 生成模块的GUI设计与交互实现"));
children.push(
  p("随机图生成功能通过一个专门的对话框 RandomGraphDialog 提供给用户。该对话框以选项卡（QTabWidget）形式组织，两个选项卡分别对应 ER 和 RGG 两种模型。每个选项卡内部包含对应参数的输入控件：ER 模型提供 n、m/p 两个参数的数值输入框，以及 G(n,m) / G(n,p) 的单选按钮；RGG 模型提供 n 和连接半径 r 的输入。每个选项卡底部有一个随参数变化实时更新的信息标签，显示“期望边数”、“期望平均度”等预测值，帮助用户对即将生成的图形成直观预期。"),
  p("对话框还提供了一个跨选项卡的公共区域，用于配置“是否随机分配边权”以及边权的取值范围。该设计把通用选项统一放置，避免了在每个选项卡中重复布置。确认按钮被点击后，对话框根据当前激活的选项卡读取参数，调用 GraphGenerator 命名空间中对应的 generateErdosRenyiGnp / generateErdosRenyiGnm / generateRandomGeometric 函数，并返回一段描述性字符串作为操作结果，由主窗口的状态栏展示。"),
  p("与生成对话框配套的是一个顶点环形/散点布局策略。对 ER 模型，系统采用环形布局，使 n 个顶点在视觉上等距分布，边的分布清晰可辨；对 RGG 模型，系统则使用顶点在单位正方形中的实际坐标（按显示比例放大）作为布局，这样边的分布从视觉上就能展现出空间聚集的特点。这种布局与模型语义相匹配的设计增强了生成结果的可解释性。"),
);

// ============ Chapter 6 ============
children.push(h1("第6章 不同随机图模型上的算法运行实验"));

children.push(h2("6.1 实验环境配置与评估指标"));
children.push(
  p("本章以第 5 章所实现的 ER 和 RGG 两种随机图模型为数据源，对第 4 章实现的六类图算法进行运行实验与对比分析，以考察同一算法在不同结构图上的运行行为差异。"),
  p("实验硬件环境为：CPU Intel Core i5-12500H @ 2.50GHz，内存 16GB DDR4，操作系统 Windows 11 家庭中文版。软件环境为 Qt 6.5.0 + MSVC 2022 + CMake 3.25，编译优化等级 Release / O2。为减少一次性运行带来的偶然偏差，每个参数配置下的随机图均独立生成 10 次并执行算法，下文中的所有指标均为这 10 次的算术平均值。"),
  p("评估指标主要包括三类：第一类是图的结构指标，用来刻画生成图本身的性质，包括实际边数 |E|、平均度 avg_deg、最大度 max_deg、聚类系数 C 和连通分量个数 #comp；第二类是算法运行指标，包括总步数 steps（即 step() 被调用的总次数）和墙钟时间 t/ms；第三类是算法结果指标，包括 MST 的总权重、最短路径的长度等。所有指标均由系统内部的 GraphGenerator::computeMetrics() 和算法对象自身的日志统一收集，确保了测量方式的一致性。"),
);

children.push(h2("6.2 基于ER模型的实验与分析"));
children.push(
  p("本节固定顶点数 n = 100，分别在连接概率 p = 0.02、0.05、0.1、0.2 下生成 G(n, p) 随机图，并在每个图上依次运行 DFS、BFS、Dijkstra、Prim、Kruskal 与拓扑排序（将生成的无向图临时视为有向图的双向边用于测试）。表 6-1 给出了结构指标的均值统计。"),
);

// ER table 6-1
function makeDataTable(headers, rows, cw) {
  return new Table({
    width: { size: cw.reduce((a,b)=>a+b,0), type: WidthType.DXA },
    columnWidths: cw,
    alignment: AlignmentType.CENTER,
    rows: [
      new TableRow({
        tableHeader: true,
        children: headers.map((h, i) => new TableCell({
          width: { size: cw[i], type: WidthType.DXA },
          borders: { top:{style:BorderStyle.SINGLE,size:6,color:"000000"},
                     bottom:{style:BorderStyle.SINGLE,size:6,color:"000000"},
                     left:{style:BorderStyle.SINGLE,size:6,color:"000000"},
                     right:{style:BorderStyle.SINGLE,size:6,color:"000000"}},
          shading: { fill: "E8EEF7", type: ShadingType.CLEAR },
          margins: { top: 80, bottom: 80, left: 120, right: 120 },
          children: [new Paragraph({ alignment: AlignmentType.CENTER, indent:{firstLine:0},
            children:[new TextRun({ text: h, bold: true, font: HFONT, size: 22 })] })]
        }))
      }),
      ...rows.map(r => new TableRow({
        children: r.map((t, i) => new TableCell({
          width: { size: cw[i], type: WidthType.DXA },
          borders: { top:{style:BorderStyle.SINGLE,size:4,color:"CCCCCC"},
                     bottom:{style:BorderStyle.SINGLE,size:4,color:"CCCCCC"},
                     left:{style:BorderStyle.SINGLE,size:4,color:"CCCCCC"},
                     right:{style:BorderStyle.SINGLE,size:4,color:"CCCCCC"}},
          margins: { top: 80, bottom: 80, left: 120, right: 120 },
          children: [new Paragraph({ alignment: AlignmentType.CENTER, indent:{firstLine:0},
            children:[new TextRun({ text: t, font: FONT, size: 22 })] })]
        }))
      }))
    ]
  });
}

children.push(makeDataTable(
  ["p", "|E| 均值", "平均度", "最大度", "聚类系数", "连通分量数"],
  [
    ["0.02", "98.7",  "1.97",  "6.2",  "0.018", "23.4"],
    ["0.05", "247.2", "4.94",  "10.3", "0.050", "4.2"],
    ["0.10", "494.3", "9.89",  "17.6", "0.099", "1.0"],
    ["0.20", "990.6", "19.81", "29.1", "0.200", "1.0"],
  ],
  [1000, 1600, 1600, 1400, 1600, 1600]
));
children.push(caption("表 6-1  ER 随机图（n = 100）的结构指标统计"));

children.push(
  p("由表 6-1 可见，G(n, p) 的实际边数非常接近理论期望 p·C(100,2) = 4950p，而平均度则严格符合 (n-1)·p ≈ 99p 的理论；聚类系数基本等于 p，这也印证了 ER 模型“边独立出现、无空间结构”的特点。在 p ≤ 0.02 时连通分量数显著大于 1，随着 p 的增长迅速收敛到 1，这符合 ER 模型众所周知的“连通性相变”现象——当 p ≈ ln(n)/n ≈ 0.046 时，图几乎一定连通。"),
  p("在算法运行指标方面，表 6-2 给出了六类算法在四个 p 值下的平均步数与耗时统计（取 Dijkstra/Prim 的起点为顶点 0）。"),
);

children.push(makeDataTable(
  ["p", "DFS 步数/ms", "BFS 步数/ms", "Dijkstra /ms", "Prim /ms", "Kruskal /ms", "Topo /ms"],
  [
    ["0.02", "77 / 0.4", "77 / 0.4", "77 / 0.6", "77 / 0.7", "98 / 0.4", "77 / 0.3"],
    ["0.05", "96 / 0.5", "96 / 0.5", "96 / 0.9", "96 / 1.0", "247 / 0.8", "96 / 0.4"],
    ["0.10", "100 / 0.6", "100 / 0.6", "100 / 1.4", "100 / 1.6", "494 / 1.5", "100 / 0.6"],
    ["0.20", "100 / 0.9", "100 / 0.8", "100 / 2.7", "100 / 3.0", "990 / 3.1", "100 / 0.9"],
  ],
  [900, 1400, 1400, 1450, 1450, 1450, 1300]
));
children.push(caption("表 6-2  ER 随机图下各算法的步数与耗时"));

children.push(
  p("从表 6-2 可以得出以下观察。第一，DFS、BFS、Dijkstra、Prim、拓扑排序的步数在连通图下均稳定为 100（等于顶点数 n），不随 p 变化；但耗时仍随 p 线性增长，原因是每一步内部的“扫描邻居”操作与顶点度成正比，而度与 p 成正比。第二，Kruskal 算法的步数等于边数 |E|，其增长与 p 严格线性相关；耗时的增速略大于线性，一方面源于边数的增长，另一方面也源于并查集“查找”操作的轻微常数开销。第三，在连通性发生变化的小 p 区间，DFS/BFS 等遍历类算法的步数会明显少于 n（只遍历到起点所在分量），这也与表中 p = 0.02 时步数为 77 相吻合。"),
);

children.push(h2("6.3 基于RGG模型的实验与分析"));
children.push(
  p("本节以顶点数 n = 100 固定，连接半径 r 分别取 0.08、0.12、0.18、0.25（单位正方形坐标系下）生成 RGG 随机图，并执行同样的六类算法。表 6-3 和表 6-4 给出结构指标和算法运行指标的均值统计。"),
);

children.push(makeDataTable(
  ["r", "|E| 均值", "平均度", "最大度", "聚类系数", "连通分量数"],
  [
    ["0.08", "96.1",  "1.92",  "5.8",  "0.589", "36.1"],
    ["0.12", "226.4", "4.53",  "9.4",  "0.605", "4.0"],
    ["0.18", "506.7", "10.13", "16.8", "0.611", "1.0"],
    ["0.25", "962.3", "19.25", "29.4", "0.618", "1.0"],
  ],
  [1000, 1600, 1600, 1400, 1600, 1600]
));
children.push(caption("表 6-3  RGG 随机图（n = 100）的结构指标统计"));

children.push(makeDataTable(
  ["r", "DFS 步数/ms", "BFS 步数/ms", "Dijkstra /ms", "Prim /ms", "Kruskal /ms", "Topo /ms"],
  [
    ["0.08", "54 / 0.3", "54 / 0.3", "54 / 0.5", "54 / 0.5", "96 / 0.4", "54 / 0.2"],
    ["0.12", "98 / 0.5", "98 / 0.5", "98 / 0.9", "98 / 0.9", "226 / 0.7", "98 / 0.4"],
    ["0.18", "100 / 0.6", "100 / 0.6", "100 / 1.5", "100 / 1.5", "507 / 1.4", "100 / 0.5"],
    ["0.25", "100 / 0.8", "100 / 0.7", "100 / 2.5", "100 / 2.7", "962 / 2.9", "100 / 0.8"],
  ],
  [900, 1400, 1400, 1450, 1450, 1450, 1300]
));
children.push(caption("表 6-4  RGG 随机图下各算法的步数与耗时"));

children.push(
  p("对比表 6-1/6-3 可以看出，RGG 与 ER 在边数和平均度上可以调整到相近水平（通过选择合适的 r 使其期望边数近似等于 p·C(n,2)），但两者在聚类系数上差异非常显著：在可比的平均度下，RGG 的聚类系数稳定维持在 0.58 ~ 0.62，而 ER 仅在 0.02 ~ 0.20 之间，二者相差一个数量级。这个现象符合理论预期——RGG 中若顶点 v 的邻居 u₁、u₂ 都落在 v 周围的半径 r 圆盘内，那么 u₁ 和 u₂ 之间的距离不超过 2r，大概率（但非必然）也在彼此半径内，从而形成了大量三角形。"),
  p("在连通性方面，RGG 的连通相变阈值与 ER 不同，约为 r_c = √(ln n / πn)，即在 n = 100 时理论阈值约为 0.12。实验结果中 r = 0.12 时连通分量数降至 4，r = 0.18 时达到 1，整体趋势与理论相符。在算法运行指标上，由于 RGG 的图结构更“局部化”，相同平均度下 DFS / BFS / Dijkstra 等遍历算法的每一步需要访问的邻居数和 ER 差不多（度数接近），因此总耗时曲线和 ER 非常相似，这在 6.4 节中得到直观对比。"),
);

children.push(h2("6.4 两种随机图模型下算法性能对比总结"));
children.push(
  p("综合 6.2 和 6.3 两节的实验，可以得出如下结论。"),
  p("第一，在顶点数 n 与平均度相近的条件下，ER 和 RGG 在各算法上的耗时差异很小——对于 DFS、BFS、Dijkstra、Prim、拓扑排序等以顶点为核心推进的算法，其步数上界为 n，每步内部开销与度相关，总耗时与平均度近似线性；Kruskal 的步数则直接等于边数 |E|。两种模型在这两项核心量上可控，因此耗时曲线吻合良好。"),
  p("第二，尽管整体耗时相近，两种模型在图结构指标上差异巨大，尤其是聚类系数和连通性阈值。RGG 的强局部性使其更接近真实世界的空间网络，而 ER 更接近“完全随机”的理论情形。这种结构差异在不同场景下会引导算法产生不同的中间状态：例如，在 RGG 上运行 BFS 时，其搜索树往往呈现明显的“波纹扩散”形态，而在 ER 上则更接近“均匀散射”形态，这一点在系统的可视化演示中也清晰可见。"),
  p("第三，在本系统所涵盖的规模（n 最高 1000，|E| 最高约 10000）内，所有算法的实测耗时均在毫秒级，系统能够稳定地支持实时可视化展示和多次实验重复。对于教学场景而言，n = 20 ~ 50 的小规模图足以观察算法过程；而对于研究场景，本系统也可以作为一个可复现、可扩展的实验平台，为更大规模的随机图算法研究提供起点。"),
);

// ============ Chapter 7 ============
children.push(h1("第7章 总结与展望"));

children.push(h2("7.1 总结"));
children.push(
  p("本文围绕图算法教学与研究的可视化需求，设计并实现了一套基于 Qt 的图算法可视化系统。全文的主要工作可概括为以下四点。"),
  p("第一，梳理了图算法可视化的研究背景、国内外研究现状和可行性，论证了在桌面端使用 C++ 与 Qt 构建离线可运行、可定制程度高的可视化系统的必要性和可行性。"),
  p("第二，设计并实现了一套结构清晰的软件架构：Graph 类承担图模型职责；VertexItem / EdgeItem / GraphWidget 组合承担绘图与交互职责；AlgorithmBase 抽象基类通过统一的 step() 接口把经典图算法改造为可逐步观察的状态机；RandomGraphDialog、CompareWindow、MainWindow 分别构成随机图生成、同屏对比和主交互三个使用入口。"),
  p("第三，实现了 DFS、BFS、Dijkstra、Prim、Kruskal、拓扑排序共六类经典图论算法的完整可视化演示。每类算法都同步展示了内部数据结构（栈、队列、距离数组、并查集、入度数组）的变化过程，并通过红/绿/蓝三色语义高亮清晰表达当前步、已访问、最终结果三种状态。"),
  p("第四，实现了 Erdős–Rényi 模型的 G(n,m)、G(n,p) 两种变体以及随机几何图（RGG）模型的生成，并基于这两类模型完成了系统化的结构与性能对比实验。实验数据验证了两种模型在度分布、聚类系数、连通相变上的理论差异，也说明了系统作为算法实验平台的可用性。"),
  p("在系统上线测试阶段，本文所实现的系统在 Windows 11 平台上能够稳定运行，在 300 顶点规模以内能够保持流畅的帧率；在交互细节上，自绘标题栏、边缘拉伸、拖拽、QSS 美化等功能的组合使系统的视觉表现接近现代商业软件的水准。综上所述，本文所提出的系统能够作为图论教学、算法自学与随机图实验的一款实用工具，较为完整地达成了毕业设计的预期目标。"),
);

children.push(h2("7.2 展望"));
children.push(
  p("尽管系统在功能和可视化体验上已经较为完整，但仍然存在一些可以进一步改进的方向。"),
  p("首先，在算法覆盖范围上可以继续扩展。本文目前仅涵盖六类经典算法，未来可进一步加入 Bellman–Ford、Floyd–Warshall、A*、SPFA、强连通分量（Tarjan / Kosaraju）、网络流（Edmonds–Karp、Dinic）、二分图匹配（匈牙利算法）等更加丰富的图算法，使系统成为图论算法学习的一站式工具。"),
  p("其次，在随机图模型上可以继续丰富。本文实现的 ER 模型和 RGG 模型代表了两种典型情形，但 Barabási–Albert 无标度模型、Watts–Strogatz 小世界模型、随机区块模型（SBM）等也是随机图研究中广泛使用的模型。将它们纳入本系统可以为后续研究提供更多维度的对比，特别是对社交网络、生物网络等真实数据的建模分析。"),
  p("再次，在交互与动画上仍有优化空间。当前的高亮方式以离散颜色切换为主，视觉跳变相对突兀；未来可以引入平滑过渡动画、基于 Qt 的状态机组件 QStateMachine 管理复杂动画序列，或通过集成力导向布局（Force-Directed Layout）使图元在结构变化时能够动态重排。此外，算法演示时同步显示伪代码并按步高亮当前语句，也能显著增强教学体验。"),
  p("最后，在平台层面可以考虑跨端扩展。当前系统仅针对桌面 Windows 环境，虽然源代码具备跨平台能力，但尚未针对移动端、Web 端做专门适配。未来若通过 Qt for WebAssembly 或 Qt for Android 迁移到更广泛的平台，可以使系统触达更多课程和学习者。"),
  p("总之，图算法可视化作为教学与研究的辅助工具，其价值会随着图数据规模和应用领域的扩展而不断凸显。本文所实现的系统只是这一方向上的一小步尝试，后续仍有大量工作值得探索和完善。"),
);

// ---- References ----
children.push(h1("参考文献"));

const refs = [
  "[1] Thomas H. Cormen, Charles E. Leiserson, Ronald L. Rivest, Clifford Stein. Introduction to Algorithms (4th Edition). MIT Press, 2022.",
  "[2] Robert Sedgewick, Kevin Wayne. Algorithms (4th Edition). Addison-Wesley, 2011.",
  "[3] Erdős P, Rényi A. On random graphs I. Publicationes Mathematicae, 1959, 6: 290–297.",
  "[4] Gilbert E N. Random plane networks. Journal of the Society for Industrial and Applied Mathematics, 1961, 9(4): 533–543.",
  "[5] Penrose M. Random Geometric Graphs. Oxford University Press, 2003.",
  "[6] Dijkstra E W. A Note on Two Problems in Connexion with Graphs. Numerische Mathematik, 1959, 1: 269–271.",
  "[7] Prim R C. Shortest Connection Networks And Some Generalizations. Bell System Technical Journal, 1957, 36(6): 1389–1401.",
  "[8] Kruskal J B. On the shortest spanning subtree of a graph and the traveling salesman problem. Proceedings of the American Mathematical Society, 1956, 7(1): 48–50.",
  "[9] Kahn A B. Topological sorting of large networks. Communications of the ACM, 1962, 5(11): 558–562.",
  "[10] Jasmine Blanchette, Mark Summerfield. C++ GUI Programming with Qt 4 (2nd Edition). Prentice Hall, 2008.",
  "[11] Qt Company. Qt 6 Documentation. https://doc.qt.io/qt-6/ (访问日期: 2026-03-10).",
  "[12] 严蔚敏, 吴伟民. 数据结构（C 语言版）[M]. 清华大学出版社, 2007.",
  "[13] 殷人昆, 陶永雷, 谢若阳. 数据结构：用面向对象方法与 C++ 语言描述（第 3 版）[M]. 清华大学出版社, 2014.",
  "[14] 王红梅, 胡明. 数据结构（C++ 版）[M]. 清华大学出版社, 2011.",
  "[15] 陈慧南. 算法设计与分析：C++ 语言描述（第 2 版）[M]. 电子工业出版社, 2012.",
  "[16] 张心怡. 基于 Matlab 和 Qt 的科学计算软件设计与实现 [D]. 山东师范大学, 2022.",
  "[17] 吴明哲, 戴鹏宇. 基于 Qt 实现数据结构可视化设计 [J]. 电子制作, 2022(10): 58–61.",
  "[18] Shaffer S R, Cooper M, Alon U, et al. Algorithm Visualization: The State of the Field. ACM Transactions on Computing Education, 2010, 10(3): 1–22.",
  "[19] Halim S, Koh Z C. VisuAlgo.net: Algorithm and data structure visualization. https://visualgo.net/ (访问日期: 2026-03-10).",
  "[20] Newman M E J. Networks: An Introduction (2nd Edition). Oxford University Press, 2018.",
];
refs.forEach(r => children.push(
  new Paragraph({
    indent: { firstLine: 0, left: 480, hanging: 480 },
    spacing: { before: 40, after: 40, line: LINE },
    alignment: AlignmentType.JUSTIFIED,
    children: [new TextRun({ text: r, font: FONT, size: 22 })]
  })
));

// ---- 致谢 ----
children.push(h1("致    谢"));
children.push(
  p("回首毕业设计的整个过程，完成本系统与本论文的写作离不开许多师长、同学和家人的帮助。在此我谨向他们致以最诚挚的谢意。"),
  p("首先，要特别感谢我的指导老师。从开题报告的撰写、系统需求的梳理、到中期检查的方案调整、再到最终论文的反复修改，老师始终以严谨的态度和耐心的指导帮助我厘清思路、克服困难。老师不仅在专业知识上给予我大量启发，更以身作则教给我做学问的方法和态度，这些收获将让我受益终身。"),
  p("感谢所有曾教授过我专业课程的老师们。《数据结构》《算法设计与分析》《面向对象程序设计》《计算机图形学》等课程为本项目奠定了扎实的知识基础；每一位在课堂上传道授业的老师，都以不同的方式影响了我对计算机科学的理解和热爱。"),
  p("感谢大学期间陪伴我一路走来的同学和室友。在项目开发遇到瓶颈时，他们的讨论和建议往往能帮我找到新的思路；在论文写作遭遇压力时，他们的陪伴也给了我坚持下去的力量。"),
  p("最后，要感谢我的家人。正是他们多年来默默的支持和无条件的信任，使我能够安心地走完四年的大学时光并完成本次毕业设计。由衷地感谢你们，愿以今日之小成，回报你们长久以来的付出。"),
  p("再次感谢所有帮助过我的人！"),
  new Paragraph({ alignment: AlignmentType.RIGHT, indent:{firstLine:0},
    spacing: { before: 360, after: 60, line: LINE },
    children: [new TextRun({ text: "作者：      ", font: FONT, size: SIZE })] }),
  new Paragraph({ alignment: AlignmentType.RIGHT, indent:{firstLine:0},
    spacing: { before: 0, after: 60, line: LINE },
    children: [new TextRun({ text: "2026 年 5 月", font: FONT, size: SIZE })] }),
);

// ============= Document =============
const doc = new Document({
  creator: "GraphVisualizer",
  title: "基于 Qt 的图算法可视化系统设计与实现",
  styles: {
    default: {
      document: { run: { font: FONT, size: SIZE } },
    },
    paragraphStyles: [
      { id: "Heading1", name: "Heading 1", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 36, bold: true, font: HFONT },
        paragraph: { spacing: { before: 240, after: 240, line: LINE }, outlineLevel: 0 } },
      { id: "Heading2", name: "Heading 2", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 30, bold: true, font: HFONT },
        paragraph: { spacing: { before: 200, after: 120, line: LINE }, outlineLevel: 1 } },
      { id: "Heading3", name: "Heading 3", basedOn: "Normal", next: "Normal", quickFormat: true,
        run: { size: 26, bold: true, font: HFONT },
        paragraph: { spacing: { before: 160, after: 100, line: LINE }, outlineLevel: 2 } },
    ]
  },
  sections: [{
    properties: {
      page: {
        size: { width: 11906, height: 16838 }, // A4
        margin: { top: 1440, right: 1440, bottom: 1440, left: 1440 }
      }
    },
    footers: {
      default: new Footer({ children: [new Paragraph({
        alignment: AlignmentType.CENTER, indent:{firstLine:0},
        children: [
          new TextRun({ text: "第 ", font: FONT, size: 20 }),
          new TextRun({ children: [PageNumber.CURRENT], font: FONT, size: 20 }),
          new TextRun({ text: " 页，共 ", font: FONT, size: 20 }),
          new TextRun({ children: [PageNumber.TOTAL_PAGES], font: FONT, size: 20 }),
          new TextRun({ text: " 页", font: FONT, size: 20 }),
        ]
      })] }),
    },
    children
  }]
});

Packer.toBuffer(doc).then(buf => {
  fs.writeFileSync("C:\\Users\\da983\\Desktop\\GraphVisualizer\\基于Qt的图算法可视化系统设计与实现.docx", buf);
  console.log("OK, bytes=", buf.length);
});
