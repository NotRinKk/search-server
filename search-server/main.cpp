#include "document.h"
#include "paginator.h"
#include "search_server.h"
#include "request_queue.h"
#include "read_input_functions.h"
#include <iostream>

using namespace std;

int main() {
    SearchServer search_server("and with"s);
    RequestQueue request_queue(search_server);

    search_server.AddDocument(1, "funny pet and nasty rat"s, DocumentStatus::ACTUAL, {7, 2, 7});
    search_server.AddDocument(2, "funny pet with curly hair"s, DocumentStatus::ACTUAL, {1, 2, 3});
    search_server.AddDocument(3, "big cat nasty hair"s, DocumentStatus::ACTUAL, {1, 2, 8});
    search_server.AddDocument(4, "big dog cat Vladislav"s, DocumentStatus::ACTUAL, {1, 3, 2});
    search_server.AddDocument(5, "big dog hamster Borya"s, DocumentStatus::ACTUAL, {1, 1, 1});
    
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;
    // 1439 запросов с нулевым результатом
    for (int i = 0; i < 1439; ++i) {
        request_queue.AddFindRequest("empty request"s);
    }

    // все еще 1439 запросов с нулевым результатом
    request_queue.AddFindRequest("Vladislav"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // новые сутки, первый запрос удален, 1438 запросов с нулевым результатом
    request_queue.AddFindRequest("funny pet"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    // первый запрос удален, 1437 запросов с нулевым результатом
    request_queue.AddFindRequest("hamster"s);
    cout << "Total empty requests: "s << request_queue.GetNoResultRequests() << endl;

    const auto search_results = search_server.FindTopDocuments("curly dog"s);
    int page_size = 2;
    const auto pages = Paginate(search_results, page_size);
    // Выводим найденные документы по страницам
    for (auto page = pages.begin(); page != pages.end(); ++page) {
        cout << *page << endl;
        cout << "Page break"s << endl;
    }
}