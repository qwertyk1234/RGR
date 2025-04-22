#include <iostream>
#include <fstream>
#include <vector>
#include <list>
#include <string>
#include <unordered_map>
#include <cctype>
#include <locale>


using namespace std;    

// Размер хеш-таблицы
const int TABLE_SIZE = 101;

// Функция хеширования
int hashFunction(const string& key) {
    int hash = 0;
    for (char c : key)
        hash = (hash * 25 + tolower(c)) % TABLE_SIZE;
    return hash;
}

// Структура хеш-таблицы с открытой адресацией
class HashTable {
private:
    vector<list<pair<string, vector<int>>>> table;

public:
    HashTable() : table(TABLE_SIZE) {}

    void insert(const string& word, const vector<int>& positions) {
        int index = hashFunction(word);
        table[index].push_back({ word, positions });
    }

    void remove(const string& word) {
        int index = hashFunction(word);
        table[index].remove_if([&](const pair<string, vector<int>>& p) {
            return p.first == word;
            });
    }

    vector<int> search(const string& word) {
        int index = hashFunction(word);
        for (const auto& pair : table[index]) {
            if (pair.first == word) return pair.second;
        }
        return {};
    }

    bool exists(const string& word) {
        return !search(word).empty();
    }
};

// Предобработка для алгоритма Бойера-Мура
void buildBadCharTable(const string& pattern, vector<int>& badChar) {
    int m = pattern.length();
    badChar.assign(256, -1);
    for (int i = 0; i < m; i++) {
        badChar[(unsigned char)pattern[i]] = i;
    }
}

// Алгоритм Бойера-Мура для поиска всех вхождений
vector<int> boyerMooreSearch(const string& text, const string& pattern) {
    vector<int> positions;
    int n = text.length();
    int m = pattern.length();
    if (m == 0 || n < m) return positions;

    vector<int> badChar;
    buildBadCharTable(pattern, badChar);

    int shift = 0;
    while (shift <= (n - m)) {
        int j = m - 1;
        while (j >= 0 && tolower(pattern[j]) == tolower(text[shift + j]))
            j--;

        if (j < 0) {
            positions.push_back(shift);
            shift += (shift + m < n) ? m - badChar[(unsigned char)tolower(text[shift + m])] : 1;
        }
        else {
            shift += max(1, j - badChar[(unsigned char)tolower(text[shift + j])]);
        }
    }
    return positions;
}

// Удаление пунктуации и приведение к нижнему регистру
string normalize(const string& word) {
    string clean;
    for (char c : word) {
        if (isalpha(c)) clean += tolower(c);
    }
    return clean;
}

int main() {

    setlocale(LC_ALL, "");

    string text;
    ifstream textFile("text.txt");
    if (!textFile.is_open()) {
        cerr << "Ошибка открытия text.txt\n";
        return 1;
    }

    string line;
    while (getline(textFile, line)) {
        text += line + " ";
    }
    textFile.close();

    ifstream wordFile("words.txt");
    if (!wordFile.is_open()) {
        cerr << "Ошибка открытия words.txt\n";
        return 1;
    }

    vector<string> words;
    string word;
    while (wordFile >> word) {
        words.push_back(normalize(word));
    }
    wordFile.close();

    HashTable table;

    // Заносим слова в хеш-таблицу, используя алгоритм Бойера-Мура
    for (const string& w : words) {
        vector<int> positions = boyerMooreSearch(text, w);
        if (!positions.empty()) {
            table.insert(w, positions);
        }
    }

    int choice;
    do {
        cout << "\n=== МЕНЮ БРАТ ===\n";
        cout << "1. Поиск слова\n";
        cout << "2. Добавить слово\n";
        cout << "3. Удалить слово\n";
        cout << "4. Показать все слова с позициями\n";
        cout << "5. Выход\n";
        cout << "Выбор: ";
        cin >> choice;

        string input;
        switch (choice) {
        case 1:
            cout << "Введите слово для поиска: ";
            cin >> input;
            input = normalize(input);

            if (find(words.begin(), words.end(), input) == words.end()) {
                cout << "-1\n";
            }
            else {
                {
                    vector<int> result = table.search(input);
                    if (result.empty()) {
                        cout << "-1\n";
                    }
                    else {
                        cout << "Позиции: ";
                        for (int pos : result)
                            cout << pos << " ";
                        cout << endl;
                    }
                }
            }
            break;

        case 2:
            cout << "Введите слово для добавления: ";
            cin >> input;
            input = normalize(input);
            {
                vector<int> positions = boyerMooreSearch(text, input);
                table.insert(input, positions);  // Добавляем слово в таблицу без проверки
                words.push_back(input); // добавляем в список слов
                cout << "Слово добавлено.\n";
            }
            break;

        case 3:
            cout << "Введите слово для удаления: ";
            cin >> input;
            input = normalize(input);
            table.remove(input);
            words.erase(remove(words.begin(), words.end(), input), words.end());
            cout << "Слово удалено, если оно было.\n";
            break;

        case 4:
            cout << "\nВсе слова в хеш-таблице:\n";
            for (const auto& w : words) {
                vector<int> pos = table.search(w);
                cout << w << ": ";
                for (int p : pos)
                    cout << p << " ";
                cout << endl;
            }
            break;

        case 5:
            cout << "Выход. Удачи, брат!\n";
            break;

        default:
            cout << "Неправильный выбор, брат. Повтори.\n";
        }

    } while (choice != 5);

    return 0;
}
