#include <QApplication>
#include <QPalette>
#include <QFont>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setStyle("Fusion");

    QFont appFont("Microsoft YaHei UI", 9);
    appFont.setHintingPreference(QFont::PreferFullHinting);
    app.setFont(appFont);

    QPalette pal;
    pal.setColor(QPalette::Window,          QColor("#F5F7FA"));
    pal.setColor(QPalette::WindowText,      QColor("#2C3E50"));
    pal.setColor(QPalette::Base,            QColor("#FFFFFF"));
    pal.setColor(QPalette::AlternateBase,   QColor("#F0F2F5"));
    pal.setColor(QPalette::ToolTipBase,     QColor("#2C3E50"));
    pal.setColor(QPalette::ToolTipText,     Qt::white);
    pal.setColor(QPalette::Text,            QColor("#2C3E50"));
    pal.setColor(QPalette::Button,          QColor("#EDF0F5"));
    pal.setColor(QPalette::ButtonText,      QColor("#2C3E50"));
    pal.setColor(QPalette::Highlight,       QColor("#3B82F6"));
    pal.setColor(QPalette::HighlightedText, Qt::white);
    pal.setColor(QPalette::Link,            QColor("#3B82F6"));
    app.setPalette(pal);

    app.setStyleSheet(R"(
        QMainWindow { background: #F5F7FA; }

        QToolBar {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #FFFFFF, stop:1 #F0F2F5);
            border-bottom: 1px solid #E0E4E8;
            spacing: 4px;
            padding: 4px 6px;
        }
        QToolBar::separator {
            width: 1px;
            background: #DDE1E6;
            margin: 4px 6px;
        }

        QToolBar QPushButton {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #FFFFFF, stop:1 #F0F2F5);
            border: 1px solid #D0D5DD;
            border-radius: 6px;
            padding: 5px 14px;
            font-size: 12px;
            color: #344054;
            min-height: 18px;
        }
        QToolBar QPushButton:hover {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #F0F4FF, stop:1 #E0E7FF);
            border-color: #93AAFD;
            color: #1D4ED8;
        }
        QToolBar QPushButton:pressed {
            background: #DBEAFE;
            border-color: #60A5FA;
        }
        QToolBar QPushButton:checked {
            background: #DBEAFE;
            border-color: #3B82F6;
            color: #1E40AF;
            font-weight: bold;
        }
        QToolBar QPushButton::menu-indicator {
            width: 0; height: 0;
        }

        QDockWidget {
            font-weight: bold;
            color: #475569;
            titlebar-close-icon: none;
        }
        QDockWidget::title {
            background: qlineargradient(x1:0,y1:0,x2:0,y2:1,
                stop:0 #F8FAFC, stop:1 #F1F5F9);
            border-bottom: 1px solid #E2E8F0;
            padding: 8px 12px;
            font-size: 13px;
        }

        QGroupBox {
            font-weight: bold;
            font-size: 12px;
            color: #475569;
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            margin-top: 14px;
            padding-top: 18px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            subcontrol-position: top left;
            left: 12px;
            padding: 0 6px;
            background: #F5F7FA;
            color: #334155;
        }

        QComboBox {
            border: 1px solid #D0D5DD;
            border-radius: 6px;
            padding: 4px 10px;
            background: white;
            min-height: 22px;
        }
        QComboBox:hover { border-color: #93AAFD; }
        QComboBox:focus { border-color: #3B82F6; }
        QComboBox::drop-down {
            border: none;
            width: 24px;
        }

        QTextEdit {
            border: 1px solid #E2E8F0;
            border-radius: 6px;
            background: #FFFFFF;
            padding: 6px;
            selection-background-color: #BFDBFE;
        }

        QSlider::groove:horizontal {
            height: 6px;
            background: #E2E8F0;
            border-radius: 3px;
        }
        QSlider::handle:horizontal {
            width: 16px; height: 16px;
            margin: -5px 0;
            background: qradialgradient(cx:0.5,cy:0.5,radius:0.5,
                fx:0.4,fy:0.4,
                stop:0 #FFFFFF, stop:1 #3B82F6);
            border: 2px solid #3B82F6;
            border-radius: 9px;
        }
        QSlider::handle:horizontal:hover {
            border-color: #2563EB;
            background: qradialgradient(cx:0.5,cy:0.5,radius:0.5,
                fx:0.4,fy:0.4,
                stop:0 #FFFFFF, stop:1 #2563EB);
        }
        QSlider::sub-page:horizontal {
            background: #93C5FD;
            border-radius: 3px;
        }

        QStatusBar {
            background: #F8FAFC;
            border-top: 1px solid #E2E8F0;
            color: #64748B;
            font-size: 12px;
            padding: 2px 8px;
        }

        QMenu {
            background: white;
            border: 1px solid #E2E8F0;
            border-radius: 8px;
            padding: 4px;
        }
        QMenu::item {
            padding: 6px 24px 6px 12px;
            border-radius: 4px;
        }
        QMenu::item:selected {
            background: #EFF6FF;
            color: #1D4ED8;
        }

        QSplitter::handle {
            background: #E2E8F0;
            height: 2px;
        }
    )");

    MainWindow window;
    window.show();

    return app.exec();
}
