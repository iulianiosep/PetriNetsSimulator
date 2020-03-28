#ifndef GRAPHDIALOG_H
#define GRAPHDIALOG_H

#include <QDialog>
#include "state.h"

namespace Ui {
class GraphDialog;
}

class GraphDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GraphDialog(QWidget *parent = 0);
    ~GraphDialog();
    void drawReachabilityGraph();
    void drawCoverabilityGraph();

private slots:
    void on_GraphDialog_finished(int result);

private:
    Ui::GraphDialog *ui;
};

#endif // GRAPHDIALOG_H
