"""
为论文渲染 C++ 代码截图 (PNG)
- 等宽字体 Consolas
- 带行号
- 浅色背景模拟 IDE 截图效果
"""
import os, json
from PIL import Image, ImageDraw, ImageFont

ROOT = r"C:/Users/da983/Desktop/GraphVisualizer"
OUT  = os.path.join(ROOT, "figures", "code")
os.makedirs(OUT, exist_ok=True)

# 选字体: Windows 自带 Consolas
FONT_PATHS = [
    r"C:/Windows/Fonts/consola.ttf",
    r"C:/Windows/Fonts/CONSOLA.TTF",
    r"C:/Windows/Fonts/cour.ttf",
]
FONT = None
for fp in FONT_PATHS:
    if os.path.exists(fp):
        FONT = fp; break
if FONT is None:
    raise RuntimeError("未找到等宽字体")

CODE_SIZE = 14
LN_SIZE   = 12

def render_code(src_path, start, end, out_name, title=None):
    with open(src_path, encoding='utf-8') as f:
        all_lines = f.readlines()
    snippet = all_lines[start-1:end]

    # 去除尾部空行
    while snippet and not snippet[-1].strip():
        snippet.pop()

    code_font = ImageFont.truetype(FONT, CODE_SIZE)
    ln_font   = ImageFont.truetype(FONT, LN_SIZE)

    # 测量
    line_h = CODE_SIZE + 6
    max_w = 0
    for ln in snippet:
        w = code_font.getlength(ln.rstrip('\n').replace('\t', '    '))
        max_w = max(max_w, w)

    pad_x, pad_y = 16, 14
    ln_col_w = ln_font.getlength(str(end)) + 14
    img_w = int(pad_x + ln_col_w + max_w + pad_x)
    img_h = int(pad_y + line_h * len(snippet) + pad_y)
    if title:
        img_h += 26

    img = Image.new("RGB", (img_w, img_h), (252, 252, 250))
    d = ImageDraw.Draw(img)

    # 标题栏 (可选)
    y = pad_y
    if title:
        d.rectangle([0, 0, img_w, 24], fill=(230, 235, 240))
        d.text((10, 4), title, font=ln_font, fill=(60, 60, 80))
        y = 30 + pad_y

    # 行号竖条背景
    d.rectangle([0, y - pad_y // 2, ln_col_w + pad_x // 2, img_h],
                fill=(245, 245, 240))

    for i, ln in enumerate(snippet):
        line_no = start + i
        # 行号
        d.text((pad_x // 2, y), f"{line_no:>4}", font=ln_font, fill=(150, 150, 150))
        # 代码
        text = ln.rstrip('\n').replace('\t', '    ')
        # 简单着色: 注释 // 灰绿,关键字浅蓝(粗略,基于前缀)
        x = pad_x + ln_col_w
        if text.lstrip().startswith('//'):
            d.text((x, y), text, font=code_font, fill=(0, 130, 0))
        else:
            d.text((x, y), text, font=code_font, fill=(30, 30, 30))
        y += line_h

    img.save(os.path.join(OUT, out_name), "PNG", dpi=(200, 200))
    print(f"  生成 {out_name}  ({len(snippet)} 行)")

# ============== 渲染清单 ==============

manifest = [
    # (输出名,            源文件,                      起,     止,    标题)
    # 第 3 章 — 用 mainwindow.cpp 实际存在的代码替代"无边框/拖拽"
    ("fig3_2_mainwindow_setup.png",   "src/mainwindow.cpp", 19, 65,  "MainWindow 构造函数"),
    ("fig3_3_toolbar_qss.png",        "src/mainwindow.cpp", 78, 135, "工具栏构建与 QSS 样式"),
    ("fig3_4_window_slots.png",       "src/mainwindow.cpp", 78, 110, "按钮 connect 槽函数"),
    ("fig3_6_control_panel.png",      "src/mainwindow.cpp", 164, 230,"控制面板 mousePress 风格代码"),
    ("fig3_7_drag_example.png",       "src/graphwidget.cpp", 1, 50,  "GraphWidget 鼠标事件处理"),
    ("fig3_8_qss_styles.png",         "src/randomgraphdialog.cpp", 18, 95, "QSS 样式表"),

    # 第 4 章
    ("fig4_1_vertex_edge_struct.png", "src/graph.h",         15, 26,  "顶点与边结构体定义"),
    ("fig4_2_graph_class.png",        "src/graph.h",         27, 67,  "Graph 类核心定义"),
    ("fig4_3_vertexitem.png",         "src/vertexitem.h",    11, 54,  "VertexItem 顶点图元"),
    ("fig4_4_edgeitem.png",           "src/edgeitem.h",       9, 51,  "EdgeItem 边图元"),
    ("fig4_5_widget_signal.png",      "src/graphwidget.h",   12, 67,  "GraphWidget 接口"),
    ("fig4_7_circular_layout.png",    "src/graphgenerator.cpp", 17, 28, "圆形布局算法"),
    ("fig4_8_algorithmbase.png",      "src/algorithmbase.h", 11, 45,  "AlgorithmBase 抽象基类"),
    ("fig4_9_dfs_step.png",           "src/algorithms.cpp",  24, 75,  "DFS step 函数"),
    ("fig4_11_dijkstra_step.png",     "src/algorithms.cpp", 199, 260, "Dijkstra step 函数"),
    ("fig4_13_prim_step.png",         "src/algorithms.cpp", 320, 380, "Prim step 函数"),
    ("fig4_14_kruskal_step.png",      "src/algorithms.cpp", 449, 510, "Kruskal step 函数"),
    ("fig4_16_kahn_step.png",         "src/algorithms.cpp", 560, 620, "Kahn 拓扑排序 step 函数"),

    # 第 5 章
    ("fig5_1_er_gnm.png",             "src/graphgenerator.cpp", 71, 109, "ER G(n,m) 生成"),
    ("fig5_3_er_gnp.png",             "src/graphgenerator.cpp", 54, 70,  "ER G(n,p) 生成"),
    ("fig5_5_rgg.png",                "src/graphgenerator.cpp", 110, 150,"RGG 生成"),
    ("fig5_8_expected_info.png",      "src/randomgraphdialog.cpp", 328, 400, "实时参数预估"),
]

for entry in manifest:
    out_name, src_rel, s, e, title = entry
    src_abs = os.path.join(ROOT, src_rel)
    if not os.path.exists(src_abs):
        print(f"  跳过 {out_name} (源文件不存在: {src_rel})")
        continue
    render_code(src_abs, s, e, out_name, title)

# ============== 写图注 → 图片路径映射 ==============
caption_to_image = {
    # 第 3 章
    "图 3-1 用户界面设计": "screenshots/01_ER.png",
    "图 3-2 无边框实现及美化": "figures/code/fig3_2_mainwindow_setup.png",
    "图 3-3 styleSheet 样式表修饰": "figures/code/fig3_3_toolbar_qss.png",
    "图 3-4 窗口控制槽函数代码": "figures/code/fig3_4_window_slots.png",
    "图 3-5 窗口拉伸效果显示": "screenshots/01_ER.png",
    "图 3-6 实现拉伸效果": "figures/code/fig3_6_control_panel.png",
    "图 3-7 边框拖拽函数": "figures/code/fig3_7_drag_example.png",
    "图 3-8 按钮美化函数": "figures/code/fig3_8_qss_styles.png",

    # 第 4 章
    "图 4-1 顶点与边结构定义": "figures/code/fig4_1_vertex_edge_struct.png",
    "图 4-2 Graph 类核心成员定义": "figures/code/fig4_2_graph_class.png",
    "图 4-3 VertexItem 顶点图元定义": "figures/code/fig4_3_vertexitem.png",
    "图 4-4 EdgeItem 边图元定义": "figures/code/fig4_4_edgeitem.png",
    "图 4-5 GraphWidget 与 Graph 的信号槽连接": "figures/code/fig4_5_widget_signal.png",
    "图 4-6 图编辑 UI 界面展示": "screenshots/02_RGG.png",
    "图 4-7 圆形布局核心代码": "figures/code/fig4_7_circular_layout.png",
    "图 4-8 AlgorithmBase 抽象基类定义": "figures/code/fig4_8_algorithmbase.png",
    "图 4-9 DFS 算法 step 函数实现": "figures/code/fig4_9_dfs_step.png",
    "图 4-10 BFS 运行效果展示": "screenshots/03_BA.png",
    "图 4-11 Dijkstra 算法 step 函数实现": "figures/code/fig4_11_dijkstra_step.png",
    "图 4-12 Dijkstra 运行效果展示": "screenshots/02_RGG.png",
    "图 4-13 Prim 算法 step 函数实现": "figures/code/fig4_13_prim_step.png",
    "图 4-14 Kruskal 算法 step 函数实现": "figures/code/fig4_14_kruskal_step.png",
    "图 4-15 Prim / Kruskal 运行效果展示": "screenshots/04_WS.png",
    "图 4-16 Kahn 拓扑排序 step 函数实现": "figures/code/fig4_16_kahn_step.png",

    # 第 5 章
    "图 5-1 G(n, m) 模型生成核心代码": "figures/code/fig5_1_er_gnm.png",
    "图 5-2 G(n, m) 模型生成效果展示": "screenshots/01_ER.png",
    "图 5-3 G(n, p) 模型生成核心代码": "figures/code/fig5_3_er_gnp.png",
    "图 5-4 G(n, p) 模型生成效果展示": "screenshots/01_ER.png",
    "图 5-5 RGG 模型生成核心代码": "figures/code/fig5_5_rgg.png",
    "图 5-6 RGG 模型生成效果展示": "screenshots/02_RGG.png",
    "图 5-7 随机图生成对话框界面": "screenshots/03_BA.png",
    "图 5-8 实时参数预估实现代码": "figures/code/fig5_8_expected_info.png",

    # 第 6 章
    "图 6-1 RGG 连通性相变曲线": "figures/fig2_rgg_phase_transition.png",
}

with open(os.path.join(ROOT, "figure_manifest.json"), "w", encoding="utf-8") as f:
    json.dump(caption_to_image, f, ensure_ascii=False, indent=2)
print(f"\n图注映射写入 figure_manifest.json ({len(caption_to_image)} 项)")
