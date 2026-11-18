//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#include "consts.h"
#include "symbols.h"
#include <vector>

SymbolTable::SymbolTable() {
    for (const auto& s : const_symbols) {
        add(s.name, s.value, s.source_file, s.line_num);
    }
}

bool SymbolTable::add(std::string name, int value,
                      std::string source_file, int line_num) {
    size_t id = symbols.size();

    auto [it, inserted] = by_name.emplace(name, id);

    if (!inserted) {
        return false;    // Duplicate symbol name
    }

    symbols.push_back({std::move(name), value, source_file, line_num});
    by_value.emplace(value, id);

    return true;
}

Symbol* SymbolTable::get(const std::string& name) {
    return const_cast<Symbol*>(
               static_cast<const SymbolTable&>(*this).get(name)
           );
}

const Symbol* SymbolTable::get(const std::string& name) const {
    auto it = by_name.find(name);
    if (it == by_name.end()) {
        return nullptr;
    }
    return &symbols[it->second];
}

std::vector<Symbol*> SymbolTable::get(int value) {
    std::vector<Symbol*> result;
    auto [first, last] = by_value.equal_range(value);
    for (auto it = first; it != last; ++it) {
        result.push_back(&symbols[it->second]);
    }
    return result;
}

std::vector<const Symbol*> SymbolTable::get(int value) const {
    std::vector<const Symbol*> result;
    auto [first, last] = by_value.equal_range(value);
    for (auto it = first; it != last; ++it) {
        result.push_back(&symbols[it->second]);
    }
    return result;
}

