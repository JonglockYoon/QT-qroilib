//
// Copyright (C) 2017
//

#include <string>
#include <QStringList>
#include <QMessageBox>
#include <QSettings>

#include "dialogmodel.h"
#include "ui_dialogmodel.h"
#include "common.h"
#include "config.h"
#include "recipedata.h"
#include "mainwindow.h"

extern MainWindow* theMainWindow;

using namespace std;

DialogModel::DialogModel(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogModel)
{
    ui->setupUi(this);
    model = new QStringListModel(this);

    QString strRootDir = gCfg.RootPath;//QApplication::applicationDirPath();
    QString dirName;
    dirName.sprintf("%s/TeachingData", strRootDir.toStdString().c_str());
    m_dir.mkdir(dirName);
    m_dir.cd(dirName);
    m_sCurrentPath = m_dir.absolutePath();
    SetData();

    connect(ui->listDirs->selectionModel(),
          SIGNAL(selectionChanged(QItemSelection,QItemSelection)),
          this, SLOT(handleSelectionChanged(QItemSelection)));

    theMainWindow->kbdNum.DeleteInstance();
    theMainWindow->kbdAlpha.Instance(this)->Init("control", "silvery", 12, 12);
}

DialogModel::~DialogModel()
{
    theMainWindow->kbdAlpha.DeleteInstance();
    delete ui;
}

void DialogModel::on_pushButtonExit_clicked()
{
    hide();
}


void DialogModel::SetData()
{
    QStringList allFiles = m_dir.entryList(QDir::Dirs | QDir::NoSymLinks | QDir::NoDot | QDir::NoDotDot);
    model->setStringList(allFiles);
    ui->listDirs->setModel(model);
}

void DialogModel::on_pushButtonAdd_clicked()
{
    QString sModelName = ui->lineEditModelName->text();
    m_dir.cd(m_sCurrentPath);
    m_dir.mkdir(sModelName);
    SetData();

    gCfg.m_sLastRecipeName = sModelName;
    gCfg.WriteConfig();
    theMainWindow->setWindowTitle(gCfg.m_sLastRecipeName);
}

void DialogModel::on_pushButtonDelete_clicked()
{
    if (QMessageBox::Yes != QMessageBox::question(this, tr("Delete"), tr("Do you want to delete?"), QMessageBox::Yes | QMessageBox::No))
        return;

    QString sModelName = ui->lineEditModelName->text();
    m_dir.rmdir(sModelName);
    SetData();
}

void DialogModel::on_pushButtonChange_clicked()
{
    if (QMessageBox::Yes != QMessageBox::question(this, tr("Change"), tr("Do you want to change?"), QMessageBox::Yes | QMessageBox::No))
        return;

    QString sModelName = ui->lineEditModelName->text();
    gCfg.m_sLastRecipeName = sModelName;
    gCfg.WriteConfig();
    theMainWindow->setWindowTitle(gCfg.m_sLastRecipeName);

    theMainWindow->setLoadRoi();
}

void DialogModel::handleSelectionChanged(const QItemSelection& selection)
{
   if(selection.indexes().isEmpty()) {
      ui->lineEditModelName->clear();
   } else {
       QModelIndexList indexes = selection.indexes();
       foreach(const QModelIndex &index, indexes) {
            QStringList list = model->stringList();
            QString str = list[index.row()];
            ui->lineEditModelName->setText(str);
            break;
       }
   }
}
