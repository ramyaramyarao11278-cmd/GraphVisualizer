#ifndef COMPAREWINDOW_H
#define COMPAREWINDOW_H

#include <QMainWindow>
#include <QComboBox>
#include <QPushButton>
#include <QSlider>
#include <QTextEdit>
#include <QLabel>
#include <QTimer>
#include "graph.h"
#include "graphwidget.h"
#include "algorithmbase.h"

// 单个算法面板（图视图 + 控制 + 数据展示）
struct AlgorithmPanel {
    Graph *graph = nullptr;
    GraphWidget *graphWidget = nullptr;
    AlgorithmBase *algorithm = nullptr;

    QComboBox *algorithmCombo = nullptr;
    QComboBox *startVertexCombo = nullptr;
    QPushButton *stepBtn = nullptr;
    QPushButton *resetBtn = nullptr;
    QTextEdit *dataDisplay = nullptr;
    QTextEdit *logDisplay = nullptr;
    QLabel *titleLabel = nullptr;
};

class CompareWindow : public QMainWindow {
    Q_OBJECT
public:
    explicit CompareWindow(Graph *sourceGraph, QWidget *parent = nullptr);
    ~CompareWindow();

private slots:
    void onSyncPlay();
    void onSyncPause();
    void onSyncStep();
    void onSyncReset();
    void onTimerTick();

    // 单面板操作
    void onPanelStep(int panelIndex);
    void onPanelReset(int panelIndex);
    void onPanelAlgorithmChanged(int panelIndex);
    void onAlgorithmStateChanged(int panelIndex);
    void onAlgorithmFinished(int panelIndex, const QString &summary);

private:
    void setupUI();
    QWidget *createPanelWidget(int panelIndex);
    void createAlgorithm(int panelIndex);
    void updateStartVertexCombo(int panelIndex);
    void applyHighlights(int panelIndex);

    AlgorithmPanel m_panels[2];
    QTimer *m_timer;
    QSlider *m_speedSlider;
    QLabel *m_speedLabel;
    QPushButton *m_playBtn;
    QPushButton *m_pauseBtn;
};

#endif // COMPAREWINDOW_H
