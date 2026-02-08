
#ifndef DISPLAY_HPP_
#define DISPLAY_HPP_

#include <QApplication>
#include <QLabel>

class AardvarkDisplay
{
    public:
    AardvarkDisplay(int &lArgc, char *lArgv[]);
    ~AardvarkDisplay();
    inline void Run(void) { if (mApp) mApp->exec(); };
    private:
    QApplication *mApp;
    QLabel *mLabel;
};

#endif // DISPLAY_HPP_