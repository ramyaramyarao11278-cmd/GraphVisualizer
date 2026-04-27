#include "mainwindow.h"
#include "algorithms.h"
#include "comparewindow.h"
#include "edgeitem.h"
#include "randomgraphdialog.h"
#include "graphgenerator.h"
#include <QToolBar>
#include <QDockWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QFileDialog>
#include <QJsonDocument>
#include <QMessageBox>
#include <QStatusBar>
#include <QSplitter>
#include <QMenu>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_algorithm(nullptr)
{
    m_graph = new Graph(Undirected, this);
    m_graphWidget = new GraphWidget(m_graph, this);
    m_timer = new QTimer(this);

    setupUI();
    setupToolBar();
    setupControlPanel();
    setupDataPanel();

    connect(m_timer, &QTimer::timeout, this, &MainWindow::onTimerTick);
    connect(m_graphWidget, &GraphWidget::statusMessage, [this](const QString &msg) {
        m_statusLabel->setText(msg);
    });

    // 模拟演示定时器
    m_demoTimer = new QTimer(this);
    m_demoStep = 0;
    connect(m_demoTimer, &QTimer::timeout, this, &MainWindow::onDemoTick);

    // 演示浮层标签（覆盖在图上方）
    m_demoOverlay = new QLabel(m_graphWidget);
    m_demoOverlay->setStyleSheet(
        "background-color: rgba(30, 30, 30, 210);"
        "color: #FFD54F;"
        "font-size: 16px;"
        "font-weight: bold;"
        "padding: 10px 28px;"
        "border-radius: 8px;"
        "border: 1px solid rgba(255,255,255,50);"
    );
    m_demoOverlay->setAlignment(Qt::AlignCenter);
    m_demoOverlay->hide();

    // 初始加载无向图示例
    loadExampleUndirected();

    setWindowTitle(QStringLiteral("图论算法可视化演示系统"));
    resize(1280, 800);
}

MainWindow::~MainWindow()
{
    delete m_algorithm;
}

void MainWindow::setupUI()
{
    m_graphWidget->setMinimumSize(400, 300);
    setCentralWidget(m_graphWidget);

    // 状态栏
    m_statusLabel = new QLabel(QStringLiteral("就绪"));
    statusBar()->addWidget(m_statusLabel, 1);
}

void MainWindow::setupToolBar()
{
    auto *toolbar = addToolBar(QStringLiteral("图编辑"));
    toolbar->setMovable(false);
    toolbar->setIconSize(QSize(20, 20));

    auto *newBtn = new QPushButton(QStringLiteral("新建图"));
    connect(newBtn, &QPushButton::clicked, this, &MainWindow::onNewGraph);
    toolbar->addWidget(newBtn);

    m_graphTypeLabel = new QLabel(QStringLiteral(" [无向图] "));
    m_graphTypeLabel->setStyleSheet("font-weight: bold; color: #336;");
    toolbar->addWidget(m_graphTypeLabel);

    auto *typeBtn = new QPushButton(QStringLiteral("切换有向/无向"));
    connect(typeBtn, &QPushButton::clicked, this, &MainWindow::onToggleGraphType);
    toolbar->addWidget(typeBtn);

    toolbar->addSeparator();

    m_addEdgeBtn = new QPushButton(QStringLiteral("添加边模式"));
    m_addEdgeBtn->setCheckable(true);
    connect(m_addEdgeBtn, &QPushButton::toggled, this, &MainWindow::onToggleAddEdge);
    toolbar->addWidget(m_addEdgeBtn);

    toolbar->addSeparator();

    auto *saveBtn = new QPushButton(QStringLiteral("保存"));
    connect(saveBtn, &QPushButton::clicked, this, &MainWindow::onSaveGraph);
    toolbar->addWidget(saveBtn);

    auto *loadBtn = new QPushButton(QStringLiteral("加载"));
    connect(loadBtn, &QPushButton::clicked, this, &MainWindow::onLoadGraph);
    toolbar->addWidget(loadBtn);

    // 示例图下拉菜单
    auto *exampleBtn = new QPushButton(QStringLiteral("示例图 ▾"));
    auto *exampleMenu = new QMenu(exampleBtn);
    exampleMenu->addAction(QStringLiteral("无向加权图 (DFS/BFS/Dijkstra/Prim/Kruskal)"),
                           this, &MainWindow::loadExampleUndirected);
    exampleMenu->addAction(QStringLiteral("有向无环图 (拓扑排序/DFS/BFS)"),
                           this, &MainWindow::loadExampleDirectedDAG);
    exampleMenu->addAction(QStringLiteral("稠密加权图 (Dijkstra/Prim/Kruskal)"),
                           this, &MainWindow::loadExampleWeightedDense);
    exampleMenu->addAction(QStringLiteral("非连通图 (测试边界情况)"),
                           this, &MainWindow::loadExampleDisconnected);
    exampleBtn->setMenu(exampleMenu);
    toolbar->addWidget(exampleBtn);

    auto *randomBtn = new QPushButton(QStringLiteral("  随机图生成  "));
    randomBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #E8F5E9, stop:1 #C8E6C9);"
        "  border: 1px solid #81C784; color: #2E7D32; font-weight: bold;"
        "  border-radius: 6px; padding: 5px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #C8E6C9, stop:1 #A5D6A7); border-color: #66BB6A; }");
    connect(randomBtn, &QPushButton::clicked, this, &MainWindow::onGenerateRandomGraph);
    toolbar->addWidget(randomBtn);

    toolbar->addSeparator();

    auto *compareBtn = new QPushButton(QStringLiteral("  对比模式  "));
    compareBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #E3F2FD, stop:1 #BBDEFB);"
        "  border: 1px solid #64B5F6; color: #1565C0; font-weight: bold;"
        "  border-radius: 6px; padding: 5px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #BBDEFB, stop:1 #90CAF9); border-color: #42A5F5; }");
    connect(compareBtn, &QPushButton::clicked, this, &MainWindow::onOpenCompare);
    toolbar->addWidget(compareBtn);

    m_demoBtn = new QPushButton(QStringLiteral("  模拟演示  "));
    m_demoBtn->setStyleSheet(
        "QPushButton { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #FFF3E0, stop:1 #FFE0B2);"
        "  border: 1px solid #FFB74D; color: #E65100; font-weight: bold;"
        "  border-radius: 6px; padding: 5px 16px; }"
        "QPushButton:hover { background: qlineargradient(x1:0,y1:0,x2:0,y2:1,"
        "  stop:0 #FFE0B2, stop:1 #FFCC80); border-color: #FFA726; }");
    connect(m_demoBtn, &QPushButton::clicked, this, &MainWindow::onStartDemo);
    toolbar->addWidget(m_demoBtn);

}

void MainWindow::setupControlPanel()
{
    auto *dock = new QDockWidget(QStringLiteral("算法控制"), this);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);
    auto *widget = new QWidget();
    auto *layout = new QVBoxLayout(widget);

    // 算法选择
    auto *algoGroup = new QGroupBox(QStringLiteral("算法选择"));
    auto *algoLayout = new QVBoxLayout(algoGroup);

    m_algorithmCombo = new QComboBox();
    m_algorithmCombo->addItem(QStringLiteral("DFS (深度优先搜索)"));
    m_algorithmCombo->addItem(QStringLiteral("BFS (广度优先搜索)"));
    m_algorithmCombo->addItem(QStringLiteral("Dijkstra (最短路径)"));
    m_algorithmCombo->addItem(QStringLiteral("Prim (最小生成树)"));
    m_algorithmCombo->addItem(QStringLiteral("Kruskal (最小生成树)"));
    m_algorithmCombo->addItem(QStringLiteral("拓扑排序"));
    connect(m_algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &MainWindow::onAlgorithmSelected);
    algoLayout->addWidget(m_algorithmCombo);

    auto *startLayout = new QHBoxLayout();
    startLayout->addWidget(new QLabel(QStringLiteral("起点:")));
    m_startVertexCombo = new QComboBox();
    startLayout->addWidget(m_startVertexCombo);
    algoLayout->addLayout(startLayout);

    layout->addWidget(algoGroup);

    // 控制按钮
    auto *ctrlGroup = new QGroupBox(QStringLiteral("演示控制"));
    auto *ctrlLayout = new QVBoxLayout(ctrlGroup);

    auto *btnLayout = new QHBoxLayout();
    btnLayout->setSpacing(4);
    m_playBtn = new QPushButton(QStringLiteral("▶ 播放"));
    m_pauseBtn = new QPushButton(QStringLiteral("⏸ 暂停"));
    m_stepBtn = new QPushButton(QStringLiteral("⏭ 单步"));
    m_resetBtn = new QPushButton(QStringLiteral("⏹ 重置"));

    m_playBtn->setStyleSheet(
        "QPushButton { background: #10B981; color: white; border: none;"
        "  border-radius: 5px; padding: 6px 4px; font-weight: bold; }"
        "QPushButton:hover { background: #059669; }"
        "QPushButton:disabled { background: #D1D5DB; }");
    m_pauseBtn->setStyleSheet(
        "QPushButton { background: #F59E0B; color: white; border: none;"
        "  border-radius: 5px; padding: 6px 4px; font-weight: bold; }"
        "QPushButton:hover { background: #D97706; }"
        "QPushButton:disabled { background: #D1D5DB; }");
    m_stepBtn->setStyleSheet(
        "QPushButton { background: #3B82F6; color: white; border: none;"
        "  border-radius: 5px; padding: 6px 4px; font-weight: bold; }"
        "QPushButton:hover { background: #2563EB; }"
        "QPushButton:disabled { background: #D1D5DB; }");
    m_resetBtn->setStyleSheet(
        "QPushButton { background: #EF4444; color: white; border: none;"
        "  border-radius: 5px; padding: 6px 4px; font-weight: bold; }"
        "QPushButton:hover { background: #DC2626; }"
        "QPushButton:disabled { background: #D1D5DB; }");
    m_pauseBtn->setEnabled(false);

    connect(m_playBtn, &QPushButton::clicked, this, &MainWindow::onPlay);
    connect(m_pauseBtn, &QPushButton::clicked, this, &MainWindow::onPause);
    connect(m_stepBtn, &QPushButton::clicked, this, &MainWindow::onStepOnce);
    connect(m_resetBtn, &QPushButton::clicked, this, &MainWindow::onReset);

    btnLayout->addWidget(m_playBtn);
    btnLayout->addWidget(m_pauseBtn);
    btnLayout->addWidget(m_stepBtn);
    btnLayout->addWidget(m_resetBtn);
    ctrlLayout->addLayout(btnLayout);

    // 速度滑块
    auto *speedLayout = new QHBoxLayout();
    speedLayout->addWidget(new QLabel(QStringLiteral("速度:")));
    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setRange(100, 2000);
    m_speedSlider->setValue(800);
    m_speedSlider->setInvertedAppearance(true);
    m_speedLabel = new QLabel(QStringLiteral("800ms"));
    connect(m_speedSlider, &QSlider::valueChanged, [this](int val) {
        m_speedLabel->setText(QStringLiteral("%1ms").arg(val));
        if (m_timer->isActive())
            m_timer->setInterval(val);
    });
    speedLayout->addWidget(m_speedSlider);
    speedLayout->addWidget(m_speedLabel);
    ctrlLayout->addLayout(speedLayout);

    layout->addWidget(ctrlGroup);

    // 颜色图例
    auto *legendGroup = new QGroupBox(QStringLiteral("颜色图例"));
    auto *legendLayout = new QVBoxLayout(legendGroup);
    legendLayout->setSpacing(4);
    auto makeLegend = [&](const QString &color, const QString &text) {
        auto *row = new QHBoxLayout();
        auto *dot = new QLabel();
        dot->setFixedSize(14, 14);
        dot->setStyleSheet(QStringLiteral(
            "background: %1; border-radius: 7px; border: 1px solid rgba(0,0,0,30);").arg(color));
        auto *label = new QLabel(text);
        label->setStyleSheet("color: #475569; font-size: 11px;");
        row->addWidget(dot);
        row->addWidget(label);
        row->addStretch();
        legendLayout->addLayout(row);
    };
    makeLegend("#B0B0B0", QStringLiteral("灰色 — 未访问"));
    makeLegend("#4CAF50", QStringLiteral("绿色 — 已访问"));
    makeLegend("#F44336", QStringLiteral("红色 — 当前正在处理"));
    makeLegend("#FF9800", QStringLiteral("橙色 — 当前探索的边"));
    makeLegend("#2196F3", QStringLiteral("蓝色 — 最终结果(路径/树)"));
    layout->addWidget(legendGroup);

    // 新手引导
    auto *guideGroup = new QGroupBox(QStringLiteral("快速上手"));
    auto *guideLayout = new QVBoxLayout(guideGroup);
    auto *guideLabel = new QLabel(QStringLiteral(
        "1. 双击空白处添加顶点\n"
        "2. 点「添加边模式」连线\n"
        "3. 或用「示例图」/「随机图生成」\n"
        "4. 左侧选择算法和起点\n"
        "5. 点「播放」自动演示\n"
        "    或「单步」逐步观察\n"
        "6. 滚轮缩放 | Delete删除\n"
        "7. 双击顶点/边可编辑"));
    guideLabel->setWordWrap(true);
    guideLabel->setStyleSheet(
        "color: #64748B; font-size: 11px; line-height: 1.5;"
        "padding: 4px;");
    guideLayout->addWidget(guideLabel);
    layout->addWidget(guideGroup);

    layout->addStretch();

    dock->setWidget(widget);
    dock->setFixedWidth(260);
    addDockWidget(Qt::LeftDockWidgetArea, dock);
}

void MainWindow::setupDataPanel()
{
    auto *dock = new QDockWidget(QStringLiteral("数据结构 & 日志"), this);
    dock->setFeatures(QDockWidget::NoDockWidgetFeatures);

    auto *splitter = new QSplitter(Qt::Vertical);

    m_dataDisplay = new QTextEdit();
    m_dataDisplay->setReadOnly(true);
    QFont dataFont("Microsoft YaHei UI", 10);
    dataFont.setStyleHint(QFont::Monospace);
    m_dataDisplay->setFont(dataFont);
    m_dataDisplay->setLineWrapMode(QTextEdit::WidgetWidth);
    m_dataDisplay->setPlaceholderText(QStringLiteral("算法数据结构将在这里展示...\n\n选择算法后点击「单步」或「播放」开始"));
    m_dataDisplay->setStyleSheet(
        "QTextEdit { background: #FEFEFE; color: #1E293B;"
        "  border: 1px solid #E2E8F0; border-radius: 6px; padding: 8px;"
        "  selection-background-color: #BFDBFE;"
        "  font-family: 'Microsoft YaHei UI', 'Consolas'; }");
    splitter->addWidget(m_dataDisplay);

    m_logDisplay = new QTextEdit();
    m_logDisplay->setReadOnly(true);
    QFont logFont("Microsoft YaHei UI", 9);
    logFont.setStyleHint(QFont::Monospace);
    m_logDisplay->setFont(logFont);
    m_logDisplay->setLineWrapMode(QTextEdit::WidgetWidth);
    m_logDisplay->setPlaceholderText(QStringLiteral("算法执行日志..."));
    m_logDisplay->setStyleSheet(
        "QTextEdit { background: #1E293B; color: #E2E8F0;"
        "  border: 1px solid #334155; border-radius: 6px; padding: 8px;"
        "  selection-background-color: #475569;"
        "  font-family: 'Microsoft YaHei UI', 'Consolas'; }");
    splitter->addWidget(m_logDisplay);

    splitter->setSizes({300, 200});
    dock->setWidget(splitter);
    dock->setFixedWidth(320);
    addDockWidget(Qt::RightDockWidgetArea, dock);
}

void MainWindow::onAlgorithmSelected(int index)
{
    onReset();
    updateStartVertexCombo();

    bool needStart = (index <= 3); // DFS, BFS, Dijkstra, Prim
    m_startVertexCombo->setEnabled(needStart);

    // 拓扑排序需要有向图，自动提示
    if (index == 5 && m_graph->type() != Directed) {
        m_statusLabel->setText(QStringLiteral("提示: 拓扑排序需要有向图，请切换图类型或加载有向图示例"));
    }
}

void MainWindow::createAlgorithm(int index)
{
    delete m_algorithm;
    m_algorithm = nullptr;

    switch (index) {
    case 0: m_algorithm = new DFSAlgorithm(m_graph, this); break;
    case 1: m_algorithm = new BFSAlgorithm(m_graph, this); break;
    case 2: m_algorithm = new DijkstraAlgorithm(m_graph, this); break;
    case 3: m_algorithm = new PrimAlgorithm(m_graph, this); break;
    case 4: m_algorithm = new KruskalAlgorithm(m_graph, this); break;
    case 5: m_algorithm = new TopologicalSortAlgorithm(m_graph, this); break;
    }

    if (m_algorithm) {
        if (m_algorithm->needsStartVertex() && m_startVertexCombo->count() > 0) {
            int startId = m_startVertexCombo->currentData().toInt();
            m_algorithm->setStartVertex(startId);
        }
        connect(m_algorithm, &AlgorithmBase::stateChanged,
                this, &MainWindow::onAlgorithmStateChanged);
        connect(m_algorithm, &AlgorithmBase::finished,
                this, &MainWindow::onAlgorithmFinished);
    }
}

void MainWindow::updateStartVertexCombo()
{
    m_startVertexCombo->clear();
    for (int id : m_graph->vertices()) {
        m_startVertexCombo->addItem(m_graph->vertex(id).label, id);
    }
}

void MainWindow::onPlay()
{
    if (m_graph->vertexCount() == 0) {
        m_statusLabel->setText(QStringLiteral("图为空，请先添加顶点和边"));
        return;
    }
    if (!m_algorithm) {
        createAlgorithm(m_algorithmCombo->currentIndex());
    }
    if (!m_algorithm) return;

    m_timer->start(m_speedSlider->value());
    m_playBtn->setEnabled(false);
    m_pauseBtn->setEnabled(true);
    m_statusLabel->setText(QStringLiteral("运行中..."));
}

void MainWindow::onPause()
{
    m_timer->stop();
    m_playBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    m_statusLabel->setText(QStringLiteral("已暂停"));
}

void MainWindow::onStepOnce()
{
    if (m_graph->vertexCount() == 0) {
        m_statusLabel->setText(QStringLiteral("图为空，请先添加顶点和边"));
        return;
    }
    if (!m_algorithm) {
        createAlgorithm(m_algorithmCombo->currentIndex());
    }
    if (!m_algorithm) return;

    m_algorithm->step();
}

void MainWindow::onReset()
{
    m_timer->stop();
    if (m_algorithm) {
        m_algorithm->reset();
        delete m_algorithm;
        m_algorithm = nullptr;
    }
    m_graphWidget->clearHighlights();
    m_dataDisplay->clear();
    m_logDisplay->clear();
    m_playBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    m_statusLabel->setText(QStringLiteral("已重置"));
}

void MainWindow::onAlgorithmStateChanged()
{
    applyHighlights();

    if (m_algorithm) {
        m_dataDisplay->setText(m_algorithm->dataDisplayLines().join("\n"));
        m_logDisplay->setText(m_algorithm->log().join("\n"));
        auto cursor = m_logDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);
        m_logDisplay->setTextCursor(cursor);
    }
}

void MainWindow::onAlgorithmFinished(const QString &summary)
{
    m_timer->stop();
    m_playBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    m_statusLabel->setText(summary);
    m_logDisplay->append(QStringLiteral("\n--- %1 ---").arg(summary));
}

void MainWindow::onTimerTick()
{
    if (m_algorithm) {
        if (!m_algorithm->step()) {
            m_timer->stop();
        }
    }
}

void MainWindow::applyHighlights()
{
    if (!m_algorithm) return;

    // 先全部恢复为灰色（未访问状态）
    m_graphWidget->clearAllToGray();

    if (m_algorithm->isFinished()) {
        // 算法完成：已访问顶点绿色 + 结果边/顶点蓝色高亮
        QList<int> visitedList = m_algorithm->visitedVertices().values();
        m_graphWidget->highlightVertices(visitedList, QColor(76, 175, 80)); // 绿色
        m_graphWidget->highlightEdges(m_algorithm->resultEdges(), QColor(33, 150, 243)); // 蓝色
        m_graphWidget->highlightVertices(m_algorithm->resultVertices(), QColor(33, 150, 243));
    } else {
        // 运行中：已访问绿色 → 当前步红色 → 当前边橙色
        QList<int> visitedList = m_algorithm->visitedVertices().values();
        m_graphWidget->highlightVertices(visitedList, QColor(76, 175, 80)); // 绿色
        m_graphWidget->highlightEdges(m_algorithm->highlightedEdges(), QColor(255, 140, 0)); // 橙色
        m_graphWidget->highlightVertices(m_algorithm->highlightedVertices(), QColor(244, 67, 54)); // 红色（覆盖绿色）
    }
}

void MainWindow::updateGraphTypeLabel()
{
    m_graphTypeLabel->setText(m_graph->type() == Directed
        ? QStringLiteral(" [有向图] ") : QStringLiteral(" [无向图] "));
}

// ============ 图编辑 ============

void MainWindow::onGenerateRandomGraph()
{
    RandomGraphDialog dlg(this);
    if (dlg.exec() != QDialog::Accepted) return;

    onReset();
    QString desc = dlg.apply(m_graph);
    m_graphWidget->rebuildFromGraph();
    updateGraphTypeLabel();
    updateStartVertexCombo();

    auto met = GraphGenerator::computeMetrics(m_graph);
    m_statusLabel->setText(QStringLiteral("%1 | V=%2  E=%3  AvgDeg=%4")
        .arg(desc).arg(met.vertexCount).arg(met.edgeCount)
        .arg(met.avgDegree, 0, 'f', 2));

    m_dataDisplay->clear();
    m_logDisplay->clear();
    m_logDisplay->append(QStringLiteral("=== %1 ===").arg(desc));
    m_logDisplay->append(QStringLiteral("顶点数: %1").arg(met.vertexCount));
    m_logDisplay->append(QStringLiteral("边  数: %1").arg(met.edgeCount));
    m_logDisplay->append(QStringLiteral("平均度: %1").arg(met.avgDegree, 0, 'f', 3));
    m_logDisplay->append(QStringLiteral("最大度 / 最小度: %1 / %2").arg(met.maxDegree).arg(met.minDegree));
    m_logDisplay->append(QStringLiteral("聚类系数 C: %1").arg(met.clusteringCoefficient, 0, 'f', 4));
    m_logDisplay->append(QStringLiteral("连通分量: %1 (最大: %2 顶点)")
        .arg(met.componentCount).arg(met.largestComponentSize));
}

void MainWindow::onNewGraph()
{
    onReset();
    m_graph->clear();
    updateStartVertexCombo();
    m_statusLabel->setText(QStringLiteral("新建空图 — 双击空白处添加顶点，然后用「添加边模式」连线"));
}

void MainWindow::onToggleGraphType()
{
    onReset();
    GraphType newType = (m_graph->type() == Undirected) ? Directed : Undirected;
    m_graph->setType(newType);
    updateGraphTypeLabel();
    // 重建视觉元素以更新箭头显示
    m_graphWidget->rebuildFromGraph();
    m_statusLabel->setText(QStringLiteral("已切换为%1").arg(
        newType == Directed ? QStringLiteral("有向图") : QStringLiteral("无向图")));
}

void MainWindow::onToggleAddEdge(bool checked)
{
    m_graphWidget->setInteractionMode(
        checked ? GraphWidget::AddEdgeMode : GraphWidget::SelectMode);
    m_statusLabel->setText(checked
        ? QStringLiteral("添加边模式: 点击选择起点，再点击终点，输入权值")
        : QStringLiteral("选择模式"));
}

void MainWindow::onSaveGraph()
{
    QString path = QFileDialog::getSaveFileName(this,
        QStringLiteral("保存图"), "", QStringLiteral("JSON (*.json)"));
    if (path.isEmpty()) return;

    QJsonDocument doc(m_graph->toJson());
    QFile file(path);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(doc.toJson(QJsonDocument::Indented));
        m_statusLabel->setText(QStringLiteral("已保存: %1").arg(path));
    }
}

void MainWindow::onLoadGraph()
{
    QString path = QFileDialog::getOpenFileName(this,
        QStringLiteral("加载图"), "", QStringLiteral("JSON (*.json)"));
    if (path.isEmpty()) return;

    QFile file(path);
    if (file.open(QIODevice::ReadOnly)) {
        QJsonDocument doc = QJsonDocument::fromJson(file.readAll());
        onReset();
        m_graph->fromJson(doc.object());
        m_graphWidget->rebuildFromGraph();
        updateStartVertexCombo();
        updateGraphTypeLabel();
        m_statusLabel->setText(QStringLiteral("已加载: %1").arg(path));
    }
}

// ============ 示例图 ============

void MainWindow::loadExampleUndirected()
{
    onReset();
    m_graph->clear();
    m_graph->setType(Undirected);
    updateGraphTypeLabel();

    // 无向加权图: 适合 DFS/BFS/Dijkstra/Prim/Kruskal
    int a = m_graph->addVertex("A", QPointF(-150, -100));
    int b = m_graph->addVertex("B", QPointF(0, -150));
    int c = m_graph->addVertex("C", QPointF(150, -100));
    int d = m_graph->addVertex("D", QPointF(-150, 100));
    int e = m_graph->addVertex("E", QPointF(0, 150));
    int f = m_graph->addVertex("F", QPointF(150, 100));

    m_graph->addEdge(a, b, 4);
    m_graph->addEdge(a, d, 2);
    m_graph->addEdge(b, c, 3);
    m_graph->addEdge(b, e, 5);
    m_graph->addEdge(b, d, 1);
    m_graph->addEdge(c, f, 1);
    m_graph->addEdge(d, e, 6);
    m_graph->addEdge(e, f, 7);

    m_graphWidget->rebuildFromGraph();
    updateStartVertexCombo();
    m_statusLabel->setText(QStringLiteral("示例: 无向加权图 (6顶点8边) — 适合 DFS/BFS/Dijkstra/Prim/Kruskal"));
}

void MainWindow::loadExampleDirectedDAG()
{
    onReset();
    m_graph->clear();
    m_graph->setType(Directed);
    updateGraphTypeLabel();

    // 有向无环图: 适合拓扑排序、DFS、BFS
    //   课程依赖关系示例:
    //   C1→C2, C1→C3, C2→C4, C3→C4, C3→C5, C4→C6, C5→C6
    int c1 = m_graph->addVertex("C1", QPointF(-200, 0));
    int c2 = m_graph->addVertex("C2", QPointF(-70, -120));
    int c3 = m_graph->addVertex("C3", QPointF(-70, 120));
    int c4 = m_graph->addVertex("C4", QPointF(70, -120));
    int c5 = m_graph->addVertex("C5", QPointF(70, 120));
    int c6 = m_graph->addVertex("C6", QPointF(200, 0));

    m_graph->addEdge(c1, c2, 1);
    m_graph->addEdge(c1, c3, 1);
    m_graph->addEdge(c2, c4, 1);
    m_graph->addEdge(c3, c4, 1);
    m_graph->addEdge(c3, c5, 1);
    m_graph->addEdge(c4, c6, 1);
    m_graph->addEdge(c5, c6, 1);

    m_graphWidget->rebuildFromGraph();
    updateStartVertexCombo();
    m_statusLabel->setText(QStringLiteral("示例: 有向无环图 (6顶点7边) — 适合拓扑排序/DFS/BFS"));
}

void MainWindow::loadExampleWeightedDense()
{
    onReset();
    m_graph->clear();
    m_graph->setType(Undirected);
    updateGraphTypeLabel();

    // 稠密加权图: 适合 Dijkstra/Prim/Kruskal 对比
    int v0 = m_graph->addVertex("S", QPointF(-200, 0));
    int v1 = m_graph->addVertex("A", QPointF(-80, -130));
    int v2 = m_graph->addVertex("B", QPointF(80, -130));
    int v3 = m_graph->addVertex("C", QPointF(200, 0));
    int v4 = m_graph->addVertex("D", QPointF(80, 130));
    int v5 = m_graph->addVertex("E", QPointF(-80, 130));

    m_graph->addEdge(v0, v1, 7);
    m_graph->addEdge(v0, v5, 14);
    m_graph->addEdge(v0, v3, 9);  // 远距离边
    m_graph->addEdge(v1, v2, 10);
    m_graph->addEdge(v1, v5, 2);
    m_graph->addEdge(v2, v3, 11);
    m_graph->addEdge(v2, v4, 15);
    m_graph->addEdge(v3, v4, 6);
    m_graph->addEdge(v4, v5, 9);
    m_graph->addEdge(v2, v5, 3);

    m_graphWidget->rebuildFromGraph();
    updateStartVertexCombo();
    m_statusLabel->setText(QStringLiteral("示例: 稠密加权图 (6顶点10边) — 适合 Dijkstra/Prim/Kruskal 对比"));
}

void MainWindow::loadExampleDisconnected()
{
    onReset();
    m_graph->clear();
    m_graph->setType(Undirected);
    updateGraphTypeLabel();

    // 非连通图: 测试算法在不连通情况下的处理
    int a = m_graph->addVertex("A", QPointF(-200, -80));
    int b = m_graph->addVertex("B", QPointF(-80, -80));
    int c = m_graph->addVertex("C", QPointF(-140, 60));

    int d = m_graph->addVertex("D", QPointF(80, -80));
    int e = m_graph->addVertex("E", QPointF(200, -80));
    int f = m_graph->addVertex("F", QPointF(140, 60));

    int g = m_graph->addVertex("G", QPointF(0, 180));

    m_graph->addEdge(a, b, 3);
    m_graph->addEdge(b, c, 5);
    m_graph->addEdge(a, c, 2);

    m_graph->addEdge(d, e, 4);
    m_graph->addEdge(e, f, 1);
    m_graph->addEdge(d, f, 6);

    // G 是孤立顶点

    m_graphWidget->rebuildFromGraph();
    updateStartVertexCombo();
    m_statusLabel->setText(QStringLiteral("示例: 非连通图 (7顶点6边，含孤立点) — 测试边界情况"));
}

// ============ 对比模式 ============

void MainWindow::onOpenCompare()
{
    if (m_graph->vertexCount() == 0) {
        m_statusLabel->setText(QStringLiteral("图为空，请先添加顶点和边再进入对比模式"));
        return;
    }
    auto *cw = new CompareWindow(m_graph, nullptr); // nullptr使其独立窗口
    cw->setAttribute(Qt::WA_DeleteOnClose);
    cw->show();
    m_statusLabel->setText(QStringLiteral("已打开对比模式窗口"));
}

// ============ 模拟演示 ============

void MainWindow::onStartDemo()
{
    if (m_demoTimer->isActive()) {
        // 停止演示
        m_demoTimer->stop();
        m_demoBtn->setText(QStringLiteral("模拟演示"));
        m_demoOverlay->hide();
        m_graphWidget->clearHighlights();
        m_statusLabel->setText(QStringLiteral("演示已停止"));
        return;
    }

    // 开始演示：重置一切
    onReset();
    m_graph->clear();
    m_graph->setType(Undirected);
    updateGraphTypeLabel();
    updateStartVertexCombo();
    m_demoVIds.clear();
    m_demoStep = 0;

    m_demoBtn->setText(QStringLiteral("停止演示"));
    m_demoTimer->start(1300);

    // 首帧立即显示
    onDemoTick();
}

void MainWindow::onDemoTick()
{
    // 清除上一步高亮
    m_graphWidget->clearHighlights();

    auto showOverlay = [this](const QString &text) {
        m_demoOverlay->setText(text);
        m_demoOverlay->adjustSize();
        m_demoOverlay->move(20, 15);
        m_demoOverlay->show();
        m_demoOverlay->raise();
        m_statusLabel->setText(text);
    };

    switch (m_demoStep++) {
    case 0: {
        showOverlay(QStringLiteral("[ 1/13 ] 新建空图"));
        break;
    }
    case 1: {
        int id = m_graph->addVertex("A", QPointF(-150, -100));
        m_demoVIds["A"] = id;
        updateStartVertexCombo();
        m_graphWidget->highlightVertices({id}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 2/13 ] 双击空白处 -> 添加顶点 A"));
        break;
    }
    case 2: {
        int id = m_graph->addVertex("B", QPointF(150, -100));
        m_demoVIds["B"] = id;
        updateStartVertexCombo();
        m_graphWidget->highlightVertices({id}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 3/13 ] 双击空白处 -> 添加顶点 B"));
        break;
    }
    case 3: {
        int id = m_graph->addVertex("C", QPointF(-80, 120));
        m_demoVIds["C"] = id;
        updateStartVertexCombo();
        m_graphWidget->highlightVertices({id}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 4/13 ] 双击空白处 -> 添加顶点 C"));
        break;
    }
    case 4: {
        int id = m_graph->addVertex("D", QPointF(250, 80));
        m_demoVIds["D"] = id;
        updateStartVertexCombo();
        m_graphWidget->highlightVertices({id}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 5/13 ] 双击空白处 -> 添加顶点 D"));
        break;
    }
    case 5: {
        int a = m_demoVIds["A"], b = m_demoVIds["B"];
        m_graph->addEdge(a, b, 3);
        m_graphWidget->highlightVertices({a, b}, QColor(255, 140, 0));
        m_graphWidget->highlightEdges({{a, b}}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 6/13 ] 添加边模式: A -> B, 权值=3"));
        break;
    }
    case 6: {
        int a = m_demoVIds["A"], c = m_demoVIds["C"];
        m_graph->addEdge(a, c, 2);
        m_graphWidget->highlightVertices({a, c}, QColor(255, 140, 0));
        m_graphWidget->highlightEdges({{a, c}}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 7/13 ] 添加边模式: A -> C, 权值=2"));
        break;
    }
    case 7: {
        int b = m_demoVIds["B"], c = m_demoVIds["C"];
        m_graph->addEdge(b, c, 5);
        m_graphWidget->highlightVertices({b, c}, QColor(255, 140, 0));
        m_graphWidget->highlightEdges({{b, c}}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 8/13 ] 添加边模式: B -> C, 权值=5"));
        break;
    }
    case 8: {
        int b = m_demoVIds["B"], d = m_demoVIds["D"];
        m_graph->addEdge(b, d, 4);
        m_graphWidget->highlightVertices({b, d}, QColor(255, 140, 0));
        m_graphWidget->highlightEdges({{b, d}}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 9/13 ] 添加边模式: B -> D, 权值=4"));
        break;
    }
    case 9: {
        int c = m_demoVIds["C"], d = m_demoVIds["D"];
        m_graph->addEdge(c, d, 6);
        m_graphWidget->highlightVertices({c, d}, QColor(255, 140, 0));
        m_graphWidget->highlightEdges({{c, d}}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("[ 10/13 ] 添加边模式: C -> D, 权值=6"));
        break;
    }
    case 10: {
        showOverlay(QStringLiteral("[ 11/13 ] 图构建完成 -- 接下来演示删除操作"));
        break;
    }
    case 11: {
        int c = m_demoVIds["C"], d = m_demoVIds["D"];
        // 先高亮即将删除的边
        m_graphWidget->highlightEdges({{c, d}}, QColor(244, 67, 54));
        m_graph->removeEdge(c, d);
        showOverlay(QStringLiteral("[ 12/13 ] 选中边 C-D -> Delete键 -> 删除边"));
        break;
    }
    case 12: {
        int d = m_demoVIds["D"];
        m_graphWidget->highlightVertices({d}, QColor(244, 67, 54));
        showOverlay(QStringLiteral("[ 13/13 ] 选中顶点 D -> Delete键 -> 删除顶点"));
        break;
    }
    case 13: {
        int d = m_demoVIds["D"];
        m_graph->removeVertex(d);
        m_demoVIds.remove("D");
        updateStartVertexCombo();
        showOverlay(QStringLiteral("顶点 D 及关联边 B-D 已自动删除"));
        break;
    }
    case 14: {
        int b = m_demoVIds["B"], c = m_demoVIds["C"];
        m_graph->setEdgeWeight(b, c, 1);
        auto *ei = m_graphWidget->edgeItem(b, c);
        if (!ei) ei = m_graphWidget->edgeItem(c, b);
        if (ei) ei->setWeight(1);
        m_graphWidget->highlightVertices({b, c}, QColor(255, 140, 0));
        m_graphWidget->highlightEdges({{b, c}}, QColor(255, 140, 0));
        showOverlay(QStringLiteral("双击边 B-C -> 修改权值: 5 -> 1"));
        break;
    }
    default: {
        m_demoTimer->stop();
        m_demoBtn->setText(QStringLiteral("模拟演示"));
        m_demoOverlay->hide();
        m_graphWidget->clearHighlights();
        m_statusLabel->setText(QStringLiteral("模拟演示完成! 最终图: A-B(3), A-C(2), B-C(1)"));
        break;
    }
    }
}
