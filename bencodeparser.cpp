/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include "bencodeparser.h"

#include <QList>
#include <QMetaType>
#include <QMap>
#include<QDebug>


using namespace czDHT;

const static QChar intChar = QChar('i');
const static QChar listChar = QChar('l');
const static QChar dictChar = QChar('d');
const static QChar strDelimChar = QChar(':');
const static QChar endChar = QChar('e');
const static QChar negChar = QChar('-');

BencodeParser::BencodeParser()
{

}

bool BencodeParser::compile(const QMap<QByteArray, QVariant> &dictContent){
    if(dictContent.isEmpty()){
        errString = QString("No dictionary content");
        return false;
    }
    this->dictContent=dictContent;
    compiledValue.clear();
    QVariant tvariant;
    tvariant.setValue(dictContent);
    return putVariant(tvariant);
}

bool BencodeParser::parse(const QByteArray &content)
{
    if (content.isEmpty()) {
        errString = QString("No content");
        return false;
    }

    this->content = content;
    index = 0;
    infoStart = 0;
    infoLength = 0;
    return getDictionary(&dictionaryValue);
}

QString BencodeParser::errorString() const
{
    return errString;
}

QMap<QByteArray, QVariant> BencodeParser::dictionary() const
{
    return dictionaryValue;
}
QByteArray BencodeParser::getcompiled() const{
    return compiledValue;
}
QByteArray BencodeParser::infoSection() const
{
    return content.mid(infoStart, infoLength);
}

bool BencodeParser::getByteString(QByteArray *byteString)
{
    const int contentSize = content.size();
    int size = -1;
    do {
        char c = content.at(index);
        if (c < '0' || c > '9') {
            if (size == -1)
                return false;
            if (c != ':') {
                errString = QString("Unexpected character at pos %1: %2")
                        .arg(index).arg(c);
                return false;
            }
            ++index;
            break;
        }
        if (size == -1)
            size = 0;
        size *= 10;
        size += c - '0';
    } while (++index < contentSize);

    if (byteString)
        *byteString = content.mid(index, size);
    index += size;
    return true;
}

bool BencodeParser::getInteger(qint64 *integer)
{
    const int contentSize = content.size();
    if (content.at(index) != 'i')
        return false;

    ++index;
    qint64 num = -1;
    bool negative = false;

    do {
        char c = content.at(index);
        if (c < '0' || c > '9') {
            if (num == -1) {
                if (c != '-' || negative)
                    return false;
                negative = true;
                continue;
            } else {
                if (c != 'e') {
                    errString = QString("Unexpected character at pos %1: %2")
                            .arg(index).arg(c);
                    return false;
                }
                ++index;
                break;
            }
        }
        if (num == -1)
            num = 0;
        num *= 10;
        num += c - '0';
    } while (++index < contentSize);

    if (integer)
        *integer = negative ? -num : num;
    return true;
}

bool BencodeParser::getList(QList<QVariant> *list)
{
    const int contentSize = content.size();
    if (content.at(index) != 'l')
        return false;

    QList<QVariant> tmp;
    ++index;

    do {
        if (content.at(index) == 'e') {
            ++index;
            break;
        }

        qint64 number;
        QByteArray byteString;
        QList<QVariant> tmpList;
        QMap<QByteArray, QVariant> dictionary;

        if (getInteger(&number))
            tmp << number;
        else if (getByteString(&byteString))
            tmp << byteString;
        else if (getList(&tmpList))
            tmp << tmpList;
        else if (getDictionary(&dictionary))
            tmp << QVariant::fromValue<QMap<QByteArray, QVariant> >(dictionary);
        else {
            errString = QString("error at index %1").arg(index);
            return false;
        }
    } while (index < contentSize);

    if (list)
        *list = tmp;
    return true;
}

bool BencodeParser::getDictionary(QMap<QByteArray, QVariant> *dictionary)
{
    const int contentSize = content.size();
    if (content.at(index) != 'd')
        return false;

    QMap<QByteArray, QVariant> tmp;
    ++index;

    do {
        if (content.at(index) == 'e') {
            ++index;
            break;
        }

        QByteArray key;
        if (!getByteString(&key))
            break;

        if (key == "info")
            infoStart = index;

        qint64 number;
        QByteArray byteString;
        QList<QVariant> tmpList;
        QMap<QByteArray, QVariant> dictionary;

        if (getInteger(&number))
            tmp.insert(key, number);
        else if (getByteString(&byteString))
            tmp.insert(key, byteString);
        else if (getList(&tmpList))
            tmp.insert(key, tmpList);
        else if (getDictionary(&dictionary))
            tmp.insert(key, QVariant::fromValue<QMap<QByteArray, QVariant> >(dictionary));
        else {
            errString = QString("error at index %1").arg(index);
            return false;
        }

        if (key == "info")
            infoLength = index - infoStart;

    } while (index < contentSize);

    if (dictionary)
        *dictionary = tmp;
    return true;
}

bool BencodeParser::putVariant(QVariant &variant){

    switch(variant.type()){
    case QMetaType::Int:{        
        compiledValue.append(QByteArray("i")+QByteArray().setNum(variant.toInt()));
        return true;
    }
    case QMetaType::LongLong:{
        compiledValue.append(QByteArray("i")+QByteArray().setNum(variant.toLongLong()));
        return true;
    }
    case QMetaType::QByteArray:{
        compiledValue.append(QByteArray().setNum(variant.toByteArray().length())+QByteArray(":"));
        compiledValue.append(variant.toByteArray());
        return true;
    }
    case QMetaType::QString:{
        compiledValue.append(QByteArray().setNum(variant.toString().toLocal8Bit().length())+QByteArray(":"));
        compiledValue.append(variant.toString().toLocal8Bit());
        return true;
    }
    case QMetaType::QVariantList:{
        compiledValue.append("l");
        QList<QVariant> tlist = variant.toList();
        QList<QVariant>::iterator it;
        for(it=tlist.begin();it!=tlist.end();it++) {
            if(!putVariant(*it))
                return false;
        }
        compiledValue.append("e");
        return true;
    }
    case QMetaType::QVariantMap:{
        compiledValue.append("d");
        QMap<QString,QVariant> tmap = variant.toMap();
        QMap<QString,QVariant>::iterator it;
        for(it=tmap.begin();it!=tmap.end();it++){
            compiledValue.append(QByteArray().setNum(it.key().toLocal8Bit().length())+QByteArray(":"));
            compiledValue.append(it.key().toLocal8Bit());
            if(!putVariant(*it))
                return false;
        }

        compiledValue.append("e");
        return true;
    }
    default:{
        if(variant.type()== QVariant::nameToType("Dictionary")
                &&variant.canConvert<Dictionary>()){// Dictionary
            Dictionary tdict = qvariant_cast<Dictionary>(variant);
            compiledValue.append("d");
            Dictionary::iterator it;
            for(it=tdict.begin();it!=tdict.end();it++){
                compiledValue.append(QByteArray().setNum(it.key().length())+QByteArray(":"));
                compiledValue.append(it.key());
                if(!putVariant(*it))
                    return false;
            }
            compiledValue.append("e");
            return true;
        }else{
            errString.append(QString("unknow type,type:%d") );
            return false;
        }
    }
    }
}

