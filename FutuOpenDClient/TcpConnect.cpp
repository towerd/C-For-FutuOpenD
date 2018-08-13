#include "TcpConnect.h"
#include <iostream>
#include <string>
#include <cassert>
#include <string.h>

using namespace std;

namespace ftq {
    
    Buffer::Buffer(int nBufLen) {
        assert(nBufLen > 0);
        
        this->nBufLen = nBufLen;
        this->pBuf = (char *)malloc(sizeof(char) * nBufLen);
    }
    
    Buffer::~Buffer() {
        free(pBuf);
        pBuf = nullptr;
        nDataLen = 0;
        nBufLen = 0;
    }
    
    int Buffer::GetRemainLen() {
        return nBufLen - nDataLen;
    }
    
    bool Buffer::Resize(int nNewLen) {
        assert(nNewLen > 0);
        
        if (nNewLen == nBufLen) {
            return true;
        }

        char *pNewBuf = (char*)realloc(pBuf, nNewLen);
        if (!pNewBuf) {
            return false;
        }
        pBuf = pNewBuf;
        nBufLen = nNewLen;
        return true;
    }
    
    void Buffer::SetDataLen(int nLen) {
        assert(nLen <= nBufLen);
        nDataLen = nLen;
    }
    
    void Buffer::RemoveFront(int nLen) {
        assert(nLen <= nDataLen);
        
        int nRemainDataLen = nDataLen - nLen;
        if (nRemainDataLen > 0) {
            memmove(pBuf, pBuf + nLen, nRemainDataLen);
        }
        nDataLen = nRemainDataLen;
    }
    
	TcpConnect::~TcpConnect(){
		uv_close((uv_handle_t*)&m_tcp, AfterClose);
	}

    bool TcpConnect::Init(uv_loop_t *pUvLoop, ITcpHandler *pHandler) {
        assert(pUvLoop);
        assert(pHandler);
        
        m_pUvLoop = pUvLoop;
        int nRet = uv_tcp_init(pUvLoop, &m_tcp);
        if (nRet != 0) {
            return false;
        }
        
        m_tcp.data = this;
        m_connect.data = this;
        m_write.data = this;
        m_pHandler = pHandler;
        return true;
    }
    
    void TcpConnect::Close() {
        uv_close((uv_handle_t*)&m_tcp, AfterClose);
    }
    
    bool TcpConnect::Connect(const char *pHost, int nPort) {
        struct sockaddr_in addr_in;
        int nRet = uv_ip4_addr(pHost, nPort, &addr_in);
        if (nRet != 0) {
            return false;
        }
        
        nRet = uv_tcp_connect(&m_connect, &m_tcp, (const struct sockaddr*)&addr_in, AfterConnect);
        if (nRet != 0) {
            return false;
        }
        return true;
    }
    
    void TcpConnect::AfterClose(uv_handle_t *pHandle) {
        //todo
    }
    
    
    void TcpConnect::AfterConnect(uv_connect_t *pReq, int nStatus) {
        TcpConnect *pSelf = (TcpConnect*)pReq->data;
        
        if (nStatus != 0) {
            pSelf->m_pHandler->OnError(pSelf, nStatus);
            return;
        }
        
        int nRet = uv_read_start(pReq->handle, OnAllocBuf, AfterRead);
        if (nRet != 0) {
            pSelf->m_pHandler->OnError(pSelf, nStatus);
            return;
        }
        
        pSelf->m_pHandler->OnConnect(pSelf);
    }
    
    void TcpConnect::AfterRead(uv_stream_t *stream, ssize_t nread, const uv_buf_t *buf) {
        TcpConnect *pSelf = (TcpConnect*)stream->data;
        
        if (nread < 0) {
            if (nread == UV_EOF) {
                cout << "remote closed" << endl;
                pSelf->m_pHandler->OnDisconnect(pSelf);
            }
            else if (nread == UV_ECONNRESET) {
                cout << "conn reset" << endl;
                pSelf->m_pHandler->OnDisconnect(pSelf);
            }
            else {
                cout << "error: " << uv_strerror((int)nread) << endl;
                pSelf->m_pHandler->OnError(pSelf, (int)nread);
            }
            return;
        }
        
        pSelf->m_readStore.SetDataLen((int)nread + pSelf->m_readStore.GetDataLen());
        pSelf->m_pHandler->OnRecv(pSelf, &pSelf->m_readStore);
//        string str;
//        str.insert(str.end(),
//                   pSelf->m_readStore.GetData(),
//                   pSelf->m_readStore.GetData() + pSelf->m_readStore.GetDataLen());
//        pSelf->m_readStore.RemoveFront(pSelf->m_readStore.GetDataLen());
//        str.append("\0");
//        std::cout << "recv: " << str << endl;
    }
    
    void TcpConnect::AfterWrite(uv_write_t *req, int status) {
        int nErr = status;
        TcpConnect *pSelf = (TcpConnect*)req->data;
        pSelf->m_writeStores[pSelf->m_nCurUsingWriteStoreIdx].clear();
        int nOtherWriteStoreIdx = pSelf->m_nCurUsingWriteStoreIdx == 0 ? 1 : 0;
        pSelf->m_nCurUsingWriteStoreIdx = -1;
        
        if (status == 0) {
            std::vector<char> *pOtherWriteStore = &pSelf->m_writeStores[nOtherWriteStoreIdx];
            if (pOtherWriteStore->size() > 0) {
                pSelf->m_nCurUsingWriteStoreIdx = nOtherWriteStoreIdx;
                pSelf->m_writeBuf = uv_buf_init(pOtherWriteStore->data(), (int)pOtherWriteStore->size());
                int nRet = uv_write(&pSelf->m_write, (uv_stream_t *)&pSelf->m_tcp, &pSelf->m_writeBuf, 1, AfterWrite);
                if (nRet != 0) {
                    nErr = nRet;
                }
            }
        }
        
        if (nErr != 0) {
            pSelf->m_pHandler->OnError(pSelf, nErr);
        }
    }
    
    void TcpConnect::OnAllocBuf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf) {
        TcpConnect *pSelf = (TcpConnect*)handle->data;
        buf->base = pSelf->m_readStore.GetWriteStart();
        buf->len = pSelf->m_readStore.GetRemainLen();
    }
    
    bool TcpConnect::Send(const char *pData, int nLen) {
        std::vector<char> *pWriteStore = nullptr;
        if (m_nCurUsingWriteStoreIdx < 0) {
            pWriteStore = &m_writeStores[0];
            m_writeStores[0].insert(m_writeStores[0].end(), pData, pData + nLen);
            m_writeBuf = uv_buf_init(m_writeStores[0].data(), (int)m_writeStores[0].size());
            m_nCurUsingWriteStoreIdx = 0;
            
            int nRet = uv_write(&m_write, (uv_stream_t *)&m_tcp, &m_writeBuf, 1, AfterWrite);
            if (nRet != 0) {
                return false;
            }
        }
        else {
            int nWriteStoreIdx = m_nCurUsingWriteStoreIdx == 0 ? 1 : 0;
            pWriteStore = &m_writeStores[nWriteStoreIdx];
            pWriteStore->insert(pWriteStore->end(), pData, pData + nLen);
        }
        
        return true;
    }

	const char* TcpConnect::GetHost(){
		return m_pHost;
	}

	int TcpConnect::GetPort(){
		return m_nPort;
	}

	void TcpConnect::SetSocket(const char* pHost, int nPort){
		m_pHost = pHost;
		m_nPort = nPort;
	}
}
