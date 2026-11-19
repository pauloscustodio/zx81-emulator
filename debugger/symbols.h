//-----------------------------------------------------------------------------
// zx81-emulator
// Copyright (C) Paulo Custodio, 2026
// License: GPL v2
//-----------------------------------------------------------------------------

#pragma once

#include <string>
#include <unordered_map>
#include <vector>

struct Symbol {
    std::string name;
    int value;
    std::string source_file;
    int line_num;
};

class SymbolTable {
public:
    explicit SymbolTable();

    // Add a symbol. Returns false if its name already exists.
    bool add(std::string name, int value,
             std::string source_file = {},
             int line_num = 0);

    // Retrieve a symbol by name.
    // Returns nullptr if not found.
    Symbol* get(const std::string& name);
    const Symbol* get(const std::string& name) const;

    // Retrieve all symbols associated with a value.
    std::vector<Symbol*> get(int value);
    std::vector<const Symbol*> get(int value) const;

private:
    std::vector<Symbol> symbols;
    std::unordered_map<std::string, size_t> by_name;
    std::unordered_multimap<int, size_t> by_value;
};
