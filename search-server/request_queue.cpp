#include "request_queue.h"

using namespace std;

vector<Document> RequestQueue::AddFindRequest(const string& raw_query, DocumentStatus status) {
    const auto result = search_server_.FindTopDocuments(raw_query, status);
    AddRequest(result.size());
    return result;
}

vector<Document> RequestQueue::AddFindRequest(const string& raw_query) {
    const auto result = search_server_.FindTopDocuments(raw_query);
    AddRequest(result.size());
    return result;
}

int RequestQueue::GetNoResultRequests() const {
    return empty_requests_;
}

void RequestQueue::AddRequest(int result_size) {
    // Увеличить время на одну минуту (запросы приходят раз в минуту)
    ++current_time_;
    // Выход из очереди запросов устаревших запросов
    if(!requests_.empty() && (current_time_ - requests_.front().time >= min_in_day_)) {
        /* Во время удаления уменьшить количество запросов с пустым вектором ответов,
        если выходит из очереди запросов запрос с пустым вектором ответов  */
        if (requests_.front().found_docs_size ==  0) {
            --empty_requests_;
        }
        requests_.pop_front();
    }

    // Добавление нового запроса в дек и обновление количества запросов без результатов поиска
    if (result_size == 0) {
        ++empty_requests_;
    }
    requests_.push_back({current_time_, result_size});
}