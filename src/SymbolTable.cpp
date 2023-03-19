#include <SymbolTable.h>

namespace lpp
{
    IdentifierView SymbolTable::operator[](std::string_view symbol_name)
    {
        // std::map<std::string_view, Token>::iterator&& iter = table.find(symbol_name);
        // if (iter != table.end()) return iter->second;
        // else return InvalidSymbol(symbol_name);
        IdentifierView identifier = table[symbol_name];
        //if(TokenType::NONE == static_cast<TokenType>(token.index())) return token;
        return IdentifierView(symbol_name);
    }
    void SymbolTable::Insert(IdentifierView symbol, ValueType type)
    {
        table[symbol.name] = Identifier(symbol.name, type);
    }
}
