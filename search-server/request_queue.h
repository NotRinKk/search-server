#pragma once
#include "search_server.h"
#include "document.h"
#include <cstdint>
#include <deque>
#include <vector>
#include <string>

class RequestQueue {
public:
    explicit RequestQueue(const SearchServer& search_server) 
        : search_server_(search_server) 
        , current_time_(0)
        , empty_requests_(0) {
    }

    template <typename DocumentPredicate>
	std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate);	

    std::vector<Document> AddFindRequest(const std::string& raw_query, DocumentStatus status);

    std::vector<Document> AddFindRequest(const std::string& raw_query);

    int GetNoResultRequests() const;

private:
    struct QueryResult {
        uint64_t time;
        int found_docs_size;
    };

    std::deque<QueryResult> requests_;
    const SearchServer& search_server_;
    const static int min_in_day_ = 1440;
    uint64_t current_time_;
    int empty_requests_;

    void AddRequest(int result_size);
};


template <typename DocumentPredicate>
std::vector<Document> RequestQueue::AddFindRequest(const std::string& raw_query, DocumentPredicate document_predicate) {
    const auto result = search_server_.FindTopDocuments(raw_query, document_predicate);
    RequestQueue::AddRequest(result.size());
    return result;
}