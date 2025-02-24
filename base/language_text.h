#pragma once
#include "language_ini.h"

class LanguageText
{
public:
    static CString Get(PCWSTR section, PCWSTR key)
    {
        return GetIniReader().Get(section, key);
    }

    static CString Get(PCWSTR section, int key)
    {
        return Get(section, FCString::From(key));
    }

    // (en.ini,English) , (pt.ini,Portugu¨ºs)
    static void FindAllLanguage(std::map<CString, CString>& language_list)
    {
        WIN32_FIND_DATA   fd = {};
        HANDLE   h = ::FindFirstFile(LanguageFolder() + L"*.ini", &fd);
        if (h != INVALID_HANDLE_VALUE)
        {
            do
            {
                CString   name;
                FCFile::INIRead(LanguageFolder() + fd.cFileName, L"LANGUAGE", name);
                if (!name.IsEmpty())
                {
                    language_list[fd.cFileName] = name;
                }
            } while (::FindNextFile(h, &fd));
            ::FindClose(h);
        }
    }

    static PCWSTR GetCurrentLanguageID()
    {
        return CURRENT_NAME();
    }

    static void SetCurrentLanguageID(PCWSTR filename)
    {
        if (StrStrI(filename, L".ini") && PathFileExists(LanguageFolder() + filename))
        {
            wcscpy_s(CURRENT_NAME(), MAX_LANGUAGE_NAME, filename);
        }
    }

    static CString GetSystemLanguageID()
    {
        WCHAR   buf[LOCALE_NAME_MAX_LENGTH] = {};
        ::GetSystemDefaultLocaleName(buf, LOCALE_NAME_MAX_LENGTH);
        CharLower(buf);

        std::wstring_view   name(buf);
        if (name.starts_with(L"zh-cn") ||
            name.starts_with(L"zh-sg") ||
            (name == L"zh-hans") ||
            (name == L"zh"))
            return L"cn_simp.ini";

        if (name.starts_with(L"zh"))  return L"cn_trad.ini";

        if (name.starts_with(L"fr-"))  return L"fr.ini";
        if (name.starts_with(L"de-"))  return L"de.ini";
        if (name.starts_with(L"es-"))  return L"es.ini";
        if (name.starts_with(L"pt-"))  return L"pt.ini";
        if (name.starts_with(L"ru"))  return L"ru.ini";
        if (name.starts_with(L"ar"))  return L"ar.ini";
        if (name.starts_with(L"it"))  return L"it.ini";
        if (name.starts_with(L"ja"))  return L"ja.ini";
        if (name.starts_with(L"ko"))  return L"ko.ini";
        if (name.starts_with(L"vi"))  return L"vi.ini";
        if (name.starts_with(L"uk"))  return L"uk.ini";
        return L"en.ini";
    }

private:
    static auto& GetIniReader()
    {
        static internal::LanguageIniReader   s(LanguageFolder() + CURRENT_NAME());
        return s;
    }

    enum { MAX_LANGUAGE_NAME = 32 };

    static WCHAR* CURRENT_NAME()
    {
        static WCHAR   buf[MAX_LANGUAGE_NAME] = L"en.ini";
        return buf;
    }

    static CString& LanguageFolder()
    {
        static CString   dir = FCFile::GetModuleFolder((HMODULE)&__ImageBase) + L"language\\";
        return dir;
    }
};
