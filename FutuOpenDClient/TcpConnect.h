#ifndef TcpConnect_hpp
#define TcpConnect_hpp

#include <stdio.h>
#include <uv.h>
#include <vector>

namespace ftq {
    
    class Buffer {
    public:
        Buffer(int nBufLen);
        ~Buffer();
        int GetTotalLen() { return nBufLen; }
        char *GetData() { return pBuf; }
        int GetDataLen() { return nDataLen; }
        void SetDataLen(int nLen);
        int GetRemainLen();
        char *GetWriteStart() { return pBuf + nDataLen; }
        bool Resize(int nNewLen);
        void RemoveFront(int nLen);
    private:
        int nDataLen{0};
        int nBufLen{0};
        char *pBuf{nullptr};
    };
    
    class TcpConnect;
    
    class ITcpHandler {
    public:
        virtual void OnConnect(TcpConnect *pConn) = 0;
        virtual void OnRecv(TcpConnect *pConn, Buffer *pBuf) = 0;
        virtual void OnError(TcpConnect *pConn, int nUvErr) = 0;
        virtual void OnDisconnect(TcpConnect *pConn) = 0;
    };
    
    class TcpConnect {
    public:
		~TcpConnect();
        bool Init(uv_loop_t *pUvLoop, ITcpHandler *pHandler);
        void Close();
        bool Connect(const char *pHost, int nPort);
        bool Send(const char *pData, int nLen);
		const char* GetHost();
		int GetPort();
		void SetSocket(const char* pHost,int nPort);
    public:
        static void AfterConnect(uv_connect_t *pReq, int nStatus);
        static void AfterClose(uv_handle_t *pHandle);
        static void AfterRead(uv_stream_t* stream,
                              ssize_t nread,
                              const uv_buf_t* buf);
        static void AfterWrite(uv_write_t* req, int status);
        static void OnAllocBuf(uv_handle_t *handle, size_t suggested_size, uv_buf_t *buf);
    private:
        uv_loop_t *m_pUvLoop{nullptr};
        uv_tcp_t m_tcp{};
        uv_connect_t m_connect{};
        uv_write_t m_write{};
        uv_buf_t m_readBuf{};
        uv_buf_t m_writeBuf{};
        std::vector<char> m_writeStores[2];
        int m_nCurUsingWriteStoreIdx{-1};
        Buffer m_readStore{10 * 1024 * 1024};
        ITcpHandler *m_pHandler{nullptr};
		const char* m_pHost;
		int m_nPort;
    };
}

#endif /* TcpConnect_hpp */
