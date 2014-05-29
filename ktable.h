#ifndef KTABLE_H
#define KTABLE_H

#include <list>
#include <QMutex>
#include <stdint.h>
#include <QByteArray>
namespace czDHT {

class Knode{
public:
    QByteArray nid;
    uint32_t ip;
    uint16_t TcpPort;
    uint16_t UdpPort;

    Knode(QByteArray nid,uint32_t ip,uint16_t UdpPort, uint16_t TcpPort=0){
        this->nid = nid;
        this->ip = ip;
        this->TcpPort = TcpPort;
        this->UdpPort = UdpPort;
    }
    ~Knode(){
        nid.clear();
    }
};

#pragma pack(1)
typedef struct {
    uint8_t nid[20];
    uint32_t ip;
    uint16_t port;
}Tnode;
#pragma pack()
#define TNODE_SIZE sizeof(Tnode)

class kTable
{
public:
    kTable(int maxSize);
    void putANode(Knode* node);
    Knode* getANode();
    bool isEmpty();
    void clear();
    QByteArray getTableID()const{return m_MyId;}

private:
    int m_imax_size;
    QByteArray m_MyId;
    std::list<Knode*> m_pnodeList;
    QMutex list_mutex;
};

}
#endif // KTABLE_H
