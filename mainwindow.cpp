#include "mainwindow.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QTextCodec>
#include <QCoreApplication>
#include <QFileDialog>
#include <QMessageBox>
#include <QDir>
#include <QRegExp>
#include <QTextCursor>
#include <QProcessEnvironment>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    bbDownProcess(nullptr),
    urlLineEdit(new QLineEdit(this)),
    selectPathBtn(new QPushButton(u8"选择下载路径", this)),
    pathLabel(new QLabel(u8"未选择路径", this)),
    downloadBtn(new QPushButton(u8"下载", this)),
    logTextEdit(new QTextEdit(this))
{
    initUI();
    setWindowTitle(u8"哔哩哔哩视频下载器 - 由 卓卓世界 开发");
    setFixedSize(700, 500);
    setStyleSheet("QMainWindow{background-color:#f5f5f5;}QLabel{font-size:12px;color:#333;min-width:80px;}QLineEdit{font-size:12px;padding:6px;border:1px solid #ddd;border-radius:4px;background-color:white;}QPushButton{font-size:12px;padding:8px 16px;background-color:#00a1d6;color:white;border:none;border-radius:4px;}QPushButton:hover{background-color:#008ec5;}QPushButton:disabled{background-color:#cccccc;color:#666666;}QTextEdit{font-size:11px;font-family:Consolas,Monaco,monospace;border:1px solid #ddd;border-radius:4px;background-color:#fafafa;}");

    logTextEdit->setReadOnly(true);
    downloadBtn->setEnabled(false);
    
    // 添加开发者署名
    logTextEdit->append(u8"========================================");
    logTextEdit->append(u8"B站视频下载工具 - 由 卓卓世界 开发");
    logTextEdit->append(u8"========================================");

    connect(selectPathBtn, &QPushButton::clicked, this, &MainWindow::on_selectPathBtn_clicked);
    connect(downloadBtn, &QPushButton::clicked, this, &MainWindow::on_downloadBtn_clicked);
}

MainWindow::~MainWindow()
{
    if (bbDownProcess && bbDownProcess->state() == QProcess::Running) {
        bbDownProcess->kill();
        bbDownProcess->waitForFinished();
    }
    delete bbDownProcess;
}

void MainWindow::initUI()
{
    QWidget* centralWidget = new QWidget(0);
    QVBoxLayout* mainLayout = new QVBoxLayout(centralWidget);
    setCentralWidget(centralWidget);

    QHBoxLayout* urlLayout = new QHBoxLayout();
    QLabel* urlLabel = new QLabel(u8"视频链接:", 0);
    urlLabel->setMinimumWidth(80);
    urlLayout->addWidget(urlLabel);
    urlLineEdit->setPlaceholderText(u8"输入哔哩哔哩视频链接");
    urlLayout->addWidget(urlLineEdit);
    mainLayout->addLayout(urlLayout);

    QHBoxLayout* pathLayout = new QHBoxLayout();
    QLabel* pathTitleLabel = new QLabel(u8"下载路径:", 0);
    pathTitleLabel->setMinimumWidth(80);
    pathLayout->addWidget(pathTitleLabel);
    pathLabel->setMinimumWidth(200);
    pathLabel->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    pathLabel->setStyleSheet("background-color: white; padding: 4px; border: 1px solid #ddd;");
    pathLayout->addWidget(pathLabel);
    selectPathBtn->setMinimumWidth(100);
    pathLayout->addWidget(selectPathBtn);
    mainLayout->addLayout(pathLayout);

    QHBoxLayout* btnLayout = new QHBoxLayout();
    btnLayout->addStretch();
    downloadBtn->setMinimumWidth(120);
    downloadBtn->setMinimumHeight(35);
    btnLayout->addWidget(downloadBtn);
    btnLayout->addStretch();
    mainLayout->addLayout(btnLayout);

    QLabel* logLabel = new QLabel(u8"下载日志:", 0);
    mainLayout->addWidget(logLabel);
    logTextEdit->setMinimumHeight(200);
    mainLayout->addWidget(logTextEdit);

    mainLayout->setSpacing(12);
    mainLayout->setContentsMargins(20, 20, 20, 20);
}

void MainWindow::on_selectPathBtn_clicked()
{
    downloadPath = QFileDialog::getExistingDirectory(0, u8"选择下载目录", QDir::homePath(), QFileDialog::ShowDirsOnly);
    if (!downloadPath.isEmpty()) {
        pathLabel->setText(downloadPath);
        downloadBtn->setEnabled(true);
        logTextEdit->append(u8"选择路径: " + downloadPath);
    }
}

void MainWindow::on_downloadBtn_clicked()
{
    QString videoUrl = urlLineEdit->text().trimmed();
    if (videoUrl.isEmpty()) {
        QMessageBox::warning(0, "警告", "请输入视频链接");
        return;
    }

    videoUrl = videoUrl.split("?").first();
    videoUrl = videoUrl.remove(QRegExp("/$"));

    if (!videoUrl.contains("bilibili.com/video/") && !videoUrl.contains("bv") && !videoUrl.contains("BV")) {
        QMessageBox::warning(0, "警告", "请输入合法的视频链接");
        return;
    }

    if (bbDownProcess && bbDownProcess->state() == QProcess::Running) {
        QMessageBox::information(0, "信息", "正在下载中，请稍后...");
        return;
    }

    bbDownProcess = new QProcess(0);
    bbDownProcess->setProcessEnvironment(QProcessEnvironment::systemEnvironment());

    connect(bbDownProcess, SIGNAL(readyReadStandardOutput()), this, SLOT(readProcessOutput()));
    connect(bbDownProcess, SIGNAL(readyReadStandardError()), this, SLOT(readProcessOutput()));
    connect(bbDownProcess, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(processFinished(int, QProcess::ExitStatus)));

    QString quotedUrl = QString("\"%1\"").arg(videoUrl);
    QString quotedPath = QString("\"%1\"").arg(downloadPath);
    QString bbDownCmd = QString("BBDown %1 --work-dir %2").arg(quotedUrl).arg(quotedPath);
    QString cmd = QString("cmd /c %1").arg(bbDownCmd);

    logTextEdit->append(u8"开始下载...");
    logTextEdit->append(u8"命令: " + bbDownCmd);

    bbDownProcess->start(cmd);

    downloadBtn->setEnabled(false);
    selectPathBtn->setEnabled(false);
    urlLineEdit->setEnabled(false);
}

void MainWindow::readProcessOutput()
{
    if (!bbDownProcess) return;

    QByteArray output = bbDownProcess->readAllStandardOutput();
    if (!output.isEmpty()) {
        logTextEdit->append(QTextCodec::codecForLocale()->toUnicode(output));
    }

    QByteArray error = bbDownProcess->readAllStandardError();
    if (!error.isEmpty()) {
        logTextEdit->append("[Error] " + QTextCodec::codecForLocale()->toUnicode(error));
    }

    QTextCursor cursor = logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    logTextEdit->setTextCursor(cursor);
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);

    if (exitCode == 0) {
        logTextEdit->append("=====================");
        logTextEdit->append(u8"下载完成！文件保存至：" + downloadPath);
        logTextEdit->append("=====================");
    }
    else {
        logTextEdit->append("=====================");
        logTextEdit->append(u8"下载失败！错误码：" + QString::number(exitCode));
        logTextEdit->append("=====================");
    }

    downloadBtn->setEnabled(true);
    selectPathBtn->setEnabled(true);
    urlLineEdit->setEnabled(true);

    bbDownProcess->deleteLater();
    bbDownProcess = nullptr;
}