#pragma once
// Minimal ZIP reader (stored + deflate entries) used to read .xlsx files.
// It replaces Qt's private QZipReader so the project only needs public Qt modules.
// No external dependencies: the inflate code below follows the classic "puff" algorithm.

#include <cstddef>
#include <cstdint>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace ziplite {

// ----------------------------- inflate (RFC 1951) -----------------------------
namespace detail {

struct State {
    const uint8_t* in;
    size_t inLen;
    size_t inPos = 0;
    long bitBuf = 0;
    int bitCnt = 0;
    std::vector<uint8_t>* out;
};

struct Huffman {
    short count[16];
    short symbol[288];
};

inline int bits(State& s, int need) {
    long val = s.bitBuf;
    while (s.bitCnt < need) {
        if (s.inPos >= s.inLen) throw std::runtime_error("unexpected end of data");
        val |= static_cast<long>(s.in[s.inPos++]) << s.bitCnt;
        s.bitCnt += 8;
    }
    s.bitBuf = val >> need;
    s.bitCnt -= need;
    return static_cast<int>(val & ((1L << need) - 1));
}

inline int decode(State& s, const Huffman& h) {
    int code = 0, first = 0, index = 0;
    for (int len = 1; len <= 15; ++len) {
        code |= bits(s, 1);
        const int count = h.count[len];
        if (code - count < first) return h.symbol[index + (code - first)];
        index += count;
        first += count;
        first <<= 1;
        code <<= 1;
    }
    throw std::runtime_error("bad huffman code");
}

inline int construct(Huffman& h, const short* length, int n) {
    for (int len = 0; len <= 15; ++len) h.count[len] = 0;
    for (int sym = 0; sym < n; ++sym) h.count[length[sym]]++;
    if (h.count[0] == n) return 0;
    int left = 1;
    for (int len = 1; len <= 15; ++len) {
        left <<= 1;
        left -= h.count[len];
        if (left < 0) return left;
    }
    short offs[16];
    offs[1] = 0;
    for (int len = 1; len < 15; ++len) offs[len + 1] = static_cast<short>(offs[len] + h.count[len]);
    for (int sym = 0; sym < n; ++sym)
        if (length[sym] != 0) h.symbol[offs[length[sym]]++] = static_cast<short>(sym);
    return left;
}

inline void codes(State& s, const Huffman& lencode, const Huffman& distcode) {
    static const short lens[29] = {3, 4, 5, 6, 7, 8, 9, 10, 11, 13, 15, 17, 19, 23, 27,
                                   31, 35, 43, 51, 59, 67, 83, 99, 115, 131, 163, 195, 227, 258};
    static const short lext[29] = {0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 2, 2, 2,
                                   2, 3, 3, 3, 3, 4, 4, 4, 4, 5, 5, 5, 5, 0};
    static const short dists[30] = {1, 2, 3, 4, 5, 7, 9, 13, 17, 25, 33, 49, 65, 97, 129,
                                    193, 257, 385, 513, 769, 1025, 1537, 2049, 3073, 4097,
                                    6145, 8193, 12289, 16385, 24577};
    static const short dext[30] = {0, 0, 0, 0, 1, 1, 2, 2, 3, 3, 4, 4, 5, 5, 6,
                                   6, 7, 7, 8, 8, 9, 9, 10, 10, 11, 11, 12, 12, 13, 13};
    for (;;) {
        int symbol = decode(s, lencode);
        if (symbol < 256) {
            s.out->push_back(static_cast<uint8_t>(symbol));
        } else if (symbol == 256) {
            return;
        } else {
            symbol -= 257;
            if (symbol >= 29) throw std::runtime_error("bad length symbol");
            const int len = lens[symbol] + bits(s, lext[symbol]);
            symbol = decode(s, distcode);
            if (symbol >= 30) throw std::runtime_error("bad distance symbol");
            const size_t dist = static_cast<size_t>(dists[symbol] + bits(s, dext[symbol]));
            if (dist > s.out->size()) throw std::runtime_error("distance too far back");
            for (int i = 0; i < len; ++i) s.out->push_back((*s.out)[s.out->size() - dist]);
        }
    }
}

inline void stored(State& s) {
    s.bitBuf = 0;
    s.bitCnt = 0;
    if (s.inPos + 4 > s.inLen) throw std::runtime_error("truncated stored block");
    const unsigned len = s.in[s.inPos] | (s.in[s.inPos + 1] << 8);
    const unsigned nlen = s.in[s.inPos + 2] | (s.in[s.inPos + 3] << 8);
    s.inPos += 4;
    if (len != (~nlen & 0xFFFF)) throw std::runtime_error("bad stored block");
    if (s.inPos + len > s.inLen) throw std::runtime_error("truncated stored block");
    s.out->insert(s.out->end(), s.in + s.inPos, s.in + s.inPos + len);
    s.inPos += len;
}

inline void fixedBlock(State& s) {
    static bool built = false;
    static Huffman lencode, distcode;
    if (!built) {
        short lengths[288];
        int sym = 0;
        for (; sym < 144; ++sym) lengths[sym] = 8;
        for (; sym < 256; ++sym) lengths[sym] = 9;
        for (; sym < 280; ++sym) lengths[sym] = 7;
        for (; sym < 288; ++sym) lengths[sym] = 8;
        construct(lencode, lengths, 288);
        for (sym = 0; sym < 30; ++sym) lengths[sym] = 5;
        construct(distcode, lengths, 30);
        built = true;
    }
    codes(s, lencode, distcode);
}

inline void dynamicBlock(State& s) {
    static const short order[19] = {16, 17, 18, 0, 8, 7, 9, 6, 10, 5, 11, 4, 12, 3, 13, 2, 14, 1, 15};
    const int nlen = bits(s, 5) + 257;
    const int ndist = bits(s, 5) + 1;
    const int ncode = bits(s, 4) + 4;
    if (nlen > 286 || ndist > 30) throw std::runtime_error("bad dynamic block counts");

    short lengths[320];
    int index = 0;
    for (; index < ncode; ++index) lengths[order[index]] = static_cast<short>(bits(s, 3));
    for (; index < 19; ++index) lengths[order[index]] = 0;

    Huffman lencode, distcode;
    if (construct(lencode, lengths, 19) != 0) throw std::runtime_error("bad code lengths");

    index = 0;
    while (index < nlen + ndist) {
        int symbol = decode(s, lencode);
        if (symbol < 16) {
            lengths[index++] = static_cast<short>(symbol);
        } else {
            int len = 0;
            int repeat;
            if (symbol == 16) {
                if (index == 0) throw std::runtime_error("no previous length");
                len = lengths[index - 1];
                repeat = 3 + bits(s, 2);
            } else if (symbol == 17) {
                repeat = 3 + bits(s, 3);
            } else {
                repeat = 11 + bits(s, 7);
            }
            if (index + repeat > nlen + ndist) throw std::runtime_error("too many lengths");
            while (repeat--) lengths[index++] = static_cast<short>(len);
        }
    }
    if (lengths[256] == 0) throw std::runtime_error("missing end-of-block");

    if (construct(lencode, lengths, nlen) < 0) throw std::runtime_error("bad literal table");
    if (construct(distcode, lengths + nlen, ndist) < 0) throw std::runtime_error("bad distance table");
    codes(s, lencode, distcode);
}

}  // namespace detail

inline bool inflateRaw(const uint8_t* src, size_t srcLen, std::vector<uint8_t>& out) {
    try {
        detail::State s;
        s.in = src;
        s.inLen = srcLen;
        s.out = &out;
        int last;
        do {
            last = detail::bits(s, 1);
            const int type = detail::bits(s, 2);
            if (type == 0)
                detail::stored(s);
            else if (type == 1)
                detail::fixedBlock(s);
            else if (type == 2)
                detail::dynamicBlock(s);
            else
                return false;
        } while (!last);
        return true;
    } catch (...) {
        return false;
    }
}

// --------------------------------- ZIP archive ---------------------------------
class Archive {
public:
    struct Entry {
        std::string name;
        uint16_t method = 0;
        uint32_t compressedSize = 0;
        uint32_t size = 0;
        uint32_t offset = 0;
    };

    bool open(std::vector<uint8_t> data) {
        data_ = std::move(data);
        entries_.clear();
        const size_t n = data_.size();
        if (n < 22) return false;

        // locate the end-of-central-directory record
        const size_t minPos = n > 22 + 65535 ? n - 22 - 65535 : 0;
        size_t eocd = n;
        for (size_t i = n - 22;; --i) {
            if (u32(i) == 0x06054b50) {
                eocd = i;
                break;
            }
            if (i == minPos) break;
        }
        if (eocd == n) return false;

        const unsigned total = u16(eocd + 10);
        size_t pos = u32(eocd + 16);
        for (unsigned k = 0; k < total; ++k) {
            if (pos + 46 > n || u32(pos) != 0x02014b50) return false;
            Entry e;
            e.method = u16(pos + 10);
            e.compressedSize = u32(pos + 20);
            e.size = u32(pos + 24);
            const size_t nameLen = u16(pos + 28);
            const size_t extraLen = u16(pos + 30);
            const size_t commentLen = u16(pos + 32);
            e.offset = u32(pos + 42);
            if (pos + 46 + nameLen > n) return false;
            e.name.assign(reinterpret_cast<const char*>(&data_[pos + 46]), nameLen);
            entries_.push_back(e);
            pos += 46 + nameLen + extraLen + commentLen;
        }
        return true;
    }

    const std::vector<Entry>& entries() const { return entries_; }

    bool read(const std::string& name, std::vector<uint8_t>& out) const {
        out.clear();
        for (const Entry& e : entries_) {
            if (e.name != name) continue;
            const size_t lh = e.offset;
            if (u32(lh) != 0x04034b50) return false;
            const size_t start = lh + 30 + u16(lh + 26) + u16(lh + 28);
            if (start + e.compressedSize > data_.size()) return false;
            if (e.method == 0) {
                out.assign(data_.begin() + start, data_.begin() + start + e.compressedSize);
                return true;
            }
            if (e.method == 8) {
                out.reserve(e.size);
                return inflateRaw(data_.data() + start, e.compressedSize, out);
            }
            return false;   // other compression methods are not supported
        }
        return false;
    }

private:
    unsigned u16(size_t p) const {
        if (p + 2 > data_.size()) return 0;
        return data_[p] | (data_[p + 1] << 8);
    }
    uint32_t u32(size_t p) const {
        if (p + 4 > data_.size()) return 0;
        return static_cast<uint32_t>(data_[p]) | (static_cast<uint32_t>(data_[p + 1]) << 8) |
               (static_cast<uint32_t>(data_[p + 2]) << 16) | (static_cast<uint32_t>(data_[p + 3]) << 24);
    }

    std::vector<uint8_t> data_;
    std::vector<Entry> entries_;
};

}  // namespace ziplite

// ------------------- Qt-facing wrapper with the same API as Qt's private QZipReader -------------------
#if __has_include(<QByteArray>)
#include <QByteArray>
#include <QFile>
#include <QList>
#include <QString>

class QZipReader {
public:
    struct FileInfo {
        QString filePath;
    };

    explicit QZipReader(const QString& path) {
        QFile file(path);
        if (!file.open(QIODevice::ReadOnly)) return;
        const QByteArray raw = file.readAll();
        std::vector<uint8_t> data(raw.begin(), raw.end());
        readable_ = archive_.open(std::move(data));
    }

    bool isReadable() const { return readable_; }
    void close() {}

    QByteArray fileData(const QString& name) const {
        std::vector<uint8_t> out;
        if (!archive_.read(name.toStdString(), out)) return QByteArray();
        return QByteArray(reinterpret_cast<const char*>(out.data()), static_cast<qsizetype>(out.size()));
    }

    QList<FileInfo> fileInfoList() const {
        QList<FileInfo> list;
        for (const auto& e : archive_.entries()) list.append({QString::fromStdString(e.name)});
        return list;
    }

private:
    ziplite::Archive archive_;
    bool readable_ = false;
};
#endif
