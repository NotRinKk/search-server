#include <algorithm>
#include <cmath>
#include <iostream>
#include <numeric>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

using namespace std;

const int MAX_RESULT_DOCUMENT_COUNT = 5;
const double EPSILON = 1e-6;

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

// Возвращает разделенный на слова текст
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

enum class DocumentStatus {
    ACTUAL,
    IRRELEVANT,
    BANNED,
    REMOVED,
};

struct Document {
    int id;
    double relevance;
    int rating;
};

class SearchServer {
public:
    // Добавляет во множество раздельных стоп-слов
    void SetStopWords(const string& text) {
        for (const string& word : SplitIntoWords(text)) {
            stop_words_.insert(word);
        }
    }

    int GetDocumentCount() const {
        return documents_.size();
    }

    void AddDocument(int document_id, const string& document, DocumentStatus status,
                     const vector<int>& ratings) {
        // Увеличение общего кол-ва документов
        const vector<string> document_words = SplitIntoWordsNoStop(document);

        // Рассчёт TF для каждого слова документа
        double tf_component = 1./static_cast<double>(document_words.size());
        for (const string& word : document_words) {

            // Добавление в инвертированный индекс
            word_to_documents_TF_[word][document_id] += tf_component;
        }

        int avg_rating = ComputeAverageRating(ratings);
        documents_.emplace(document_id, DocumentData{avg_rating, status});
    }

    // Возвращает самые релевантные документы в виде вектора документов {id, релевантность, рейтинг}
    template <typename Filter>
    vector<Document> FindTopDocuments(const string& raw_query,
                                      Filter filter) const {
        const Query query = ParseQuery(raw_query);
        auto matched_documents = FindAllDocuments(query, filter);
        
        sort(matched_documents.begin(), matched_documents.end(),
             [](const Document& lhs, const Document& rhs) {
                 if (abs(lhs.relevance - rhs.relevance) < EPSILON) {
                     return lhs.rating > rhs.rating;
                 }
				 return lhs.relevance > rhs.relevance;
             });

        if (matched_documents.size() > MAX_RESULT_DOCUMENT_COUNT) {
            matched_documents.resize(MAX_RESULT_DOCUMENT_COUNT);
        }
        return matched_documents;
    }

    vector<Document> FindTopDocuments(const string& raw_query, DocumentStatus status) const {
        auto filter = [status](int, DocumentStatus doc_status, int) { return doc_status == status;};
        return FindTopDocuments(raw_query, filter);
    }     

    vector<Document> FindTopDocuments(const string& raw_query) const {
        return FindTopDocuments(raw_query, DocumentStatus::ACTUAL);
    }

    tuple<vector<string>, DocumentStatus> MatchDocument(const string& raw_query,
                                                        int document_id) const {
        const Query query = ParseQuery(raw_query);
        vector<string> matched_words;
        for (const string& word : query.plus_words) {
            if (word_to_documents_TF_.count(word) == 0) {
                continue;
            }
            if (word_to_documents_TF_.at(word).count(document_id)) {
                matched_words.push_back(word);
            }
        }
        for (const string& word : query.minus_words) {
            if (word_to_documents_TF_.count(word) == 0) {
                continue;
            }
            if (word_to_documents_TF_.at(word).count(document_id)) {
                matched_words.clear();
                break;
            }
        }
        return {matched_words, documents_.at(document_id).status};
    }

private:
    struct DocumentData {
        int rating;
        DocumentStatus status;
    };

    struct Query {
    set<string> plus_words;
    set<string> minus_words;
    };

    // Словарь слов документов {слово, {индекс, частота}}
    map<string, map<int,double>> word_to_documents_TF_;
    set<string> stop_words_;
    map<int, DocumentData> documents_;

     bool IsStopWord(const string& word) const {
        return stop_words_.count(word) > 0;
    }
    
    // Возвращает разделённый на слова текст без стоп-слов
    vector<string> SplitIntoWordsNoStop(const string& text) const {
        vector<string> words;
        for (const string& word : SplitIntoWords(text)) {
            if (!IsStopWord(word)) {
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

        // Возвращает кол-во документов с заданным словом
    double FindAllDocumentsWithWord(const string& word) const {
        if(word_to_documents_TF_.count(word) == 0){
            return 0;
        }
        return static_cast<double>(word_to_documents_TF_.at(word).size());
    }

    double CalculateIdf(const string& word) const {
        return log(GetDocumentCount() * 1.0/FindAllDocumentsWithWord(word));
    }

    static int ComputeAverageRating(const vector<int>& ratings) {
        if (ratings.size() == 0){
            return 0;
        }
		
        return accumulate(ratings.begin(), ratings.end(), 0) / static_cast<int>(ratings.size());
	}
    
    // Для каждого найденного документа возвращает его релевантность и id (пару)
    template <typename Filter>
    vector<Document> FindAllDocuments(const Query& query, Filter filter) const {

        map<int, double> documents_id_relevence;

        // Рассчёт релевантности слов -> документа
        for (const auto& query_word : query.plus_words) {
            if (word_to_documents_TF_.count(query_word) != 0){

                const double idf = CalculateIdf(query_word);

                for (const auto& [id, tf] : word_to_documents_TF_.at(query_word)){
                    auto document_data = documents_.at(id);
                    if (filter(id, document_data.status, document_data.rating)){
                        documents_id_relevence[id] += idf * tf;
                    }
                }
            }    
        }
        
        // Исключение всех документов с минус-словами
        for (const auto& query_word : query.minus_words){
            if (word_to_documents_TF_.count(query_word) != 0){
                    for (const auto& [id, relevance] : word_to_documents_TF_.at(query_word)){
                    documents_id_relevence.erase(id);
                }
            }
        }

        // Копирование в результирующий вектор
        vector<Document> matched_documents;
        for (const auto& [id, relevance] : documents_id_relevence){
            matched_documents.push_back(
                {id, relevance, documents_.at(id).rating});
        }
        return matched_documents;
    }

};