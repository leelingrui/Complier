#ifndef SYMBOLTABLE_H
#define SYMBOLTABLE_H
#include <Token.h>
#include <map>

namespace lpp
{
    class SymbolTable
    {
    public:
        [[nodiscard]]IdentifierView operator[](std::string_view symbol_name);
        void Insert(IdentifierView symbol, ValueType type);
    private:
        std::map<std::string_view, Identifier> table;
    };
};



#endif