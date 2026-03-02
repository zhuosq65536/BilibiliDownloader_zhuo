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
#include <QTimer>
#include <QCheckBox>

MainWindow::MainWindow(QWidget* parent)
    : QMainWindow(parent),
    bbDownProcess(nullptr),
    urlLineEdit(new QLineEdit(this)),
    selectPathBtn(new QPushButton(u8"选择下载路径", this)),
    pathLabel(new QLabel(u8"未选择路径", this)),
    audioOnlyCheckBox(new QCheckBox(u8"仅下载音频", this)),
    downloadBtn(new QPushButton(u8"下载", this)),
    logTextEdit(new QTextEdit(this)),
    progressBar(new QProgressBar(this)),
    currentProgress(0),
    progressTimer(new QTimer(this))
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

    // 初始化进度条
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    progressBar->setFormat(u8"等待下载...");

    // 连接定时器模拟进度
    connect(progressTimer, &QTimer::timeout, [this]() {
        if (currentProgress < 95) {
            currentProgress += 5;
            progressBar->setValue(currentProgress);
            progressBar->setFormat(QString(u8"正在下载: %1%").arg(currentProgress));
        }
    });

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

    // 添加音频选项复选框
    QHBoxLayout* audioLayout = new QHBoxLayout();
    audioLayout->addWidget(new QLabel(u8"下载选项:", 0));
    audioOnlyCheckBox->setStyleSheet("QCheckBox{font-size:12px;spacing:5px;}");
    audioLayout->addWidget(audioOnlyCheckBox);
    audioLayout->addStretch();
    mainLayout->addLayout(audioLayout);

    // 添加进度条
    progressBar->setMinimumHeight(25);
    progressBar->setStyleSheet("QProgressBar{border:1px solid #ddd;border-radius:4px;background-color:#f0f0f0;text-align:center;}QProgressBar::chunk{background-color:#00a1d6;border-radius:3px;}");
    mainLayout->addWidget(progressBar);

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
    
    // 根据复选框状态添加音频下载参数
    QString bbDownCmd;
    if (audioOnlyCheckBox->isChecked()) {
        bbDownCmd = QString("BBDown %1 --work-dir %2 --audio-only").arg(quotedUrl).arg(quotedPath);
        logTextEdit->append(u8"模式: 仅下载音频");
    } else {
        bbDownCmd = QString("BBDown %1 --work-dir %2").arg(quotedUrl).arg(quotedPath);
        logTextEdit->append(u8"模式: 下载视频+音频");
    }
    
    QString cmd = QString("cmd /c %1").arg(bbDownCmd);

    logTextEdit->append(u8"开始下载...");
    logTextEdit->append(u8"命令: " + bbDownCmd);

    bbDownProcess->start(cmd);

    // 开始模拟进度（每2秒更新一次）
    progressTimer->start(2000);

    downloadBtn->setEnabled(false);
    selectPathBtn->setEnabled(false);
    urlLineEdit->setEnabled(false);
    audioOnlyCheckBox->setEnabled(false);
}

void MainWindow::readProcessOutput()
{
    if (!bbDownProcess) return;

    // 同时读取标准输出和标准错误，进度条信息可能在stderr中
    QByteArray output = bbDownProcess->readAllStandardOutput();
    QByteArray errorOutput = bbDownProcess->readAllStandardError();
    
    QString allOutput;
    if (!output.isEmpty()) {
        allOutput += QTextCodec::codecForLocale()->toUnicode(output);
    }
    if (!errorOutput.isEmpty()) {
        allOutput += QTextCodec::codecForLocale()->toUnicode(errorOutput);
    }
    
    if (!allOutput.isEmpty()) {
        logTextEdit->append(allOutput);
        
        // 解析进度（如果有）
        if (bbDownProcess->state() == QProcess::Running) {
            // 尝试解析进度百分比
            QRegExp progressRegex("(\\d+)%");
            int pos = 0;
            while ((pos = progressRegex.indexIn(allOutput, pos)) != -1) {
                QString percentStr = progressRegex.cap(1);
                bool ok;
                int percent = percentStr.toInt(&ok);
                if (ok && percent >= 0 && percent <= 100 && percent > currentProgress) {
                    currentProgress = percent;
                    progressBar->setValue(percent);
                    progressBar->setFormat(QString(u8"正在下载: %1%").arg(percent));
                    break;
                }
                pos += progressRegex.matchedLength();
            }
            
            // 如果没有找到百分比，但看到下载相关的关键词，设置为"正在下载"状态
            if (currentProgress == 0 && 
                (allOutput.contains("开始下载") || allOutput.contains("下载P1视频") || allOutput.contains("下载P1音频"))) {
                progressBar->setFormat(u8"正在下载...");
            }
        }
    }

    QTextCursor cursor = logTextEdit->textCursor();
    cursor.movePosition(QTextCursor::End);
    logTextEdit->setTextCursor(cursor);
}

void MainWindow::processFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitStatus);

    // 停止模拟进度定时器
    progressTimer->stop();

    if (exitCode == 0) {
        progressBar->setValue(100);
        progressBar->setFormat(u8"下载完成: 100%");
        logTextEdit->append("=====================");
        logTextEdit->append(u8"下载完成！文件保存至：" + downloadPath);
        logTextEdit->append("=====================");
    }
    else {
        progressBar->setFormat(u8"下载失败");
        logTextEdit->append("=====================");
        logTextEdit->append(u8"下载失败！错误码：" + QString::number(exitCode));
        logTextEdit->append("=====================");
    }

    downloadBtn->setEnabled(true);
    selectPathBtn->setEnabled(true);
    urlLineEdit->setEnabled(true);
    audioOnlyCheckBox->setEnabled(true);

    bbDownProcess->deleteLater();
    bbDownProcess = nullptr;
}