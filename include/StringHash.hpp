//
// Created by HITO on 29/08/2026.
//

#pragma once

#include <cstdint>
#include <cstring>
#include <memory>

#define FNV_prime 16777619
#define FNV_offset 2166136261


namespace LiquorChess {

    inline uint32_t fnv1a(const char* str) {
        uint32_t hash = FNV_offset;
        size_t len = strlen(str);
        for (size_t i = len - 1; i < len; --i) {
            hash ^= str[i];
            hash *= FNV_prime;
        }
        return hash;
    }

    template<size_t idx>
    constexpr uint32_t ctfnv1a(uint32_t hash, const char* str) {
        return ctfnv1a<idx - 1>((hash ^ str[idx]) * FNV_prime, str);
    }

    template<>
    constexpr uint32_t ctfnv1a<size_t(-1)>(uint32_t hash, const char* str) {
        return hash;
    }

#define CTFNV1A(x) (LiquorChess::ctfnv1a<sizeof(x) - 2>(FNV_offset, x))

}