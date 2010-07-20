/////////////////////////////////////////////////////////////////////
// File: IniFile.cpp                                               //
// Date: 5/7/10                                                    //
// Desc: Provides functionality to read INI files. The INI files   //
//   used by this application are composed of group headers and    //
//   lines of comma-separated values. Groups are specified using   //
//   square brackets in the same way as ordinary ini files.        //
/////////////////////////////////////////////////////////////////////

/********************************************************************
  Window Detective
  Copyright (C) 2010 XTAL256

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
********************************************************************/

#include "IniFile.h"
#include "Logger.h"

/*------------------------------------------------------------------+
| Constructor.                                                      |
| Reads each line of the given file into a string list. Blank lines |
| or comments (lines starting with "//") will be stripped.          |
+------------------------------------------------------------------*/
IniFile::IniFile(String fileName) :
    currentLine(0), lines(), groupName() {
    QFile file(fileName);
    if (file.exists()) {
        if (file.open(QFile::ReadOnly)) {
            QTextStream stream(&file);
            String line;
            while (!stream.atEnd()) {
                line = stream.readLine().trimmed();
                if (line.isEmpty() || line.startsWith("//"))
                    continue;
                lines << line;
            }

            // The first line may be the start of a group.
            if (!lines.isEmpty()) {
                String firstLine = lines.first();
                if (firstLine.startsWith('[')) {
                    groupName = firstLine.mid(1, firstLine.size()-2);
                    currentLine++;
                }
            }
        }
        else {
            Logger::warning(TR("Could not read data file: ")+fileName);
        }
    }
    else {
        Logger::warning(TR("Data file does not exist: ")+fileName);
    }
}

/*------------------------------------------------------------------+
| Finds the first group with the given name, sets it as the current |
| group and sets the current line to be after it.                   |
| Returns false if no group is found, else returns true.            |
+------------------------------------------------------------------*/
bool IniFile::selectGroup(String name) {
    int index;
    for (index = 0; index < lines.size(); index++) {
        if (lines[index] == "["+name+"]") {
            groupName = name;
            currentLine = index + 1;
            return true;
        }
    }
    return false;
}

/*------------------------------------------------------------------+
| Sets the current line to the next one. If the next line is a new  |
| group, it also sets the current group.                            |
+------------------------------------------------------------------*/
void IniFile::selectNextEntry() {
    currentLine++;
    if (isAtEnd()) return;
    String line = lines[currentLine];
    if (line.startsWith('[')) {
        groupName = line.mid(1, line.size()-2);
        currentLine++;
    }
}

/*------------------------------------------------------------------+
| Sets the current line to the next one as long as it is within the |
| same group.                                                       |
+------------------------------------------------------------------*/
void IniFile::selectNextInGroup() {
    if (!isEndOfGroup())
        currentLine++;
}

/*------------------------------------------------------------------+
| Checks if the current line is at the end of the list.             |
+------------------------------------------------------------------*/
bool IniFile::isAtEnd() {
    return currentLine >= lines.size();
}

/*------------------------------------------------------------------+
| Checks if the current line is at the end of the current group.    |
| Note that since groups do not have an end marker, this simply     |
| checks if the current line is the *start* of a group.             
+------------------------------------------------------------------*/
bool IniFile::isEndOfGroup() {
    if (isAtEnd()) return true;
    return lines[currentLine].startsWith('[');
}

/*------------------------------------------------------------------+
| Reads and parses the current line. The line should be a           |
| comma-separated list of values.                                   |
| TODO: Optimise by writing my own parser instead of using regex    |
+------------------------------------------------------------------*/
QStringList IniFile::readLine() {
    QStringList values;
    String str;

    if (isAtEnd()) return QStringList();
    String line = lines[currentLine];
    QRegExp rx("\\s*((?:\"[^\"]*\")|(?:\\w|\\d)+)\\s*(,|$)");
    int pos = 0;
    while ((pos = rx.indexIn(line, pos)) != -1) {
        str = rx.cap(1);
        if (str.startsWith('\"') && str.endsWith('\"')) {
            str = str.mid(1, str.size()-2);   // Strip quotes
            str = str.replace("\\n", "\n");   // Replace any \n with newline
        }
        values << str;
        pos += rx.matchedLength();
    }

    return values;
}