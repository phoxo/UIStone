#pragma once
_UISTONE_BEGIN

class CLanguageText
{
public:
    static CString Get(PCWSTR section, PCWSTR key)
    {
        CString   s;
        FCFile::INIRead(GetLanguageFile(CURRENT_NAME()), key, s, section);
        return s;
    }

    static CString Get(PCWSTR section, int key)
    {
        CString   s;
        s.Format(L"%d", key);
        return Get(section, s);
    }

    static CString GetLanguageName(PCWSTR filename)
    {
        CString   s;
        FCFile::INIRead(GetLanguageFile(filename), L"LANGUAGE", s);
        return s;
    }

    // (en.ini,English) , (pt.ini,Portugu¨ºs)
    static void FindAllLanguage(std::map<CString, CString>& language_list)
    {
        WIN32_FIND_DATA   fd = { 0 };
        HANDLE   h = ::FindFirstFile(LANGUAGE_FOLDER() + L"*.ini", &fd);
        if (h == INVALID_HANDLE_VALUE)
            return;
        do
        {
            CString   name = GetLanguageName(fd.cFileName);
            if (!name.IsEmpty())
            {
                language_list[fd.cFileName] = name;
            }
        } while (::FindNextFile(h, &fd));
        ::FindClose(h);
    }

    static CString GetCurrentLanguageID() { return CURRENT_NAME(); }

    static void SetCurrentLanguageID(PCWSTR filename)
    {
        if (StrStrI(filename, L".ini") && PathFileExists(GetLanguageFile(filename)))
        {
            wcscpy_s(CURRENT_NAME(), MAX_LANGUAGE_NAME, filename);
        }
    }

    static CString GetSystemLanguageID()
    {
        WCHAR   name[LOCALE_NAME_MAX_LENGTH] = { 0 };
        ::GetSystemDefaultLocaleName(name, LOCALE_NAME_MAX_LENGTH);

        if ((StrStrI(name, L"zh-CN") == name) ||
            (StrStrI(name, L"zh-SG") == name) ||
            (StrCmpI(name, L"zh-Hans") == 0) ||
            (StrCmpI(name, L"zh") == 0))
            return L"cn_simp.ini";

        if (StrStrI(name, L"zh") == name)  return L"cn_trad.ini";

        if (StrStrI(name, L"fr-") == name)  return L"fr.ini";
        if (StrStrI(name, L"de-") == name)  return L"de.ini";
        if (StrStrI(name, L"es-") == name)  return L"es.ini";
        if (StrStrI(name, L"pt-") == name)  return L"pt.ini";
        if (StrStrI(name, L"ru") == name)  return L"ru.ini";
        if (StrStrI(name, L"ar") == name)  return L"ar.ini";
        if (StrStrI(name, L"it") == name)  return L"it.ini";
        if (StrStrI(name, L"ja") == name)  return L"ja.ini";
        if (StrStrI(name, L"ko") == name)  return L"ko.ini";
        if (StrStrI(name, L"vi") == name)  return L"vi.ini";
        return L"en.ini";
    }

    static CString& LANGUAGE_FOLDER()
    {
        static CString   dir = FCFile::GetModuleFolder((HMODULE)&__ImageBase) + L"language\\";
        return dir;
    }

private:
    enum { MAX_LANGUAGE_NAME = 32 };

    static WCHAR* CURRENT_NAME()
    {
        static WCHAR   buf[MAX_LANGUAGE_NAME] = L"en.ini";
        return buf;
    }

    static CString GetLanguageFile(PCWSTR filename) { return LANGUAGE_FOLDER() + filename; }
};

_UISTONE_END
