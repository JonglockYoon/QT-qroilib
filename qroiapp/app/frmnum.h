#ifndef FRMNUM_H
#define FRMNUM_H

#include <QWidget>
#include <QMouseEvent>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QTimer>
#include <QMutexLocker>

namespace Ui
{
class frmNum;
}

class frmNum : public QWidget
{
    Q_OBJECT

public:
    frmNum(QWidget *parent = 0);
    ~frmNum();

    //singleton mode, to ensure that a program only exists an input method instance object
    frmNum *Instance(QWidget *parent = 0) {
        if (_instance) {
            //delete _instance;
            _instance = nullptr;
        }
        if (!_instance)
        {
            //static QMutex mutex;
            //QMutexLocker locker(&mutex);
            _instance = this;//new frmNum(parent);
        }
        return _instance;
    }
    void DeleteInstance() {
        //if (_instance)
        //    delete _instance;
        _instance = nullptr;
    }

    // Initialize panel status, including font size
    void Init(QString style, int fontSize);
protected:
    //event filter, handle the mouse press the pop-up input method panel
    bool eventFilter(QObject *obj, QEvent *event);
    // mouse drag events
    void mouseMoveEvent(QMouseEvent *e);
    // mouse press events
    void mousePressEvent(QMouseEvent *e);
    // mouse release events
    void mouseReleaseEvent(QMouseEvent *);

public slots:
    //focus change event slot function processing
    void focusChanged(QWidget *oldWidget, QWidget *nowWidget);

private slots:
    // input method panel key processing
    void btn_clicked();
    //change the input method panel style
    void changeStyle(QString topColor, QString bottomColor,
                     QString borderColor, QString textColor);
    //timer handles the backspace key
    //void reClicked();
    //display panel
    void ShowPanel();

private:
    Ui::frmNum *ui;
    frmNum *_instance;       //instance object
    QWidget *_parent;

    int deskWidth; // desktop width
    int deskHeight; // desktop height
    int frmWidth; // form width
    int frmHeight; // form height

    //bool isPress;                   // whether to press Backspace
    QPushButton *btnPress;          // ong press the button
    //QTimer *timerPress;             // backspace key timer
    bool checkPress();              //check the current long press button

    bool bClosePressed;

    QPoint mousePoint;              //coordinates when the mouse drags the custom title bar
    bool mousePressed;              //whether the mouse is pressed

    //bool isFirst;                   //whether to load for the first time
    void InitForm();                //initialize the form data
    void InitProperty();            //initialize the property
    void ChangeStyle();             //change the style

    QWidget *currentWidget;         //The object of the current focus
    QLineEdit *currentLineEdit;     //single-line text box for current focus
    QTextEdit *currentTextEdit;     // multi-line text box for current focus
    QPlainTextEdit *currentPlain;   //current focus of the rich text box
    QTextBrowser *currentBrowser;   //current focus text browsing box

    QString currentStyle;           //current input method panel style
    int currentFontSize;            //current input panel font size
    void insertValue(QString value);//insert the value into the current focus control
    void backValue();             //delete a character of the current focus control
    void deleteValue();
public:
    QString currentEditType;        //The type of the current focus control
    QPushButton *currentPushButton;
    QString val;
};

#endif // FRMNUM_H
