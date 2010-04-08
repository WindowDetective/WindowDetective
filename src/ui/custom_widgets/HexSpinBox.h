/////////////////////////////////////////////////////////////////////
// File: HexSpinBox.h                                              //
// Date: 30/3/10                                                   //
// Desc: Subclass of QSpinBox that handles the input of            //
//   hexadecimal (base 16) numbers as well as decimal.             //
/////////////////////////////////////////////////////////////////////

#ifndef HEX_SPIN_BOX_H
#define HEX_SPIN_BOX_H

#include "window_detective/include.h"

class HexSpinBox : public QSpinBox {
    Q_OBJECT
private:
    QValidator* validator;
    bool displayAsHex;

public:
    HexSpinBox(QWidget *parent = 0);
    ~HexSpinBox();

protected:
    bool isHexString(const String& text) const;
    QValidator::State validate(String& text, int& pos) const;
    int valueFromText(const String& text) const;
    String textFromValue(int value) const;
};

#endif   // HEX_SPIN_BOX_H