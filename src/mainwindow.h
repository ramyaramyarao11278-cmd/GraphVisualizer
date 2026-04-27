#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>
#include <QMenu>
#include "graph.h"
#include "graphwidget.h"
#include "algorithmbase.h"

class MainWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAlgorithmSelected(int index);
    void onPlay();
    void onPause();
    void onStepOnce();
    void onReset();
    void onAlgorithmStateChanged();
    void onAlgorithmFinished(const QString &summary);
    void onTimerTick();

    // 图编辑
    void onNewGraph();
    void onToggleGraphType();
    void onToggleAddEdge(bool checked);
    void onSaveGraph();
    void onLoadGraph();

    // 示例图
    void loadExampleUndirected();
    void loadExampleDirectedDAG();
    void loadExampleWeightedDense();
    void loadExampleDisconnected();

    // 随机图生成
    void onGenerateRandomGraph();

    // 对比模式
    void onOpenCompare();

    // 模拟演示
    void onStartDemo();
    void onDemoTick();

private:
    void setupUI();
    void setupToolBar();
    void setupControlPanel();
    void setupDataPanel();
    void createAlgorithm(int index);
    void updateStartVertexCombo();
    void applyHighlights();
    void updateGraphTypeLabel();

    Graph *m_graph;
    GraphWidget *m_graphWidget;
    AlgorithmBase *m_algorithm;
    QTimer *m_timer;

    // 控制面板
    QComboBox *m_algorithmCombo;
    QComboBox *m_startVertexCombo;
    QPushButton *m_playBtn;
    QPushButton *m_pauseBtn;
    QPushButton *m_stepBtn;
    QPushButton *m_resetBtn;
    QSlider *m_speedSlider;
    QLabel *m_speedLabel;
    QLabel *m_statusLabel;

    // 数据面板
    QTextEdit *m_dataDisplay;
    QTextEdit *m_logDisplay;

    // 工具栏
    QPushButton *m_addEdgeBtn;
    QLabel *m_graphTypeLabel;

    // 模拟演示
    QPushButton *m_demoBtn;
    QLabel *m_demoOverlay;
    QTimer *m_demoTimer;
    int m_demoStep;
    QMap<QString, int> m_demoVIds;
};

#endif // MAINWINDOW_H
