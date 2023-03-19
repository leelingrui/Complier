#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <SymbolTable.h>
#include <Lexer.h>
#include <Common.h>
#include <string>
#include <llvm/ADT/SmallString.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <list>
#include <TypeSymbolTable.h>

namespace lpp
{
    class TokenObjCmp
    {
    public:
        bool operator()(const TokenObj& lhs, const TokenObj& rhs) const;
    };
    class Parser final : public DFA
    {
    public: 
        using parser_input = std::variant<llvm::Value*, llvm::Type*, llvm::Function*, Token>;
        enum ParserInputType
        {
            value, type, function, token
        };
        Parser();
        Parser(std::string_view source_file);
        void init_node(status strat, const parser_input& status_change, DFA_STATUS_CHANGE_NODE end);
        void next_status(parser_input next);
    private:
        static llvm::LLVMContext context;
        std::vector<std::map<parser_input, status>> goto_table;
        size_t exprs;
        std::deque<parser_input> process_stack;
        std::stack<parser_input> symbol_stack;
        void reduce_operatorbrace();
        void reduce_rbraces();
        void reduce_expr();
        std::map<status, std::map<parser_input, DFA_STATUS_CHANGE_NODE>> status_change_map;
        std::unique_ptr<StructTypeFinder> struct_finder;
        llvm::IRBuilder<> builder;
        Lexer lexer;
        Token except_token;
        SymbolTable symbol_table;
        std::unordered_map<TokenObj, unsigned char, TokenObjHash, TokenObjCmp> priority_map = {
            { Punchation('['), 1 },
            { Punchation(']'), 1 },
            { Punchation('('), 1 },
            { Punchation(')'), 1 },
            { Punchation::POINTERTO, 1 },
            { Punchation('.'), 1 },
            { Punchation::DEREF, 2 },
            { Punchation('&'), 2 },
            { Punchation('!'), 2 },
            { Punchation('~'), 2 },
            { Punchation('/'), 3 },
            { Punchation::MUL, 3 },
            { Punchation('%'), 3 },
            { Punchation('+'), 4 },
            { Punchation('-'), 4 },
            { Punchation::LSHIFT, 5 },
            { Punchation::RSHIFT, 5 },
            { Punchation('>'), 6 },
            { Punchation::LARGEEQ, 6 },
            { Punchation('<'), 6 },
            { Punchation::LESSEQ, 6 },
            { Punchation::EQU, 7 },
            { Punchation::NOTEQU, 7 },
            { Punchation::BITAND, 8 },
            { Punchation::BITXOR, 9 },
            { Punchation::BITOR, 10 },
            { Punchation::ADD, 11 },
            { Punchation::OR, 12 },
            { Punchation('?'), 13 },
            { Punchation('!'), 13 },
            { Punchation('='), 14 },
            { Punchation::DIVASSIGN, 14 },
            { Punchation::MULASSIGN, 14 },
            { Punchation::MODASSIGN, 14 },
            { Punchation::ADDASSIGN, 14 },
            { Punchation::SUBASSING, 14 },
            { Punchation::LSHIFTASSIGN, 14 },
            { Punchation::RSHIFTASSIGN, 14 },
            { Punchation::BITANDASSIGN, 14 },
            { Punchation::BITXORASSIGN, 14 },
            { Punchation::BITORASSIGN, 14 },
            { Punchation(','), 15 },
            { KeyWord::AS, 1 }
        };
    };

    class NameSpaceManager
    {
    public:
        void goback();
        void insert(std::string space);
        void insert_tmp(std::string space);
        void clear_tmp();
        std::string get_name();
    private:
        std::list<std::string> current_namespace, tmp_namespace;
    }

};

#endif