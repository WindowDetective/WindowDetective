/////////////////////////////////////////////////////////////////////
// File: ColourButton.h                                            //
// Date: 26/3/10                                                   //
// Desc: A button which allows the user to pick a colour.          //
/////////////////////////////////////////////////////////////////////

#ifndef COLOUR_BUTTON_H
#define COLOUR_BUTTON_H

#include "window_detective/include.h"

class ColourButton : public QPushButton {
    Q_OBJECT
private:
    const static int PaddingHor;
    const static int PaddingVert;
    QColor colour;

public:
    ColourButton(QWidget *parent = 0);
    ~ColourButton() {}

    QColor getColour() { return colour; }
    void setColour(const QColor& c) { colour = c; }
protected:
    void paintEvent(QPaintEvent* e);
private slots:
    void showColourChooser();
};

#endif   // COLOUR_BUTTON_H