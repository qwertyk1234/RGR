#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <cwctype>
#include <locale>
#include <codecvt>
#include <algorithm>
#include <windows.h>
#include <fcntl.h>   // <-- для _O_U8TEXT
#include <io.h>      // <-- для _setmode

using namespace std;

// Размер хеш-таблицы
const int TABLE_SIZE = 101;

// Хеш-функция
int hashFunction(const wstring& key) {
    int hash = 0;
    for (wchar_t c : key)
        hash = (hash * 33 + towlower(c)) % TABLE_SIZE;
    return hash;
}

class HashTable {
private:
    vector<list<pair<wstring, vector<int>>>> table;

public:
    HashTable() : table(TABLE_SIZE) {}

    void insert(const wstring& word, const vector<int>& positions) {
        int index = hashFunction(word);
        table[index].push_back({ word, positions });
    }

    void remove(const wstring& word) {
        int index = hashFunction(word);
        table[index].remove_if([&](const pair<wstring, vector<int>>& p) {
            return p.first == word;
            });
    }

    vector<int> search(const wstring& word) {
        int index = hashFunction(word);
        for (const auto& pair : table[index]) {
            if (pair.first == word) return pair.second;
        }
        return {};
    }
};

// Таблица плохих символов
void buildBadCharTable(const wstring& pattern, vector<int>& badChar) {
    badChar.assign(65536, -1);
    for (int i = 0; i < pattern.length(); i++) {
        badChar[towlower(pattern[i])] = i;
    }
}

// Бойер-Мур
vector<int> boyerMooreSearch(const wstring& text, const wstring& pattern) {
    vector<int> positions;
    int n = text.length();
    int m = pattern.length();
    if (m == 0 || n < m) return positions;

    vector<int> badChar;
    buildBadCharTable(pattern, badChar);

    int shift = 0;
    while (shift <= (n - m)) {
        int j = m - 1;
        while (j >= 0 && towlower(text[shift + j]) == towlower(pattern[j])) j--;

        if (j < 0) {
            positions.push_back(shift);
            shift += (shift + m < n) ? m - badChar[towlower(text[shift + m])] : 1;
        }
        else {
            shift += max(1, j - badChar[towlower(text[shift + j])]);
        }
    }
    return positions;
}

wstring normalize(const wstring& word) {
    wstring clean;
    for (wchar_t c : word) {
        if (iswalpha(c)) clean += towlower(c);
    }
    return clean;
}

pair<wstring, wstring> log() {
    wcout << L"Какой текст?\n1. Для РГР\n2. Для души\n?: ";
    int ch;
    wcin >> ch;
    if (ch == 1) return { L"text.txt", L"words.txt" };
    if (ch == 2) return { L"text2.txt", L"words2.txt" };

    wcout << L"Ни-ху-я, не попал! ВХВХВХВХХВ\n"
        L"Используем дефолт, прошу прощения!" << endl;
    return { L"text.txt", L"words.txt" };
}

int main() {
    // Настройка консоли на UTF-8
    SetConsoleOutputCP(CP_UTF8);
    SetConsoleCP(CP_UTF8);
    locale::global(locale("en_US.utf8"));  
    wcin.imbue(locale());
    wcout.imbue(locale());

    auto files = log();

    // Чтение текста из UTF-8 файла
    wifstream textFile(files.first);
    textFile.imbue(locale(textFile.getloc(), new codecvt_utf8<wchar_t>));
    if (!textFile.is_open()) {
        wcerr << L"Ошибка открытия файла: " << files.first << endl;
        return 1;
    }

    wstring text, line;
    while (getline(textFile, line)) {
        text += line + L" ";
    }
    textFile.close();

    // Чтение слов из UTF-8 файла
    wifstream wordFile(files.second);
    wordFile.imbue(locale(wordFile.getloc(), new codecvt_utf8<wchar_t>));
    if (!wordFile.is_open()) {
        wcerr << L"Ошибка открытия файла: " << files.second << endl;
        return 1;
    }

    vector<wstring> words;
    wstring word;
    while (wordFile >> word) {
        words.push_back(normalize(word));
    }
    wordFile.close();

    HashTable table;
    for (const auto& w : words) {
        vector<int> positions = boyerMooreSearch(text, w);
        table.insert(w, positions);
    }

    int choice;
    do {
        wcout << L"\n=== МЕНЮ БРАТ ===\n"
            << L"1. Поиск слова\n"
            << L"2. Добавить слово\n"
            << L"3. Удалить слово\n"
            << L"4. Показать все слова с позициями\n"
            << L"5. Выход\n"
            << L"Выбор: ";
        wcin >> choice;

        wstring input;
        switch (choice) {
        case 1:
            wcout << L"Введите слово для поиска: ";
            wcin >> input;
            input = normalize(input);
            {
                auto result = table.search(input);
                if (result.empty()) {
                    wcout << L"-1\n";
                }
                else {
                    wcout << L"Позиции: ";
                    for (int pos : result) wcout << pos << L" ";
                    wcout << endl;
                }
            }
            break;

        case 2:
            wcout << L"Введите слово для добавления: ";
            wcin >> input;
            input = normalize(input);
            {
                auto positions = boyerMooreSearch(text, input);
                table.insert(input, positions);
                words.push_back(input);
                wcout << L"Слово добавлено.\n";
            }
            break;

        case 3:
            wcout << L"Введите слово для удаления: ";
            wcin >> input;
            input = normalize(input);
            table.remove(input);
            words.erase(remove(words.begin(), words.end(), input), words.end());
            wcout << L"Слово удалено.\n";
            break;

        case 4:
            wcout << L"\nСлова в хеш-таблице:\n";
            for (const auto& w : words) {
                auto pos = table.search(w);
                wcout << w << L": ";
                for (int p : pos) wcout << p << L" ";
                wcout << endl;
            }
            break;

        case 5:
            wcout << L"Выход. Удачи, брат!\n";
            break;

        default:
            wcout << L"Неверный выбор. Повтори.\n";
        }
    } while (choice != 5);

    return 0;
}
