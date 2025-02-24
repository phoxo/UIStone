#pragma once

_PHOXO_INTERNAL_BEGIN

enum class IniLineType
{
    Section,
    Skip, // 可能是空行 or 注释 or 无效的行
    KeyValue,
};

struct IniSection : public std::map<std::wstring_view, std::wstring_view>
{
    IniLineType ParseLine(const std::wstring_view& sv)
    {
        size_t   pos = sv.find('=');
        if (pos == sv.npos)
        {
            if ((sv.find('[') != sv.npos) && (sv.find(']') != sv.npos))
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
        auto   it = find(key);
        if (it != end())
            return FCString::Make(it->second);
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

    CString Get(PCWSTR section, PCWSTR key)
    {
        return (section && key) ? GetSection(section).Get(key) : CString();
    }

private:
    const IniSection& GetSection(PCWSTR name)
    {
        if (auto it = m_sections.find(name); it != m_sections.end())
            return it->second;

        auto   & sec = m_sections[name];
        ParseSection(name, sec);
        return sec;
    }

    void ParseSection(PCWSTR name, IniSection& section) const
    {
        auto   curr = FindSection(name);
        while (!curr.empty())
        {
            auto   line = PopCurrentLine(curr);
            if (section.ParseLine(line) == IniLineType::Section)
                break;
        }
    }

    std::wstring_view FindSection(const CString& name) const
    {
        if (auto ptr = StrStrI((PCWSTR)m_buf.data(), '[' + name + ']'); ptr)
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
            line = sv; // 下面没有行了，我就是最后一行
            sv = {};
        }
        else
        {
            line = sv.substr(0, pos);
            sv.remove_prefix(pos + 1);
            TrimLeadingChars(sv, L"\r\n"); // 可能有一些空行
        }
        return line;
    }
};

_PHOXO_NAMESPACE_END
