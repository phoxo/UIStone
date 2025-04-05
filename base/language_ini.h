#pragma once

_PHOXO_INTERNAL_BEGIN

using CStringRef = const CString&;

enum class IniLineType
{
    Section,
    Skip, // �����ǿ��� or ע�� or ��Ч����
    KeyValue,
};

struct IniSection : public std::map<std::wstring_view, std::wstring_view>
{
    static bool IsSectionLine(const std::wstring_view& sv)
    {
        size_t   first = sv.find_first_not_of(L" \t");
        size_t   end = sv.find_last_not_of(L" \t");
        return (first != sv.npos) && (end != sv.npos) && (sv[first] == '[') && (sv[end] == ']');
    }

    IniLineType ParseLine(const std::wstring_view& sv)
    {
        size_t   pos = sv.find('=');
        if (pos == sv.npos)
        {
            if (IsSectionLine(sv))
                return IniLineType::Section;
            return IniLineType::Skip;
        }

        auto   key = sv.substr(0, pos); assert(key.size());
        auto   val = sv.substr(pos + 1);
        insert_or_assign(key, val);
        return IniLineType::KeyValue;
    }

    CString Get(PCWSTR key) const
    {
        if (key)
        {
            if (auto it = find(key); it != end())
                return FCString::Make(it->second);
        }
        return L"";
    }
};

class LanguageIniReader
{
private:
    std::map<CString, IniSection>   m_sections;
    std::vector<BYTE>   m_buf;

public:
    LanguageIniReader(PCWSTR inifile)
    {
        FCFile::Read(inifile, m_buf);
        m_buf.insert(m_buf.end(), 2, 0);
    }

    CString Get(const CString& section, PCWSTR key)
    {
        return GetSection(section).Get(key);
    }

private:
    const IniSection& GetSection(CStringRef name)
    {
        if (auto it = m_sections.find(name); it != m_sections.end())
            return it->second;

        auto   & sec = m_sections[name];
        ParseSection(name, sec);
        return sec;
    }

    void ParseSection(CStringRef name, IniSection& section) const
    {
        auto   curr = FindSection(name);
        while (!curr.empty())
        {
            auto   line = PopCurrentLine(curr);
            if (section.ParseLine(line) == IniLineType::Section)
                break;
        }
    }

    std::wstring_view FindSection(CStringRef name) const
    {
        if (auto ptr = StrStrI((PCWSTR)m_buf.data(), '[' + name + ']'))
        {
            std::wstring_view   sv(ptr);
            PopCurrentLine(sv);
            return sv;
        }
        return {};
    }

    static void TrimLeadingChars(auto& sv, PCWSTR token)
    {
        while (sv.size() && wcschr(token, sv[0]))
        {
            sv.remove_prefix(1);
        }
    }

    static auto PopCurrentLine(auto& sv)
    {
        std::wstring_view   line;
        size_t   pos = sv.find_first_of(L"\r\n");
        if (pos == sv.npos)
        {
            line = sv; // ����û�����ˣ��Ҿ������һ��
            sv = {};
        }
        else
        {
            line = sv.substr(0, pos);
            sv.remove_prefix(pos + 1);
            TrimLeadingChars(sv, L"\r\n"); // ������һЩ����
        }
        return line;
    }
};

_PHOXO_NAMESPACE_END
