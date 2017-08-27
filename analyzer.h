#ifndef ANALYZER_H
#define ANALYZER_H

#include <vector>
#include <unordered_set>
#include <limits>

class Analyzer {
private:
    struct Node {
        unsigned int x;
        unsigned int y;
        unsigned int way;
        char color;
    };

    size_t data_length;
    std::vector<Node> data;
    size_t pos;
    size_t size;
    bool enabled;
    bool autoAnalyzer;

public:
    Analyzer() {
        data = std::vector<Node>();
        enabled = false;
        pos = 0;
        data_length = 10000;
        size = 0;
    }

    void setDataLength(size_t l) {
        data_length = l;
    }

    void setEnabled(bool enabled) {
        this->enabled = enabled;
    }

    bool isEnabled() {
        return enabled;
    }

    void setAutoAnalyzer(bool enabled) {
        autoAnalyzer = enabled;
    }

    bool isAutoAnalyzerEnabled() {
        return autoAnalyzer;
    }

    Node operator [] (size_t i) {
        i %= data_length;
        if (pos > i) {
            return data[pos - 1 - i];
        }
        //qDebug(std::to_string(pos + data_length - 1 - i).c_str());
        return data[pos + data_length - 1 - i];
    }

    void add(unsigned int x, unsigned int y, unsigned int way, char color) {
        if (data.size() != data_length) {
            data.resize(data_length);
        }
        data[pos++] = Node{x, y, way, color};
        pos %= data_length;
        if (size < data_length) {
            ++size;
        }
    }

    void clear() {
        pos = 0;
        size = 0;
        data.clear();
    }

    void pop() {
        if (size > 0) {
            --size;
        }
    }

    bool eq(size_t i, size_t j) {
        return (*this)[i].color == (*this)[j].color && (*this)[i].way == (*this)[j].way;
    }

    bool special_checker(size_t k) {
        std::unordered_set<unsigned long long> set;
        for (size_t i = 0; i != k; ++i) {
            Node tmp = (*this)[i];
            if (tmp.color != 0) {
                set.insert(static_cast<unsigned long long>(tmp.x) * UINT_MAX
                           + static_cast<unsigned long long>(tmp.y));
            } else {
                set.erase(static_cast<unsigned long long>(tmp.x) * UINT_MAX
                          + static_cast<unsigned long long>(tmp.y));
            }
        }
        for (size_t i = k; i != 2 * k; ++i) {
            Node tmp = (*this)[i];
            set.erase(static_cast<unsigned long long>(tmp.x) * UINT_MAX
                      + static_cast<unsigned long long>(tmp.y));
        }
        if (set.size() == 0) {
            return true;
        }
        return false;
    }

    size_t analyze() {
        if (size <= 1) {
            return 0;
        }
        std::vector<uint> pref(size);
        pref[0] = 0;
        for (size_t i = 1; i != size; ++i) {
            uint k = pref[i - 1];
            while (k > 0 && !eq(i, k)) {
                k = pref[k-1];
            }
            if (eq(i, k)) {
                ++k;
            }
            pref[i] = k;
            if (i + 1 == k * 2) {
                if (special_checker(k)) {
                    return k;
                }
            }
        }
        return 0;
        // debug print
        qDebug("================");
        std::string s;
        for (auto elem : pref) {
            s += std::to_string(elem);
        }
        s += '\n';
        for (size_t i = 0; i != size; ++i) {
            Node tmp = (*this)[i];
            s += std::to_string(tmp.color) + " " + std::to_string(tmp.way) + "   ";
        }
        qDebug(s.c_str());
    }

    std::string statistic(size_t end, size_t colors_num) {
        std::vector<size_t> stat(colors_num);
        std::string s;
        for (size_t i = 0; i != end; ++i) {
            Node tmp = (*this)[i];
            ++stat[static_cast<size_t>(tmp.color)];
            s += std::to_string(tmp.color) + " ";
        }
        s += "\r\nNum: ";
        for (size_t i = 0; i != stat.size(); ++i) {
            s += std::to_string(stat[i]) + " ";
        }
        s += "\r\nDelta: ";
        unsigned int x, y;
        if ((*this)[0].x < (*this)[end].x) {
            x = (*this)[end].x - (*this)[0].x;
        } else {
            x = (*this)[0].x - (*this)[end].x;
        }
        if ((*this)[0].y < (*this)[end].y) {
            y = (*this)[end].y - (*this)[0].y;
        } else {
            y = (*this)[0].y - (*this)[end].y;
        }
        s += std::to_string(x) + " " + std::to_string(y);
        return s;
    }
};

#endif // ANALYZER_H
