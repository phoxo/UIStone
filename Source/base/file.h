#pragma once
#include <atlpath.h>
#include <Shlobj.h>
#pragma comment(lib, "Shell32.lib")
UISTONE_BEGIN

/// File helper.
class FCFile
{
public:
    static CString GetModulePath(HMODULE mod = nullptr)
    {
        CPath   t;
        ::GetModuleFileName(mod, t.m_strPath.GetBuffer(MAX_PATH), MAX_PATH);
        t.m_strPath.ReleaseBuffer();
        t.Canonicalize();
        return t;
    }

    /// Get folder that module locate, with backslash('\') append \n
    static CString GetModuleFolder(HMODULE mod = nullptr)
    {
        return GetFileFolder(GetModulePath(mod));
    }

    /// Get folder that file locate, with backslash('\') append.
    static CString GetFileFolder(PCWSTR filepath)
    {
        CPath   t(filepath);
        t.RemoveFileSpec();
        t.AddBackslash();
        return t;
    }

    static CString GetAppDataFolder(PCWSTR sub_folder_name)
    {
        CPath   dir;
        SHGetFolderPath(NULL, CSIDL_APPDATA, NULL, SHGFP_TYPE_CURRENT, dir.m_strPath.GetBuffer(MAX_PATH));
        dir.m_strPath.ReleaseBuffer();
        dir.Canonicalize();
        dir.Append(sub_folder_name);
        dir.AddBackslash();
        return dir;
    }

    static void RemoveReadOnlyAttribute(LPCTSTR filepath)
    {
        auto   prop = GetFileAttributes(filepath);
        if ((prop != INVALID_FILE_ATTRIBUTES) && (prop & FILE_ATTRIBUTE_READONLY))
        {
            SetFileAttributes(filepath, prop & ~FILE_ATTRIBUTE_READONLY);
        }
    }

    /// Get file's size.
    static INT64 GetSize(PCWSTR filepath)
    {
        WIN32_FILE_ATTRIBUTE_DATA   fd = {};
        if (GetFileAttributesEx(filepath, GetFileExInfoStandard, &fd))
        {
            LARGE_INTEGER   n;
            n.LowPart = fd.nFileSizeLow;
            n.HighPart = (LONG)fd.nFileSizeHigh;
            return n.QuadPart;
        }
        return 0;
    }

    /// Get file's extension name, not including leading period.
    static CString GetExtName(PCWSTR filepath)
    {
        CString   s(PathFindExtension(filepath));
        if (!s.IsEmpty())
        {
            s.Delete(0, 1);
        }
        return s;
    }

    /// Read file to memory.
    static bool Read(PCWSTR filepath, std::vector<BYTE>& file_data)
    {
        file_data.clear();

        bool     b = false;
        HANDLE   f = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
        if (f != INVALID_HANDLE_VALUE)
        {
            DWORD   len = ::GetFileSize(f, NULL);
            if (len && (len != INVALID_FILE_SIZE))
            {
                file_data.reserve(len + 2);
                file_data.resize(len);

                DWORD   dwRead = 0;
                ::ReadFile(f, file_data.data(), len, &dwRead, NULL);
                b = (dwRead == len);
            }
            else
            {
                b = true;
            }
            CloseHandle(f);
        }
        assert(b);
        return b;
    }

    /// Write buffer to file, if file already exist, it will be delete before write.
    static bool Write(PCWSTR filepath, const void* p, int write_bytes)
    {
        RemoveReadOnlyAttribute(filepath);

        bool     b = false;
        HANDLE   f = CreateFile(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (f != INVALID_HANDLE_VALUE)
        {
            DWORD   nWrite = 0;
            ::WriteFile(f, p, write_bytes, &nWrite, NULL);
            b = ((int)nWrite == write_bytes);
            CloseHandle(f);
        }
        assert(b);
        return b;
    }

    static CString GetTempFolder()
    {
        CPath   t;
        ::GetTempPath(MAX_PATH, t.m_strPath.GetBuffer(MAX_PATH));
        t.m_strPath.ReleaseBuffer();
        t.AddBackslash();
        return t.m_strPath;
    }

    /// @name Read/Write INI file.
    //@{
    /// Read string key from ini file, return false if key doesn't exist.
    static bool INIRead(PCWSTR filepath, PCWSTR key, CString& s, PCWSTR section = L"app")
    {
        WCHAR   b[256] = {};
        DWORD   dwWrite = GetPrivateProfileString(section, key, L"\n", b, 256, filepath);
        if ((b[0] == '\n') && (b[1] == 0))
            return false;

        if (dwWrite > (256 - 4))
        {
            std::vector<WCHAR>   buf(2048, 0);
            GetPrivateProfileString(section, key, L"\n", buf.data(), (DWORD)buf.size(), filepath);
            s = buf.data();
        }
        else
        {
            s = b;
        }
        return true;
    }

    /// Read int key from ini file, return false if key doesn't exist.
    static bool INIRead(PCWSTR filepath, PCWSTR key, INT64& n, PCWSTR section = L"app")
    {
        WCHAR   b[32] = {};
        GetPrivateProfileString(section, key, L"\n", b, 32, filepath);
        if ((b[0] == '\n') && (b[1] == 0))
            return false;
        if (b[0] == 0)
            return false;

        n = _ttoi64(b);
        return true;
    }
    //@}
};

UISTONE_END
