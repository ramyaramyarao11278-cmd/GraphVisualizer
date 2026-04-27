#include "comparewindow.h"
#include "algorithms.h"
#include <QSplitter>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGroupBox>
#include <QToolBar>
#include <QStatusBar>

CompareWindow::CompareWindow(Graph *sourceGraph, QWidget *parent)
    : QMainWindow(parent)
{
    m_timer = new QTimer(this);
    connect(m_timer, &QTimer::timeout, this, &CompareWindow::onTimerTick);

    // 为左右面板各克隆一份图
    m_panels[0].graph = sourceGraph->clone(this);
    m_panels[1].graph = sourceGraph->clone(this);

    setupUI();

    // 初始化起点选择
    updateStartVertexCombo(0);
    updateStartVertexCombo(1);

    // 默认左DFS右BFS
    m_panels[0].algorithmCombo->setCurrentIndex(0);
    m_panels[1].algorithmCombo->setCurrentIndex(1);

    setWindowTitle(QStringLiteral("算法对比模式"));
    resize(1400, 800);
}

CompareWindow::~CompareWindow()
{
    for (int i = 0; i < 2; ++i) {
        delete m_panels[i].algorithm;
    }
}

void CompareWindow::setupUI()
{
    // 同步控制工具栏
    auto *toolbar = addToolBar(QStringLiteral("同步控制"));
    toolbar->setMovable(false);

    auto *syncLabel = new QLabel(QStringLiteral("  同步控制: "));
    syncLabel->setStyleSheet("font-weight: bold;");
    toolbar->addWidget(syncLabel);

    m_playBtn = new QPushButton(QStringLiteral("▶ 同步播放"));
    connect(m_playBtn, &QPushButton::clicked, this, &CompareWindow::onSyncPlay);
    toolbar->addWidget(m_playBtn);

    m_pauseBtn = new QPushButton(QStringLiteral("⏸ 暂停"));
    m_pauseBtn->setEnabled(false);
    connect(m_pauseBtn, &QPushButton::clicked, this, &CompareWindow::onSyncPause);
    toolbar->addWidget(m_pauseBtn);

    auto *syncStepBtn = new QPushButton(QStringLiteral("⏭ 同步单步"));
    connect(syncStepBtn, &QPushButton::clicked, this, &CompareWindow::onSyncStep);
    toolbar->addWidget(syncStepBtn);

    auto *syncResetBtn = new QPushButton(QStringLiteral("⏹ 同步重置"));
    connect(syncResetBtn, &QPushButton::clicked, this, &CompareWindow::onSyncReset);
    toolbar->addWidget(syncResetBtn);

    toolbar->addSeparator();

    toolbar->addWidget(new QLabel(QStringLiteral("  速度:")));
    m_speedSlider = new QSlider(Qt::Horizontal);
    m_speedSlider->setRange(100, 2000);
    m_speedSlider->setValue(800);
    m_speedSlider->setInvertedAppearance(true);
    m_speedSlider->setMaximumWidth(150);
    m_speedLabel = new QLabel(QStringLiteral("800ms"));
    connect(m_speedSlider, &QSlider::valueChanged, [this](int val) {
        m_speedLabel->setText(QStringLiteral("%1ms").arg(val));
        if (m_timer->isActive())
            m_timer->setInterval(val);
    });
    toolbar->addWidget(m_speedSlider);
    toolbar->addWidget(m_speedLabel);

    // 左右分屏
    auto *splitter = new QSplitter(Qt::Horizontal);

    splitter->addWidget(createPanelWidget(0));
    splitter->addWidget(createPanelWidget(1));
    splitter->setSizes({700, 700});

    setCentralWidget(splitter);
}

QWidget *CompareWindow::createPanelWidget(int idx)
{
    auto &p = m_panels[idx];

    auto *container = new QWidget();
    auto *mainLayout = new QVBoxLayout(container);
    mainLayout->setContentsMargins(4, 4, 4, 4);

    // 标题
    p.titleLabel = new QLabel(idx == 0 ? QStringLiteral("◀ 左侧算法") : QStringLiteral("▶ 右侧算法"));
    p.titleLabel->setStyleSheet("font-size: 14px; font-weight: bold; color: #336; padding: 4px;");
    p.titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(p.titleLabel);

    // 算法选择行
    auto *ctrlLayout = new QHBoxLayout();

    p.algorithmCombo = new QComboBox();
    p.algorithmCombo->addItem(QStringLiteral("DFS (深度优先搜索)"));
    p.algorithmCombo->addItem(QStringLiteral("BFS (广度优先搜索)"));
    p.algorithmCombo->addItem(QStringLiteral("Dijkstra (最短路径)"));
    p.algorithmCombo->addItem(QStringLiteral("Prim (最小生成树)"));
    p.algorithmCombo->addItem(QStringLiteral("Kruskal (最小生成树)"));
    p.algorithmCombo->addItem(QStringLiteral("拓扑排序"));
    connect(p.algorithmCombo, QOverload<int>::of(&QComboBox::currentIndexChanged),
            [this, idx](int) { onPanelAlgorithmChanged(idx); });
    ctrlLayout->addWidget(new QLabel(QStringLiteral("算法:")));
    ctrlLayout->addWidget(p.algorithmCombo);

    ctrlLayout->addWidget(new QLabel(QStringLiteral("起点:")));
    p.startVertexCombo = new QComboBox();
    ctrlLayout->addWidget(p.startVertexCombo);

    p.stepBtn = new QPushButton(QStringLiteral("单步"));
    connect(p.stepBtn, &QPushButton::clicked, [this, idx]() { onPanelStep(idx); });
    ctrlLayout->addWidget(p.stepBtn);

    p.resetBtn = new QPushButton(QStringLiteral("重置"));
    connect(p.resetBtn, &QPushButton::clicked, [this, idx]() { onPanelReset(idx); });
    ctrlLayout->addWidget(p.resetBtn);

    mainLayout->addLayout(ctrlLayout);

    // 图视图
    p.graphWidget = new GraphWidget(p.graph);
    p.graphWidget->setMinimumHeight(300);
    mainLayout->addWidget(p.graphWidget, 3);

    // 数据和日志
    auto *dataSplitter = new QSplitter(Qt::Horizontal);

    p.dataDisplay = new QTextEdit();
    p.dataDisplay->setReadOnly(true);
    p.dataDisplay->setFont(QFont("Consolas", 9));
    p.dataDisplay->setPlaceholderText(QStringLiteral("数据结构..."));
    p.dataDisplay->setMaximumHeight(200);
    dataSplitter->addWidget(p.dataDisplay);

    p.logDisplay = new QTextEdit();
    p.logDisplay->setReadOnly(true);
    p.logDisplay->setFont(QFont("Consolas", 9));
    p.logDisplay->setPlaceholderText(QStringLiteral("执行日志..."));
    p.logDisplay->setMaximumHeight(200);
    dataSplitter->addWidget(p.logDisplay);

    mainLayout->addWidget(dataSplitter, 1);

    // 构建图视图
    p.graphWidget->rebuildFromGraph();

    return container;
}

void CompareWindow::updateStartVertexCombo(int idx)
{
    auto &p = m_panels[idx];
    p.startVertexCombo->clear();
    for (int id : p.graph->vertices()) {
        p.startVertexCombo->addItem(p.graph->vertex(id).label, id);
    }
}

void CompareWindow::createAlgorithm(int idx)
{
    auto &p = m_panels[idx];
    delete p.algorithm;
    p.algorithm = nullptr;

    int algoIdx = p.algorithmCombo->currentIndex();
    switch (algoIdx) {
    case 0: p.algorithm = new DFSAlgorithm(p.graph, this); break;
    case 1: p.algorithm = new BFSAlgorithm(p.graph, this); break;
    case 2: p.algorithm = new DijkstraAlgorithm(p.graph, this); break;
    case 3: p.algorithm = new PrimAlgorithm(p.graph, this); break;
    case 4: p.algorithm = new KruskalAlgorithm(p.graph, this); break;
    case 5: p.algorithm = new TopologicalSortAlgorithm(p.graph, this); break;
    }

    if (p.algorithm) {
        if (p.algorithm->needsStartVertex() && p.startVertexCombo->count() > 0) {
            p.algorithm->setStartVertex(p.startVertexCombo->currentData().toInt());
        }
        connect(p.algorithm, &AlgorithmBase::stateChanged,
                [this, idx]() { onAlgorithmStateChanged(idx); });
        connect(p.algorithm, &AlgorithmBase::finished,
                [this, idx](const QString &s) { onAlgorithmFinished(idx, s); });
    }
}

// ============ 同步控制 ============

void CompareWindow::onSyncPlay()
{
    for (int i = 0; i < 2; ++i) {
        if (!m_panels[i].algorithm)
            createAlgorithm(i);
    }
    m_timer->start(m_speedSlider->value());
    m_playBtn->setEnabled(false);
    m_pauseBtn->setEnabled(true);
}

void CompareWindow::onSyncPause()
{
    m_timer->stop();
    m_playBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
}

void CompareWindow::onSyncStep()
{
    for (int i = 0; i < 2; ++i) {
        if (!m_panels[i].algorithm)
            createAlgorithm(i);
        if (m_panels[i].algorithm && !m_panels[i].algorithm->isFinished())
            m_panels[i].algorithm->step();
    }
}

void CompareWindow::onSyncReset()
{
    m_timer->stop();
    m_playBtn->setEnabled(true);
    m_pauseBtn->setEnabled(false);
    for (int i = 0; i < 2; ++i)
        onPanelReset(i);
}

void CompareWindow::onTimerTick()
{
    bool anyRunning = false;
    for (int i = 0; i < 2; ++i) {
        if (m_panels[i].algorithm && !m_panels[i].algorithm->isFinished()) {
            m_panels[i].algorithm->step();
            anyRunning = true;
        }
    }
    if (!anyRunning) {
        m_timer->stop();
        m_playBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
    }
}

// ============ 单面板操作 ============

void CompareWindow::onPanelStep(int idx)
{
    auto &p = m_panels[idx];
    if (!p.algorithm) createAlgorithm(idx);
    if (p.algorithm && !p.algorithm->isFinished())
        p.algorithm->step();
}

void CompareWindow::onPanelReset(int idx)
{
    auto &p = m_panels[idx];
    if (p.algorithm) {
        p.algorithm->reset();
        delete p.algorithm;
        p.algorithm = nullptr;
    }
    p.graphWidget->clearHighlights();
    p.dataDisplay->clear();
    p.logDisplay->clear();
}

void CompareWindow::onPanelAlgorithmChanged(int idx)
{
    onPanelReset(idx);
    updateStartVertexCombo(idx);

    int algoIdx = m_panels[idx].algorithmCombo->currentIndex();
    bool needStart = (algoIdx <= 3);
    m_panels[idx].startVertexCombo->setEnabled(needStart);

    // 更新标题（含图类型）
    QString algoName = m_panels[idx].algorithmCombo->currentText();
    QString side = (idx == 0) ? QStringLiteral("◀ 左: ") : QStringLiteral("▶ 右: ");
    QString graphType = m_panels[idx].graph->type() == Directed
        ? QStringLiteral(" [有向图]") : QStringLiteral(" [无向图]");
    m_panels[idx].titleLabel->setText(side + algoName + graphType);
}

void CompareWindow::onAlgorithmStateChanged(int idx)
{
    applyHighlights(idx);

    auto &p = m_panels[idx];
    if (p.algorithm) {
        p.dataDisplay->setText(p.algorithm->dataDisplayLines().join("\n"));
        p.logDisplay->setText(p.algorithm->log().join("\n"));
        auto cursor = p.logDisplay->textCursor();
        cursor.movePosition(QTextCursor::End);
        p.logDisplay->setTextCursor(cursor);
    }
}

void CompareWindow::onAlgorithmFinished(int idx, const QString &summary)
{
    auto &p = m_panels[idx];
    p.logDisplay->append(QStringLiteral("\n--- %1 ---").arg(summary));
    applyHighlights(idx);

    // 检查两侧是否都完成
    bool allDone = true;
    for (int i = 0; i < 2; ++i) {
        if (m_panels[i].algorithm && !m_panels[i].algorithm->isFinished())
            allDone = false;
    }
    if (allDone) {
        m_timer->stop();
        m_playBtn->setEnabled(true);
        m_pauseBtn->setEnabled(false);
    }
}

void CompareWindow::applyHighlights(int idx)
{
    auto &p = m_panels[idx];
    if (!p.algorithm) return;

    p.graphWidget->clearAllToGray();

    if (p.algorithm->isFinished()) {
        QList<int> visitedList = p.algorithm->visitedVertices().values();
        p.graphWidget->highlightVertices(visitedList, QColor(76, 175, 80));
        p.graphWidget->highlightEdges(p.algorithm->resultEdges(), QColor(33, 150, 243));
        p.graphWidget->highlightVertices(p.algorithm->resultVertices(), QColor(33, 150, 243));
    } else {
        QList<int> visitedList = p.algorithm->visitedVertices().values();
        p.graphWidget->highlightVertices(visitedList, QColor(76, 175, 80));
        p.graphWidget->highlightEdges(p.algorithm->highlightedEdges(), QColor(255, 140, 0));
        p.graphWidget->highlightVertices(p.algorithm->highlightedVertices(), QColor(244, 67, 54));
    }
}
