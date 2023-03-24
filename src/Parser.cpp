#include <Parser.h>

namespace lpp
{
    Parser::Parser() : struct_finder(new StructTypeFinder), builder(context), last_priority(255)
    {
    
    }

    Parser::Parser(std::istream* source_file) : struct_finder(new StructTypeFinder), builder(context), pointer_level(0), last_priority(255)
    {
        lexer.set_source(source_file);
        std::fstream lex("D:\\Cfile\\Complier\\Lethon.lex", std::ios::in);
        lexer.load(lex);
        init_all_punc(0, 1);
        init_node(0, TokenObj(Identifier()), 2);
        init_node(0, TokenObj(KeyWord::LET), 3);
        init_all_punc(2, std::bind(static_cast<void(Parser::*)()>(&Parser::check_priority), this));
        override_node(2, Punchation::SEMICOLON, std::bind(&Parser::back_ward_solve_expr, this));
        init_node(3, Punchation::MUL, std::bind(&Parser::define_pointer, this));
        init_node(3, TokenObj(Identifier()), std::bind(&Parser::define_variable, this));
        init_node(4, TokenObj(Punchation::COMMA), 5);
        init_node(4, TokenObj(Punchation::SEMICOLON), std::bind(&Parser::define_finished, this));
        init_node(5, TokenObj(Identifier()), std::bind(&Parser::define_again, this));
        init_node(5, Punchation::MUL, std::bind(&Parser::define_pointer, this));
        init_node(7, Identifier(), 2);
        init_all_punc(7, 2);
        init_node(7, Digit(), 2);
    }

    void Parser::parse()
    {
        mod = new llvm::Module("", context);
        while (get_next_token());
        delete mod;
    }

    llvm::Value *Parser::parser_input_to_value(const parser_input &input)
    {
        llvm::Value* result;
        switch (static_cast<ParserInputType>(input.index()))
        {
        case ParserInputType::value:
            result = std::get<llvm::Value*>(input);
            break;
        case ParserInputType::type:
            block_scope_manager.find_value(std::get<Identifier>(*std::get<Token>(input).token).name);
            break;
        default:
            throw std::runtime_error("current input can't convert to value");
            break;
        }
        return result;
    }

    BlockScopeNode *Parser::token_to_llvm_ptr(const Identifier &ID)
    {
        return block_scope_manager.find_value(ID.name);
    }

    std::pair<llvm::Function*, llvm::Value*> Parser::take_invocable(llvm::ArrayRef<llvm::Value*> Args)
    {
        std::pair<llvm::Function*, bool> callee;
        bool flag = true;
        while (symbol_stack.size() && flag)
        {
            process_stack.push_back(std::move(symbol_stack.top()));
            symbol_stack.pop();
            status_stack.pop();
            switch (std::get<Punchation>(*std::get<Token>(symbol_stack.top()).token))
            {
            case Punchation::DOT: case Punchation::POINTERTO:
                process_stack.push_back(std::move(symbol_stack.top()));
                symbol_stack.pop();
                status_stack.pop();
                break;
            default:
                flag = false;
                break;
            }
        }
        flag = true;
        while (process_stack.size() && flag)
        {
            parser_input lhs, rhs;
            lhs = std::move(process_stack.back());
            symbol_stack.pop();
            status_stack.pop();
            llvm::Value *lvalue, *rvalue;
            Punchation punc = std::get<Punchation>(*std::get<Token>(process_stack.back()).token);
            process_stack.pop_back();
            switch (punc)
            {
            case Punchation::DOT:
                if (lvalue = parser_input_to_value(lhs))
                {
                    rhs = std::move(process_stack.back());
                    process_stack.pop_back();
                    block_scope_manager.find_and_enter(std::get<Identifier>(*std::get<Token>(rhs).token).name);
                }
                else
                {
                    rhs = std::move(process_stack.back());
                    process_stack.pop_back();
                    std::string func_name(std::get<Identifier>(*std::get<Token>(rhs).token).name);
                    for (int var = 0; var < Args.size(); var++)
                    {
                        std::string arg_name = get_type_name(Args[var]->getType());
                        func_name += std::move(arg_name);
                    }
                    callee = block_scope_manager.find_function(func_name);
                    flag = false;
                }
                
                break;
            case Punchation::POINTERTO:
                throw std::runtime_error("unsupport now");
                break;
            }
        }
        if (callee.first)
        {
            if (process_stack.empty()) 
            {
                // std::pair<llvm::Function*, llvm::Value*> result;
                // result.first = callee.first;
                // if (callee.second) result.second = block_scope_manager.get_current_value_accept_seq();
                // else result.second = nullptr;
                // block_scope_manager.clear_tmp_scope();
                // return std::move(result);
            }
            else throw std::runtime_error("failed to solve callee");
        }
        else
        {
            std::string func_name;
            func_name = block_scope_manager.get_function_name() + "10operator()@P";
            for (int var = 0; var < Args.size(); var++)
            {
                std::string arg_name = get_type_name(Args[var]->getType());
                func_name += std::move(arg_name);
            }
            callee = block_scope_manager.find_function(func_name);
            std::pair<llvm::Function*, llvm::Value*> result;
            result.first = callee.first;
            // if (callee.second) result.second = builder.CreateStructGEP(, block_scope_manager.get_current_value_accept_seq());
            // else result.second = nullptr;
            block_scope_manager.clear_tmp_scope();
            return std::move(result);
        }
    }

    bool Parser::get_next_token()
    {
        current_input = lexer.get_next_token();
        if (current_input.token.has_value())
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
            return true;
        }
        else return false;
    }

    bool Parser::check_sizeof(const parser_input& input) const
    {
        return static_cast<ParserInputType>(input.index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(input).token->index()) == TokenType::KEYWORD && std::get<KeyWord>(*std::get<Token>(input).token) == KeyWord::SIZEOF;
    }

    void Parser::reduce_operatorbrace()
    {
        std::vector<llvm::Value*> args;
        llvm::SmallString<256> func_name("_FN");
        while (process_stack.size())
        {
            args.push_back(std::get<llvm::Value*>(process_stack.back()));
            process_stack.pop_back();
            Punchation punc = std::get<Punchation>(*std::get<Token>(process_stack.back()).token);
            process_stack.pop_back();
        }
        if (check_sizeof(symbol_stack.top())) 
        {
            if (args.size() != 1) throw std::runtime_error("error input for sizeof");
            const llvm::DataLayout& layout = builder.GetInsertBlock()->getModule()->getDataLayout();
            symbol_stack.push(builder.getInt64(layout.getTypeAllocSize(args[0]->getType())));
        }
        else 
        {
            std::pair<llvm::Function*, llvm::Value*> callee = take_invocable(args);
            if (callee.second) 
            {
                args.insert(args.begin(), callee.second);
            }
            symbol_stack.push(static_cast<llvm::Value*>(builder.CreateCall(callee.first, callee.second)));
        }

 
        std::map<parser_input, status>::iterator&& iter = goto_table[get_current_status()].find(symbol_stack.top());
        if (iter == goto_table[get_current_status()].end()) status_stack.emplace(iter->second);
        else
        {
            throw std::runtime_error("undefind next status");
        }
    }

    void Parser::reduce_rparenthese()
    {

        // if case '(' flag will be set true
        bool left_brace_flag = false;
        while (!symbol_stack.empty())
        {
            parser_input &token = symbol_stack.top();
            // if token == ')' end the loop
            if (static_cast<ParserInputType>(token.index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(token).token->index()) == TokenType::PUNCHATION && std::get<Punchation>(*std::get<Token>(token).token) == Punchation::RBRACES)
            {
                left_brace_flag = true;
                break;
            }
            process_stack.push_back(std::move(token));
            symbol_stack.pop();
        }
        if (left_brace_flag)
        {
            if (invocable(symbol_stack.top()))
            {
                reduce_operatorbrace();
            }
            else
            {
                precess_tokens();
            }
        }
        else
        {
            except_token = std::move(std::get<Token>(process_stack.back()));
            throw std::runtime_error("unclosed brace");
        }
    }

    void Parser::reduce_expr()
    {
        Punchation punc = std::get<Punchation>(*current_input.token);
        bool flag = false;
        TokenType last_token_type;
        while (!symbol_stack.empty())
        {
            parser_input &token = symbol_stack.top();
            last_token_type = static_cast<TokenType>(token.index());
            if (static_cast<ParserInputType>(token.index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(token).token->index()) == TokenType::PUNCHATION && std::get<Punchation>(*std::get<Token>(token).token) == Punchation::RBRACES)
            {
                flag = true;
                break;
            }
            process_stack.push_back(std::move(token));
            symbol_stack.pop();
        }
    }

    void Parser::get_function_name()
    {
        func_param.arg_name = std::move(std::get<Identifier>(symbol_stack.top()));
        symbol_stack.pop();
        symbol_stack.pop();
    }

    void Parser::define_pointer()
    {
        pointer_level++;
    }

    void Parser::define_again()
    {
        symbol_stack.pop();
        status_stack.pop();
    }

    void Parser::define_finished()
    {
        status_stack.pop();
        symbol_stack.pop();
        status_stack.pop();
        symbol_stack.pop();
    }

    void Parser::define_variable()
    {
        if (!block_scope_manager.variable_define(std::get<Identifier>(*current_input.token).name, pointer_level))
        {
            except_token = std::move(current_input);
            throw std::runtime_error("variable \"" + std::get<Identifier>(*current_input.token).name + "\" has already existed");
        }
        else 
        {
            status_stack.push(4);
            symbol_stack.emplace(std::move(current_input));
        }
        pointer_level = 0;
    }

    void Parser::check_priority()
    {
        unsigned char priority;
        switch (std::get<Punchation>(*current_input.token))
        {
        case Punchation::RPARENTHESES:
            reduce_rparenthese();
            break;
        case Punchation::RBRACKETS:
            reduce_rbrackets();
            break;
        case Punchation::SEMICOLON:
            back_ward_solve_expr();
            return;
        default:
            break;
        }
        std::unordered_map<TokenObj, unsigned char, HashToken, TokenObjCmp>::iterator&& iter = priority_map.find(*current_input.token);
        if (priority_map.end() == iter) throw std::runtime_error("unexpect token");
        else priority = iter->second;
        if (priority > last_priority)
        {
            back_ward_solve_expr();
        }
        else
        {
            last_priority = priority;
        }
        symbol_stack.emplace(current_input);
        status_stack.emplace(7);
    }

    bool Parser::check_priority(const Punchation punc)
    {
        unsigned char priority;
        std::unordered_map<TokenObj, unsigned char, HashToken, TokenObjCmp>::iterator&& iter = priority_map.find(punc);
        if (priority_map.end() == iter) throw std::runtime_error("unexpect token");
        else priority = iter->second;
        if (priority > last_priority)
        {
            last_priority = priority;
            return false;
        }
        else
        {
            last_priority = priority;
            return true;
        }
    }

    void Parser::reduce_rbrackets()
    {
    }

    inline bool Parser::current_token_is_star(const Token &_Token)
    {
        return (static_cast<TokenType>(_Token.token->index()) == TokenType::PUNCHATION && std::get<Punchation>(*_Token.token) == Punchation::MUL);
    }

    void Parser::back_ward_solve_expr()
    {
        parser_input value, punc = Token();
        
        while (static_cast<ParserInputType>(punc.index()) == ParserInputType::token && symbol_stack.size() >= 2)
        {
            value = std::move(symbol_stack.top());
            symbol_stack.pop();
            status_stack.pop();

            process_stack.push_back(std::move(value));
get_value:  punc = std::move(symbol_stack.top());
            symbol_stack.pop();
            status_stack.pop();
            if (check_priority(std::get<Punchation>(*std::get<Token>(punc).token))) process_stack.push_back(std::move(punc));
            else break;
            switch (static_cast<ParserInputType>(value.index()))
            {
            case ParserInputType::token:
            {
                switch (static_cast<TokenType>(std::get<Token>(value).token->index()))
                {
                case TokenType::PUNCHATION:
                    sovle_unary(std::get<Punchation>(*std::get<Token>(punc).token));
                    goto get_value;
                    break;
                default:
                    break;
                }
                break;
            }
            case ParserInputType::function:
                throw std::runtime_error("unexpect function type in current location");
                break;
            default:
                break;
            }
        }
        process_stack.push_back(std::move(symbol_stack.top()));
        symbol_stack.pop();
        status_stack.pop();
        clear_process_stack();
    }

    void Parser::clear_process_stack()
    {
        while (process_stack.size() > 1)
        {
            parser_input lhs = std::move(process_stack.back());
            process_stack.pop_back();
            Punchation punc = std::get<Punchation>(*std::get<Token>(process_stack.back()).token);
            process_stack.pop_back();
            parser_input rhs = std::move(process_stack.back());
            process_stack.pop_back();
            switch (punc)
            {
            case Punchation::ASSIGN:
            {
                llvm::Value* rvalue,* lvalue;
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, rvalue->getType(), true);
                builder.CreateStore(lvalue, lvalue);
            }
            }
            //llvm::Value* lvalue = parser_input_to_llvmValue(lhs);
        }

    }

    bool Parser::invocable(const parser_input& token)
    {
        return static_cast<ParserInputType>(token.index()) == ParserInputType::function || static_cast<ParserInputType>(token.index()) == ParserInputType::value || static_cast<ParserInputType>(token.index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(token).token->index()) == TokenType::SYMBOL;
    }

    void Parser::sovle_unary(Punchation punc)
    {
        parser_input symbol = std::move(process_stack.back());
        process_stack.pop_back();
        llvm::Value* value;
        switch (punc)
        {
        case Punchation::POSITIVE:
        {
            process_stack.push_back(parser_input_to_value(parser_input(value)));
            break;
        }
        case Punchation::NEGATIVE:
        {
            value = builder.CreateNeg(value);
            process_stack.push_back(value);
            break;
        }
        case Punchation::DEREF:
        {
            value = get_value(symbol);
            process_stack.push_back(value);
            break;
        }
        case Punchation::GETADDR:
        {
            //builder.CreatePo
            break;
        }
        default:
            break;
        }
    }

    llvm::Value *Parser::get_value(parser_input &input, llvm::Type* rvalue_type, bool load_effective_address)
    {
        llvm::Value* result;
        switch (static_cast<ParserInputType>(input.index()))
        {
        case ParserInputType::function:
            throw std::runtime_error("function type unable to calculate");
            break;
        case ParserInputType::token:
        {
            Token& token = std::get<Token>(input);
            switch (static_cast<TokenType>(token.token->index()))
            {
            case TokenType::DIGIT:
            {
                if (load_effective_address) throw std::runtime_error("lvalue unable to load address");
                digit2llvmvlue transformer(builder.getContext());
                result = transformer(std::get<Digit>(*token.token));
                break;
            }
            case TokenType::SYMBOL:
            {
                // llvm::ValueSymbolTable* st = builder.GetInsertBlock()->getValueSymbolTable();
                // result = st->lookup(std::get<Identifier>(*token.token).name);
                BlockScopeNode* bs = block_scope_manager.find_value(std::get<Identifier>(*token.token).name);
                if (!bs)
                {
                    except_token = std::move(token);
                    throw std::runtime_error(std::string("undefined symbol \"") + std::get<Identifier>(*token.token).name + "\"");
                }
                if (!bs->get_address())
                {
                    if (bs->get_parent()->is_root()) bs->set_current_value(mod->getOrInsertGlobal(std::get<Identifier>(*std::get<Token>(input).token).name, rvalue_type), rvalue_type);
                    else 
                    {
                        bs->set_current_value(builder.CreateAlloca(rvalue_type), rvalue_type);
                    }
                }
                result = bs->get_address();
                if (!load_effective_address)
                {
                    result = builder.CreateLoad(bs->get_type(), result);
                }
                break;
            }
            default:
                break;
            }
            break;
        }
        case ParserInputType::value:
        if (load_effective_address) throw std::runtime_error("lvalue unable to load address");
            result = std::get<llvm::Value*>(input);
        default:
            break;
        }
        return result;
    }

    void Parser::precess_tokens()
    {
        while (process_stack.size() > 2)
        {
            //parser_input 
        }
    }

    void Parser::init_node(status start, const TokenObj& status_change, DFA_STATUS_CHANGE_NODE end)
    {
        std::unordered_map<TokenObj, DFA_STATUS_CHANGE_NODE, HashToken, TokenObjCmp>::iterator&& iter = status_change_map[start].find(status_change);
        if (iter == status_change_map[start].end())
        {
            status_change_map[start].insert(std::pair<TokenObj, DFA_STATUS_CHANGE_NODE>(status_change, end));
        }
        else 
        {
               throw std::runtime_error("status has already existed");
        }
        
    }
    void Parser::init_all_node(status start, DFA_STATUS_CHANGE_NODE end)
    {
        init_node(start, Identifier(), end);
        init_node(start, Digit(), end);

        init_all_keyword(start, end);
        init_all_punc(start, end);

    }
    void Parser::init_all_keyword(status start, DFA_STATUS_CHANGE_NODE end)
    {
        for (int var = 0; var < static_cast<int>(KeyWord::NUM); var++)
        {
            init_node(start, static_cast<KeyWord>(var), end);
        }
    }
    void Parser::init_all_punc(status start, DFA_STATUS_CHANGE_NODE end)
    {
        for (int punc = 0; punc < 256; punc++)
        {
            init_node(start, TokenObj(static_cast<Punchation>(punc)), end);
        }
    }
    void Parser::override_node(status start, const TokenObj &status_change, DFA_STATUS_CHANGE_NODE end)
    {
        std::unordered_map<TokenObj, DFA_STATUS_CHANGE_NODE, HashToken, TokenObjCmp>::iterator&& iter = status_change_map[start].find(status_change);
        status_change_map[start].insert(std::pair<TokenObj, DFA_STATUS_CHANGE_NODE>(status_change, end));
    }
    void Parser::next_status()
    {
        // std::map<TokenObj, DFA_STATUS_CHANGE_NODE>::iterator&& iter = status_change_map[status_stack.top()].find(*current_input.token);
        // if (iter != status_change_map[status_stack.top()].end())
        // {
        //     DFA_STATUS_CHANGE_NODE& status_change_node = iter->second;
        //     switch (static_cast<DFA_PROCESS_TYPE>(status_change_node.index()))
        //     {
        //     //true means reduce
        //     case DFA_PROCESS_TYPE::R:
        //         std::get<std::function<void()>>(status_change_node)();
        //         break;
        //     case DFA_PROCESS_TYPE::S:
        //         status_stack.push(std::get<status>(status_change_node));
        //         symbol_stack.push(std::move(current_input));
        //         break;
        //     }
        // }
        // else 
        // {
        //     throw std::runtime_error("undefind next status");
        // }
    }

    bool TokenObjCmp::operator()(const TokenObj &lhs, const TokenObj &rhs) const
    {
        switch (static_cast<TokenType>(lhs.value().index()))
        {
        case TokenType::DIGIT: case TokenType::SYMBOL:
            if (static_cast<TokenType>(rhs.value().index()) == TokenType::DIGIT || static_cast<TokenType>(rhs.value().index()) == TokenType::SYMBOL) return true;
            else return false;
        default:
            return lhs == rhs;
        }
    }
    void NameSpaceManager::goback()
    {
        if (current_namespace.size())
        {
            current_namespace.pop_back();
        }
        else throw std::runtime_error("no parent namespace");
    }
    void NameSpaceManager::insert(std::string space)
    {
        current_namespace.push_back(std::move(space));
    }
    void NameSpaceManager::insert_tmp(std::string space)
    {
        tmp_namespace.push_back(std::move(space));
    }
    void NameSpaceManager::clear_tmp()
    {
        tmp_namespace.clear();
    }
    std::string NameSpaceManager::get_name()
    {
        std::string name("_FN");
        for (auto i : current_namespace)
        {
            name += std::to_string(i.size());
            name += i; 
        }
        for (auto i : tmp_namespace)
        {
            name += std::to_string(i.size());
            name += i; 
        }
        name += "@P";
        return std::move(name);
    }
    BlockScopeManager::BlockScopeManager()
    {
        root_node = std::make_unique<BlockScopeNode>("");
        current_node = root_node.get();
    }
    void BlockScopeManager::enter_scope(llvm::StringRef Name)
    {
        BlockScopeNode* tmp_node = current_node->get_child_block(Name);
        if (!tmp_node) 
        {
            current_node->insert_member_value(Name, nullptr, 0);
            tmp_node = current_node->get_child_block(Name);
        }
        tmp_node = current_node = tmp_node;
    }
    void BlockScopeManager::enter_scope()
    {
        enter_scope(std::string("anon") + std::to_string(current_node->name_no++));
    }
    void BlockScopeManager::leave_scope()
    {
        if (current_node->name.size()) tmp_node = current_node = const_cast<BlockScopeNode*>(current_node->parent);
        else throw std::runtime_error("no parent scope");
    }

    void BlockScopeManager::insert_function(llvm::Module* m, std::string Name, llvm::Type* return_type, llvm::ArrayRef<llvm::Type *> Args, bool is_static, bool is_var_ags)
    {
        llvm::FunctionType* _Ft;
        std::string func_name = get_function_name();
        if (is_static) _Ft = llvm::FunctionType::get(return_type, Args, is_var_ags);
        else
        {
            std::vector<llvm::Type*> _Args;
            _Args.push_back(current_node->current_value->getType());
            _Args.insert(_Args.end(), Args.begin(), Args.end());
            _Ft = llvm::FunctionType::get(return_type, _Args, is_var_ags);
        }
        func_name += Name;
        func_name += "@P";
        for (llvm::Type* arg_type : Args)
        {
            std::string arg_name = get_type_name(arg_type);
            func_name += std::to_string(arg_name.size());
            func_name += arg_name;
            Name += std::to_string(arg_name.size());
            Name += arg_name;
        }
        llvm::Function* fn = llvm::Function::Create(_Ft, llvm::GlobalValue::LinkageTypes::ExternalLinkage, func_name, m);
        current_node->insert_function(Name, std::pair(fn, is_static));
    }

    std::string BlockScopeManager::get_function_name()
    {
        BlockScopeNode* tmp_scope = current_node;
        std::list<std::string*> names;
        std::string name = "_FN";
        while (tmp_scope->name.size())
        {
            names.push_front(&tmp_scope->name);
            tmp_scope = const_cast<BlockScopeNode*>(tmp_scope->parent);
        }
        for (auto i : names)
        {
            name += std::to_string(i->size());
            name += *i;
        }
        return std::move(name);
    }

    const std::pair<llvm::Function*, bool> BlockScopeManager::find_function(llvm::StringRef Name) const
    {
        for (BlockScopeNode* block : search_scope)
        {
            const std::pair<llvm::Function*, bool>& func = block->get_function(Name);
            if (!func.first) throw std::runtime_error(std::string("no such function named \"") + std::string(Name) + "\"");
            return func;
        }
        throw std::runtime_error("no specified function named \"" + std::string(Name) + "\"");
    }

    BlockScopeNode *BlockScopeManager::find_value(llvm::StringRef Name)
    {
        BlockScopeNode* value = nullptr;
        value = current_node->get_child_block(Name);
        if (!value) throw std::runtime_error("no such value named \"" + std::string(Name) + "\"");
        return value;
    }

    llvm::ArrayRef<llvm::Value*> BlockScopeManager::get_current_value_accept_seq()
    {
        return element_pointer_seq;
    }

    bool BlockScopeManager::variable_define(llvm::StringRef Name, size_t _pointer_level)
    {
        return current_node->insert_member_value(Name, nullptr, _pointer_level);
    }

    bool BlockScopeManager::type_define(llvm::StringRef Name)
    {
        return current_node->insert_member_value(Name, nullptr, 0);
    }

    void BlockScopeManager::find_and_enter(llvm::StringRef Name)
    {
        tmp_node = tmp_node->get_child_block(Name);
        element_pointer_seq.push_back(tmp_node->StructPointerNo);
    }

    void BlockScopeManager::clear_tmp_scope()
    {
        tmp_node = current_node;
    }

    BlockScopeNode::BlockScopeNode(llvm::StringRef Name, BlockScopeNode *_parent, llvm::Value* _Value, size_t _pointer_level) : name(Name), name_no(0), parent(_parent), current_value(_Value), value_type(nullptr), pointer_level(_pointer_level)
    {

    }
    const std::pair<llvm::Function*, bool> BlockScopeNode::get_function(llvm::StringRef Name)
    {
        llvm::StringMap<std::pair<llvm::Function*, bool>>::const_iterator it = member_functions.find(Name);
        if (it != member_functions.end()) return it->second;
        return std::pair<llvm::Function*, bool>(nullptr, false);
    }
    BlockScopeNode *BlockScopeNode::get_child_block(llvm::StringRef Name)
    {
        llvm::StringMap<std::unique_ptr<BlockScopeNode>>::const_iterator it = childs.find(Name);
        if (it != childs.end()) return it->second.get();
        return nullptr;
    }
    void BlockScopeNode::insert_function(llvm::StringRef Name, std::pair<llvm::Function*, bool> Func)
    {
        member_functions.insert(std::pair(Name, std::move(Func)));
    }

    bool BlockScopeNode::insert_member_value(llvm::StringRef Name, llvm::Value *_Value, size_t _pointer_level)
    {
        ;
        if (!this->get_child_block(Name))
        {
            childs.insert(std::pair(Name, std::unique_ptr<BlockScopeNode>(new BlockScopeNode(Name, this, _Value, _pointer_level))));
            return true;
        }
        else return false;
    }
    void BlockScopeNode::set_parent(BlockScopeNode *_parent)
    {
        parent = parent;
    }
    void BlockScopeNode::allocate(llvm::Value *value_ptr, llvm::Type *_type)
    {
        current_value = value_ptr;
        value_type = _type;
    }
    size_t Parser::HashToken::operator()(const TokenObj &_token) const
    {
        size_t result = 0;
        switch (static_cast<TokenType>(_token->index()))
        {
        case TokenType::DIGIT:
            result = -2;
            break;
        case TokenType::SYMBOL:
            result = -3;
            break;
        case TokenType::PUNCHATION:
            result = static_cast<size_t>(std::get<Punchation>(_token.value()));
            break;
        case TokenType::KEYWORD:
            result = static_cast<size_t>(std::get<KeyWord>(*_token)) + 256;
            break;
        default:
            result = -1;
            break;
        }
        return result;
    }
    void FunctionParameters::clear()
    {
        arg_name.clear();
        arg_type.clear();
        return_type = nullptr;
        function_name.clear();
    }
}
