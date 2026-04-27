#include "randomgraphdialog.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QGroupBox>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QRadioButton>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QtMath>

RandomGraphDialog::RandomGraphDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowTitle(QStringLiteral("生成随机图"));
    setMinimumWidth(460);

    setStyleSheet(R"(
        QDialog { background: #F8FAFC; }
        QTabWidget::pane {
            border: 1px solid #E2E8F0;
            border-radius: 0 0 8px 8px;
            background: white;
            padding: 12px;
        }
        QTabBar::tab {
            background: #F1F5F9;
            border: 1px solid #E2E8F0;
            border-bottom: none;
            padding: 8px 20px;
            margin-right: 2px;
            border-radius: 6px 6px 0 0;
            font-weight: bold;
            color: #64748B;
        }
        QTabBar::tab:selected {
            background: white;
            color: #1E40AF;
            border-bottom: 2px solid white;
        }
        QTabBar::tab:hover:!selected {
            background: #E8F0FE;
            color: #3B82F6;
        }
        QSpinBox, QDoubleSpinBox {
            border: 1px solid #D0D5DD;
            border-radius: 6px;
            padding: 4px 8px;
            background: white;
            min-height: 24px;
        }
        QSpinBox:focus, QDoubleSpinBox:focus {
            border-color: #3B82F6;
        }
        QRadioButton { spacing: 6px; color: #334155; }
        QRadioButton::indicator {
            width: 16px; height: 16px;
        }
        QCheckBox { spacing: 6px; color: #334155; }
        QGroupBox {
            font-weight: bold; color: #475569;
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            margin-top: 12px; padding-top: 16px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 10px; padding: 0 4px;
            background: #F8FAFC;
        }
        QPushButton {
            border-radius: 6px;
            padding: 7px 20px;
            font-weight: bold;
            font-size: 13px;
        }
        QDialogButtonBox QPushButton[text="生成"] {
            background: #3B82F6; color: white; border: none;
        }
        QDialogButtonBox QPushButton[text="生成"]:hover {
            background: #2563EB;
        }
        QDialogButtonBox QPushButton[text="取消"] {
            background: #F1F5F9; color: #64748B;
            border: 1px solid #D0D5DD;
        }
        QDialogButtonBox QPushButton[text="取消"]:hover {
            background: #E2E8F0;
        }
    )");

    auto *mainLayout = new QVBoxLayout(this);

    m_tabs = new QTabWidget;
    auto *erTab  = new QWidget;
    auto *rggTab = new QWidget;
    auto *baTab  = new QWidget;
    auto *wsTab  = new QWidget;
    setupERTab(erTab);
    setupRGGTab(rggTab);
    setupBATab(baTab);
    setupWSTab(wsTab);
    m_tabs->addTab(erTab,  QStringLiteral("Erdős-Rényi"));
    m_tabs->addTab(rggTab, QStringLiteral("随机几何图 RGG"));
    m_tabs->addTab(baTab,  QStringLiteral("Barabási-Albert"));
    m_tabs->addTab(wsTab,  QStringLiteral("Watts-Strogatz"));
    mainLayout->addWidget(m_tabs);

    // 权值设置
    auto *wGroup = new QGroupBox(QStringLiteral("边权值"));
    auto *wLayout = new QHBoxLayout(wGroup);
    m_weightedCheck = new QCheckBox(QStringLiteral("赋随机权值"));
    m_weightMin = new QSpinBox;
    m_weightMin->setRange(1, 999);
    m_weightMin->setValue(1);
    m_weightMin->setEnabled(false);
    m_weightMax = new QSpinBox;
    m_weightMax->setRange(1, 999);
    m_weightMax->setValue(10);
    m_weightMax->setEnabled(false);
    wLayout->addWidget(m_weightedCheck);
    wLayout->addWidget(new QLabel(QStringLiteral("范围:")));
    wLayout->addWidget(m_weightMin);
    wLayout->addWidget(new QLabel(QStringLiteral("~")));
    wLayout->addWidget(m_weightMax);
    wLayout->addStretch();
    connect(m_weightedCheck, &QCheckBox::toggled, m_weightMin, &QSpinBox::setEnabled);
    connect(m_weightedCheck, &QCheckBox::toggled, m_weightMax, &QSpinBox::setEnabled);
    mainLayout->addWidget(wGroup);

    // 按钮
    auto *btnBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    btnBox->button(QDialogButtonBox::Ok)->setText(QStringLiteral("生成"));
    btnBox->button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    connect(btnBox, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(btnBox, &QDialogButtonBox::rejected, this, &QDialog::reject);
    mainLayout->addWidget(btnBox);

    connect(m_tabs, &QTabWidget::currentChanged, this, [this]{ updateExpectedInfo(); });
    updateExpectedInfo();
}

// ====================== ER tab ======================

void RandomGraphDialog::setupERTab(QWidget *tab) {
    auto *layout = new QVBoxLayout(tab);

    m_erGnpRadio = new QRadioButton(QStringLiteral("G(n, p) — 概率模型"));
    m_erGnmRadio = new QRadioButton(QStringLiteral("G(n, m) — 固定边数"));
    m_erGnpRadio->setChecked(true);

    auto *form = new QFormLayout;
    m_erN = new QSpinBox;
    m_erN->setRange(2, 500);
    m_erN->setValue(20);
    m_erP = new QDoubleSpinBox;
    m_erP->setRange(0.0, 1.0);
    m_erP->setSingleStep(0.05);
    m_erP->setDecimals(3);
    m_erP->setValue(0.15);
    m_erM = new QSpinBox;
    m_erM->setRange(0, 124750);
    m_erM->setValue(30);
    m_erM->setEnabled(false);

    form->addRow(QStringLiteral("顶点数 n:"), m_erN);
    form->addRow(m_erGnpRadio);
    form->addRow(QStringLiteral("  概率 p:"), m_erP);
    form->addRow(m_erGnmRadio);
    form->addRow(QStringLiteral("  边数 m:"), m_erM);
    layout->addLayout(form);

    m_erInfo = new QLabel;
    m_erInfo->setStyleSheet("color: #555; font-style: italic;");
    m_erInfo->setWordWrap(true);
    layout->addWidget(m_erInfo);
    layout->addStretch();

    connect(m_erGnpRadio, &QRadioButton::toggled, [this](bool on){
        m_erP->setEnabled(on);
        m_erM->setEnabled(!on);
        updateExpectedInfo();
    });
    connect(m_erN, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]{ updateExpectedInfo(); });
    connect(m_erP, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this]{ updateExpectedInfo(); });
    connect(m_erM, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]{ updateExpectedInfo(); });
}

// ====================== RGG tab ======================

void RandomGraphDialog::setupRGGTab(QWidget *tab) {
    auto *layout = new QVBoxLayout(tab);

    auto *desc = new QLabel(QStringLiteral(
        "在单位正方形 [0,1]² 内均匀撒 n 个点, 两点欧氏距离 ≤ r 时连边。\n"
        "邻近性导致高聚类, 适合刻画无线传感网、地理邻接网络等空间场景。"));
    desc->setWordWrap(true);
    desc->setStyleSheet("color: #444;");
    layout->addWidget(desc);

    auto *form = new QFormLayout;
    m_rggN = new QSpinBox;
    m_rggN->setRange(2, 500);
    m_rggN->setValue(50);
    m_rggR = new QDoubleSpinBox;
    m_rggR->setRange(0.0, 1.5);
    m_rggR->setSingleStep(0.02);
    m_rggR->setDecimals(3);
    m_rggR->setValue(0.20);
    form->addRow(QStringLiteral("顶点数 n:"), m_rggN);
    form->addRow(QStringLiteral("连接半径 r:"), m_rggR);
    layout->addLayout(form);

    m_rggInfo = new QLabel;
    m_rggInfo->setStyleSheet("color: #555; font-style: italic;");
    m_rggInfo->setWordWrap(true);
    layout->addWidget(m_rggInfo);
    layout->addStretch();

    connect(m_rggN, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this]{ updateExpectedInfo(); });
    connect(m_rggR, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this]{ updateExpectedInfo(); });
}

// ====================== BA tab ======================

void RandomGraphDialog::setupBATab(QWidget *tab) {
    auto *layout = new QVBoxLayout(tab);

    auto *desc = new QLabel(QStringLiteral(
        "优先连接机制: 新顶点倾向连接到高度数的已有顶点，\n"
        "产生少数枢纽-多数低度的无标度(scale-free)网络。"));
    desc->setWordWrap(true);
    desc->setStyleSheet("color: #444;");
    layout->addWidget(desc);

    auto *form = new QFormLayout;
    m_baN = new QSpinBox;
    m_baN->setRange(3, 500);
    m_baN->setValue(30);
    m_baM = new QSpinBox;
    m_baM->setRange(1, 50);
    m_baM->setValue(2);
    m_baM0 = new QSpinBox;
    m_baM0->setRange(2, 50);
    m_baM0->setValue(3);
    form->addRow(QStringLiteral("总顶点数 n:"), m_baN);
    form->addRow(QStringLiteral("每步连边数 m:"), m_baM);
    form->addRow(QStringLiteral("初始顶点数 m₀:"), m_baM0);
    layout->addLayout(form);

    m_baInfo = new QLabel;
    m_baInfo->setStyleSheet("color: #555; font-style: italic;");
    m_baInfo->setWordWrap(true);
    layout->addWidget(m_baInfo);
    layout->addStretch();

    auto updateBA = [this]{
        if (m_baM0->value() < m_baM->value())
            m_baM0->setValue(m_baM->value());
        if (m_baM0->value() > m_baN->value())
            m_baM0->setValue(m_baN->value());
        updateExpectedInfo();
    };
    connect(m_baN, QOverload<int>::of(&QSpinBox::valueChanged), this, updateBA);
    connect(m_baM, QOverload<int>::of(&QSpinBox::valueChanged), this, updateBA);
    connect(m_baM0, QOverload<int>::of(&QSpinBox::valueChanged), this, updateBA);
}

// ====================== WS tab ======================

void RandomGraphDialog::setupWSTab(QWidget *tab) {
    auto *layout = new QVBoxLayout(tab);

    auto *desc = new QLabel(QStringLiteral(
        "从规则环形格子开始，以概率 p 随机重连边，\n"
        "兼具高聚类系数与短平均路径的小世界(small-world)特性。"));
    desc->setWordWrap(true);
    desc->setStyleSheet("color: #444;");
    layout->addWidget(desc);

    auto *form = new QFormLayout;
    m_wsN = new QSpinBox;
    m_wsN->setRange(4, 500);
    m_wsN->setValue(20);
    m_wsK = new QSpinBox;
    m_wsK->setRange(2, 50);
    m_wsK->setSingleStep(2);
    m_wsK->setValue(4);
    m_wsP = new QDoubleSpinBox;
    m_wsP->setRange(0.0, 1.0);
    m_wsP->setSingleStep(0.05);
    m_wsP->setDecimals(3);
    m_wsP->setValue(0.3);
    form->addRow(QStringLiteral("顶点数 n:"), m_wsN);
    form->addRow(QStringLiteral("近邻数 K (偶数):"), m_wsK);
    form->addRow(QStringLiteral("重连概率 β:"), m_wsP);
    layout->addLayout(form);

    m_wsInfo = new QLabel;
    m_wsInfo->setStyleSheet("color: #555; font-style: italic;");
    m_wsInfo->setWordWrap(true);
    layout->addWidget(m_wsInfo);
    layout->addStretch();

    auto updateWS = [this]{
        int k = m_wsK->value();
        if (k % 2 != 0) m_wsK->setValue(k + 1);
        updateExpectedInfo();
    };
    connect(m_wsN, QOverload<int>::of(&QSpinBox::valueChanged), this, updateWS);
    connect(m_wsK, QOverload<int>::of(&QSpinBox::valueChanged), this, updateWS);
    connect(m_wsP, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, [this]{ updateExpectedInfo(); });
}

// ====================== 实时预估 ======================

void RandomGraphDialog::updateExpectedInfo() {
    int tab = m_tabs->currentIndex();

    if (tab == 0) {
        // ER
        int n = m_erN->value();
        qint64 maxE = qint64(n) * (n - 1) / 2;
        if (m_erGnpRadio->isChecked()) {
            double p = m_erP->value();
            double expE = p * maxE;
            double avgDeg = p * (n - 1);
            m_erInfo->setText(QStringLiteral(
                "预期: ~%1 条边, 平均度 ~%2, 最大可能边 %3\n"
                "p ≈ ln(n)/n ≈ %4 时图几乎必然连通")
                .arg(int(expE)).arg(avgDeg, 0, 'f', 2).arg(maxE)
                .arg(std::log(double(n)) / n, 0, 'f', 4));
        } else {
            int m = m_erM->value();
            double avgDeg = 2.0 * m / n;
            m_erInfo->setText(QStringLiteral(
                "固定 %1 条边, 平均度 = %2, 最大可能边 %3")
                .arg(m).arg(avgDeg, 0, 'f', 2).arg(maxE));
        }
    } else if (tab == 1) {
        // RGG
        int n = m_rggN->value();
        double r = m_rggR->value();
        double expE = (n * (n - 1) / 2.0) * M_PI * r * r;
        double avgDeg = (n - 1) * M_PI * r * r;
        double rc = std::sqrt(std::log(double(n)) / (M_PI * n));
        m_rggInfo->setText(QStringLiteral(
            "预期: ~%1 条边, 平均度 ~%2 (忽略边界效应)\n"
            "连通性阈值 r_c ≈ √(ln n / (π n)) ≈ %3")
            .arg(int(expE)).arg(avgDeg, 0, 'f', 2)
            .arg(rc, 0, 'f', 4));
    } else if (tab == 2) {
        // BA
        int n = m_baN->value();
        int m = m_baM->value();
        int m0 = m_baM0->value();
        int initEdges = m0 * (m0 - 1) / 2;
        int growEdges = (n - m0) * m;
        int totalE = initEdges + growEdges;
        double avgDeg = 2.0 * totalE / n;
        m_baInfo->setText(QStringLiteral(
            "预期: %1 条边 (初始 K_%2: %3 + 增长: %4), 平均度 ~%5\n"
            "度分布近似幂律 P(k) ~ k^{-3}")
            .arg(totalE).arg(m0).arg(initEdges).arg(growEdges)
            .arg(avgDeg, 0, 'f', 2));
    } else {
        // WS
        int n = m_wsN->value();
        int k = m_wsK->value();
        int totalE = n * k / 2;
        double avgDeg = double(k);
        m_wsInfo->setText(QStringLiteral(
            "环形格子: %1 条边, 每顶点度 = %2\n"
            "β=0 → 规则图 (C 高, L 高); β=1 → 随机图 (C 低, L 低)\n"
            "β≈0.1~0.3 → 小世界 (C 仍高, L 降低)")
            .arg(totalE).arg(avgDeg, 0, 'f', 0));
    }
}

// ====================== 生成 ======================

QString RandomGraphDialog::apply(Graph *graph) const {
    GraphGenerator::WeightConfig wc;
    wc.randomWeights = m_weightedCheck->isChecked();
    wc.minWeight = m_weightMin->value();
    wc.maxWeight = m_weightMax->value();

    GraphGenerator::LayoutConfig lc;

    int tab = m_tabs->currentIndex();

    if (tab == 0) {
        int n = m_erN->value();
        if (m_erGnpRadio->isChecked()) {
            double p = m_erP->value();
            GraphGenerator::generateErdosRenyiGnp(graph, n, p, wc, lc);
            return QStringLiteral("Erdős-Rényi G(%1, p=%2)").arg(n).arg(p, 0, 'f', 3);
        } else {
            int m = m_erM->value();
            GraphGenerator::generateErdosRenyiGnm(graph, n, m, wc, lc);
            return QStringLiteral("Erdős-Rényi G(%1, m=%2)").arg(n).arg(m);
        }
    } else if (tab == 1) {
        int n = m_rggN->value();
        double r = m_rggR->value();
        GraphGenerator::generateRandomGeometric(graph, n, r, wc, lc);
        return QStringLiteral("RGG (n=%1, r=%2)").arg(n).arg(r, 0, 'f', 3);
    } else if (tab == 2) {
        int n = m_baN->value();
        int m = m_baM->value();
        int m0 = m_baM0->value();
        GraphGenerator::generateBarabasiAlbert(graph, n, m, m0, wc, lc);
        return QStringLiteral("Barabási-Albert (n=%1, m=%2, m₀=%3)").arg(n).arg(m).arg(m0);
    } else {
        int n = m_wsN->value();
        int k = m_wsK->value();
        double p = m_wsP->value();
        GraphGenerator::generateWattsStrogatz(graph, n, k, p, wc, lc);
        return QStringLiteral("Watts-Strogatz (n=%1, K=%2, β=%3)").arg(n).arg(k).arg(p, 0, 'f', 3);
    }
}
