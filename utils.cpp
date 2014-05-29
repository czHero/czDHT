#include "utils.h"
#include <stdlib.h>
#include <QByteArray>
#include <QCryptographicHash>
#include <ctime>

Utils::Utils()
{
}
QByteArray Utils::randByteArray(int lenArray)
{
    QByteArray ret="";
    srand(unsigned(time(0)));
    for(int i=0; i < lenArray;i++){
        int temp = rand()%255;
        ret.append((unsigned char)temp);
    }
    return ret;
}

QByteArray Utils::randID(){
    QByteArray ret;
    QCryptographicHash *hash=new QCryptographicHash(QCryptographicHash::Sha1);
    QByteArray in = randByteArray(20);
    hash->addData(in);
    ret = hash->result();
    delete hash;
    return ret;
}
