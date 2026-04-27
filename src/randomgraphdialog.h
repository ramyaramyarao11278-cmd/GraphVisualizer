#ifndef RANDOMGRAPHDIALOG_H
#define RANDOMGRAPHDIALOG_H

#include <QDialog>
#include "graphgenerator.h"

class Graph;
class QTabWidget;
class QSpinBox;
class QDoubleSpinBox;
class QRadioButton;
class QCheckBox;
class QLabel;

class RandomGraphDialog : public QDialog {
    Q_OBJECT
public:
    explicit RandomGraphDialog(QWidget *parent = nullptr);

    QString apply(Graph *graph) const;

private:
    void setupERTab(QWidget *tab);
    void setupBATab(QWidget *tab);
    void setupWSTab(QWidget *tab);
    void updateExpectedInfo();

    QTabWidget *m_tabs;

    // ER
    QRadioButton *m_erGnpRadio;
    QRadioButton *m_erGnmRadio;
    QSpinBox     *m_erN;
    QDoubleSpinBox *m_erP;
    QSpinBox     *m_erM;
    QLabel       *m_erInfo;

    // BA
    QSpinBox *m_baN;
    QSpinBox *m_baM;
    QSpinBox *m_baM0;
    QLabel   *m_baInfo;

    // WS
    QSpinBox       *m_wsN;
    QSpinBox       *m_wsK;
    QDoubleSpinBox *m_wsP;
    QLabel         *m_wsInfo;

    // 公共
    QCheckBox *m_weightedCheck;
    QSpinBox  *m_weightMin;
    QSpinBox  *m_weightMax;
};

#endif
