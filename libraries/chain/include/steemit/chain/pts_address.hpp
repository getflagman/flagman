#pragma once

#include <fc/array.hpp>
#include <string>

namespace fc {
    namespace ecc {
        class public_key;
    }
}

namespace steemit {
    namespace chain {

        /**
         *  Implements address stringification and validation from PTS
         */
        struct pts_address {
            pts_address(); ///< constructs empty / null address
            pts_address(const std::string &base58str);   ///< converts to binary, validates checksum
            pts_address(const fc::ecc::public_key &pub, bool compressed = true, uint8_t version = 56); ///< converts to binary

            uint8_t version() const {
                return addr.at(0);
            }

            bool is_valid() const;

            operator std::string() const; ///< converts to base58 + checksum

            fc::array<char, 25> addr; ///< binary representation of address
        };

        inline bool operator==(const pts_address &a, const pts_address &b) {
            return a.addr == b.addr;
        }

        inline bool operator!=(const pts_address &a, const pts_address &b) {
            return a.addr != b.addr;
        }

        inline bool operator<(const pts_address &a, const pts_address &b) {
            return a.addr < b.addr;
        }

    }
}

namespace std {
    template<>
    struct hash<graphene::chain::pts_address> {
    public:
        size_t operator()(const graphene::chain::pts_address &a) const {
            size_t s;
            memcpy((char *)&s, &a.addr.data[sizeof(a) - sizeof(s)], sizeof(s));
            return s;
        }
    };
}

#include <fc/reflect/reflect.hpp>

FC_REFLECT(steemit::chain::pts_address, (addr))

namespace fc {
    void to_variant(const steemit::chain::pts_address &var, fc::variant &vo);

    void from_variant(const fc::variant &var, steemit::chain::pts_address &vo);
}