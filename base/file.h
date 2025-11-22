#pragma once
#include <atlpath.h>
#include <Shlobj.h>
#pragma comment(lib, "Shell32.lib")

/// File helper.
class FCFile
{
public:
    static CString GetModulePath(HMODULE mod = nullptr)
    {
        CPath   t;
        ::GetModuleFileName(mod, t.m_strPath.GetBuffer(MAX_PATH), MAX_PATH);
        t.m_strPath.ReleaseBuffer();
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
        dir.Append(sub_folder_name);
        dir.AddBackslash();
        return dir;
    }

    static void RemoveReadOnlyAttribute(PCWSTR filepath)
    {
        DWORD   prop = GetFileAttributes(filepath);
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
    static void Read(PCWSTR filepath, std::vector<BYTE>& out)
    {
        HANDLE   f = CreateFile(filepath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
        if (f != INVALID_HANDLE_VALUE)
        {
            DWORD   len = ::GetFileSize(f, NULL), read = 0;
            if (len != INVALID_FILE_SIZE)
            {
                out.reserve(len + 2); //·½±ã×ª»»ÎªWCHAR*
                out.resize(len);
                ::ReadFile(f, out.data(), len, &read, NULL);
            }
            CloseHandle(f);
        }
    }

    /// Write buffer to file, if file already exist, it will be delete before write.
    static BOOL Write(PCWSTR filepath, LPCVOID p, DWORD write_bytes)
    {
        SetFileAttributes(filepath, FILE_ATTRIBUTE_NORMAL);

        BOOL   ret = false;
        HANDLE   f = CreateFile(filepath, GENERIC_WRITE, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
        if (DWORD result = 0; f != INVALID_HANDLE_VALUE)
        {
            ret = ::WriteFile(f, p, write_bytes, &result, NULL);
            CloseHandle(f);
        }
        return ret;
    }

    static CString GetTempFolder()
    {
        CPath   t;
        ::GetTempPath(MAX_PATH, t.m_strPath.GetBuffer(MAX_PATH));
        t.m_strPath.ReleaseBuffer();
        t.AddBackslash();
        return t.m_strPath;
    }

    /// ext: such as ".jpg"
    static CString CreateTempFilename(const CString& ext)
    {
        return GetTempFolder() + phoxo::Utils::NewGUIDString() + ext;
    }

    /// Read a string value from INI.
    static void INIRead(PCWSTR filepath, PCWSTR key, CString& s, PCWSTR section = L"app")
    {
        WCHAR   b[256] = {};
        if (DWORD read = GetPrivateProfileString(section, key, NULL, b, 256, filepath))
        {
            if (read > (256 - 4))
            {
                std::vector<WCHAR>   buf(2048);
                GetPrivateProfileString(section, key, NULL, buf.data(), (DWORD)buf.size(), filepath);
                s = buf.data();
            }
            else
            {
                s = b;
            }
        }
    }

    static void ForceDeleteOnReboot(PCWSTR filepath)
    {
        DeleteFile(filepath);
        if (PathFileExists(filepath))
        {
            CString   newname = filepath;
            newname.AppendFormat(L"_%d.delete", (int)GetTickCount());
            MoveFile(filepath, newname);
            MoveFileEx(newname, NULL, MOVEFILE_DELAY_UNTIL_REBOOT);
        }
    }

#ifdef _AFX
    static void FindFolderFiles(const CString& folder, std::deque<CString>& collected)
    {
        CFileFind   fd;
        BOOL   working = fd.FindFile(folder);
        while (working)
        {
            working = fd.FindNextFile();
            collected.push_back(fd.GetFileName());
        }
    }
#endif
};
