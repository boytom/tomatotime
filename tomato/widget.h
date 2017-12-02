#ifndef WIDGET_H
#define WIDGET_H

#include <QDialog>
#include <time.h>
#if defined(Q_OS_LINUX)
#include <sys/time.h>
#include <sys/times.h>
#include <sys/cdefs.h>
#endif
#include <sys/timeb.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>

#include <QtMultimedia/QMediaPlayer>

#if defined(Q_OS_LINUX)
# define unlikely(cond)	__builtin_expect ((cond), 0)
# define likely(cond)	__builtin_expect ((cond), 1)
#else
# define unlikely(cond)	 (cond)
# define likely(cond)	 (cond)
#endif

namespace Ui {
  class Widget;
}

class QMediaPlayeList;
class QCloseEvent;

class Widget : public QDialog
{
  Q_OBJECT

public:

  enum{WORK, REST,
       //WORKTIME = 1499, // 25 分钟
       //RESTTIME1 = 299, // 5  分钟
       WORKTIME = 40 * 60 - 1, // 40 分钟
       RESTTIME1 = 599, // 10  分钟
       FACTOR = 4, RESTTIME2 = 2399};

  typedef struct memo_record {
    wchar_t mode;
    unsigned long long second; // unsigned long long，保证在32位和64位机器上都是 64 位
    wchar_t memo[0];
  } memo_record_t;

  typedef struct config {
    int mode, work_time, rest1_time, rest2_time, factor;
  } config_t;

  explicit Widget(QWidget *parent = 0);
  ~Widget();

signals:
  void show_parent_forground();

protected:
  virtual void closeEvent(QCloseEvent *event) override;
  virtual void showEvent(QShowEvent *event) override;

protected slots:
  virtual void accept() override;
  virtual void done(int r) override;
  virtual void reject() override;

private:
  Ui::Widget *ui;
  config_t config;
  int mode_{WORK}, interval_;
  QMediaPlayer *media_player_{nullptr};
  QMediaPlaylist *media_play_list_work_{nullptr}, *media_play_list_rest_;

  void display(int timeval);
  void setLCDColor(const QColor &color);

  void playSound();
  void showMemo(bool shown);

  int save_memo();
  int load_memo(const QDate *date);
  void add_item_to_browser(QString *text, const QString &title, const QString &data, int index);
  int get_today_date(QDate *date);
  //long unsigned int get_data_path(char *restrict data_path, int size_in_bytes, const struct tm *restrict tm);
  long long unsigned int get_data_path_tm(wchar_t *data_path, int size_in_bytes, const struct tm *tm);
  long long unsigned int get_data_path_qdate(wchar_t *data_path, int size_in_bytes, const QDate *date);

  void get_config_path(QString *config_path);
  void reset_default_config();
  void load_config();
  void save_config();
  void get_config_from_ui();
  void set_config_to_ui();
  void init_play_list();
  int minute2second(int minute);
  int second2minute(int second);

public slots:
  void updatelcd();

private slots:

  void on_button_save_clicked();
  void on_del_pushbutton_clicked();
  void on_edit_memo_textChanged();
  void on_tabWidget_currentChanged(int index);
  void on_edit_date_dateChanged(const QDate &date);
  void on_time_combobox_currentIndexChanged(int index);
  void on_loaddefault_pushbutton_clicked();
  void on_saveconfig_pushbutton_clicked();
};

#endif // WIDGET_H
