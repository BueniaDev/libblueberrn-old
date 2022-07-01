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
    Spanish,
    French,
    German,
    Italian,
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
    {"Load machine...", "Load machine..."},
    {"Stop machine...", "Stop machine..."},
    {"Quit", "Quit"},
    {"Select a machine...", "Select a machine..."},
    {"Language...", "Language..."},
};

vector<BerrnTranslate> ja_trans =
{
    {"File", u8"ファイル"},
    {"Load machine...", u8"マシンをロードします..."},
    {"Stop machine...", u8"マシンを停止します..."},
    {"Quit", u8"辞める"},
    {"Select a machine...", u8"マシンを選択..."},
    {"Language...", u8"言語..."},
};

vector<BerrnTranslate> ko_trans = 
{
    {"File", u8"파일"},
    {"Load machine...", u8"로드 머신..."},
    {"Stop machine...", u8"기계 그만..."},
    {"Quit", u8"그만두 다"},
    {"Select a machine...", u8"머신 선택..."},
    {"Language...", u8"언어..."},
};

vector<BerrnTranslate> es_trans =
{
    {"File", u8"Archivo"},
    {"Load machine...", u8"Cargar máquina..."},
    {"Stop machine...", u8"Detener la máquina..."},
    {"Quit", u8"Renunciar"},
    {"Select a machine...", u8"Seleccione una máquina..."},
    {"Language...", u8"Idioma..."},
};

vector<BerrnTranslate> fr_trans =
{
    {"File", u8"Déposer"},
    {"Load machine...", u8"Charger la machine..."},
    {"Stop machine...", u8"Arrêter la machine..."},
    {"Quit", u8"Quitter"},
    {"Select a machine...", u8"Sélectionnez une machine..."},
    {"Language...", u8"Langue..."},
};

vector<BerrnTranslate> de_trans =
{
    {"File", u8"Datei"},
    {"Load machine...", u8"Maschine laden..."},
    {"Stop machine...", u8"Maschine stoppen..."},
    {"Quit", u8"Aufhören"},
    {"Select a machine...", u8"Wählen Sie eine Maschine aus..."},
    {"Language...", u8"Sprache..."},
};

vector<BerrnTranslate> it_trans =
{
    {"File", u8"File"},
    {"Load machine...", u8"Carica macchina..."},
    {"Stop machine...", u8"Ferma la macchina..."},
    {"Quit", u8"Uscire"},
    {"Select a machine...", u8"Seleziona una macchina..."},
    {"Language...", u8"Lingua..."},
};

unordered_map<UiLanguage, vector<BerrnTranslate>> translations =
{
    {English, en_trans},
    {Japanese, ja_trans},
    {Korean, ko_trans},
    {Spanish, es_trans},
    {French, fr_trans},
    {German, de_trans},
    {Italian, it_trans},
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