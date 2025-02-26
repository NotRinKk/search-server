#pragma once
#include <cstdint>
#include <sstream>
#include <vector>

// Страница вывода (диапазон)
template <typename Iterator>
class IteratorRange{
public:
    IteratorRange(Iterator first, Iterator last)
        : first_(first), last_(last) {
     }

    Iterator begin() const {
        return first_;
    }

    Iterator end() const {
        return last_;
    }

    int size() const {
        return distance(first_, last_);
    }

private:
    Iterator first_;
    Iterator last_;
};

template <typename Iterator>
class Paginator {
public:
    // Разбивает результаты поиска (диапазон) на страницы
     Paginator(Iterator it_begin, Iterator it_end, size_t page_size) {
        for (auto it = it_begin; it != it_end;) {
            auto next = it;  // Сохраняем текущее значение итератора it в next
            advance(next, std::min(page_size, static_cast<size_t>(distance(it, it_end))));
            page_range_.emplace_back(it, next);  // Добавляем новый диапазон в page_range_
            it = next;  // Обновляем it до next для следующей итерации
        }
    }

    auto begin() const {
        return page_range_.begin();
    }

    auto end() const {
        return page_range_.end();
    }

private:
    // вектор страниц (пар итераторов на начало и конец страниц)
    std::vector<IteratorRange<Iterator>> page_range_;
};

template <typename Container>
auto Paginate(const Container& c, size_t page_size) {
    return Paginator(begin(c), end(c), page_size);
}

// Оператор вывода для IteratorRange
template <typename Iterator>
std::ostream& operator<<(std::ostream& output, const IteratorRange<Iterator>& range) {
    for (auto it = range.begin(); it != range.end(); ++it) {
        output << *it;
    }
    return output;
}