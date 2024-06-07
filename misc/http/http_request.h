/*
    Copyright (C) =USTC= Fu Li

    Author   :  Fu Li
    Create   :  2009-10-1
    Home     :  http://www.phoxo.com
    Mail     :  crazybitwps@hotmail.com

    This file is part of UIStone

    The code distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.

    Redistribution and use the source code, with or without modification,
    must retain the above copyright.
*/
#pragma once
#include <process.h>
#include <Wininet.h>
#pragma comment(lib, "Wininet.lib")
#include <Urlmon.h>
#pragma comment(lib, "Urlmon.lib")

#include "http_request_header.inl"
#include "http_response_info.inl"
#include "http_request_inner.inl"

class FCHttpRequestManager ; // external class

//-------------------------------------------------------------------------------------
/**
    Connect and send HTTP request to server.
*/
class FCHttpRequest
{
public:
    FCHttpRequest (int task_id, HWND notify_wnd, const HTTP_REQUEST_HEADER& request_header)
    {
        m_id = task_id ;
        m_notify_wnd = notify_wnd ;
        m_header = request_header ;
        m_header.GetUrlComponent (m_tmp_host, m_tmp_object, m_tmp_scheme, m_tmp_port) ;

        // add range into header
        m_tmp_header = request_header.m_header ;
        if (request_header.m_start)
        {
            m_tmp_header.AppendFormat (_T("Range: bytes=%d-\r\n"), request_header.m_start) ;
        }

        m_session.Attach (CreateSessionHandle()) ;

        m_connect_thread.Attach ((HANDLE)_beginthreadex (NULL, 0, http_connect_proc, this, 0, NULL)) ;
        m_start_tick = GetTickCount() ;
    }

    virtual ~FCHttpRequest()
    {
        // all threads has finish or has been terminated
        m_request.Close() ;
        m_connect.Close() ;
        m_session.Close() ;
    }

    /// Get request url.
    const CString& GetURL() const {return m_header.m_url;}
    /// Get send request header.
    const HTTP_REQUEST_HEADER& GetRequestHeader() const {return m_header;}
    /// Get response info.
    const HTTP_RESPONSE_INFO& GetResponseInfo() const {return m_response;}
    /// Get nonzero task ID, the ID is assigned by system and used to unique identify the task.
    int GetTaskID() const {return m_id;}
    /// Get request running time, in milliseconds.
    int GetRunningTime() const
    {
        INT64   n = GetTickCount() - m_start_tick ;
        if (n < 0)
            n += 0xFFFFFFFFi64 ;
        return (int)n ;
    }
    /// Get total received size.
    int GetTotalReceiveByte() const {return m_read_buf.m_total_receive;}
    /**
        Popup received data from download buffer of task \n
        Received data will be stored in inner memory buffer of download task object, so it's totally ok for small files, \n
        but for some very big file, such: 500Mb, 1Gb, it will take up large memory, so you need create a timer to loop call this function \n\n
        typical, there are two ways to use it : \n
        ( 1 ) create a timer, loop call in timer callback to get all data, this way are high recommended when file is very big \n
        ( 2 ) call in FCHttpRequestManager::OnAfterRequestFinish once to get all data
    */
    void PopReceived (std::string& receive_data)
    {
        m_read_buf.Lock() ;
        receive_data = m_read_buf.m_buf ;
        m_read_buf.m_buf.clear() ;
        m_read_buf.UnLock() ;
    }

    /**
        Get local cache file stored in IE cache folder \n
        return empty if no cache
    */
    static CString GetLocalCacheFile (LPCTSTR sUrl)
    {
        DWORD   nSize = 0 ;
        ::GetUrlCacheEntryInfo (sUrl, NULL, &nSize) ;
        if ((::GetLastError() == ERROR_INSUFFICIENT_BUFFER) && nSize)
        {
            std::vector<BYTE>   buf (nSize) ;

            INTERNET_CACHE_ENTRY_INFO   * pInfo = (INTERNET_CACHE_ENTRY_INFO*)&buf[0] ;
            ::GetUrlCacheEntryInfo (sUrl, pInfo, &nSize) ;
            return pInfo->lpszLocalFileName ;
        }
        return _T("") ;
    }

private:
    int   m_id ;
    HWND  m_notify_wnd ;
    HTTP_REQUEST_HEADER   m_header ;
    INT64   m_start_tick ;

    INTERNET_SCHEME  m_tmp_scheme ;
    INTERNET_PORT    m_tmp_port ;
    CString   m_tmp_host ; // parsed for thread
    CString   m_tmp_object ;
    CString   m_tmp_header ;

    HTTP_RESPONSE_INFO      m_response ;
    inner::CReceiveBuffer   m_read_buf ;

    inner::FCInternetHandle   m_session ;
    inner::FCInternetHandle   m_connect ;
    inner::FCInternetHandle   m_request ;

    inner::FCThreadHandle   m_connect_thread ;
    inner::FCThreadHandle   m_request_thread ;

private:
    static unsigned __stdcall http_connect_proc (LPVOID lpParameter)
    {
        FCHttpRequest   & r = *(FCHttpRequest*)lpParameter ;
        r.m_connect.Attach (InternetConnect (r.m_session, r.m_tmp_host, r.m_tmp_port, NULL, NULL, INTERNET_SERVICE_HTTP, 0, 0)) ;
        ::SendNotifyMessage (r.m_notify_wnd, WM_UISTONE_HTTP_EVENT, r.GetTaskID(), HTTP_EVENT_CONNECT_FINISH) ;
        return 0 ;
    }

    static void http_send_request_proc (FCHttpRequest& r)
    {
        DWORD  nSize = (DWORD)r.m_header.m_post_data.size() ;
        VOID   * pPost = const_cast<char*>(r.m_header.m_post_data.c_str()) ;
        HttpSendRequest (r.m_request, r.m_tmp_header, r.m_tmp_header.GetLength(), nSize ? pPost : NULL, nSize) ;
    }

    enum
    {
        RECV_BUFFER_BYTE = 8 * 1024,
    };

    static unsigned __stdcall http_request_proc (LPVOID lpParameter)
    {
        FCHttpRequest   & r = *(FCHttpRequest*)lpParameter ;

        http_send_request_proc(r) ;
        ::SendNotifyMessage (r.m_notify_wnd, WM_UISTONE_HTTP_EVENT, r.GetTaskID(), HTTP_EVENT_SEND_REQUEST) ;

        BYTE   buf[RECV_BUFFER_BYTE] ;
        BOOL   bReadResult ;
        while (true)
        {
            DWORD   dw = 0 ;
            bReadResult = InternetReadFile (r.m_request, buf, RECV_BUFFER_BYTE, &dw) ;

            if (bReadResult && dw)
            {
                r.m_read_buf.Write (buf, dw) ;
            }
            else
            {
                break ;
            }
        }
        r.m_response.m_final_read_result = bReadResult ;
        ::SendNotifyMessage (r.m_notify_wnd, WM_UISTONE_HTTP_EVENT, r.GetTaskID(), HTTP_EVENT_REQUEST_FINISH) ;
        return 0 ;
    }

private:
    enum
    {
        WM_UISTONE_HTTP_EVENT = (WM_APP + 0x30),
    };

    enum
    {
        HTTP_EVENT_CONNECT_FINISH,
        HTTP_EVENT_SEND_REQUEST,
        HTTP_EVENT_REQUEST_FINISH,
    };

    BOOL IsAllThreadFinish() const
    {
        return (!m_connect_thread.IsRunning() && !m_request_thread.IsRunning()) ;
    }

    HANDLE CloseHttpHandleForDelete()
    {
        if (m_request_thread.IsRunning())
        {
            m_request.Close() ;
            return m_request_thread ;
        }
        if (m_connect_thread.IsRunning())
        {
            m_session.Close() ;
            return m_connect_thread ;
        }
        return NULL ;
    }

    HINTERNET CreateSessionHandle() const
    {
        HINTERNET   hSession ;
        if (m_header.m_proxy_ip.GetLength())
        {
            CString   s ;
            s.Format(_T("%s:%d"), m_header.m_proxy_ip, m_header.m_proxy_port) ;
            hSession = InternetOpen (m_header.m_user_agent, INTERNET_OPEN_TYPE_PROXY, s, NULL, 0) ;
        }
        else
        {
            hSession = InternetOpen (m_header.m_user_agent, INTERNET_OPEN_TYPE_PRECONFIG, NULL, NULL, 0) ;
        }

        // set proxy username and password
        if (m_header.m_proxy_username.GetLength())
            InternetSetOption (hSession, INTERNET_OPTION_PROXY_USERNAME, (LPVOID)(LPCTSTR)m_header.m_proxy_username, m_header.m_proxy_username.GetLength()) ;
        if (m_header.m_proxy_password.GetLength())
            InternetSetOption (hSession, INTERNET_OPTION_PROXY_PASSWORD, (LPVOID)(LPCTSTR)m_header.m_proxy_password, m_header.m_proxy_password.GetLength()) ;

        return hSession ;
    }

    void OnConnectThreadFinish()
    {
        m_connect_thread.Close() ;

        // open request
        if (m_tmp_scheme == INTERNET_SCHEME_HTTPS)
        {
            m_header.m_open_flag |= INTERNET_FLAG_SECURE ;
        }
        m_request.Attach (HttpOpenRequest (m_connect, m_header.GetVerbText(), m_tmp_object, NULL, NULL, NULL, m_header.m_open_flag, 0)) ;

        // start request thread
        m_request_thread.Attach ((HANDLE)_beginthreadex (NULL, 0, http_request_proc, this, 0, NULL)) ;
    }

friend class FCHttpRequestManager ;
};
