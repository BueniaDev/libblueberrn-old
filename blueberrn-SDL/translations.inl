struct BerrnTranslate
{
    string source;
    string translation;
};

enum UiLanguage : int
{
    English,
    Japanese,
    Korean,
    Max,
};

UiLanguage current_language = English;

void set_current_language(UiLanguage language)
{
    current_language = language;
}

vector<BerrnTranslate> en_trans =
{
    {"File", "File"},
    {"Load driver...", "Load driver..."},
    {"Stop driver...", "Stop driver..."},
    {"Quit", "Quit"},
    {"Select a driver...", "Select a driver..."},
    {"Language...", "Language..."},
};

vector<BerrnTranslate> ja_trans =
{
    {"File", u8"ファイル"},
    {"Load driver...", u8"ロードドライバ..."},
    {"Stop driver...", u8"ストップドライバー..."},
    {"Quit", u8"辞める"},
    {"Select a driver...", u8"ドライバーを選択..."},
    {"Language...", "言語..."},
};

vector<BerrnTranslate> ko_trans = 
{
    {"File", u8"파일"},
    {"Load driver...", u8"로드 드라이버..."},
    {"Stop driver...", u8"중지 드라이버..."},
    {"Quit", u8"그만두 다"},
    {"Select a driver...", u8"드라이버를 선택..."},
    {"Language...", "언어..."},
};

unordered_map<UiLanguage, vector<BerrnTranslate>> translations =
{
    {English, en_trans},
    {Japanese, ja_trans},
    {Korean, ko_trans},
};

const char* get_translation(string source)
{
    auto iter = translations.find(current_language);

    if (iter != translations.end())
    {
	auto lang_trans = iter->second;

	for (auto &text : lang_trans)
	{
	    if (text.source == source)
	    {
		return text.translation.c_str();
	    }
	}
    }

    return "unknown";
}