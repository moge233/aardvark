
#ifndef DISPLAY_HPP_
#define DISPLAY_HPP_

#include <iostream>

#include <QtCore/QCoreApplication>

class AardvarkDisplay
{
    public:
    AardvarkDisplay(int &lArgc, char *lArgv[]);
    ~AardvarkDisplay();
    inline void Run(void) { std::cout << "Hello!!" << std::endl; mApp.exec(); }
    private:
    QCoreApplication mApp;
};

#endif // DISPLAY_HPP_