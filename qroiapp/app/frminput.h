#ifndef FRMINPUT_H
#define FRMINPUT_H

#include <QWidget>
#include <QMouseEvent>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QTextEdit>
#include <QPlainTextEdit>
#include <QTextBrowser>
#include <QPushButton>
#include <QTimer>
#include <QShowEvent>
#include <QMutexLocker>

using namespace std;

namespace Ui
{
class frmInput;
}

class frmInput : public QWidget
{
    Q_OBJECT

public:
    frmInput(QWidget *parent = 0);
    ~frmInput();

    //Singleton mode, to ensure that a program only exists an input method instance object
    frmInput *Instance(QWidget *parent = 0) {
        if (_instance) {
            //delete _instance;
            _instance = nullptr;
        }
        if (!_instance)
        {
            //static QMutex mutex;
            //QMutexLocker locker(&mutex);
            _instance = this;//new frmInput(parent);
        }
        return _instance;
    }
    void DeleteInstance() {
        //if (_instance)
        //    delete _instance;
        _instance = nullptr;
    }

    //Initialize panel status, including font size
    void Init(QString position, QString style, int btnFontSize, int labFontSize);

protected:
    //Event filter, handle the mouse click on the action at the Chinese character label
    bool eventFilter(QObject *obj, QEvent *event);
    //Mouse drag events
    void mouseMoveEvent(QMouseEvent *e);
    //Mouse press event
    void mousePressEvent(QMouseEvent *e);
    //Mouse release event
    void mouseReleaseEvent(QMouseEvent *);

private slots:
    //Focus change event slot function processing
    void focusChanged(QWidget *oldWidget, QWidget *nowWidget);
    //Input method panel key processing
    void btn_clicked();
    //Change the input method panel style
    void changeStyle(QString topColor, QString bottomColor,
                     QString borderColor, QString textColor);
    //The timer handles the backspace key
    void reClicked();

private:
    Ui::frmInput * ui;
    frmInput *_instance; // instance object
    QWidget *_parent;

    int deskWidth; // desktop width
    int deskHeight; // desktop height
    int frmWidth; // form width
    int frmHeight; // form height

    QPoint mousePoint; // coordinates when the mouse drags the custom title bar
    bool mousePressed; // whether the mouse is pressed

    bool isPress; // whether to press Backspace
    QPushButton * btnPress; // long press the button
    QTimer * timerPress; // backspace key timer
    bool checkPress (); // check the current long press button

    bool bClosePressed;

    bool isFirst; // whether to load for the first time
    void InitForm (); // initialize the form data
    void InitProperty (); // initialize the property
    void ChangeStyle (); // change the style
    void ChangeFont (); // change the font size
    void ShowPanel (); // display the input method panel

    QWidget * currentWidget; // The object of the current focus
    QLineEdit * currentLineEdit; // single-line text box for current focus
    QTextEdit * currentTextEdit; // multi-line text box for current focus
    QPlainTextEdit * currentPlain; // current focus of the rich text box
    QTextBrowser * currentBrowser; // current focus text browsing box

    QString currentEditType; // The type of the current focus control
    QString currentPosition; // Current input method panel position type
    QString currentStyle; // current input method panel style
    int btnFontSize; // current input method panel button font size
    int labFontSize; // current input method panel label font size
    void insertValue (QString value); // insert the value into the current focus control
    void deleteValue (); // delete a character of the current focus control

    QString currentType; // current input method type
    void changeType (QString type); // change the input method type
    void changeLetter (bool isUpper); // change the letter case

};

#endif // FRMINPUT_H
