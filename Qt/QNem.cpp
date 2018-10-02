//
// Created by Taylor Whatley on 2018-09-28.
//

#include "QNem.h"

QNem::QNem(int& count, char** args): QApplication(count, args) {
    setApplicationName("Nemulator");

    if (count <= 1) throw Nem::CouldNotCreateWindowException();

    window = new QNemWindow(string(args[1]));
}
QNem::~QNem() {
    delete window;
}