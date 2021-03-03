#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include<Qsci/qsciscintilla.h>
#include<Qsci/qscilexer.h>
#include<Qsci/qsciapis.h>
#include<QKeyEvent>
#include<QFontComboBox>
#include<QDialog>
#include<QtCore>
#include<QtGui>
#include<QDirModel>

class QLineEdit;
class QDialog;
class QLabel;
class QAction;
class QMenu;
class QsciScintilla;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QsciScintilla * myCodeEditor = new QsciScintilla (this);
    void newfile();
    bool maybeSave();
    bool save();
    bool saveAs();
    bool savefile(const QString &filename);
    bool loadfile(const QString &filename);
    void codeReplace();
    void mywheelEvent(QWheelEvent * event);
    void countLineNumber();
    void setCareColor(QMouseEvent *event);
    bool compile();
    void runCode();
    void commentALL();
    void fontSetting();
    void showHelpInformation();
private slots:
    void on_actionNew_N_triggered();

    void on_actionSave_S_triggered();

    void on_actionSaveAs_A_triggered();

    void on_actionOpen_O_triggered();

    void showFindText();

    void on_actionfind_F_triggered();

    void on_actionreplace_R_triggered();

    void codeRepText();


    void on_actionUndo_Z_triggered();

    void on_actionCopy_C_triggered();

    void on_actionPaste_V_triggered();

    void on_actionCut_X_triggered();

    void on_actionstart_triggered();

    void on_actionrun_R_triggered();

    void countInfo();

    void on_actioncommentAll_triggered();

    void on_actioninsert_I_triggered();

    void handlePuncComplete(int key);

    //字体设置槽函数
    void fontSetConfirmed();

    void fontSetCanceled();

    //断点调试槽函数
    void on_margin_clicked(int, int, Qt::KeyboardModifiers);

    //
    void on_treeView_doubleClicked(const QModelIndex &index);

    void on_actionClose_triggered();

    void on_actionExit_E_triggered();

    void on_actionHideComment_H_triggered();

    void on_actionShowComment_D_triggered();

    void on_actionHelp_H_triggered();

    //关闭帮助窗口槽函数
    void closeHelpDia();
private:
    Ui::MainWindow *ui;
    bool isUntitled;//真表示文件未保存过，假则表示文件已保存过
    QString curFile;//保存当前文件路径]
    QString curFileName;//保存当前文件名
    QLineEdit *findLineEdit;
    QDialog *findDlg;
    QDialog *codeRepDlg;
    QLineEdit * repCodeEdit;
    QLineEdit * origCodeEdit;
    //行列数统计参数
    QLabel * statusLabel1;
    //版本显示参数
    QLabel * editionInfo;
    //字体设置参数
    QLabel *fontSize;
    QLabel * fontFormat;
    QLineEdit * fontSizeInput;
    QFontComboBox * fontBox;
    QDialog *myFontSetting;
    int LineNum;
    int columnNum;
    bool isCompiled;
    //bool eventFilter(QObject * obj,QEvent * event);
    QDirModel * model;
    QStringList commentList;
    //注释隐藏结构体
    struct commentHide{
        QString curComment;
        int startPoint;
        int commentLength;
    } mycomment[1000];
    //注释是否已经折叠
    bool isCommentFold;
    //折叠后操作副本
    QString curFileModify;
    //原文本副本
    QString curFileReplica;
    int capCount=0;
    //帮助显示对话框
    QDialog * helpDia;
};

class MykeyPress: public QObject
{
    Q_OBJECT

signals:
    void keyPressSiganl_puncComplete(int);
protected:
    bool eventFilter(QObject *obj,QEvent *event);
};

#endif // MAINWINDOW_H
