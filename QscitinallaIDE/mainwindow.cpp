#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QMessageBox>
#include <QPushButton>
#include <QFileDialog>
#include <QTextStream>
#include<QString>
#include<Qsci/qsciscintilla.h>
#include<Qsci/qscilexer.h>
#include<Qsci/qsciapis.h>
#include<Qsci/qscilexercpp.h>
#include<QLayout>
#include<QLineEdit>
#include<QDialog>
#include<QLabel>
#include<fstream>
#include<string>
#include<stdio.h>
#include<QLabel>
#include<QTextCursor>
#include<QDebug>
#include<QKeyEvent>
#include<QFont>
#include<QFontComboBox>
#include<QFontDatabase>
#include<QFontDialog>
#include<QRegExp>
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    curFile="Untitled.c";
    isUntitled=true;//默认未命名
    isCommentFold=false;//默认注释行未折叠
    //设置自适应窗口
    ui->gridLayout_2->addWidget(myCodeEditor);
    //设置行号提示
    myCodeEditor->setMarginType(0,QsciScintilla::NumberMargin);
    myCodeEditor->setMarginLineNumbers(0,true);
    myCodeEditor->setMarginWidth(0,30);
    myCodeEditor->SendScintilla(QsciScintilla::SCI_SETCODEPAGE,QsciScintilla::SC_CP_UTF8);

    //显示当前选中行
    myCodeEditor->setCaretLineVisible(true);
    myCodeEditor->setCaretLineBackgroundColor(QColor(255,192,203));

    //设置关键字与注释高亮
    QsciLexerCPP * cppCodeTextLexer = new QsciLexerCPP;
    cppCodeTextLexer->setColor(QColor(Qt::green),QsciLexerCPP::CommentLine);
    cppCodeTextLexer->setColor(QColor(Qt::red),QsciLexerCPP::Keyword);
    cppCodeTextLexer->setColor(QColor(Qt::gray),QsciLexerCPP::Comment);    //设置自带的注释行为灰色
    cppCodeTextLexer->setFoldComments(true);//设置所谓注释折叠

    myCodeEditor->setLexer(cppCodeTextLexer);
    QsciAPIs * myApis= new QsciAPIs(cppCodeTextLexer);
    if(!myApis->load(QString(":/keyrwords/prefix1/keywords.txt")))
    {
        QMessageBox::warning(this,tr("错误"),tr("加载关键字文件失败"));
    }
    myApis->prepare();

    //设置自动完成项
    myCodeEditor->setAutoCompletionSource(QsciScintilla::AcsAll);
    myCodeEditor->setAutoCompletionCaseSensitivity(true);//设置大小写的敏感
    myCodeEditor->setAutoCompletionThreshold(2);//输入2个字符自动补全
    myCodeEditor->setBraceMatching(QsciScintilla::SloppyBraceMatch);//设置括号匹配
    myCodeEditor->setAutoIndent(true);//开启自动缩进
    myCodeEditor->setIndentationGuides(QsciScintilla::SC_IV_LOOKBOTH);//设置缩进方式

    //查找与替换功能实现
    findDlg= new QDialog(this);
    findDlg->setWindowTitle(tr("查找"));
    findLineEdit= new QLineEdit(findDlg);
    QPushButton * findBtn=new QPushButton(tr("继续查找"),findDlg);
    QVBoxLayout *boxlayout=new QVBoxLayout(findDlg);
    boxlayout->addWidget(findLineEdit);
    boxlayout->addWidget(findBtn);
    connect(findBtn,SIGNAL(clicked()),this,SLOT(showFindText()));

    //当前行号列数显示
    QStatusBar *countBar=ui->statusBar;
    statusLabel1 = new QLabel;
    statusLabel1->setMinimumSize(150,20);
    statusLabel1->setFrameShape(QFrame::WinPanel);
    statusLabel1->setContentsMargins(15,0,0,0);
    countBar->setStyleSheet("QFrame{border: 0px;}");
    countBar->addWidget(statusLabel1);
    statusLabel1->setText("Line: 0  Column: 0");
    connect(myCodeEditor,SIGNAL(cursorPositionChanged(int,int)),this,SLOT(countInfo()));

    //版本号显示
    editionInfo = new QLabel;
    editionInfo->setMinimumSize(150,20);
    editionInfo->setFrameShape(QFrame::WinPanel);
    editionInfo->setContentsMargins(15,0,0,0);
    editionInfo->setText("老爷车队IDE 1.0");
    countBar->addWidget(editionInfo);

    //设置代码折叠功能
    myCodeEditor->setMarginType(3,QsciScintilla::SymbolMargin);
    myCodeEditor->setMarginLineNumbers(3,false);
    myCodeEditor->setMarginWidth(3,15);
    myCodeEditor->setMarginSensitivity(3,true);
    myCodeEditor->setFolding(QsciScintilla::BoxedTreeFoldStyle,3);

    //自动补全括号，引号
    MykeyPress *keypressEater;
    keypressEater = new MykeyPress;
    myCodeEditor->installEventFilter(keypressEater);
    connect(keypressEater,SIGNAL(keyPressSiganl_puncComplete(int)),this,SLOT(handlePuncComplete(int)));

    //断点调试功能
    myCodeEditor->setMarginType(1,QsciScintilla::SymbolMargin);
    myCodeEditor->setMarginLineNumbers(1,false);
    myCodeEditor->setMarginWidth(1,20);
    myCodeEditor->setMarginSensitivity(1,true);//设置断点可见
    myCodeEditor->setMarginsBackgroundColor(QColor(135,206,235));//设置边框背景颜色
    myCodeEditor->setMarginMarkerMask(1,0x02);
    connect(myCodeEditor,SIGNAL(marginClicked(int, int, Qt::KeyboardModifiers)),this,SLOT(on_margin_clicked(int, int, Qt::KeyboardModifiers)));
    myCodeEditor->markerDefine(QsciScintilla::Circle,1);//设置圆形的断点
    myCodeEditor->setMarkerBackgroundColor(QColor(255,69,0),1);

    //单步调试显示区域
    myCodeEditor->setMarginType(2,QsciScintilla::SymbolMargin);
    myCodeEditor->setMarginLineNumbers(2,false);
    myCodeEditor->setMarginWidth(2,20);
    myCodeEditor->setMarginSensitivity(2,false);
    myCodeEditor->setMarginMarkerMask(2,0x04);
    myCodeEditor->setMarkerBackgroundColor(QColor("#eaf593"),2);

    //构建文件树 by 张航
    model = new QDirModel(this);
    model->setReadOnly(false);
    model->setSorting(QDir::DirsFirst | QDir::IgnoreCase | QDir::Name);

    ui->treeView->setModel(model);
    QModelIndex index = model->index("F:/");

    ui->treeView->expand(index);
    ui->treeView->scrollTo(index);
    ui->treeView->setCurrentIndex(index);
    ui->treeView->resizeColumnToContents(0);

    //自制注释折叠
    myCodeEditor->SendScintilla(myCodeEditor->SCI_STYLESETHOTSPOT,1,true);
    myCodeEditor->SendScintilla(myCodeEditor->SCI_INSERTTEXT);
}

void MainWindow::on_margin_clicked(int margin,int line,Qt::KeyboardModifiers state)//断点调试槽函数
{
    Q_UNUSED(state);
    if(margin==1)
    {
        if(myCodeEditor->markersAtLine(line)!=0)//已加标记点
        {
            myCodeEditor->markerDelete(line,1);
        }
        else//未加标记点
        {
            myCodeEditor->markerAdd(line,1);
            int curLine;
            int curColumn;
            myCodeEditor->getCursorPosition(&curLine,&curColumn);
            qDebug()<<curLine<<"    "<<curColumn;
        }
    }
}

void MainWindow::countInfo()//行列号统计函数
{
    myCodeEditor->getCursorPosition(&LineNum,&columnNum);
    QString info=QString("Line: ")+QString::number(LineNum)+QString("   ")+QString("Column ")+QString::number(columnNum);
    statusLabel1->setText(info);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::newfile()
{
    if(maybeSave())
    {
        isUntitled=true;//新建文件未命名
        curFile=tr("unTitle.c");
        setWindowTitle(curFile);
        myCodeEditor->clear();
        myCodeEditor->setVisible(true);
    }
}

bool MainWindow:: maybeSave()
{
    if(myCodeEditor->isModified())//判断文档是否被更改了
    {
        QMessageBox box;
        box.setWindowTitle(tr("警告"));
        box.setIcon(QMessageBox::Warning);
        box.setText(curFile+tr("尚未保存，是否保存"));
        QPushButton * yesBtn=box.addButton(tr("是(&Y)"),QMessageBox::YesRole);
        QPushButton * noBtn=box.addButton(tr("否(&N)"),QMessageBox::NoRole);
        QPushButton * cancelBtn= box.addButton(tr("取消(&c)"),QMessageBox::RejectRole);
        box.exec();
        if(box.clickedButton()==yesBtn)
            return save();
        else if(box.clickedButton()==cancelBtn)
            return false;
        else {
            box.close();
            return false;
        }
    }
    return true;
}

bool MainWindow:: save()
{
    if(isUntitled)
    {
        return saveAs();//文件未保存过，执行此操作
    }
    else {
        return savefile(curFile);//文件保存过，执行此操作
    }
}

bool MainWindow::saveAs()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("另存为"),curFile);//参数：父类 对话框名称 文件名
    if(filename.isEmpty())
        return false;
    return savefile(filename);
}

bool MainWindow::savefile(const QString &filename)
{
    QFile file(filename);
    if(!file.open(QFile::WriteOnly|QFile::Text))//文件读取失败
    {
           QMessageBox::warning(this,tr("多文档编辑器"),tr("无法写入文件%1:/n %2").arg(filename).arg(file.errorString()));
           return false;
    }
    QTextStream out(&file);//将当前文档输出
    QApplication::setOverrideCursor(Qt::WaitCursor);//鼠标指针设为等待状态
    //int codeLength=myCodeEditor->SCI_GETLENGTH();
    //int codeLength= myCodeEditor->SCI_GETTEXTLENGTH;
    out<< myCodeEditor->text();//字符流输出
    QApplication::restoreOverrideCursor();//鼠标指针恢复原状
    isUntitled=false;//文件已经保存
    curFile=QFileInfo(filename).canonicalFilePath();//获得文件的路径
    setWindowTitle(QFileInfo(filename).canonicalFilePath());//显示文件名
    curFileName= QFileInfo(filename).fileName();
    // out<< curFile.toUtf8();
    return true;
}

void MainWindow::on_actionNew_N_triggered()
{
    newfile();
}

void MainWindow::on_actionSave_S_triggered()
{
    save();
}

void MainWindow::on_actionSaveAs_A_triggered()
{
    saveAs();
}

bool MainWindow::loadfile(const QString &filename)
{
    QFile file(filename);//新建一个文本对象
    if(!file.open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox::warning(this,tr("myIde error"),tr("无法读取文件 %1:\n%2").arg(filename).arg(file.errorString()));//提示报错信息
        return false;
    }
    QTextStream readIn(&file);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    myCodeEditor->setText(readIn.readAll());
    QApplication::restoreOverrideCursor();
    curFile=QFileInfo(filename).canonicalFilePath();//设置当前文件路径
    setWindowTitle(QFileInfo(filename).fileName());//窗口设为当前文件名字
    isUntitled=false;//文件之前保存过
    return true;
}


void MainWindow::on_actionOpen_O_triggered()
{
    if(maybeSave())
    {
        QString openFilename= QFileDialog::getOpenFileName(this);
        if(!openFilename.isEmpty())
        {
            loadfile(openFilename);
            myCodeEditor->setVisible(true);
        }
    }
}

void MainWindow::showFindText()
{
    QString searchStr=findLineEdit->text();
    if(!myCodeEditor->findFirst(searchStr,false,false,true,true,true,-1,-1,true,false,false))
    {
        QMessageBox::warning(this,tr("未找到"),tr("未找到%1").arg(searchStr));
    }
}
void MainWindow::on_actionfind_F_triggered()
{
    findDlg->show();
}

void MainWindow::on_actionreplace_R_triggered()
{
    codeReplace();
}

void MainWindow::codeReplace()

{
    codeRepDlg= new QDialog(this);
    codeRepDlg->setWindowTitle(tr("替换文本"));
    QLabel * origCode =new QLabel(codeRepDlg);
    origCode->setText(tr("被替换文本"));
    QLabel * repCode = new QLabel(codeRepDlg);
    repCode->setText(tr("替换后文本"));
    origCodeEdit = new QLineEdit(codeRepDlg);
    origCodeEdit->setPlaceholderText(tr("请输入被替换文本"));
    repCodeEdit =new QLineEdit(codeRepDlg);
    repCodeEdit->setPlaceholderText(tr("请输入替换后的文本"));
    QPushButton * codeReplaceBtn = new QPushButton(tr("继续替换"),codeRepDlg);
    QGridLayout * repGridLayout = new QGridLayout(codeRepDlg);
    repGridLayout->addWidget(origCode);
     repGridLayout->addWidget(origCodeEdit);
    repGridLayout->addWidget(repCode);
    repGridLayout->addWidget(repCodeEdit);
    repGridLayout->addWidget(codeReplaceBtn);
    connect(codeReplaceBtn,SIGNAL(clicked()),this,SLOT(codeRepText()));
    codeRepDlg->show();

}

void MainWindow::codeRepText()
{
    QString findString = origCodeEdit->text();
    if(!myCodeEditor->findFirst(findString,false,false,true,true,true,-1,-1,true,false,false))
    {
        QMessageBox::warning(this,tr("未找到"),tr("未找到%1").arg(findString));
    }
    QString replaceText = repCodeEdit->text();
    myCodeEditor->replace(replaceText);
}

void MainWindow::mywheelEvent(QWheelEvent *event)//放大与缩小
{
    if(event->delta()>0)
    {
       myCodeEditor->zoomIn();
    }
    else {
        myCodeEditor->zoomOut();
    }
}



void MainWindow::on_actionUndo_Z_triggered()
{
    myCodeEditor->undo();
}

void MainWindow::on_actionCopy_C_triggered()
{
    myCodeEditor->copy();
}

void MainWindow::on_actionPaste_V_triggered()
{
    myCodeEditor->paste();
}

void MainWindow::on_actionCut_X_triggered()
{
    myCodeEditor->cut();
}



bool MainWindow::compile()
{
    if(save())
    {
        QString complieCmd;
        QString orgName;
        orgName.sprintf(curFile.toStdString().data());
        int nameLength=orgName.length();
        orgName.remove(nameLength-2,2);
        complieCmd.sprintf("gcc -o %s.exe %s.c 2>error.txt",orgName.toStdString().data(),orgName.toStdString().data());
        system(complieCmd.toStdString().data());
        QFile errorFile("error.txt");
        if(!errorFile.open(QFile::ReadOnly|QFile::Text))
        {
            QMessageBox::warning(this,tr("can't find error.txt"),tr("无法读取编译器报错信息 %1:\n%2").arg(errorFile.fileName()).arg(errorFile.errorString()));
            return false;
        }
        else//错误文件打开，判断是否为空
        {
            if(errorFile.size()==0)
            {
                ui->errorTextEdit->setText("compile success\n");
                return true;
            }
            else
            {
                //文件不为空，输出错误信息
                QMessageBox::warning(this,tr("compile error"),tr("编译失败"));
                QTextStream readError(&errorFile);
                QApplication::setOverrideCursor(Qt::WaitCursor);
                ui->errorTextEdit->setText(readError.readAll());
                QApplication::restoreOverrideCursor();
                return false;
            }
        }
    }
}



void MainWindow::runCode()
{
    if(compile())
    {
        QString runCmd;
        QString orgName;
        orgName.sprintf(curFile.toStdString().data());
        int namelength=orgName.length();
        orgName.remove(namelength-2,2);
        runCmd.sprintf("%s.exe",orgName.toStdString().data());
        system(runCmd.toStdString().data());
    }
}

void MainWindow::on_actionstart_triggered()//开始编译
{
    compile();
}


void MainWindow::on_actionrun_R_triggered()
{
    runCode();
}

void MainWindow::commentALL()//多行注释原函数
{
    int from;
    int start;
    int to;
    int end;
    int temp;
    myCodeEditor->getSelection(&from,&start,&to,&end);
    if(from>to)//假若反选
    {
        temp=from;
        from=to;
        to=temp;
    }
    for(int i=from;i<=to;i++)
    {
        myCodeEditor->insertAt(tr("//"),i,0);
    }
}

void MainWindow::on_actioncommentAll_triggered()//多行注释槽函数
{
    commentALL();
}



void MainWindow::on_actioninsert_I_triggered()//字体设置槽函数
{
    fontSetting();
}

void MainWindow::handlePuncComplete(int key)//自动补全括号函数
{
    int cursorline;
    int cursorIndex;
    myCodeEditor->getCursorPosition(&cursorline,&cursorIndex);
    if(key==34)
        myCodeEditor->insert(tr("\"\""));
    if(key==39)
        myCodeEditor->insert(tr("''"));
    if(key==40)
        myCodeEditor->insert(tr("()"));
    if(key==91)
        myCodeEditor->insert(tr("[]"));
    if(key==123)
        myCodeEditor->insert(tr("{}"));

    myCodeEditor->setCursorPosition(cursorline,cursorIndex+1);
}


bool MykeyPress::eventFilter(QObject *obj, QEvent *event)//自动补全括号事件过滤器
{
    if(event->type()==QEvent::KeyPress)
    {
        QKeyEvent *keyevent=static_cast<QKeyEvent *>(event);
        if(keyevent->key()==40||keyevent->key()==91|keyevent->key()==123||keyevent->key()==123||keyevent->key()==34||keyevent->key()==39)
        {
            emit keyPressSiganl_puncComplete(keyevent->key());
            return true;
        }
        else return QObject::eventFilter(obj,event);
    }
    else {
        return QObject::eventFilter(obj,event);
    }
}

void MainWindow::fontSetting()
{
    myFontSetting=new QDialog(this);
    myFontSetting->setWindowTitle(tr("字体设置"));
    fontSize=new QLabel(myFontSetting);
    fontSize->setText(tr("字体大小"));
    fontFormat=new QLabel(myFontSetting);
    fontFormat->setText(tr("字体选择"));
    fontSizeInput=new QLineEdit(myFontSetting);
    fontSizeInput->setPlaceholderText(tr("请输入非0整数值"));
    fontBox =new QFontComboBox(myFontSetting);
    QGridLayout * fontBoxLayout= new QGridLayout(myFontSetting);
    QPushButton * confirmBtn = new QPushButton(myFontSetting);
    QPushButton * cancelBtn= new QPushButton(myFontSetting);
    confirmBtn->setText("确定");
    cancelBtn->setText("取消");

    fontBoxLayout->addWidget(fontSize);
    fontBoxLayout->addWidget(fontSizeInput);
    fontBoxLayout->addWidget(fontFormat);
    fontBoxLayout->addWidget(fontBox);
    fontBoxLayout->addWidget(confirmBtn);
    fontBoxLayout->addWidget(cancelBtn);
    myFontSetting->show();
    //链接两个槽函数
    connect(confirmBtn,SIGNAL(clicked()),this,SLOT(fontSetConfirmed()));
    connect(cancelBtn,SIGNAL(clicked()),this,SLOT(fontSetCanceled()));
}

void MainWindow::fontSetConfirmed()
{
    QFont font;
    int pointSize;
    if(fontSize->text().isEmpty())
    {
        QMessageBox::warning(this,tr("Input Error"),tr("请输入字体大小"));
    }
    else
    {
        pointSize=fontSizeInput->text().toInt();
        if(pointSize==0)//输入非整数值
        {
            QMessageBox::warning(this,tr("Input Error"),tr("请输入正确的字体大小值"));

        }
        else
        {
            font.setPointSize(pointSize);
            QString curFontFamily = fontBox->currentFont().toString();
            font.setFamily(curFontFamily);
            qDebug()<<font.pointSize()<<font.family();
            ui->errorTextEdit->setFont(font);
            myFontSetting->close();
        }
    }
}

void MainWindow::fontSetCanceled()
{
    myFontSetting->close();
}

//文件树中打开文件 by 张航
void MainWindow::on_treeView_doubleClicked(const QModelIndex &index)
{
    QString openFilepath= (*model).filePath(index);
        QFile openFile(openFilepath);
        if(!openFile.open(QFile::ReadOnly|QFile::Text))
        {
            QMessageBox::warning(this,tr("加载失败"),tr("无法加载当前文件 %1:\n%2").arg(openFilepath).arg(openFile.errorString()));
            return;
        }
        else
        {
          QTextStream readIn(&openFile);
          QApplication::setOverrideCursor(Qt::WaitCursor);
          myCodeEditor->setText(readIn.readAll());
          QApplication::restoreOverrideCursor();
          curFile=QFileInfo(openFilepath).canonicalFilePath();
          setWindowTitle(QFileInfo(openFilepath).fileName());
          isUntitled=false;//文件之前保存过
          return;
        }
}

void MainWindow::on_actionClose_triggered()
{
    if(maybeSave())
    {
        myCodeEditor->setVisible(false);
    }
}

void MainWindow::on_actionExit_E_triggered()
{
    on_actionClose_triggered();
    qApp->exit();
}

void MainWindow::on_actionHideComment_H_triggered()
{
    if(!isCommentFold)
    {
        QRegExp rxSingleLine("\\/\\/[^\n]*");
        QRegExp rxMutipleLine("/\\*([^*]|[\r\n]|(\\*+([^*/]|[\r\n])))*\\*+/");
        int posSingleLIne=0;
        capCount=0;
        int Remove=0;
        curFileModify = myCodeEditor->text().toStdString().data();
        curFileReplica=myCodeEditor->text().toStdString().data();
        while((posSingleLIne=rxSingleLine.indexIn(myCodeEditor->text(),posSingleLIne))!=-1)
        {
                mycomment[capCount].startPoint=posSingleLIne;
                mycomment[capCount].curComment.sprintf(rxSingleLine.cap(0).toStdString().data());
                qDebug()<<mycomment[capCount].curComment;
                mycomment[capCount].commentLength=rxSingleLine.matchedLength();
                posSingleLIne+=rxSingleLine.matchedLength();
                curFileModify.remove(mycomment[capCount].startPoint-Remove,rxSingleLine.matchedLength());
                Remove+=rxSingleLine.matchedLength();
                capCount++;
        }
        myCodeEditor->setText(curFileModify);
        isCommentFold=true;
    }
    else
    {
        QMessageBox::warning(this,tr("注释行折叠错误"),tr("注释行已经折叠"));
    }
}



void MainWindow::on_actionShowComment_D_triggered()
{
    if(isCommentFold)
    {
        myCodeEditor->setText(curFileReplica);
        isCommentFold=false;
    }
    else
    {
        QMessageBox::warning(this,tr("注释显示错误"),tr("注释未折叠"));
    }
}

void MainWindow::on_actionHelp_H_triggered()
{
    showHelpInformation();
}

void MainWindow::showHelpInformation()
{
    helpDia = new QDialog(this);
    QVBoxLayout * helpLayout = new QVBoxLayout(helpDia);
    QTextEdit * help = new QTextEdit;
    QPushButton * confirmBtn =new QPushButton;
    confirmBtn->setText("确定");
    QFile helpFile(":/keyrwords/prefix1/ide help.txt");
    if(!helpFile.open(QFile::ReadOnly|QFile::Text))
    {
        QMessageBox::warning(this,tr("加载帮助文档失败"),tr("找不到帮助文档"));
        return;
    }
    QTextStream helpReadIn(&helpFile);
    helpLayout->addWidget(help);
    helpLayout->addWidget(confirmBtn);
    QApplication::setOverrideCursor(Qt::WaitCursor);
    help->setText(helpReadIn.readAll());
    QApplication::restoreOverrideCursor();
    connect(confirmBtn,SIGNAL(clicked()),this,SLOT(closeHelpDia()));
    helpDia->show();
}

void MainWindow::closeHelpDia()
{
    helpDia->close();
}
