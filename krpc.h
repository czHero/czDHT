#ifndef KRPC_H
#define KRPC_H

#include <QThread>
#include <QTimerEvent>
#include <QtNetwork/QUdpSocket>
#include <QtNetwork/QHostAddress>
#include <stdint.h>
#include <vector>


#ifndef Dictionary
typedef QMap<QByteArray,QVariant> Dictionary;
#endif

#define TID_LENGTH  4
#define KRPC_TIMEOUT 10

namespace czDHT {


class BencodeParser;
class Knode;
class kTable;
/**
 * @brief The KRPC class
 */
class KRPC : public QThread
{
    Q_OBJECT
public:
    explicit KRPC(uint32_t ip, uint16_t port,QObject *parent = 0);
    explicit KRPC(QString ip, uint16_t port,QObject *parent = 0);
    ~KRPC();

signals:

public slots:
    void Recv();                //接收网络数据流后的处理
protected:
    bool join_success;          //成功加入标志
    QUdpSocket  *udpSocket;     //套接字对象
    uint32_t m_ip;              //ip地址
    QString m_strip;            //ip地址
    uint16_t m_port;            //端口号
    QByteArray get_neighbor(QByteArray &id);
    void send_krpc(Dictionary &msg ,QHostAddress addr, uint16_t port);
    virtual void processAnode(Knode* node)=0;
    virtual void get_peers_received(Dictionary &msg ,QHostAddress addr, uint16_t port)=0;

private:

    BencodeParser *bencode;     //bencode编解码器

    void response_received(Dictionary &msg ,QHostAddress addr, uint16_t port);
    void query_received(Dictionary &msg ,QHostAddress addr, uint16_t port);

};
/**
 * @brief The KrpcClient class
 */
class KrpcClient:public KRPC
{
    Q_OBJECT
private:
    std::vector<QString> org_addrs;
    std::vector<uint16_t> org_ports;
    kTable *table;
    bool isWorking;

    void findNode(QHostAddress addr, uint16_t port, QByteArray nid=NULL);
    virtual void processAnode(Knode* node);
//    virtual void get_peers_received(Dictionary &msg ,QHostAddress addr, uint16_t port);
public:
    KrpcClient(kTable *table, uint32_t ip, uint16_t port,int timerDelay, QObject *parent = 0);
    KrpcClient(kTable *table, QString ip, uint16_t port,int timerDelay, QObject *parent = 0);
    ~KrpcClient();
    void initOrgAddr();
    void joinDHT();
    void stopWork();

protected:
    void run();
    void timerEvent( QTimerEvent *event);
    int m_timerID;
};
/**
 * @brief The KrpcServer class
 */
class KrpcServer:public KrpcClient
{
    Q_OBJECT
public:
    KrpcServer(kTable *table, uint32_t ip, uint16_t port,int timerDelay, QString logFileName, QObject *parent = 0);
    KrpcServer(kTable *table, QString ip, uint16_t port,int timerDelay, QString logFileName, QObject *parent = 0);

signals:
    void updata();
protected:
    virtual void get_peers_received(Dictionary &msg ,QHostAddress addr, uint16_t port);

private:
    QString m_LogFileName;

};
}
#endif // KRPC_H
