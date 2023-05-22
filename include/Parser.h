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
#include <llvm/IR/Verifier.h>
#include <llvm/IR/Instructions.h>
#include <list>

namespace lpp
{
    class BlockScopeNode;
    class NameSpaceManager;
    using CondParam = struct ConditionParameters
    {
        ConditionParameters();
        llvm::BasicBlock* entery_block, *body1, *body2, *exit_block;
        inline void clear()
        {
            entery_block = body1 = body2 = exit_block = nullptr;
        }
    };
    using FuncParam = struct FunctionParameters
    {
        FunctionParameters();
        llvm::SmallString<256> function_name;
        std::vector<llvm::Type*> arg_type;
        std::vector<llvm::SmallString<256>> arg_name;
        llvm::BasicBlock* return_block;
        std::vector<llvm::BasicBlock*> basic_blocks;
        llvm::BasicBlock::iterator inserter;
        llvm::Type* return_type;
        bool is_static, is_var_args;
        inline void clear()
        {
            arg_name.clear();
            arg_type.clear();
            return_type = nullptr;
            return_block = nullptr;
            function_name.clear();
            basic_blocks.clear();
            is_static = is_var_args = false;
        }
    };
    class BlockScopeManager final
    {
    public:
        BlockScopeManager();
        void enter_scope(llvm::StringRef Name);
        void enter_scope();
        void leave_scope();
        void insert_function(FuncParam& func_param, const llvm::DataLayout &DL);
        void declear_function(llvm::Module*m , FuncParam& func_params);
        void define_function(llvm::Module* m, FuncParam& func_params, const llvm::DataLayout& DL);
        const std::pair<llvm::Function*, bool> find_function(llvm::StringRef Name) const;
        BlockScopeNode* find_value(llvm::StringRef Name);
        llvm::ArrayRef<llvm::Value*> get_current_value_accept_seq();
        bool variable_define(llvm::StringRef Name, size_t _pointer_level);
        void find_and_enter(llvm::StringRef Name);
        void clear_tmp_scope();
        std::string get_function_name();
        bool type_define(llvm::StringRef _Name, llvm::Type* _Type);
        inline BlockScopeNode* get_current_node() { return current_node; };
        BlockScopeNode* get_function_root_node();
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
        bool insert_type(llvm::StringRef Name, llvm::Type* _Type);
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
        inline void set_function(std::pair<llvm::Function*, bool> _func) { function_ptr = _func; };
        inline std::pair<llvm::Function*, bool>& get_function() { return function_ptr; };
        inline bool is_function_root() { return is_func_root; };
        inline bool current_value_is_signed() { return is_signed; };
        inline bool current_node_is_returned() { return has_return; };
        inline void set_current_node_is_returned() { has_return = true; };
    private:
        size_t pointer_level;
        llvm::Type* value_type;
        llvm::Value* current_value,* StructPointerNo;
        std::pair<llvm::Function*, bool> function_ptr;
        std::string name;
        size_t name_no;
        BlockScopeNode* parent;
        llvm::StringMap<std::unique_ptr<BlockScopeNode>> children;
        friend class BlockScopeManager;
        size_t is_struct : 1;
        size_t has_return : 1;
        size_t is_signed : 1;
        size_t is_func_root : 1;
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
        using parser_input = std::variant<llvm::Value*, llvm::Type*, llvm::Function*, Token, CondParam*>;
        enum ParserInputType
        {
            value, type, function, token, condition_parameter
        };
        Parser();
        Parser(std::istream* source_file);
        std::unique_ptr<llvm::Module> parse();
        void init_node(status strat, const TokenObj& status_change, DFA_STATUS_CHANGE_NODE end);
        void init_all_node(status start, DFA_STATUS_CHANGE_NODE end);
        void init_all_keyword(status start, DFA_STATUS_CHANGE_NODE end);
        void init_all_punc(status start, DFA_STATUS_CHANGE_NODE end);
        void override_node(status start, const TokenObj& status_change, DFA_STATUS_CHANGE_NODE end);
    private:
        Lexer lexer;
        void next_status();
        static llvm::LLVMContext context;
        std::unique_ptr<llvm::Module> mod;
        std::vector<std::map<parser_input, status>> goto_table;
        size_t exprs;
        Token current_input;
        size_t pointer_level, scope_count;
        std::deque<parser_input> process_stack;
        std::stack<parser_input> symbol_stack;
        llvm::Value* parser_input_to_value(const parser_input & input);
        BlockScopeNode* token_to_llvm_ptr(const Identifier & token);
        std::pair<llvm::Function*, llvm::Value*> take_invocable(llvm::ArrayRef<llvm::Value*> Args);
        inline bool process_token()
        {
            if (current_input.token.has_value())
            {
                if (!block_scope_manager.get_current_node()->current_node_is_returned() && !scope_count)
                {
                    std::unordered_map<TokenObj, DFA_STATUS_CHANGE_NODE, HashToken, TokenObjCmp>::iterator iter = status_change_map[get_current_status()].find(current_input.token);
                    switch (static_cast<DFA_PROCESS_TYPE>(iter->second.index()))
                    {
                    case DFA_PROCESS_TYPE::S:
                    {
                        symbol_stack.emplace(std::move(current_input));
                        status_stack.push(std::get<status>(iter->second));
                        break;
                    }
                    case DFA_PROCESS_TYPE::R:
                    {
                        std::get<std::function<void()>>(iter->second)();
                        break;
                    }
                    }
                }
                else
                {
                    if (static_cast<TokenType>(current_input.token->index()) == TokenType::PUNCHATION)
                    {
                        if (std::get<Punchation>(*current_input.token) == Punchation::RBRACES)
                        {
                            if (scope_count) scope_count--;
                            else
                            {
                                std::unordered_map<TokenObj, DFA_STATUS_CHANGE_NODE, HashToken, TokenObjCmp>::iterator iter = status_change_map[get_current_status()].find(current_input.token);
                                switch (static_cast<DFA_PROCESS_TYPE>(iter->second.index()))
                                {
                                case DFA_PROCESS_TYPE::S:
                                {
                                    symbol_stack.emplace(std::move(current_input));
                                    status_stack.push(std::get<status>(iter->second));
                                    break;
                                }
                                case DFA_PROCESS_TYPE::R:
                                {
                                    std::get<std::function<void()>>(iter->second)();
                                    break;
                                }
                                }
                            }
                        }
                        else if (std::get<Punchation>(*current_input.token) == Punchation::LBRACES)
                        {
                            scope_count++;
                        }
                    }
                }
                return true;
            }
            else return false;
        }
        void leave_parentheses();
        void enter_parentheses();
        void start_while_condition();
        void start_if_condition();
        void end_while_body();
        void end_if_body1();
        void end_if_body2();
        bool get_next_token();
        bool check_sizeof(const parser_input& input) const;
        void reduce_operatorbrace();
        void reduce_rparenthese();
        void reduce_expr();
        void get_function_name();
        void get_argument();
        void define_pointer();
        void define_again();
        void define_finished();
        void wait_function_return_type();
        void define_variable();
        void check_priority();
        bool check_priority(const Punchation punc);
        void reduce_rbrackets();
        void function_body_define_strart();
        void function_body_define_finished();
        void reduce_condition_calculate();
        void get_function_return_type();
        void function_declear();
        void has_return_type_function_define();
        inline bool static is_punc(const parser_input& _Token);
        inline bool static current_token_is_star(const Token& _Token);
        inline bool static is_rbarces(const Token& _Token);
        inline bool static is_symbol(const parser_input& _Token);
        void check_if_body2();
        void static skip();
        bool back_ward_solve_expr();
        void clear_process_stack();
        static bool invocable(const parser_input& tken);
        void sovle_unary(Punchation punc);
        llvm::Value* get_value(parser_input& input, llvm::Type* rvalue_type = nullptr, bool load_effective_address = false, bool reset_value = false);
        std::stack<uint8_t> priority_stack;
        std::pair<llvm::Function*, bool> get_callee(parser_input& input, llvm::ArrayRef<llvm::Value*> args);
        static bool is_comma(const parser_input& punc);
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