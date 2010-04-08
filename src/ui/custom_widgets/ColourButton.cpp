/////////////////////////////////////////////////////////////////////
// File: ColourButton.cpp                                          //
// Date: 26/3/10                                                   //
// Desc: A button which allows the user to pick a colour.          //
/////////////////////////////////////////////////////////////////////


#include "ColourButton.h"

const int ColourButton::PaddingHor   = 10;
const int ColourButton::PaddingVert  = 5;

ColourButton::ColourButton(QWidget* parent) :
    QPushButton(parent),
    colour() {

    // Not sure if there is a better way of doing this (event maybe)
    connect(this, SIGNAL(clicked()), this, SLOT(showColourChooser()));
}

void ColourButton::showColourChooser() {
    QColor chosen = QColorDialog::getColor(colour, this, tr("Select colour"));
    if (chosen.isValid())
        colour = chosen;
}

void ColourButton::paintEvent(QPaintEvent* e) {
    // Paint the standard button first
    QPushButton::paintEvent(e);

    // Then paint a coloured rectangle
    if (colour.isValid()) {
        QPainter painter(this);
        painter.fillRect(PaddingHor, PaddingVert,
                width()-(PaddingHor*2), height()-(PaddingVert*2), colour);
    }
}
