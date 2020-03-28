#include "state.h"
#include "ui_state.h"

State::State(): QWidget(0), ui(0), level(0) {
    this->parentState = 0;
}

State::State(QList<Element *> &places): State() {
    this->tokenCounts.resize(places.count());
    for (Element *place: places) {
        this->tokenCounts[place->number()] = ((Place *)place)->tokenCount();
    }
}

State::State(const State &state): State() {
    this->tokenCounts = state.tokenCounts;
}

State::~State() {
    if (ui) {
        delete ui;
    }
}

bool State::operator==(const State &stateToCompareTo) {
    return this->tokenCounts == stateToCompareTo.tokenCounts;
}

QString State::description() {
    QString description = "(";
    if (this->tokenCounts.count() != 0) {
        if (this->tokenCounts[0] != -1) {
            description += QString::number(this->tokenCounts[0]);
        }
        else {
            description += "inf";
        }
        for (int i = 1; i < this->tokenCounts.count(); ++i) {
            description += ",";
            int tokenCount = this->tokenCounts[i];
            if (tokenCount != -1) {
                description += QString::number(tokenCount);
            }
            else {
                description += "inf";
            }
        }
    }
    description += ")";
    return description;
}

void State::setupUi(QWidget *parent)
{
    this->setParent(parent);
    ui = new Ui::State;
    ui->setupUi(this);
    this->show();
    QString text = "(";
    for (int tokenCount: this->tokenCounts) {
        text += QString::number(tokenCount) + ",";
    }
    text += ")";
    for (State::StateConnection connection: this->outgoingConnections) {
        text += "T" + QString::number(connection.transition->number()) + ",";
    }
    ui->tokenCountsLabel->setText(text);
    this->adjustSize();
}
