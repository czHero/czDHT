#include "ktable.h"
#include "utils.h"

using namespace czDHT;

kTable::kTable(int maxSize)
{
    m_imax_size = maxSize;
    m_MyId = Utils::randID();
    m_pnodeList.clear();
}

void kTable::putANode(Knode* node)
{
    if(m_pnodeList.size()>m_imax_size)
        return;
    list_mutex.lock();
    m_pnodeList.push_back(node);
    list_mutex.unlock();
}

Knode* kTable::getANode(){
    Knode* ret;
    list_mutex.lock();
    ret = m_pnodeList.front();
    m_pnodeList.pop_front();
    list_mutex.unlock();
    return ret;
}
bool kTable::isEmpty(){
    bool ret;
    list_mutex.lock();
    ret = m_pnodeList.empty();
    list_mutex.unlock();
    return ret;
}

void kTable::clear(){
    list_mutex.lock();
    for(std::list<Knode*>::iterator it = m_pnodeList.begin();
        it!=m_pnodeList.end();it++)
        delete *it;
    m_pnodeList.clear();
    list_mutex.unlock();
}
