#ifndef PARSER_H
#define PARSER_H

#include <memory>
#include <SymbolTable.h>
#include <Lexer.h>
#include <Common.h>
#include <string>
#include <fstream>
#include <llvm/ADT/SmallString.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <TypeSymbolTable.h>
#include <llvm/ADT/ArrayRef.h>
#include <list>

namespace lpp
{
    class BlockScopeNode;
    class NameSpaceManager;

    class BlockScopeManager final
    {
    public:
        BlockScopeManager();
        void enter_scope(llvm::StringRef Name);
        void enter_scope();
        void leave_scope();
        void insert_function(llvm::Module* m, std::string Name, llvm::Type* return_type, llvm::ArrayRef<llvm::Type*> Args, bool is_static = false, bool is_var_ags = false);
        const std::pair<llvm::Function*, bool> find_function(llvm::StringRef Name) const;
        BlockScopeNode* find_value(llvm::StringRef Name);
        llvm::ArrayRef<llvm::Value*> get_current_value_accept_seq();
        bool variable_define(llvm::StringRef Name, size_t _pointer_level);
        bool type_define(llvm::StringRef Name);
        void find_and_enter(llvm::StringRef Name);
        void clear_tmp_scope();
        std::string get_function_name();
    private:
        std::vector<llvm::Value*> element_pointer_seq;
        std::unique_ptr<BlockScopeNode> root_node;
        std::list<BlockScopeNode*> search_scope;
        BlockScopeNode* current_node, *tmp_node;
    };

    class BlockScopeNode
    {
    public:
        BlockScopeNode(llvm::StringRef Name, BlockScopeNode* _parent = nullptr, llvm::Value* _Value = nullptr, size_t _pointer_level = 0);
        const std::pair<llvm::Function*, bool> get_function(llvm::StringRef Name);
        BlockScopeNode* get_child_block(llvm::StringRef Name);
        void insert_function(llvm::StringRef Name, std::pair<llvm::Function*, bool> Func);
        bool insert_member_value(llvm::StringRef Name, llvm::Value* _Value, size_t _pointer_level);
        void set_parent(BlockScopeNode* _parent);
        void allocate(llvm::Value* value_ptr, llvm::Type* _type);
        inline void set_current_value(llvm::Value* _value, llvm::Type* _type)
        {
            current_value = _value;
            value_type = _type;
        }
        inline llvm::Type* get_type() { return value_type; };
        inline llvm::Value* get_address() { return current_value; };
        inline BlockScopeNode* get_parent() { return parent; };
        inline void set_type(llvm::Type* _type) { value_type = _type; };
        inline bool is_root() { return !name.size(); };
    private:
        llvm::Type* value_type;
        size_t pointer_level;
        llvm::Value* current_value,* StructPointerNo;
        std::string name;
        size_t name_no;
        BlockScopeNode* parent;
        llvm::StringMap<std::unique_ptr<BlockScopeNode>> childs;
        llvm::StringMap<std::pair<llvm::Function*, bool>> member_functions;
        friend class BlockScopeManager;
    };
    
    using FuncParam = struct FunctionParameters
    {
        llvm::SmallString<256> function_name;
        std::vector<llvm::Type*> arg_type;
        std::vector<llvm::SmallString<256>> arg_name;
        llvm::Type* return_type;
        void clear();
    };

    class TokenObjCmp
    {
    public:
        bool operator()(const TokenObj& lhs, const TokenObj& rhs) const;
    };
    class Parser final : public DFA
    {
    public: 
        class HashToken
        {
        public:
            size_t operator()(const TokenObj& _token) const;
        };
        using parser_input = std::variant<llvm::Value*, llvm::Type*, llvm::Function*, Token>;
        enum ParserInputType
        {
            value, type, function, token
        };
        Parser();
        Parser(std::istream* source_file);
        void parse();
        void init_node(status strat, const TokenObj& status_change, DFA_STATUS_CHANGE_NODE end);
        void init_all_node(status start, DFA_STATUS_CHANGE_NODE end);
        void init_all_keyword(status start, DFA_STATUS_CHANGE_NODE end);
        void init_all_punc(status start, DFA_STATUS_CHANGE_NODE end);
        void override_node(status start, const TokenObj& status_change, DFA_STATUS_CHANGE_NODE end);
    private:
        Lexer lexer;
        void next_status();
        llvm::LLVMContext context;
        llvm::Module* mod;
        std::vector<std::map<parser_input, status>> goto_table;
        size_t exprs;
        Token current_input;
        size_t pointer_level;
        std::deque<parser_input> process_stack;
        std::stack<parser_input> symbol_stack;
        llvm::Value* parser_input_to_value(const parser_input & input);
        BlockScopeNode* token_to_llvm_ptr(const Identifier & token);
        std::pair<llvm::Function*, llvm::Value*> take_invocable(llvm::ArrayRef<llvm::Value*> Args);
        bool get_next_token();
        bool check_sizeof(const parser_input& input) const;
        void reduce_operatorbrace();
        void reduce_rparenthese();
        void reduce_expr();
        void get_function_name();
        void define_pointer();
        void define_again();
        void define_finished();
        void define_variable();
        void check_priority();
        bool check_priority(const Punchation punc);
        void reduce_rbrackets();
        inline bool static current_token_is_star(const Token& _Token);
        void back_ward_solve_expr();
        void clear_process_stack();
        static bool invocable(const parser_input& tken);
        void sovle_unary(Punchation punc);
        llvm::Value* get_value(parser_input& input, llvm::Type* rvalue_type = nullptr, bool load_effective_address = false);
        void precess_tokens();
        BlockScopeManager block_scope_manager;
        std::map<status, std::unordered_map<TokenObj, DFA_STATUS_CHANGE_NODE, HashToken, TokenObjCmp>> status_change_map;
        std::unique_ptr<StructTypeFinder> struct_finder;
        llvm::IRBuilder<> builder;
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
        FuncParam func_param;
        unsigned char last_priority;
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
    };
};

#endif