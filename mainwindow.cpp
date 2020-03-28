#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "pnsglobal.h"
#include "simulationengine.h"
#include "graphdialog.h"
#include <QMessageBox>
#include "reportdialog.h"

const QString MainWindow::DefaultWindowTitle = "Petri-net Simulator";

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    currentProjectFilePath(QString())
{
    ui->setupUi(this);
    this->matejkoCanvas = ui->matejkoCanvas;
    this->matejkoCanvas->places = &(this->places);
    this->matejkoCanvas->transitions = &(this->transitions);
    this->matejkoCanvas->arrows = &(this->arrows);

    SimulationEngine &engine = SimulationEngine::getInstance();
    engine.places = &(this->places);
    engine.transitions = &(this->transitions);
    engine.arrows = &(this->arrows);

    QWidget* leftSpacer = new QWidget(this);
    QWidget* rightSpacer = new QWidget(this);
    leftSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    rightSpacer->setSizePolicy(QSizePolicy::Expanding,QSizePolicy::Preferred);
    ui->simulationToolBar->insertWidget(ui->actionExecuteTransition, leftSpacer);
    ui->simulationToolBar->addWidget(rightSpacer);

    statusBarLabel = new QLabel("Design mode", this);
    ui->statusBar->addWidget(statusBarLabel);

    ui->simulationToolBar->setEnabled(false);
    ui->statusBar->setStyleSheet("background-color: rgb(165,226,224);");

    setWindowTitle(MainWindow::DefaultWindowTitle + " - untitled");
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::clearLists()
{
    qDeleteAll(places);
    qDeleteAll(transitions);
    qDeleteAll(arrows);
    places.clear();
    transitions.clear();
    arrows.clear();
}

void MainWindow::on_actionNew_Project_triggered()
{
    this->currentProjectFilePath = QFileDialog::getSaveFileName(this, "Create new project", ".", "JSON Files (*.json)");
    if (!this->currentProjectFilePath.isEmpty() && this->resolveIfOverwrite(this->currentProjectFilePath)){
        setWindowTitleForProject(this->currentProjectFilePath);
        clearLists();
        dataHandler.save(this->currentProjectFilePath.toStdString(), this->places, this->transitions, this->arrows);
        this->matejkoCanvas->update();
    }
}

void MainWindow::on_actionOpen_project_triggered()
{
    this->currentProjectFilePath = QFileDialog::getOpenFileName(this, "Open project", ".", "JSON Files (*.json)");
    if (!this->currentProjectFilePath.isEmpty()){
        this->setWindowTitleForProject(this->currentProjectFilePath);
        this->dataHandler.load(this->currentProjectFilePath.toStdString(), this->matejkoCanvas, this->places, this->transitions, this->arrows);
        this->matejkoCanvas->update();
    }
}

void MainWindow::on_actionSave_project_triggered()
{
    if (this->currentProjectFilePath.isEmpty() || this->currentProjectFilePath.isNull()){
        this->currentProjectFilePath = QFileDialog::getSaveFileName(this, "Create new project", ".", "JSON Files (*.json)");
        if (!this->currentProjectFilePath.isEmpty() && this->resolveIfOverwrite(this->currentProjectFilePath)){
            setWindowTitleForProject(this->currentProjectFilePath);
        }
        else{
            return;
        }
    }
    dataHandler.save(currentProjectFilePath.toStdString(), this->places, this->transitions, this->arrows);
}

void MainWindow::on_actionExecuteTransition_triggered() {
    Element *selectedElement = this->matejkoCanvas->selectedElement();
    if (qobject_cast<Transition *>(selectedElement) && selectedElement->active()) {
        SimulationEngine::getInstance().executeTransition(this->matejkoCanvas->selectedElement());
    }
}

void MainWindow::on_actionToggleSimulationMode_toggled(bool simulationModeOn) {
    matejkoCanvas->setSimulationModeOn(simulationModeOn);
    ui->simulationToolBar->setEnabled(simulationModeOn);
    ui->actionNew_Project->setEnabled(!simulationModeOn);
    ui->actionOpen_project->setEnabled(!simulationModeOn);
    ui->actionSave_project->setEnabled(!simulationModeOn);
    ui->actionSave_as->setEnabled(!simulationModeOn);

    if (simulationModeOn) {
        this->statusBarLabel->setText("Simulation mode");
        ui->statusBar->setStyleSheet("background-color: rgb(180, 255, 180);");
        ui->netAnalysisToolBar->setEnabled(true);
        this->matejkoCanvas->saveBoardState();
        SimulationEngine::getInstance().beginSimulation();
    }
    else{
        this->statusBarLabel->setText("Design mode");
        ui->statusBar->setStyleSheet("background-color: rgb(165,226,224);");
        ui->netAnalysisToolBar->setEnabled(false);
        this->matejkoCanvas->restoreBoardState();
        SimulationEngine::getInstance().endSimulation();
    }
}

void MainWindow::on_actionCoverabilityGraph_triggered()
{
    GraphDialog *coverabilityGraphDialog = new GraphDialog(this);
    coverabilityGraphDialog->setWindowTitle("Coverability graph");
    coverabilityGraphDialog->drawCoverabilityGraph();
    coverabilityGraphDialog->show();
}

void MainWindow::on_actionReachabilityGraph_triggered()
{
    GraphDialog *reachabilityGraphDialog = new GraphDialog(this);
    reachabilityGraphDialog->setWindowTitle("Reachability graph");
    reachabilityGraphDialog->drawReachabilityGraph();
    reachabilityGraphDialog->show();
}


void MainWindow::on_actionSave_as_triggered()
{
    QString filename = QFileDialog::getSaveFileName(this, "Save as", ".", "JSON Files (*.json)");
    if (!filename.isEmpty() && this->resolveIfOverwrite(filename)){
        this->currentProjectFilePath = filename;
        setWindowTitleForProject(this->currentProjectFilePath);
        dataHandler.save(this->currentProjectFilePath.toStdString(), this->places, this->transitions, this->arrows);
    }
}

void MainWindow::setWindowTitleForProject(const QString &projectPath)
{
    int separatorCount = projectPath.count(QDir::separator());
    QString projectName = projectPath.section(QDir::separator(), separatorCount, separatorCount).remove(".json");
    setWindowTitle(MainWindow::DefaultWindowTitle + " - " + projectName);
}

bool MainWindow::resolveIfOverwrite(const QString &projectPath)
{
    if (QFile::exists(projectPath)){
        if (QMessageBox::Yes == QMessageBox(QMessageBox::Information, "File exists", "Do you want to overwrite it?", QMessageBox::Yes|QMessageBox::No).exec()){
            return true;
        }
        else {
            return false;
        }
    }
    return true;
}

void MainWindow::on_actionLiveness_triggered()
{
    ReportDialog *reportDialog = new ReportDialog(this);
    connect(reportDialog, SIGNAL(accepted()), reportDialog, SLOT(deleteLater()));
    reportDialog->setWindowTitle("Liveness");
    reportDialog->transitionLivenesses();
    reportDialog->show();
}

void MainWindow::on_actionConservation_triggered()
{
    ReportDialog *reportDialog = new ReportDialog(this, ReportDialog::ConservationView);
    connect(reportDialog, SIGNAL(accepted()), reportDialog, SLOT(deleteLater()));
    reportDialog->setWindowTitle("Conservation");
    reportDialog->netConservation();
    reportDialog->show();
}

void MainWindow::on_actionBoundedness_triggered() {
    ReportDialog *reportDialog = new ReportDialog(this);
    connect(reportDialog, SIGNAL(accepted()), reportDialog, SLOT(deleteLater()));
    reportDialog->setWindowTitle("Boundedness");
    reportDialog->reportBoundedness();
    reportDialog->show();
}

void MainWindow::on_actionIncidence_triggered() {
    ReportDialog *reportDialog = new ReportDialog(this);
    connect(reportDialog, SIGNAL(accepted()), reportDialog, SLOT(deleteLater()));
    reportDialog->setWindowTitle("Incidence");
    reportDialog->netIncidence();
    reportDialog->show();
}

void MainWindow::on_actionReversability_triggered() {
    ReportDialog *reportDialog = new ReportDialog(this);
    reportDialog->setWindowTitle("Reversability");
    reportDialog->reportReversability();
    reportDialog->show();
}
