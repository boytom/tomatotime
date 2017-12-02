#include "mainwindow.h"
#include <QMouseEvent>
#include <QMenu>
#include <QTimer>
#include <QX11Info>
#include "lunar.h"
#if defined(_MSC_VER) || defined(__MINGW32__)
#include <windows.h>
#endif

#if defined(_MSC_VER) || defined(__MINGW32__)
#include <time.h>
#include <sys/timeb.h>
#endif

#if 0
#include <windows.h>
#endif

#if 0
#if defined(_MSC_VER)
#pragma execution_character_set("utf-16le")
#endif
#endif

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),m_drag(false)
{
  setupUi(this);
  /*
  Ui_MainWindow::menuBar->setVisible(false);
  mainToolBar->setVisible(false);
  Ui_MainWindow::statusBar->setVisible(false);
  */

  //ui->actionOpen->setIcon(QIcon(”:/icons/open_icon.png”));
  //QIcon(":/images/open.png")
  //托盘初始化
  //QIcon icon = QIcon("icon.png");
  trayIcon = new QSystemTrayIcon(this);
  //trayIcon->setIcon(icon);
  trayIcon->setIcon(QIcon(":/tomato/png/image/wristwatch.png"));
  trayIcon->setToolTip("A professional tomato timer");
  trayIcon->show(); //必须调用，否则托盘图标不显示

  //创建菜单项动作(以下动作只对windows有效)
  //minimizeAction = new QAction(QString::fromWCharArrayL"最小化(&N)"), this);
  minimizeAction = new QAction(QString::fromWCharArray(L"隐藏(&N)"), this);
  connect(minimizeAction, SIGNAL(triggered()), this, SLOT(hide())); //绑定信号槽

  maximizeAction = new QAction(QString::fromWCharArray(L"最大化(&X)"), this);
  connect(maximizeAction, SIGNAL(triggered()), this, SLOT(showMaximized()));

  //restoreAction = new QAction(QString::fromWCharArray(L"还原(&R)"), this);
  restoreAction = new QAction(QString::fromWCharArray(L"显示(&R)"), this);
  connect(restoreAction, SIGNAL(triggered()), this, SLOT(showNormal()));

  quitAction = new QAction(QString::fromWCharArray(L"关闭(&C)"), this);
  connect(quitAction, SIGNAL(triggered()), qApp, SLOT(quit())); //关闭应用，qApp对应的是程序全局唯一指针

  //创建托盘菜单(必须先创建动作，后添加菜单项，还可以加入菜单项图标美化)
  trayIconMenu = new QMenu(this);
  trayIconMenu->addAction(restoreAction);
  trayIconMenu->addAction(minimizeAction);
  //trayIconMenu->addAction(maximizeAction);
  trayIconMenu->addSeparator();
  trayIconMenu->addAction(quitAction);
  trayIcon->setContextMenu(trayIconMenu);


  connect(trayIcon,SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
          this,SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

  m_timer = new QTimer();
  connect(m_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
  widget_ = new Widget(this);

  connect(m_timer, &QTimer::timeout, widget_, &Widget::updatelcd);
  connect(widget_, &Widget::show_parent_forground, this, &MainWindow::on_show_parent_forground, Qt::QueuedConnection);

  //m_timer->start(0);
  m_timer->start(1000);

  label_am->setVisible(false);
  label_pm->setVisible(false);
  label_temperature->setVisible(false);
  lcd_temperature->setVisible(false);
  label_shidu->setVisible(false);
  lcd_shidu->setVisible(false);
  lcd_lunarmonth->setVisible(false);
  lcd_lunarday->setVisible(false);
  setWindowFlags(Qt::FramelessWindowHint);// |  Qt::ToolTip); // 设置无边框风格，加上 Qt::ToolTip会让窗口位于顶层
#ifdef _MSC_VER
  setWindowOpacity(0); // 窗口整体透明度，０－１从全透明到不透明
#ifdef Q_WS_X11
  if(QX11Info::isCompositingManagerRunning())
#endif
  setAttribute(Qt::WA_TranslucentBackground); // 设置背景透明，允许鼠标穿透，不透明的地方不穿透
#endif
#ifdef __linux__
    if(QX11Info::isCompositingManagerRunning())
        setAttribute(Qt::WA_TranslucentBackground);
#endif
  //setWindowFlags(Qt::CustomizeWindowHint | Qt::WindowSystemMenuHint);
}

/*
http://stackoverflow.com/questions/7922177/qtwa-translucentbackground-available-everywhere

For Linux you should check whether compositing is enabled:

bool QX11Info::isCompositingManagerRunning() [static]

e.g.

#ifdef Q_WS_X11
    if(QX11Info::isCompositingManagerRunning())
        setAttribute(Qt::WA_TranslucentBackground);
#endif

This question is old, but this might help someone.

And, for anyone who wanders in off Google, if you find yourself needing to
support non-composited desktops for something like a rounded OSD or a speech
balloon popup, the Qt "ShapedClock" example demonstrates how to use setMask to
produce a non-square window without compositing. – ssokolow Sep 13 '16 at 11:17
*/

MainWindow::~MainWindow()
{
  m_timer->stop();
  delete m_timer;
  delete widget_;
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{

    if (event->button() == Qt::LeftButton)  {
        m_drag = true;
        m_dragPosition = event->globalPos() - pos();
        event->accept();
    }
#if 0
    if (ReleaseCapture())
        SendMessage(HWND(this->winId()), WM_SYSCOMMAND, SC_MOVE + HTCAPTION, 0);
    event->ignore();
#endif
}

void MainWindow::mouseMoveEvent(QMouseEvent *event)
{
    if (m_drag && (event->buttons() && Qt::LeftButton))  {
        move(event->globalPos() - m_dragPosition);
        event->accept();
    }
}

void MainWindow::mouseReleaseEvent(QMouseEvent *event)
{
  m_drag = false;
  event->accept();
}

void MainWindow::on_pushButtonMemo_clicked()
{
  widget_->exec();
}

void MainWindow::on_pushButtonHide_clicked()
{
  hide();
}

void MainWindow::on_pushButtonClose_clicked()
{
  close();
}

void MainWindow::on_show_parent_forground()
{
  show();
  widget_->exec();
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch(reason) {
    case QSystemTrayIcon::Trigger:
        trayIcon->showMessage(QString::fromWCharArray(L"title"),
                              QString::fromWCharArray(L"你单击了")); //后面两个默认参数
        break;
    case QSystemTrayIcon::DoubleClick:
        trayIcon->showMessage(QString::fromWCharArray(L"title"),
                              QString::fromWCharArray(L"你双击了"));
        break;
    case QSystemTrayIcon::MiddleClick:
        trayIcon->showMessage(QString::fromWCharArray(L"title"),
                              QString::fromWCharArray(L"你中键了"));
        break;
    default:
        break;
    }
}

void MainWindow::onTimer()
{
#if defined(_MSC_VER) || defined(__MINGW32__)
  __time64_t tim;
#else
  time_t tim;
#endif
  struct tm date_time;
  wchar_t wbuf[16];

#if defined(_MSC_VER) || defined(__MINGW32__)
  _time64(&tim);
  _localtime64_s(&date_time, &tim);
#else
  time(&tim);
  localtime_r(&tim, &date_time);
#endif

  memset(wbuf, 0, sizeof(wbuf));
  int nwchars = wcsftime(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%H:%M:%S", &date_time);
  lcd_time->display(QString::fromWCharArray(wbuf, nwchars));

  nwchars = wcsftime(wbuf, sizeof(wbuf) / sizeof(wbuf[0]), L"%Y-%m-%d", &date_time);
  lcd_date->display(QString::fromWCharArray(wbuf, nwchars));
  lcd_wday->display(date_time.tm_wday == 0 ? 8 : date_time.tm_wday);

  label_lunardate->setText(QString::fromWCharArray(get_lunar_date_name(&date_time)));

//#endif
}
