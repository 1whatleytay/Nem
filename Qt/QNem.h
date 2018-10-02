//
// Created by Taylor Whatley on 2018-09-28.
//

#ifndef NEM_QNEM_H
#define NEM_QNEM_H

#include <QApplication>

#include "../Emulator.h"
#include "QNemWindow.h"

class QNem: public QApplication {
    Q_OBJECT

    QNemWindow* window = nullptr;
public:
    QNem(int& count, char** args);
    ~QNem();
};

#endif //NEM_QNEM_H
