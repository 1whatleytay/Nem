#include "QNem.h"

#include <iostream>

int main(int count, char** args) {
    QNem app(count, args);

    return app.exec();
}