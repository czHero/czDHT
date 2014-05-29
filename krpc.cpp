#include "krpc.h"
#include "bencodeparser.h"
#include "utils.h"
#include "ktable.h"
#include <stdio.h>
#include <QDataStream>
#include <QtEndian>
#include <QFile>
#include <QIODevice>

using namespace czDHT;

KRPC::KRPC(uint32_t ip, uint16_t port, QObject *parent) :
    QThread(parent),m_ip(ip),m_port(port)
{
    bencode = new BencodeParser();
    join_success = false;
    udpSocket = new QUdpSocket();
    udpSocket->bind(port);
    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(Recv()));
}

KRPC::KRPC(QString ip, uint16_t port, QObject *parent) :
    QThread(parent),m_strip(ip),m_port(port)
{
    bencode = new BencodeParser();
    join_success = false;
    udpSocket = new QUdpSocket();
    udpSocket->bind(port);
    connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(Recv()));
}

KRPC::~KRPC(){
    delete bencode;
    delete udpSocket;
}

QByteArray KRPC::get_neighbor(QByteArray &id){
    QByteArray result = id.left(10);
    result.append(Utils::randID().right(10));
    return result;
}

void KRPC::Recv()
{
    while (udpSocket->hasPendingDatagrams())
    {
        QByteArray datagram;
        QHostAddress from_ip;
        uint16_t from_prot;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size(),
                                &from_ip,&from_prot);

        if(bencode->parse(datagram)){
            Dictionary result = bencode->dictionary();
            if(result["y"]=="q"){
                query_received(result, from_ip, from_prot);
            }else if(result["y"]=="r"){
                response_received(result, from_ip, from_prot);
            }
        }
    }
}
void KRPC::response_received(Dictionary &msg ,QHostAddress addr, uint16_t port){
    //    qDebug()<<"response_received";
    join_success=true;
    Tnode node;
    if(msg["r"].canConvert<Dictionary>()){
        Dictionary Dres = qvariant_cast<Dictionary>(msg["r"]);
        if(Dres["nodes"].canConvert<QByteArray>()){
            QByteArray ByteNodes = qvariant_cast<QByteArray>(Dres["nodes"]);
            QDataStream  os(&ByteNodes, QIODevice::ReadOnly);
            while(!os.atEnd()){
                if(TNODE_SIZE == (os.readRawData((char*)&node, TNODE_SIZE))){
                    Knode *knode = new Knode(QByteArray((const char*)(node.nid),20),node.ip,node.port);
                    processAnode(knode);
                }else
                    break;
            }
        }
    }
}

void KRPC::query_received(Dictionary &msg ,QHostAddress addr, uint16_t port){
//    qDebug()<<msg["q"].toByteArray();
    if(msg["q"].toByteArray()=="get_peers"){
        qDebug()<<msg;
        if(msg["a"].canConvert<Dictionary>()){
            Dictionary dmsg = qvariant_cast<Dictionary>(msg["a"]);
            get_peers_received(dmsg,addr,port);
        }
    }
}

void KRPC::send_krpc(Dictionary &msg ,QHostAddress addr, uint16_t port){
    if(bencode->compile(msg)){
        QByteArray bencodeMsg = bencode->getcompiled();
//        qDebug()<<bencodeMsg;
        udpSocket->writeDatagram(bencodeMsg,addr,port);
    }
}

////////////////////////////////////////////////////////////////////////////
///KrpcClient  Part
////////////////////////////////////////////////////////////////////////////
KrpcClient::KrpcClient(kTable *table, uint32_t ip, uint16_t port, int timerDelay, QObject *parent)
        :KRPC(ip, port, parent)
{
    this->table=table;
    m_timerID = startTimer(timerDelay);
    isWorking= true;
}

KrpcClient::KrpcClient(kTable *table, QString ip, uint16_t port, int timerDelay, QObject *parent)
        :KRPC(ip, port, parent)
{
    this->table=table;
    m_timerID = startTimer(timerDelay);
    isWorking=true;
}
KrpcClient::~KrpcClient(){
    if ( m_timerID != 0 )
        killTimer(m_timerID);
}

void KrpcClient::initOrgAddr(){
    org_addrs.push_back("67.215.242.138");
    org_ports.push_back(6881);
    org_addrs.push_back("88.190.242.141");
    org_ports.push_back(6881);
    org_addrs.push_back("67.215.242.139");
    org_ports.push_back(6881);
}

void KrpcClient::findNode(QHostAddress addr, uint16_t port, QByteArray nid)
{
    QByteArray Nid = nid!=NULL?(get_neighbor(nid)):(table->getTableID());
    QByteArray Tid = Utils::randByteArray(TID_LENGTH);
    Dictionary toSend,cmsg;
    cmsg["id"]=Nid;
    cmsg["target"]=Utils::randID();
    toSend["t"]=Tid;
    toSend["y"]="q";
    toSend["q"]=QByteArray("find_node");
    toSend["a"]=QVariant::fromValue<QMap<QByteArray, QVariant> >(cmsg);
    send_krpc(toSend,addr,port);
}

void KrpcClient::processAnode(Knode* node){
    table->putANode(node);
}

//void KrpcClient::get_peers_received(Dictionary &msg ,QHostAddress addr, uint16_t port){
//    //TODO
//}


void KrpcClient::joinDHT(){
    for(int i=0; i<(int)org_addrs.size(); i++){
//        qDebug()<<org_addrs[i]<<" "<<org_ports[i];
        findNode(QHostAddress(org_addrs[i]), org_ports[i]);
    }
}

void KrpcClient::stopWork(){
    isWorking = false;
}

void KrpcClient::run(){
    initOrgAddr();
    joinDHT();
    while(isWorking){
        if(table->isEmpty()){
            join_success=false;
            usleep(1000);
            continue;
        }
        while(!table->isEmpty()){
            Knode* node = table->getANode();
            node->ip = qToBigEndian(node->ip);
            node->UdpPort = qToBigEndian(node->UdpPort);
            findNode(QHostAddress(node->ip), node->UdpPort,node->nid);
            delete node;
        }
    }
}
void KrpcClient::timerEvent( QTimerEvent *event ){
    if(m_timerID==event->timerId()){
        if(!join_success){
            joinDHT();
        }
    }
}
////////////////////////////////////////////////////////////////////////////
///KrpcServer  Part
////////////////////////////////////////////////////////////////////////////
KrpcServer::KrpcServer(kTable *table, uint32_t ip, uint16_t port,int timerDelay, QString logFileName,QObject *parent)
    :KrpcClient(table,ip,port,timerDelay,parent)
{
    m_LogFileName = logFileName;
}

KrpcServer::KrpcServer(kTable *table, QString ip, uint16_t port,int timerDelay, QString logFileName, QObject *parent)
    :KrpcClient(table,ip,port,timerDelay,parent)
{
    m_LogFileName = logFileName;
}

void KrpcServer::get_peers_received(Dictionary &msg ,QHostAddress addr, uint16_t port){
    if(!m_LogFileName.isEmpty())
    {
        QFile myFile(m_LogFileName);
        myFile.open(QIODevice::WriteOnly|QIODevice::Append);
        QTextStream myLog(&myFile);
        myLog << (msg["info_hash"].toByteArray()) << endl;
        updata();
    }
}
