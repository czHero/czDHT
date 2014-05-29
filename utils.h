#ifndef UTILS_H
#define UTILS_H
class QByteArray;
class Utils
{
public:
    Utils();
    QByteArray static randByteArray(int lenArray);
    QByteArray static randID();
};

#endif // UTILS_H
