#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QProcess>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QTextEdit>
#include <QProgressBar>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QRegExp>
#include <QTimer>

class MainWindow : public QMainWindow
{
  Q_OBJECT // 必须保留，信号槽的核心宏

public:
  MainWindow(QWidget* parent = nullptr);
  ~MainWindow();

private slots: // 明确声明为slots，Qt5兼容
  void on_selectPathBtn_clicked();
  void on_downloadBtn_clicked();
  void readProcessOutput();
  void processFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
  void initUI();

  QProcess* bbDownProcess;
  QString downloadPath;

  QLineEdit* urlLineEdit;
  QPushButton* selectPathBtn;
  QLabel* pathLabel;
  QCheckBox* audioOnlyCheckBox;
  QPushButton* downloadBtn;
  QTextEdit* logTextEdit;
  QProgressBar* progressBar;
  int currentProgress;
  QTimer* progressTimer;
};

#endif // MAINWINDOW_H