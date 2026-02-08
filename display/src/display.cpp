
#include "display.hpp"

AardvarkDisplay::AardvarkDisplay(int &lArgc, char *lArgv[])
{
    mApp = new QApplication(lArgc, lArgv);
    if (!mApp)
    {
        exit(EXIT_FAILURE);
    }

    mLabel = new QLabel("Hello, world!");
    if (!mLabel)
    {
        exit(EXIT_FAILURE);
    }

    mLabel->resize(300, 100);
    mLabel->setAlignment(Qt::AlignCenter);

    mLabel->show();
}

AardvarkDisplay::~AardvarkDisplay()
{
    if (mLabel)
    {
        delete mLabel;
    }

    if (mApp)
    {
        mApp->exit(0);
        delete mApp;
    }
}
