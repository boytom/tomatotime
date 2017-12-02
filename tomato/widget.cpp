#include "widget.h"
#include "ui_widget.h"
#if defined(Q_OS_LINUX)
#include <unistd.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include <QTimer>
#include <QDebug>
#include <QStandardPaths>
#include <QString>
#include <QDir>
#include <QFile>
#include <string>
//#include <QtMultimediaWidgets>
#include <QMediaPlayer>
#include <QMessageBox>
#include <QWidget>
#include <QMediaPlaylist>
#include <QCloseEvent>

Widget::Widget(QWidget *parent) :
  QDialog(parent), ui(new Ui::Widget)
{
  ui->setupUi(this);

  load_config();
  set_config_to_ui();

  setWindowFlags(windowFlags() | Qt::WindowStaysOnTopHint);
  showMemo(true);

  mode_ = config.mode;

#if 0
  // 在widget的构造函数中设置LCD的颜色，只在超时时或把widget当成一个主窗口时才
  // 有效，其它时候，比如在这个程序中，先创建窗口，通定点击一个按钮再显示，在
  // 构造函数中设置的LCD颜色就无效了。
  if(mode_ == WORK) {
    setLCDColor(Qt::red);
    interval_ = config.work_time;
    ui->label_memo->setText(QString::fromWCharArray(L"该<font color='#FF0000'>工作</font>了，在这里输入你接下来要做的事情，备忘："));

  }
  else {
    setLCDColor(Qt::green);
    interval_ = config.rest1_time;
    ui->label_memo->setText(QString::fromWCharArray(L"该<font color='#00FF00'>休息</font>了，在这里输入你接下来要做的事情，备忘："));
  }
#endif
  interval_ = mode_ == WORK ? config.work_time : config.rest1_time;

  init_play_list();

  display(interval_);

  QIntValidator *iv = new QIntValidator(1, 65535, this);
  ui->work_lineedit->setValidator(iv);
  ui->rest1_lineedit->setValidator(iv);
  ui->rest2_lineedit->setValidator(iv);
}

Widget::~Widget()
{
  delete ui;
  delete media_player_;
  delete media_play_list_work_;
  delete media_play_list_rest_;
}

void Widget::updatelcd()
{
  static int restcount;
  wchar_t x[32], *xend;
  int length;

#if defined(_MSC_VER)
  // _snwprintf_s 为什么要改动它不需要关心的内存？如果用memset把x清空，调用_snwprintf_s以后，你会发现x中的0都成了0xfeff
  xend = x + (length = _snwprintf_s(x, sizeof(x) / sizeof(x[0]), sizeof(x) / sizeof(x[0]) - 1 , L"番茄时间 "));
#else
  xend = x + (length = swprintf(x, sizeof(x) / sizeof(x[0]), L"番茄时间 "));
#endif
  memset(xend, 0, sizeof(x) - sizeof(x[0]) * length);
  if(--interval_ == -1) {
    if(mode_ == WORK) {
      mode_ = REST;
      //secondnumber->setPalette(Qt::green);
      setLCDColor(Qt::green);
      wmemset(xend, L'X', ++restcount);
      ui->xlabel->setText(QString::fromWCharArray(x));
      interval_ = restcount == config.factor ? config.rest2_time : config.rest1_time;
      //ui->label_memo->setText("该休息了，在这里输入你接下来要做的事情，备忘：");
      ui->label_memo->setText(QString::fromWCharArray(L"该<font color='#00FF00'>休息</font>了，在这里输入你接下来要做的事情，备忘："));
      playSound();
      emit show_parent_forground();
      showMemo(true);
    }
    else {
      mode_ = WORK;
      if(restcount == config.factor) {
        restcount = 0;
        ui->xlabel->setText(QString::fromWCharArray(x));
      }
      setLCDColor(Qt::red);
      //secondnumber->setPalette(Qt::red);
      interval_ = config.work_time;
      //hide();
      //showMinimized();
      ui->label_memo->setText(QString::fromWCharArray(L"该<font color='#FF0000'>工作</font>了，在这里输入你接下来要做的事情，备忘："));
      playSound();
      emit show_parent_forground();
      showMemo(true);
    }
  }
  display(interval_);
}

void Widget::setLCDColor(const QColor &color)
{
  QPalette lcdp;

  lcdp = ui->lcd_second->palette();
  lcdp.setColor(QPalette::Normal,QPalette::WindowText, color);
  ui->lcd_second->setPalette(lcdp);

  lcdp = ui->lcd_minute->palette();
  lcdp.setColor(QPalette::Normal,QPalette::WindowText, color);
  ui->lcd_minute->setPalette(lcdp);

  lcdp = ui->xlabel->palette();
  lcdp.setColor(QPalette::Normal,QPalette::WindowText, color);
  ui->xlabel->setPalette(lcdp);
}

void Widget::display(int timeval)
{
  char buf[8];

  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%02d", timeval / 60);
  ui->lcd_minute->display(buf);
  memset(buf, 0, sizeof(buf));
  snprintf(buf, sizeof(buf), "%02d", timeval % 60);
  ui->lcd_second->display(buf);
}

#if 0
void Widget::forceForground()
{
  //hide();
  show();
  ((QWidget *)parent())->show();
  raise();
  showNormal();
  setFocus(Qt::ActiveWindowFocusReason);
  activateWindow();
}
#endif

void Widget::playSound()
{
  if (mode_ == WORK)
    media_player_->setPlaylist(media_play_list_work_);
    //media_player_->setMedia(QStringLiteral(u"d:\\music\\少林寺钟声.mp3"));
  else
    media_player_->setPlaylist(media_play_list_rest_);
    //media_player_->setMedia(QStringLiteral(u"d:\\music\\超好听的纯音乐铃声.mp3"));
  media_player_->setVolume(100);
  media_player_->play();
}

void Widget::showMemo(bool shown)
{
  ui->edit_memo->setVisible(shown);
  ui->button_save->setVisible(shown);
  ui->edit_memo->setFocus();
  if(shown) {
    ui->edit_memo->clear();
    ui->button_save->setEnabled(false);
  }
}

void Widget::on_button_save_clicked()
{
  save_memo();
  //showMemo(false);
  //ui->label_memo->setText(mode_ == WORK ? "<font color='#FF0000'>工作中……</font>" : "<font color='#00FF00'>休息中……</font>");
}

void Widget::on_del_pushbutton_clicked()
{
  ui->time_combobox->clear();
  ui->memo_textbrowser->clear();

#if defined(Q_OS_LINUX)
  wchar_t data_path[PATH_MAX];
#else
  wchar_t data_path[_MAX_PATH];
#endif
  QDate date;
  date = ui->edit_date->date();
  get_data_path_qdate(data_path, sizeof(data_path), &date);

#if defined(_MSC_VER)
  _wremove(data_path);
  _wunlink(data_path);
#else
  //nlink(data_path);
  QFile f(QString::fromWCharArray(data_path));
  f.remove();
#endif
}

void Widget::on_edit_memo_textChanged()
{
  ui->button_save->setEnabled(ui->edit_memo->toPlainText().length() > 0);
}

void Widget::on_tabWidget_currentChanged(int index)
{
  QDate date;

  switch(index) {
    case 1:
      get_today_date(&date);
      if(ui->edit_date->date() == date)
        on_edit_date_dateChanged(date); // 日期相同时不会产生该事件，要主动调用
      else
        ui->edit_date->setDate(date);
      break;
    default:
      break;
  }
  on_saveconfig_pushbutton_clicked();
}

void Widget::on_edit_date_dateChanged(const QDate &date)
{
  load_memo(&date);
}

void Widget::on_time_combobox_currentIndexChanged(int index)
{
  QString text;
  int i;

  text = "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.0//EN\" \"http://www.w3.org/TR/REC-html40/strict.dtd\">\n"
      "<html><head><meta name=\"qrichtext\" content=\"1\" /><style type=\"text/css\">\np, li { white-space: pre-wrap; }\n"
      "<meta http-equiv=\"Content-Type\" content=\"text/html; charset=UTF-8\"/>\n"
      "</style></head><body style=\" font-family:'Sans Serif'; font-size:10pt; font-weight:400; font-style:normal;\">\n"
      //"<h1>历史记录</h1><hr>\n"
      //"<p style=\"-qt-paragraph-type:empty; margin-top:0px; margin-bottom:0px; margin-left:0px; margin-right:0px; "
      //"-qt-block-indent:0; text-indent:0px;\"><br /></p>";
      ;
  switch(index) {
    case 0:
      for(i = 1; i < ui->time_combobox->count(); i++)
        add_item_to_browser(&text, ui->time_combobox->itemText(i), ui->time_combobox->itemData(i).toString(), i);
      break;
    case -1:
      return;
      break;
    default:
      add_item_to_browser(&text, ui->time_combobox->currentText(), ui->time_combobox->currentData().toString(), index);
  }


  text.append("</body></html>");
  ui->memo_textbrowser->setHtml(text);
}

void Widget::on_loaddefault_pushbutton_clicked()
{
  reset_default_config();
  set_config_to_ui();
  save_config();
}

void Widget::on_saveconfig_pushbutton_clicked()
{
  get_config_from_ui();
  save_config();
}

int Widget::save_memo()
{
  std::wstring memo;

  memo = ui->edit_memo->toPlainText().toStdWString();
  if(memo.length() == 0)
    return -1;

  time_t now;
  struct tm tm;
#if defined(Q_OS_LINUX)
  wchar_t data_path[PATH_MAX];
#else
  wchar_t data_path[_MAX_PATH];
#endif

  now = time(NULL);
#if defined(Q_OS_LINUX)
  localtime_r(&now, &tm);
#else
  localtime_s(&tm, &now);
#endif

  get_data_path_tm(data_path, sizeof(data_path) / sizeof(data_path[0]), &tm);
  qDebug() << "Load from path: " << QString::fromWCharArray(data_path);

  FILE *fp;

#if defined(Q_OS_LINUX)
  if(unlikely((fp = fopen(QString::fromWCharArray(data_path).toStdString().c_str(), "a")) == NULL))
    return -1;
#else
  if(_wfopen_s(&fp, data_path, L"ab") != 0)
    return -1;
#endif

  long long unsigned int length;

  length = sizeof(wchar_t) + sizeof(long long unsigned int) + memo.length();
  // 写记录的总长度
  fwrite(&length, sizeof(length), 1LU, fp);

  // 写当前状态
  wchar_t m = mode_;
  fwrite(&m, sizeof(m), 1LU, fp);

  // 写当前时间
  length = now;  // 保证时间也是 sizeof(long long unsigned int)（64） 位
  fwrite(&length, sizeof(length), 1LU, fp);

  // 写记录
  fwrite(memo.c_str(), sizeof(wchar_t), memo.length(), fp);
  fclose(fp);
  QMessageBox::information(this, QStringLiteral(u"tomato 提示"), QStringLiteral(u"保存成功！"));
//  QMessageBox mess(this);
//  mess.setWindowTitle("tomato 提示");
//  mess.setIcon(QMessageBox::Information);
//  mess.setText( "保存成功！");
//  mess.addButton("确定", QMessageBox::AcceptRole);
//  mess.exec();

  return 0;
}

int Widget::load_memo(const QDate *date)
{
#if defined(Q_OS_LINUX)
  wchar_t data_path[PATH_MAX];
#else
  wchar_t data_path[_MAX_PATH];
#endif
  get_data_path_qdate(data_path, sizeof(data_path) / sizeof(data_path[0]), date);
  qDebug() << "Load from path: " << QString::fromWCharArray(data_path);

  ui->time_combobox->clear();
  ui->memo_textbrowser->clear();

  FILE *fp;
#if defined (Q_OS_LINUX)
  if(unlikely((fp = fopen(QString::fromWCharArray(data_path).toStdString().c_str(), "r")) == NULL))
    return -1;
#else
  if(_wfopen_s(&fp, data_path, L"rb") != 0)
    return -1;
#endif

  wchar_t *memo, mode;
  long long unsigned int length, when;
  struct tm tm;
  wchar_t when_str[32];

  ui->time_combobox->clear();
  while(!ferror(fp) && !feof(fp) && fread(&length, sizeof(length), 1LU, fp) == 1) {
    time_t tmp; // 可能是32位，也可能是64位
    if(fread(&mode, sizeof(mode), 1LU, fp) != 1LU)
      break;
    if(fread(&when, sizeof(when), 1LU, fp) != 1LU)
      break;
    memset(&tm, 0, sizeof(struct tm));
    tmp = when; // 转换成系统的time_t
#if defined(Q_OS_LINUX)
  localtime_r(&tmp, &tm);
#else
  localtime_s(&tm, &tmp);
#endif
    memset(when_str, 0, sizeof(when_str));
    if (mode == WORK)
      when = wcsftime(when_str, sizeof(when_str) / sizeof(when_str[0]), L"%H:%M:%S 工作", &tm);
    else
      when = wcsftime(when_str, sizeof(when_str) / sizeof(when_str[0]), L"%H:%M:%S 休息", &tm);
    length -= sizeof(when) + sizeof(mode);
    if(unlikely((memo = (wchar_t *)malloc(length * sizeof(wchar_t) + 1LU)) == NULL))
      continue;
    memset(memo, 0, length * sizeof(wchar_t) + 1LU);
    fread(memo, sizeof(wchar_t), length, fp);
    ui->time_combobox->addItem(QString::fromWCharArray(when_str), QVariant(QString::fromWCharArray(memo, length)));
    free(memo);
  }
  int res(-1);
  if(ui->time_combobox->count() > 0) {
    res = 0;
    ui->time_combobox->insertItem(0, QString::fromWCharArray(L"所有时间"));
    ui->time_combobox->setCurrentIndex(0);
  }
  fclose(fp);
  return res;
}

void Widget::add_item_to_browser(QString *text, const QString &title, const QString &data, int index)
{
  char buf[16];

  memset(buf, 0, sizeof(buf));
  if(title.indexOf(QString::fromWCharArray(L" 工作")) > 0)
    text->append("<h3><font color='#FF0000'>");
  else
    text->append("<h3><font color='#00FF00'>");
  text->append(title);
  text->append("</font><input type=\"checkbox\" name=\"");
  snprintf(buf, sizeof(buf), "%d", index);
  text->append(buf);
  text->append("\">");
  text->append("</h3>");
  text->append(data);
  text->append("<hr>\n");
}

int Widget::get_today_date(QDate *date)
{
  if(unlikely(date == NULL))
    return -1;

  time_t now;
  struct tm tm;

  time(&now);
#if defined(Q_OS_LINUX)
  localtime_r(&now, &tm);
#else
  localtime_s(&tm, &now);
#endif
  date->setDate(tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday);
  return 0;
}

long long unsigned int Widget::get_data_path_tm(wchar_t *data_path, int size_in_bytes, const struct tm *tm)
{
#if defined(Q_OS_LINUX)
  if(unlikely(size_in_bytes < 0 || size_in_bytes > PATH_MAX))
    return -1;
#else
  if(unlikely(size_in_bytes < 0 || size_in_bytes > _MAX_PATH))
    return -1;
#endif

  int length;
  QString writableLocation;
  writableLocation = QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation);
  writableLocation.append("/tomato");
  QDir d;
  d.mkpath(writableLocation);
  memset(data_path, 0, size_in_bytes);
#if defined(_MSC_VER)
  length = _snwprintf_s(data_path, size_in_bytes, size_in_bytes - 1, L"%ls", writableLocation.toStdWString().c_str());
#else
  length = swprintf(data_path, size_in_bytes, L"%ls", writableLocation.toStdWString().c_str());
#endif
  return wcsftime(data_path + length, size_in_bytes - length, L"/%Y-%m-%d.dat", tm);
}

long long unsigned int Widget::get_data_path_qdate(wchar_t *data_path, int size_in_bytes, const QDate *date)
{
  struct tm tm;

  memset(&tm, 0, sizeof(tm));
  tm.tm_year = date->year() - 1900;
  tm.tm_mon = date->month() - 1;
  tm.tm_mday = date->day();
  return get_data_path_tm(data_path, size_in_bytes, &tm);
}

void Widget::get_config_path(QString *config_path)
{
  if(unlikely(config_path == NULL))
    return;

  *config_path = QStandardPaths::writableLocation(QStandardPaths::ConfigLocation);
  QDir d;
  d.mkpath(*config_path);
  config_path->append("/tomoto.config");
}

void Widget::reset_default_config()
{
  config.factor = FACTOR;
  config.rest1_time = RESTTIME1;
  config.rest2_time = RESTTIME2;
  config.work_time = WORKTIME;
  config.mode = WORK;
}

void Widget::load_config()
{
  QString config_path;

  get_config_path(&config_path);

  FILE *fp;

  if(unlikely((fp = fopen(config_path.toStdString().c_str(), "r")) == NULL)) {
    reset_default_config();
    return;
  }
  if(fread(&config, sizeof(config), 1LU, fp) != 1LU)
    reset_default_config();

  fclose(fp);
}

void Widget::save_config()
{

  QString config_path;

  get_config_path(&config_path);

  FILE *fp;

  if(unlikely((fp = fopen(config_path.toStdString().c_str(), "w")) == NULL)) {
    qDebug() << strerror(errno);
    return;
  }

  fwrite(&config, sizeof(config), 1LU, fp);
  fclose(fp);
}

void Widget::get_config_from_ui()
{
  if(unlikely(!ui->work_lineedit->hasAcceptableInput()
     || !ui->rest1_lineedit->hasAcceptableInput()
     || !ui->rest2_lineedit->hasAcceptableInput()))
    return;

  config.work_time = minute2second(ui->work_lineedit->text().toInt());
  config.rest1_time = minute2second(ui->rest1_lineedit->text().toInt());
  config.rest2_time = minute2second(ui->rest2_lineedit->text().toInt());
  config.factor = FACTOR;
  config.mode = ui->mode_combobox->currentIndex();
}

void Widget::set_config_to_ui()
{
  QString text;

  text.setNum(second2minute(config.work_time));

  ui->work_lineedit->setText(text);

  text.setNum(second2minute(config.rest1_time));
  ui->rest1_lineedit->setText(text);

  text.setNum(second2minute(config.rest2_time));
  ui->rest2_lineedit->setText(text);

  ui->mode_combobox->clear();
  ui->mode_combobox->addItem(QString::fromWCharArray(L"工作"));
  ui->mode_combobox->addItem(QString::fromWCharArray(L"休息"));
  ui->mode_combobox->setCurrentIndex(config.mode);
}

void Widget::init_play_list()
{
  media_play_list_work_ = new QMediaPlaylist(this);
  media_play_list_rest_ = new QMediaPlaylist(this);

  //media_play_list_work_->addMedia(QUrl::fromLocalFile(QStringLiteral(u"d:\\music\\少林寺钟声.mp3")));
  media_play_list_work_->addMedia(QUrl(QStringLiteral(u"qrc:/tomato/ring/music/shao_lin.mp3")));
  //media_play_list_work_->addMedia(QUrl::fromLocalFile(QStringLiteral(u"d:\\music\\005.王向荣 - 泪蛋蛋抛在沙蒿蒿林（信天游）.mp3")));
  media_play_list_work_->setCurrentIndex(0);
  media_play_list_work_->setPlaybackMode(QMediaPlaylist::Random);

  //media_play_list_rest_->addMedia(QUrl::fromLocalFile(QStringLiteral(u"d:\\music\\超好听的纯音乐铃声.mp3")));
  media_play_list_rest_->addMedia(QUrl(QStringLiteral(u"qrc:/tomato/ring/music/chao_hao_ting.mp3")));
  //media_play_list_rest_->addMedia(QUrl::fromLocalFile(QStringLiteral(u"d:\\music\\005.王向荣 - 泪蛋蛋抛在沙蒿蒿林（信天游）.mp3")));
  media_play_list_rest_->setCurrentIndex(0);
  media_play_list_rest_->setPlaybackMode(QMediaPlaylist::Random);

  media_player_ = new QMediaPlayer;
  media_player_->setObjectName(QStringLiteral(u"media_player"));
}

void Widget::closeEvent(QCloseEvent *event)
{
  media_player_->stop();
}

void Widget::showEvent(QShowEvent *event)
{
  if(mode_ == WORK) {
    setLCDColor(Qt::red);
    ui->label_memo->setText(QString::fromWCharArray(L"该<font color='#FF0000'>工作</font>了，在这里输入你接下来要做的事情，备忘："));

  }
  else {
    setLCDColor(Qt::green);
    ui->label_memo->setText(QString::fromWCharArray(L"该<font color='#00FF00'>休息</font>了，在这里输入你接下来要做的事情，备忘："));
  }
}

void Widget::accept()
{
  media_player_->stop();
  QDialog::accept();
}

void Widget::done(int r)
{
  media_player_->stop();
  QDialog::done(r);
}

void Widget::reject()
{
  media_player_->stop();
  QDialog::reject();
}

int Widget::minute2second(int minute)
{
  return minute * 60 - 1;
}

int Widget::second2minute(int second)
{
  return (second + 1) / 60;
}
