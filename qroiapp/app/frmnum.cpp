//
// Copyright (C) 2017
// All rights reserved by 영진하이텍
//
#include "frmnum.h"
#include "ui_frmnum.h"
#include "qdesktopwidget.h"
#include <qdebug.h>

frmNum::frmNum(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::frmNum)
{
    _instance = nullptr;
    _parent = parent;
    ui->setupUi(this);
    this->InitForm();
    this->InitProperty();
    this->ChangeStyle();
    bClosePressed = false;
}

frmNum::~frmNum()
{
    delete ui;

}

void frmNum::Init(QString style, int fontSize)
{
    this->currentStyle = style;
    this->currentFontSize = fontSize;
    this->ChangeStyle();
}

void frmNum::mouseMoveEvent(QMouseEvent *e)
{
    if (!_instance)
        return;
    if (mousePressed && (e->buttons() && Qt::LeftButton)) {
        this->move(e->globalPos() - mousePoint);
        e->accept();
    }
}

void frmNum::mousePressEvent(QMouseEvent *e)
{
    if (!_instance)
        return;
    if (e->button() == Qt::LeftButton) {
        mousePressed = true;
        mousePoint = e->globalPos() - this->pos();
        e->accept();
    }
}

void frmNum::mouseReleaseEvent(QMouseEvent *)
{
    if (!_instance)
        return;
    mousePressed = false;
}

void frmNum::InitForm()
{
    this->mousePressed = false;
    this->setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint | Qt::FramelessWindowHint);

    QDesktopWidget w;
    deskWidth = w.availableGeometry().width();
    deskHeight = w.availableGeometry().height();
    frmWidth = this->width();
    frmHeight = this->height();

    //isFirst = true;
    //isPress = false;
    //timerPress = new QTimer(this);
    //connect(timerPress, SIGNAL(timeout()), this, SLOT(reClicked()));
    currentWidget = 0;

    // If you need to change the input method panel style, change the currentStyle this variable can be
    // blue - light blue dev - dev style black - black brown - gray black lightgray - light gray darkgray - dark gray gray - gray silvery - silver
    currentStyle = "";

    //nput method font font size, if you need to change the panel font size, which can be
    currentFontSize = 8;

    QList<QPushButton *> btn = this->findChildren<QPushButton *>();
    foreach (QPushButton * b, btn) {
        connect(b, SIGNAL(clicked()), this, SLOT(btn_clicked()));
    }

    //Bind the global change of the focus signal slot
    connect(qApp, SIGNAL(focusChanged(QWidget *, QWidget *)),
            this, SLOT(focusChanged(QWidget *, QWidget *)));
    //Bind the global change of the focus signal slot
    qApp->installEventFilter(this);
}

void frmNum::InitProperty()
{
    ui->btn0->setProperty("btnNum", true);
    ui->btn1->setProperty("btnNum", true);
    ui->btn2->setProperty("btnNum", true);
    ui->btn3->setProperty("btnNum", true);
    ui->btn4->setProperty("btnNum", true);
    ui->btn5->setProperty("btnNum", true);
    ui->btn6->setProperty("btnNum", true);
    ui->btn7->setProperty("btnNum", true);
    ui->btn8->setProperty("btnNum", true);
    ui->btn9->setProperty("btnNum", true);
    ui->btnminus->setProperty("btnNum", true);

    ui->btnDot->setProperty("btnOther", true);
    ui->btnBack->setProperty("btnOther", true);
    ui->btnDelete->setProperty("btnOther", true);
}

void frmNum::ShowPanel()
{
    this->setVisible(true);
}

void frmNum::focusChanged(QWidget *oldWidget, QWidget *nowWidget)
{
    if (!_instance)
        return;
    if (bClosePressed) {
        bClosePressed = false;
        return;
    }

    qDebug() << "oldWidget:" << oldWidget << " nowWidget:" << nowWidget;
    if (nowWidget != 0 && !this->isAncestorOf(nowWidget)) {
        // In Qt5 and linux systems (except for embedded linux), when the input method panel is closed, the focus becomes nothing, and then the focus moves to the focus control again
        // This led to the input method of the panel button does not work, close immediately after the control to obtain the focus and display.
        // for this, increase the judgment, when the focus is from the object to no object and then converted to an object do not show.
        // Here again a judge, if the first form of the first focus is to fall into the object can be entered, you have to filter out
#if 1//ndef __arm__
//        if (oldWidget == 0x0 && !isFirst) {
//            QTimer::singleShot(0, this, SLOT(hide()));
//            return;
//        }
#endif

        //If the corresponding attribute noinput is true, it is not displayed
        if (nowWidget->property("noinput").toBool()) {
            QTimer::singleShot(0, this, SLOT(hide()));
            return;
        }

        //isFirst = false;
        if (nowWidget->inherits("QLineEdit")) {
            currentLineEdit = (QLineEdit *)nowWidget;
            currentEditType = "QLineEdit";
            ShowPanel();
        } else if (nowWidget->inherits("QTextEdit")) {
            currentTextEdit = (QTextEdit *)nowWidget;
            currentEditType = "QTextEdit";
            ShowPanel();
        } else if (nowWidget->inherits("QPlainTextEdit")) {
            currentPlain = (QPlainTextEdit *)nowWidget;
            currentEditType = "QPlainTextEdit";
            ShowPanel();
        } else if (nowWidget->inherits("QTextBrowser")) {
            currentBrowser = (QTextBrowser *)nowWidget;
            currentEditType = "QTextBrowser";
            ShowPanel();
        } else if (nowWidget->inherits("QComboBox")) {
            QComboBox *cbox = (QComboBox *)nowWidget;
            //Only when the drop-down selection box is in edit mode can you enter it
            if (cbox->isEditable()) {
                currentLineEdit = cbox->lineEdit() ;
                currentEditType = "QLineEdit";
                ShowPanel();
            }
        } else if (nowWidget->inherits("QSpinBox") ||
                   nowWidget->inherits("QDoubleSpinBox") ||
                   nowWidget->inherits("QDateEdit") ||
                   nowWidget->inherits("QTimeEdit") ||
                   nowWidget->inherits("QDateTimeEdit")) {
            currentWidget = nowWidget;
            currentEditType = "QWidget";
            ShowPanel();
        } else {
            currentWidget = 0;
            currentLineEdit = 0;
            currentTextEdit = 0;
            currentPlain = 0;
            currentBrowser = 0;
            currentEditType = "";
            this->setVisible(false);
        }

        QRect rect = nowWidget->rect();
        QPoint pos = QPoint(rect.left(), rect.bottom() + 2);
        pos = nowWidget->mapToGlobal(pos);

        int x = pos.x();
        int y = pos.y();
        if (pos.x() + frmWidth > deskWidth) {
            x = deskWidth - frmWidth;
        }
        if (pos.y() + frmHeight > deskHeight) {
            y = pos.y() - frmHeight - rect.height() - 35;
        }

        this->setGeometry(x, y, frmWidth, frmHeight);
    } else {
        qDebug() << oldWidget;
        if (oldWidget != 0 && oldWidget->inherits("QPushButton"))
        {
            QRect rect = oldWidget->rect();
            QPoint pos = QPoint(rect.left(), rect.bottom() + 2);
            pos = oldWidget->mapToGlobal(pos);

            int x = pos.x();
            int y = pos.y();
            if (pos.x() + frmWidth > deskWidth) {
                x = deskWidth - frmWidth;
            }
            if (pos.y() + frmHeight > deskHeight) {
                y = pos.y() - frmHeight - rect.height() - 35;
            }

            this->setGeometry(x, y, frmWidth, frmHeight);
        }
    }
}

bool frmNum::eventFilter(QObject *obj, QEvent *event)
{
    if (!_instance)
        return false;
    if (event->type() == QEvent::MouseButtonPress) {
        if (currentEditType != "") {
            if (obj != ui->btnClose) {
                QString objName = obj->objectName();
                if (!obj->property("noinput").toBool() && objName != "frmMainWindow"
                        && objName != "frmInputWindow" && objName != "qt_edit_menu") {
                    ShowPanel();
                }
            }
            btnPress = (QPushButton *)obj;
//            if (checkPress()) {
//                isPress = true;
//                timerPress->start(500);
//            }
        }
        return false;
    } else if (event->type() == QEvent::MouseButtonRelease) {
        btnPress = (QPushButton *)obj;
//        if (checkPress()) {
//            isPress = false;
//            timerPress->stop();
//        }
        return false;
    }
    return QWidget::eventFilter(obj, event);
}

bool frmNum::checkPress()
{
    //Only the legal key belonging to the numeric keypad will continue to be processed
    bool num_ok = btnPress->property("btnNum").toBool();
    bool other_ok = btnPress->property("btnOther").toBool();
    if (num_ok || other_ok) {
        return true;
    }
    return false;
}

//void frmNum::reClicked()
//{
//    if (isPress) {
//        timerPress->setInterval(30);
//        btnPress->click();
//    }
//}

void frmNum::btn_clicked()
{
    //If the current focus control type is empty, the return does not need to continue processing
    if (currentEditType == "") {
        return;
    }

    QPushButton *btn = (QPushButton *)sender();
    QString objectName = btn->objectName();
    if (objectName == "btnDelete") {
        deleteValue();
    } else if (objectName == "btnClose") {
        currentEditType = "";
        bClosePressed = true;
        this->setVisible(false);
    } else if (objectName == "btnEnter") {
        this->setVisible(false);
    } else if (objectName == "btnBack") {
        backValue();
    } else {
        QString value = btn->text();
        insertValue(value);
    }
}

void frmNum::insertValue(QString value)
{
    if (currentEditType == "QLineEdit") {
        currentLineEdit->insert(value);
    } else if (currentEditType == "QTextEdit") {
        currentTextEdit->insertPlainText(value);
    } else if (currentEditType == "QPlainTextEdit") {
        currentPlain->insertPlainText(value);
    } else if (currentEditType == "QTextBrowser") {
        currentBrowser->insertPlainText(value);
    } else if (currentEditType == "QPushButton") {
        val += value;
        currentPushButton->setText(val);
    } else if (currentEditType == "QWidget") {
        QKeyEvent keyPress(QEvent::KeyPress, 0, Qt::NoModifier, QString(value));
        QApplication::sendEvent(currentWidget, &keyPress);
    }
}

void frmNum::deleteValue()
{
    if (currentEditType == "QLineEdit") {
        currentLineEdit->del();
    } else if (currentEditType == "QTextEdit") {
        //Get the current QTextEdit cursor, if the cursor is selected, remove the selected character, otherwise delete the cursor before a character
        QTextCursor cursor = currentTextEdit->textCursor();
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
        } else {
            cursor.deleteChar();
        }
    } else if (currentEditType == "QPlainTextEdit") {
        //Get the current QTextEdit cursor, if the cursor is selected, remove the selected character, otherwise delete the cursor before a character
        QTextCursor cursor = currentPlain->textCursor();
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
        } else {
            cursor.deleteChar();
        }
    } else if (currentEditType == "QTextBrowser") {
        //Get the current QTextEdit cursor, if the cursor is selected, remove the selected character, otherwise delete the cursor before a character
        QTextCursor cursor = currentBrowser->textCursor();
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
        } else {
            cursor.deleteChar();
        }
    } else if (currentEditType == "QWidget") {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Delete, Qt::NoModifier, QString());
        QApplication::sendEvent(currentWidget, &keyPress);
    } else if (currentEditType == "QPushButton") {
        int len = val.length();
        val = val.mid(0, len-1);
        currentPushButton->setText(val);
    }
}

void frmNum::backValue()
{
    if (currentEditType == "QLineEdit") {
        currentLineEdit->backspace();
    } else if (currentEditType == "QTextEdit") {
        //Get the current QTextEdit cursor, if the cursor is selected, remove the selected character, otherwise delete the cursor before a character
        QTextCursor cursor = currentTextEdit->textCursor();
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
        } else {
            cursor.deletePreviousChar();
        }
    } else if (currentEditType == "QPlainTextEdit") {
        //Get the current QTextEdit cursor, if the cursor is selected, remove the selected character, otherwise delete the cursor before a character
        QTextCursor cursor = currentPlain->textCursor();
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
        } else {
            cursor.deletePreviousChar();
        }
    } else if (currentEditType == "QTextBrowser") {
        //Get the current QTextEdit cursor, if the cursor is selected, remove the selected character, otherwise delete the cursor before a character
        QTextCursor cursor = currentBrowser->textCursor();
        if(cursor.hasSelection()) {
            cursor.removeSelectedText();
        } else {
            cursor.deletePreviousChar();
        }
    } else if (currentEditType == "QWidget") {
        QKeyEvent keyPress(QEvent::KeyPress, Qt::Key_Backspace, Qt::NoModifier, QString());
        QApplication::sendEvent(currentWidget, &keyPress);
    } else if (currentEditType == "QPushButton") {
        int len = val.length();
        val = val.mid(0, len-1);
        currentPushButton->setText(val);
    }
}

void frmNum::ChangeStyle()
{
    if (currentStyle == "blue") {
        changeStyle("#DEF0FE", "#C0DEF6", "#C0DCF2", "#386487");
    } else if (currentStyle == "dev") {
        changeStyle("#C0D3EB", "#BCCFE7", "#B4C2D7", "#324C6C");
    } else if (currentStyle == "gray") {
        changeStyle("#E4E4E4", "#A2A2A2", "#A9A9A9", "#000000");
    } else if (currentStyle == "lightgray") {
        changeStyle("#EEEEEE", "#E5E5E5", "#D4D0C8", "#6F6F6F");
    } else if (currentStyle == "darkgray") {
        changeStyle("#D8D9DE", "#C8C8D0", "#A9ACB5", "#5D5C6C");
    } else if (currentStyle == "black") {
        changeStyle("#4D4D4D", "#292929", "#D9D9D9", "#CACAD0");
    } else if (currentStyle == "brown") {
        changeStyle("#667481", "#566373", "#C2CCD8", "#E7ECF0");
    } else if (currentStyle == "silvery") {
        changeStyle("#E1E4E6", "#CCD3D9", "#B2B6B9", "#000000");
    }
}

void frmNum::changeStyle(QString topColor, QString bottomColor, QString borderColor, QString textColor)
{
    QStringList qss;
    qss.append(QString("QWidget#widget_title{background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);}")
               .arg(topColor).arg(bottomColor));
    qss.append("QPushButton{padding:5px;border-radius:3px;}");
    qss.append(QString("QPushButton:hover{background:qlineargradient(spread:pad,x1:0,y1:0,x2:0,y2:1,stop:0 %1,stop:1 %2);}")
               .arg(topColor).arg(bottomColor));
    qss.append(QString("QLabel,QPushButton{font-size:%1pt;color:%2;}")
               .arg(currentFontSize).arg(textColor));
    qss.append(QString("QPushButton#btnPre,QPushButton#btnNext,QPushButton#btnClose{padding:5px;}"));
    qss.append(QString("QPushButton{border:1px solid %1;background:rgba(0,0,0,0);}")
               .arg(borderColor));
    qss.append(QString("QLineEdit{border:1px solid %1;border-radius:5px;padding:2px;background:none;selection-background-color:%2;selection-color:%3;}")
               .arg(borderColor).arg(bottomColor).arg(topColor));
    this->setStyleSheet(qss.join(""));
}
