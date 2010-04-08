/////////////////////////////////////////////////////////////////////
// File: HexSpinBox.h                                              //
// Date: 30/3/10                                                   //
// Desc: Subclass of QSpinBox that handles the input of            //
//   hexadecimal (base 16) numbers as well as decimal.             //
/////////////////////////////////////////////////////////////////////


#include "HexSpinBox.h"

HexSpinBox::HexSpinBox(QWidget *parent) :
    QSpinBox(parent),
    validator(NULL),
    displayAsHex(true) {

    // Because the QSpinBox operates on signed integers, we have to use
    // the range -2147483648 to 2147483647 and wrap it so that
    // 0x7FFFFFFF goes to 0x80000000 instead of being the start and end
    setMinimum(-2147483648);
    setMaximum(2147483647);
    setWrapping(true);

    const QRegExp hexDigit("\\s*(0x)?[0-9A-Fa-f]+\\s*");
    validator = new QRegExpValidator(hexDigit, this);
}

HexSpinBox::~HexSpinBox() {
    if (validator)
        delete validator;
}

QValidator::State HexSpinBox::validate(String& text, int& pos) const {
    QValidator::State result = validator->validate(text, pos);

    if (result == QValidator::Acceptable) {
        String valString = text.trimmed();
        bool ok, isHex = isHexString(valString);
        valString.toUInt(&ok, isHex ? 16 : 10);
        if (!ok) {
            // Parsing the string fails, probably because the
            // number is too big to fit in an unsigned int
            result = QValidator::Invalid;
        }
    }
    return result;
}

/*------------------------------------------------------------------+
 | Parses the string which is a number in either hex (base 16) or   |
 | decimal (base 10). The string is deemed to be a hex number if it |
 | starts with "0x" or contains the letters A-F.                    |
 +------------------------------------------------------------------*/
bool HexSpinBox::isHexString(const String& text) const {
    static char hexLetters[] = {'A','B','C','D','E','F',
                                'a','b','c','d','e','f'};
    if (text.startsWith("0x")) {
        return true;
    }
    else {
        for (int i = 0; i < sizeof(hexLetters)/sizeof(char); i++) {
            if (text.contains(hexLetters[i])) {
                return true;
            }
        }
    }
    return false;
}

int HexSpinBox::valueFromText(const String& text) const {
    String valString = text.trimmed();
    bool ok, isHex = isHexString(valString);
    return (int)valString.toUInt(&ok, isHex ? 16 : 10);
}

String HexSpinBox::textFromValue(int value) const {
    if (displayAsHex)
        return hexString((uint)(value));
    else
        return String::number((uint)(value));
}