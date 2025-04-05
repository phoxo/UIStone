#pragma once

class FileSaveAgent
{
private:
    const CString   m_dest_path;
    const CString   m_temp_file;

public:
    FileSaveAgent(PCWSTR dest_path) : m_dest_path(dest_path), m_temp_file(BuildTempName(dest_path))
    {
    }

    ~FileSaveAgent()
    {
        DeleteFile(m_temp_file);
    }

    auto& GetTempFile() const
    {
        return m_temp_file;
    }

    bool CommitReplace() const
    {
        if (PathFileExists(m_temp_file))
        {
            SetFileAttributes(m_dest_path, FILE_ATTRIBUTE_NORMAL);
            if (CopyFile(m_temp_file, m_dest_path, FALSE))
                return true;
        }
        assert(false);
        return false;
    }

private:
    static CString BuildTempName(PCWSTR dest_path)
    {
        return FCFile::CreateTempFilename(PathFindExtension(dest_path));
    }
};
