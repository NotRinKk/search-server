#include "document.h"

Document::Document() = default;

Document::Document(int doc_id, double doc_relevance, int doc_rating)
    : id(doc_id)
    , relevance(doc_relevance)
    , rating(doc_rating) {
}

std::ostream& operator<<(std::ostream& os, const Document& doc) {
    os << "{ document_id = " << doc.id 
       << ", relevance = " << doc.relevance 
       << ", rating = " << doc.rating << " }";
    return os;
}
