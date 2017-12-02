#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "ui_mainwindow.h"
#include "widget.h"
#include <QSystemTrayIcon>

class QMouseEvent;
class QMenu;
class QTimer;

class MainWindow : public QMainWindow, private Ui::MainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:

  bool m_drag;
  QPoint m_dragPosition;

  QSystemTrayIcon *trayIcon{nullptr};
  QMenu *trayIconMenu{nullptr};

  QTimer *m_timer{nullptr};
  Widget *widget_{nullptr};

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void mouseReleaseEvent(QMouseEvent *event);

  //窗口管理动作  
  QAction *minimizeAction;  
  QAction *maximizeAction;  
  QAction *restoreAction;  
  QAction *quitAction;

private slots:  
  void on_pushButtonMemo_clicked();
  void on_pushButtonHide_clicked();
  void on_pushButtonClose_clicked();

  void on_show_parent_forground();

  void iconActivated(QSystemTrayIcon::ActivationReason reason);
  void onTimer();
};

#endif // MAINWINDOW_H
