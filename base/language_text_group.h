#pragma once

class LanguageTextGroup
{
private:
    std::deque<CString>   m_text_list;

public:
    /// key == "t" means total, read all text, then split
    LanguageTextGroup(PCWSTR section, PCWSTR key = L"t")
    {
        CString   txt = LanguageText::Get(section, key);
        FCString::SplitTextByToken(txt, m_text_list, L"|");
    }

    CString operator[](int index) const
    {
        try
        {
            return m_text_list.at(index);
        }
        catch (const std::out_of_range&) { assert(false); }
        return L"";
    }
};
