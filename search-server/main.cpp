#include <algorithm>
#include <cmath>
#include <iostream>
#include <map>
#include <set>
#include <string>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;

struct DocumentContent{
    int id;
    vector<string> words;
};

string ReadLine() {
    string s;
    getline(cin, s);
    return s;
}

int ReadLineWithNumber() {
    int result = 0;
    cin >> result;
    ReadLine();
    return result;
}

vector<string> SplitIntoWords(const string& text) {
    vector<string> words;
    string word;
    for (const char& c : text) {
        if (c == ' ') {
            if (!word.empty()) {
                words.push_back(word);
                word.clear();
            }
        }
        else {
            word += c;
        }
    }
    if (!word.empty()) {
        words.push_back(word);
    }
    return words;
}

struct Document {
    int id;
    double relevance;
};

class SearchServer {
public:
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    void AddDocument(int document_id, const string& document) {
        const vector<string> document_words = SplitIntoWordsNoStop(document);

        ++document_count_;

        double tf_component = 1./static_cast<double>(document_words.size());

        for (const string& word : document_words) {
            word_to_documents_tf_[word][document_id] += tf_component;
        }
    }

    vector<Document> FindTopDocuments(const string& raw_query) const {
        const Query query_words = ParseQuery(raw_query);

        auto matched_documents = FindAllDocuments(query_words);

        sort(matched_documents.begin(), matched_documents.end(), 
                [](const Document& lhs, const Document& rhs) {
            return lhs.relevance > rhs.relevance;
        });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

private:
    struct DocumentContent {
        int id = 0;
        vector<string> words;
    };

    struct Query {
        set<string> plus_words;
        set<string> minus_words;
    };

    map<string, map<int,double>> word_to_documents_tf_;
    set<string> stop_words_;
    int document_count_ = 0;

    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;

        for (const string& word : SplitIntoWords(text)) {
            if (stop_words_.count(word) == 0) {
                words.push_back(word);
            }
        }
        return words;
    }

    // Возвращает запрос без стоп-слов
    Query ParseQuery(const string& text) const {
        Query query;

        for (const string& word : SplitIntoWordsNoStop(text)) {
            if (word[0] == '-'){
                query.minus_words.insert(word.substr(1));
            }
            else {
                query.plus_words.insert(word);
            }
        }
        return query;
    }

    double FindAllDocumentsWithWord(const string& word) const {
        if(word_to_documents_tf_.count(word) == 0){
            return 0;
        }
        return static_cast<double>(word_to_documents_tf_.at(word).size());
    }

    double CalculateIdf(const string& word) const {
        return log(static_cast<double>(document_count_)/FindAllDocumentsWithWord(word));
    }

    // Возвращает вектор "пар" id и релевантность для подходящих документов
    vector<Document> FindAllDocuments(const Query& query_words) const {
        map<int, double> documents_id_relevence;

        // Рассчёт релевантности
        for (const auto& query_word : query_words.plus_words) {
            if (word_to_documents_tf_.count(query_word) != 0){
                double idf = CalculateIdf(query_word);

                for (const auto& [id, tf] : word_to_documents_tf_.at(query_word)){
                    documents_id_relevence[id] += idf * tf;
                }
            }
        }

        // Исключение всех документов с минус-словами
        for (const auto& query_word : query_words.minus_words){
            if (word_to_documents_tf_.count(query_word) != 0){
                    for (const auto& [id, relevance] : word_to_documents_tf_.at(query_word)){
                    documents_id_relevence.erase(id);
                }
            }
        }

        vector<Document> matched_documents;
        // Копирование в результирующий вектор
        for (const auto& [id, relevance] : documents_id_relevence){
            matched_documents.push_back({id, relevance});
        }
        return matched_documents;
    }

};

SearchServer CreateSearchServer() {
    SearchServer server;
    const string stop_words_joined = ReadLine();
    server.SetStopWords(stop_words_joined);

    const int document_count = ReadLineWithNumber();
    for (int document_id = 0; document_id < document_count; ++document_id) {
        server.AddDocument(document_id, ReadLine());
    }
    return server;
}

int main() {
    const SearchServer server = CreateSearchServer();

    const string query = ReadLine();
    for (const auto [document_id, relevance] : server.FindTopDocuments(query)) {
        cout << "{ document_id = "s << document_id << ", relevance = "s << relevance << " }"s
             << endl;
    }
}