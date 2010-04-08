///////////////////////////////////////////////////////////////////////////////
////////                                                                     //
//                                                                           //
//    W I N D O W                                                            //
//                                                                           //
//            D E T E C T I V E                                              //
//                                                                           //
//                                                                     ////////
///////////////////////////////////////////////////////////////////////////////

#ifndef MAIN_H
#define MAIN_H

#include "include.h"
#include "inspector/inspector.h"

// Unique ID for this app. Used to ensure only one instance
#define APP_GUID   "{7f44ce61-463f-47cf-b282-d66e9fea2e9c}"

void loadPickerCursor();
void showPickerCursor();
void restoreCursor();
void setAppStyle(String name);

#endif  // MAIN_H
