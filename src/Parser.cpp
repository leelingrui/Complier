#include <Parser.h>

namespace lpp
{
    llvm::LLVMContext Parser::context;
    Parser::Parser() : struct_finder(new StructTypeFinder), builder(context), last_priority(255), block_scope_manager()
    {
        scope_count = 0;
        std::fstream lex("D:\\Cfile\\Complier\\Lethon.lex", std::ios::in);
        lexer.load(lex);
        block_scope_manager.type_define("bool", builder.getInt1Ty());
        block_scope_manager.type_define("i8", builder.getInt8Ty());
        block_scope_manager.type_define("u8", builder.getInt8Ty());
        block_scope_manager.type_define("i16", builder.getInt16Ty());
        block_scope_manager.type_define("u16", builder.getInt16Ty());
        block_scope_manager.type_define("i32", builder.getInt32Ty());
        block_scope_manager.type_define("u32", builder.getInt32Ty());
        block_scope_manager.type_define("i64", builder.getInt32Ty());
        block_scope_manager.type_define("u64", builder.getInt32Ty());
        block_scope_manager.type_define("f32", builder.getFloatTy());
        block_scope_manager.type_define("f64", builder.getDoubleTy());
    
        init_node(0, TokenObj(Identifier()), 2);
        init_node(0, KeyWord::LET, 3);
        init_node(0, KeyWord::FN, 8);
        init_node(0, Enter(), std::bind(&Parser::skip));
        init_node(0, Digit(), 2);
        init_all_punc(2, std::bind(static_cast<void(Parser::*)()>(&Parser::check_priority), this));
        init_node(2, Enter(), std::bind(reduce_condition_calculate, this));
        override_node(2, Punchation::LPARENTHESES, 0);
        init_node(3, Punchation::MUL, std::bind(&Parser::define_pointer, this));
        init_node(3, TokenObj(Identifier()), std::bind(&Parser::define_variable, this));
        init_node(4, TokenObj(Punchation::COMMA), 5);
        init_node(4, TokenObj(Punchation::SEMICOLON), std::bind(&Parser::define_finished, this));
        init_node(5, TokenObj(Identifier()), std::bind(&Parser::define_again, this));
        init_node(5, Punchation::MUL, std::bind(&Parser::define_pointer, this));
        init_node(7, Identifier(), 2);
        init_all_punc(7, 2);
        init_node(7, Digit(), 2);
        override_node(7, Punchation::LPARENTHESES, 7);
        init_node(8, Identifier(), 9);
        init_node(9, Punchation::LPARENTHESES, std::bind(&Parser::get_function_name, this));
        init_node(10, Identifier(), 11);
        init_node(10, Punchation::RPARENTHESES, std::bind(&Parser::wait_function_return_type, this));
        init_node(10, Punchation::COMMA, std::bind(skip));
        init_node(11, Punchation::COLON, 12);
        init_node(12, Identifier(), std::bind(&Parser::get_argument, this));
        init_node(14, Punchation::LBRACES, std::bind(&Parser::function_body_define_strart, this));
        init_node(14, Enter(), std::bind(&Parser::skip));
        init_all_punc(15, 1);
        override_node(15, Punchation::RBRACES, std::bind(&Parser::function_body_define_finished, this));
        init_node(15, TokenObj(Identifier()), 2);
        init_node(15, KeyWord::LET, 3);
        init_node(15, KeyWord::RETURN, 16);
        init_node(15, Enter(), std::bind(&Parser::skip));
        init_node(15, KeyWord::IF, std::bind(&Parser::start_if_condition, this));
        init_node(15, KeyWord::WHILE, std::bind(&Parser::start_while_condition, this));
        init_node(16, Identifier(), 2);
        init_node(16, Digit(), 2);
        init_all_punc(16, 1);
        init_all_punc(17, 1);
        init_node(17, Identifier(), 2);
        init_node(17, Digit(), 2);
        init_node(18, Punchation::LBRACES, 19);
        init_all_punc(19, 1);
        init_node(19, Identifier(), 2);
        init_node(19, Digit(), 2);
        init_node(19, Enter(), std::bind(&Parser::skip));
        override_node(19, Punchation::RBRACES, std::bind(&Parser::end_if_body1, this));
        init_node(19, KeyWord::RETURN, 16);
        init_node(19, KeyWord::WHILE, std::bind(&Parser::start_while_condition, this));
        init_node(19, KeyWord::IF, std::bind(&Parser::start_if_condition, this));
        init_all_node(20, std::bind(&Parser::check_if_body2, this));
        init_node(20, Enter(), std::bind(&Parser::skip));
        init_node(21, Punchation::LBRACES, 22);
        init_node(21, Enter(), std::bind(&Parser::skip));
        init_all_punc(22, 1);
        init_node(22, Identifier(), 2);
        init_node(22, Digit(), 2);
        init_node(22, Enter(), std::bind(&Parser::skip));
        init_node(22, KeyWord::RETURN, 16);
        override_node(22, Punchation::RBRACES, std::bind(&Parser::end_if_body2, this));
        init_node(22, KeyWord::WHILE, std::bind(&Parser::start_while_condition, this));
        init_node(22, KeyWord::IF, std::bind(&Parser::start_if_condition, this));
        init_all_punc(23, 1);
        init_node(23, Identifier(), 2);
        init_node(23, Digit(), 2);
        init_node(24, Enter(), std::bind(&Parser::skip));
        init_node(24, KeyWord::WHILE, std::bind(&Parser::start_while_condition, this));
        init_node(24, KeyWord::IF, std::bind(&Parser::start_if_condition, this));
        init_all_punc(24, 1);
        override_node(24, Punchation::RBRACES, std::bind(&Parser::end_while_body, this));
        init_node(24, Identifier(), 2);
        init_node(24, Digit(), 2);
        init_node(25, Enter(), std::bind(&Parser::skip));
        init_node(25, Punchation::LBRACES, 24);
    }

    Parser::Parser(std::istream* source_file) : struct_finder(new StructTypeFinder), builder(context), pointer_level(0), last_priority(255)
    {
        new(this) Parser();
        lexer.set_source(source_file);
        // init_node(13, Punchation::RPARENTHESES, std::bind(Parser::));
    }

    std::unique_ptr<llvm::Module> Parser::parse()
    {
        mod = std::make_unique<llvm::Module>("test", context);
        llvm::FunctionType* _Ft = llvm::FunctionType::get(builder.getVoidTy(), false);
        while (get_next_token());
        return std::move(mod);
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
            if (is_punc(symbol_stack.top()))
            {
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
            else
            {
                flag = false;
            }
        }
        flag = true;
        while (process_stack.size() > 1 && flag)
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
        if (flag) callee = get_callee(process_stack.back(), Args);
        if (callee.second)
        {

        }
        else
        {
            return std::pair(callee.first, nullptr);
        }
    }

    void Parser::start_while_condition()
    {
        status_stack.push(23);
        CondParam *cond_param = new CondParam;
        symbol_stack.push(cond_param);
        cond_param->entery_block = llvm::BasicBlock::Create(context);
        func_param.basic_blocks.push_back(cond_param->entery_block);
        builder.CreateBr(cond_param->entery_block);
        builder.SetInsertPoint(cond_param->entery_block);
        block_scope_manager.enter_scope();
    }

    void Parser::start_if_condition()
    {
        status_stack.push(17);
        CondParam *cond_param = new CondParam;
        symbol_stack.push(cond_param);
        block_scope_manager.enter_scope();
    }

    void Parser::end_while_body()
    {
        status_stack.pop();
        symbol_stack.pop();
        CondParam* cond_param = std::get<CondParam*>(symbol_stack.top());
        builder.CreateBr(cond_param->entery_block);
        builder.SetInsertPoint(cond_param->exit_block);
        block_scope_manager.leave_scope();
        delete cond_param;
        status_stack.pop();
        symbol_stack.pop();
    }

    void Parser::end_if_body1()
    {
        status_stack.pop();
        status_stack.push(20);
        parser_input tmp = std::move(symbol_stack.top());
        symbol_stack.pop();
        CondParam* cond_param = std::get<CondParam*>(symbol_stack.top());
        symbol_stack.push(std::move(tmp));
        BlockScopeNode* node = block_scope_manager.get_current_node();
        if (node->current_node_is_returned())
        {
            builder.SetInsertPoint(cond_param->body2);
        }
        else
        {
            cond_param->exit_block = llvm::BasicBlock::Create(context);
            builder.SetInsertPoint(cond_param->body2);
        }
        block_scope_manager.leave_scope();
    }

    void Parser::end_if_body2()
    {
        status_stack.pop();
        symbol_stack.pop();
        CondParam* cond_param = std::get<CondParam*>(symbol_stack.top());
        BlockScopeNode* node = block_scope_manager.get_current_node();
        if (node->current_node_is_returned())
        {
            if (cond_param->exit_block)
            {
                builder.SetInsertPoint(cond_param->exit_block);
                block_scope_manager.leave_scope();
            }
            else
            {
                block_scope_manager.leave_scope();
                block_scope_manager.get_current_node()->set_current_node_is_returned();
            }
        }
        else
        {
            if (cond_param->exit_block)
            {
                builder.SetInsertPoint(cond_param->exit_block);
            }
            else
            {
                cond_param->entery_block = llvm::BasicBlock::Create(context);
                builder.CreateBr(cond_param->exit_block);
                builder.SetInsertPoint(cond_param->exit_block);
            }
            block_scope_manager.leave_scope();
        }
        delete cond_param;
        status_stack.pop();
        symbol_stack.pop();
    }

    bool Parser::get_next_token()
    {
        current_input = lexer.get_next_token();
        if (!current_input.token->index())
        {
            std::cout << current_input.line << std::endl;
        }
        return process_token();
    }

    bool Parser::check_sizeof(const parser_input& input) const
    {
        return static_cast<ParserInputType>(input.index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(input).token->index()) == TokenType::KEYWORD && std::get<KeyWord>(*std::get<Token>(input).token) == KeyWord::SIZEOF;
    }

    void Parser::reduce_operatorbrace()
    {
        std::vector<llvm::Value*> args;
        llvm::SmallString<256> func_name("_FN");
        while (process_stack.size() > 1)
        {
            args.push_back(get_value(process_stack.back()));
            process_stack.pop_back();
            Punchation punc = std::get<Punchation>(*std::get<Token>(process_stack.back()).token);
            process_stack.pop_back();
        }
        args.push_back(get_value(process_stack.back()));
        process_stack.pop_back();
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
            symbol_stack.push(static_cast<llvm::Value*>(builder.CreateCall(callee.first, args)));
            status_stack.push(2);
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
            if (static_cast<ParserInputType>(token.index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(token).token->index()) == TokenType::PUNCHATION && std::get<Punchation>(*std::get<Token>(token).token) == Punchation::LPARENTHESES)
            {
                symbol_stack.pop();
                left_brace_flag = true;
                status_stack.pop();
                break;
            }
            process_stack.push_back(std::move(token));
            symbol_stack.pop();
            status_stack.pop();
        }
        if (left_brace_flag)
        {
            if (invocable(std::get<Token>(symbol_stack.top())))
            {
                reduce_operatorbrace();
            }
            else
            {
                clear_process_stack();
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
        func_param.function_name = std::move(std::get<Identifier>(*std::get<Token>(symbol_stack.top()).token).name);
        symbol_stack.pop();
        status_stack.pop();
        symbol_stack.emplace(current_input);
        status_stack.emplace(10);
    }

    void Parser::get_argument()
    {
        Token func_arg_name;
        Token func_arg_type;
        func_arg_type = std::move(current_input);
        symbol_stack.pop();
        func_arg_name = std::move(std::get<Token>(symbol_stack.top()));
        symbol_stack.pop();
        status_stack.pop();
        status_stack.pop();
        BlockScopeNode *block_scope;
        block_scope = block_scope_manager.find_value(std::get<Identifier>(*func_arg_type.token).name);
        if (block_scope && !block_scope->get_address())
        {
            func_param.arg_type.push_back(block_scope->get_type());
            func_param.arg_name.emplace_back(std::move(std::get<Identifier>(*func_arg_name.token).name));
        }
    }

    void Parser::define_pointer()
    {
        pointer_level++;
    }

    void Parser::define_again()
    {
        define_variable();
        symbol_stack.pop();
        status_stack.pop();
    }

    void Parser::define_finished()
    {
        status_stack.pop();
        symbol_stack.pop();
    }

    void Parser::wait_function_return_type()
    {
        status_stack.pop();
        symbol_stack.pop();
        status_stack.pop();
        status_stack.push(14);
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
            status_stack.pop();
            status_stack.push(4);
        }
        pointer_level = 0;
    }

    void Parser::check_priority()
    {
        unsigned char priority;
        switch (std::get<Punchation>(*current_input.token))
        {
        case Punchation::RPARENTHESES:
        {
            reduce_rparenthese();
            std::unordered_map<TokenObj, unsigned char, HashToken, TokenObjCmp>::iterator&& iter = priority_map.find(*current_input.token);
            if (priority_map.end() == iter) throw std::runtime_error("unexpect token");
            else priority = iter->second;
            return;
            break;
        }
        case Punchation::RBRACKETS:
            reduce_rbrackets();
            break;
        case Punchation::SEMICOLON:
        {
            while (back_ward_solve_expr());
            last_priority = 255;
            status_stack.pop();
            if (status_stack.top() == 16) 
            {
                llvm::MaybeAlign Align;
                llvm::Value* Ret = get_value(symbol_stack.top());
                BlockScopeNode* func_root_node = block_scope_manager.get_function_root_node();
                // builder.CreateStore(Ret, func_root_node->get_address());
                if (!func_param.return_block)
                {
                    func_param.return_block = llvm::BasicBlock::Create(context);
                    llvm::IRBuilder<> retbuilder(func_param.basic_blocks[0], func_param.basic_blocks[0]->begin());
                    const llvm::DataLayout &DL = mod->getDataLayout();
                    llvm::Align AllocaAlign = DL.getPrefTypeAlign(Ret->getType());
                    unsigned AddrSpace = DL.getAllocaAddrSpace();
                    llvm::Value* rval = retbuilder.Insert(new llvm::AllocaInst(Ret->getType(), AddrSpace, nullptr, AllocaAlign), "");
                    func_root_node->set_current_value(rval, Ret->getType());
                    retbuilder.SetInsertPoint(func_param.return_block);
                    if (!Align) 
                    {
                        const llvm::DataLayout &DL = mod->getDataLayout();
                        Align = DL.getABITypeAlign(func_root_node->get_type());
                    }
                    llvm::Value* ret_value = retbuilder.Insert(new llvm::LoadInst(func_root_node->get_type(), func_root_node->get_address(), llvm::Twine(), false, *Align), "");
                    retbuilder.CreateRet(ret_value);
                }
                else 
                {
                    if (func_root_node->get_type() != Ret->getType()) throw std::runtime_error("function can only have one return type!");
                }
                if (!Align) {
                    const llvm::DataLayout &DL = mod->getDataLayout();
                    Align = DL.getABITypeAlign(func_root_node->get_type());
                }
                block_scope_manager.get_current_node()->set_current_node_is_returned();
                builder.Insert(new llvm::StoreInst(Ret, func_root_node->get_address(), false, *Align));
                builder.CreateBr(func_param.return_block);
                symbol_stack.pop();
                symbol_stack.pop();
                status_stack.pop();
                return;
            }
            symbol_stack.pop();

            while (is_comma(symbol_stack.top()))
            {
                symbol_stack.pop();
                status_stack.pop();
                symbol_stack.pop();
                status_stack.pop();
            }
            return;
        }
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
        if (priority_map.end() == iter) return false;
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

    void Parser::function_body_define_strart()
    {
        const llvm::DataLayout& DL = mod->getDataLayout();
        symbol_stack.push(current_input);
        status_stack.push(15);
        llvm::BasicBlock* bb = llvm::BasicBlock::Create(context);
        builder.SetInsertPoint(bb);
        func_param.basic_blocks.push_back(bb);
        block_scope_manager.insert_function(func_param, DL);
        func_param.inserter = builder.GetInsertPoint();
        func_param.inserter--;
    }

    void Parser::function_body_define_finished()
    {
        BlockScopeNode* node = block_scope_manager.get_current_node();
        llvm::Type* ret_type;
        if (func_param.return_block)
        {
            ret_type = node->get_type();
        }
        else ret_type = builder.getVoidTy();
        if (!node->current_node_is_returned())
        {
            if (func_param.return_block)
            {
                builder.CreateBr(func_param.return_block);
            }
        }
        const llvm::DataLayout &DL = mod->getDataLayout();
        block_scope_manager.set_function(mod.get(), func_param, ret_type, DL);
        block_scope_manager.leave_scope();
        status_stack.pop();
        symbol_stack.pop();
        status_stack.pop();
        symbol_stack.pop();
        func_param.clear();
    }

    void Parser::reduce_condition_calculate()
    {
        CondParam *cond_param;
        while (back_ward_solve_expr());
        last_priority = 255;
        llvm::Value* cond = get_value(symbol_stack.top());
        symbol_stack.pop();
        status_stack.pop();
        cond_param = std::get<CondParam*>(symbol_stack.top());
        switch (status_stack.top())
        {
        case 17:
        {
            cond_param->body1 = llvm::BasicBlock::Create(context);
            cond_param->body2 = llvm::BasicBlock::Create(context);
            func_param.basic_blocks.push_back(cond_param->body1);
            func_param.basic_blocks.push_back(cond_param->body2);
            builder.CreateCondBr(cond, cond_param->body1, cond_param->body2);
            builder.SetInsertPoint(cond_param->body1);
            status_stack.pop();
            status_stack.push(18);
            break;
        }
        case 23:
        {
            cond_param->body1 = llvm::BasicBlock::Create(context);
            cond_param->exit_block = llvm::BasicBlock::Create(context);
            func_param.basic_blocks.push_back(cond_param->body1);
            func_param.basic_blocks.push_back(cond_param->exit_block);
            builder.CreateCondBr(cond, cond_param->body1, cond_param->exit_block);
            builder.SetInsertPoint(cond_param->body1);
            status_stack.pop();
            status_stack.push(25);
            break;
        }
        default:
            break;
        }
    }

    inline bool Parser::is_punc(const parser_input &_Token)
    {
        return ParserInputType::token == static_cast<ParserInputType>(_Token.index()) && TokenType::PUNCHATION == static_cast<TokenType>(std::get<Token>(_Token).token->index());
    }

    inline bool Parser::current_token_is_star(const Token &_Token)
    {
        return (static_cast<TokenType>(_Token.token->index()) == TokenType::PUNCHATION && std::get<Punchation>(*_Token.token) == Punchation::MUL);
    }

    inline bool Parser::is_rbarces(const Token &_Token)
    {
        return static_cast<TokenType>(_Token.token->index()) == TokenType::PUNCHATION && std::get<Punchation>(*_Token.token) == Punchation::RBRACES;
    }

    inline bool Parser::is_symbol(const parser_input &_Token)
    {
        return ParserInputType::token == static_cast<ParserInputType>(_Token.index()) && TokenType::SYMBOL == static_cast<TokenType>(std::get<Token>(_Token).token->index());
    }

    void Parser::check_if_body2()
    {
        CondParam* cond_param;
        status_stack.pop();
        symbol_stack.pop();
        cond_param = std::get<CondParam*>(symbol_stack.top());
        if (static_cast<TokenType>(current_input.token->index()) == TokenType::KEYWORD && std::get<KeyWord>(*current_input.token) == KeyWord::ELSE)
        {
            block_scope_manager.enter_scope();
            builder.SetInsertPoint(cond_param->body2);
            status_stack.pop();
            status_stack.push(21);
        }
        else
        {
            builder.SetInsertPoint(cond_param->body2);
            delete cond_param;
            status_stack.pop();
            symbol_stack.pop();
            process_token(); 
        }
    }

    void Parser::skip()
    {
    }

    bool Parser::back_ward_solve_expr()
    {
        parser_input value, punc = Token();
        bool flag = false;
        while (static_cast<ParserInputType>(punc.index()) == ParserInputType::token && symbol_stack.size() >= 2)
        {
            value = std::move(symbol_stack.top());
            symbol_stack.pop();
            status_stack.pop();

            process_stack.push_back(std::move(value));
get_value:  punc = std::move(symbol_stack.top());
            symbol_stack.pop();
            if (is_punc(punc) && check_priority(std::get<Punchation>(*std::get<Token>(punc).token))) process_stack.push_back(std::move(punc));
            else break;
            flag = true;
            status_stack.pop();
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
        symbol_stack.push(std::move(punc));
        if (!flag)
        {
            symbol_stack.emplace(std::move(process_stack.back()));
            process_stack.pop_back();
            status_stack.push(2);
        }
        else
        {
            clear_process_stack();
        }
        return flag;
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
            llvm::Value* rvalue,* lvalue;
            switch (punc)
            {
            case Punchation::ASSIGN:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, rvalue->getType(), true);
                llvm::MaybeAlign Align;
                if (!Align) {
                    const llvm::DataLayout &DL = mod->getDataLayout();
                    Align = DL.getABITypeAlign(rvalue->getType());
                }
                builder.Insert(new llvm::StoreInst(rvalue, lvalue, false, *Align));
                process_stack.push_back(rvalue);
                break;
            }
            case Punchation::LARGETHAN:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, nullptr);
                rvalue = builder.CreateICmpSLT(lvalue, rvalue);
                process_stack.push_back(rvalue);
                break;
            }
            case Punchation::MOD:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, nullptr);
                rvalue = builder.CreateSRem(lvalue, rvalue);
                process_stack.push_back(rvalue);
                break;
            }
            case Punchation::DIV:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, nullptr);
                rvalue = builder.CreateNSWSub(lvalue, rvalue);
                process_stack.push_back(rvalue);
                break;
            }
            case Punchation::ADD:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, nullptr);
                rvalue = builder.CreateNSWAdd(lvalue, rvalue);
                process_stack.push_back(rvalue);
                break;
            }
            case Punchation::SUB:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, nullptr);
                rvalue = builder.CreateNSWSub(lvalue, rvalue);
                process_stack.push_back(rvalue);
                break;                
            }
            case Punchation::MUL:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, nullptr);
                rvalue = builder.CreateNSWMul(lvalue, rvalue);
                process_stack.push_back(rvalue);
                break;     
            }
            case Punchation::EQU:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, nullptr);
                rvalue = builder.CreateICmpEQ(lvalue, rvalue);
                process_stack.push_back(rvalue);
                break;
            }
            case Punchation::LESSTHAN:
            {
                rvalue = get_value(rhs, nullptr);
                lvalue = get_value(lhs, nullptr);
                rvalue = builder.CreateICmpSLT(lvalue, rvalue);
                process_stack.push_back(rvalue);
                break;
            }
            default:
                throw std::runtime_error("unexpect binary operator");
            }
            //llvm::Value* lvalue = parser_input_to_llvmValue(lhs);
        }
        status_stack.emplace(2);
        symbol_stack.emplace(std::move(process_stack.back()));
        process_stack.pop_back();

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

    llvm::Value *Parser::get_value(parser_input &input, llvm::Type* rvalue_type, bool load_effective_address, bool reset_value)
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
                        const llvm::DataLayout &DL = mod->getDataLayout();
                        llvm::Align AllocaAlign = DL.getPrefTypeAlign(rvalue_type);
                        unsigned AddrSpace = DL.getAllocaAddrSpace();
                        llvm::Value* rval = builder.Insert(new llvm::AllocaInst(rvalue_type, AddrSpace, nullptr, AllocaAlign), "");
                        bs->set_current_value(rval, rvalue_type);
                    }
                }
                result = bs->get_address();
                if (!load_effective_address)
                {
                    llvm::MaybeAlign Align;
                    if (!Align) 
                    {
                        const llvm::DataLayout &DL = mod->getDataLayout();
                        Align = DL.getABITypeAlign(bs->get_type());
                    }
                    result = builder.Insert(new llvm::LoadInst(bs->get_type(), result, llvm::Twine(), false, *Align), "");
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

    std::pair<llvm::Function *, bool> Parser::get_callee(parser_input &input, llvm::ArrayRef<llvm::Value*> args)
    {
        BlockScopeNode* node = block_scope_manager.get_current_node();
        if (is_symbol(input))
        {
            std::string func_name = std::to_string(std::get<Identifier>(*std::get<Token>(input).token).name.size());
            func_name += std::get<Identifier>(*std::get<Token>(input).token).name;
            func_name += "@P";
            for (llvm::Value* arg : args)
            {
                std::string&& tmp = get_type_name(arg->getType());
                func_name += std::to_string(tmp.size());
                func_name += tmp;
            }
            std::pair<llvm::Function*, bool> function_ptr;
            while (node)
            {
                function_ptr = node->get_function(func_name);
                if (function_ptr.first) return std::move(function_ptr);
                node = node->get_parent();
            }
            throw std::runtime_error("function\"" + func_name + "\" has not defined in current scoup");
        }
        else
        {
            throw std::runtime_error("current input is not function name");
        }

    }

    bool Parser::is_comma(const parser_input &_Token)
    {
        return ParserInputType::token == static_cast<ParserInputType>(_Token.index()) && TokenType::PUNCHATION == static_cast<TokenType>(std::get<Token>(_Token).token->index()) && Punchation::COMMA == std::get<Punchation>(*std::get<Token>(_Token).token);
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
        if (iter != status_change_map[start].end())
        {
            iter->second = std::move(end);
        }
        else throw std::runtime_error("current node not set can't override!");
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
        tmp_node = current_node = root_node.get();
    }
    void BlockScopeManager::enter_scope(llvm::StringRef Name)
    {
        BlockScopeNode* _tmp_node = current_node->get_child_block(Name);
        if (!_tmp_node) 
        {
            current_node->insert_member_value(Name, nullptr, 0);
            _tmp_node = current_node->get_child_block(Name);
        }
        tmp_node = current_node = _tmp_node;
    }
    void BlockScopeManager::enter_scope()
    {
        enter_scope(std::string("anon.") + std::to_string(current_node->name_no++));
    }
    void BlockScopeManager::leave_scope()
    {
        if (current_node->name.size()) tmp_node = current_node = const_cast<BlockScopeNode*>(current_node->parent);
        else throw std::runtime_error("no parent scope");
    }

    void BlockScopeManager::insert_function(FuncParam& params, const llvm::DataLayout &DL)
    {
        llvm::SmallString<256> func_name = llvm::StringRef(std::to_string(params.function_name.size()));
        llvm::Align AllocaAlign;
        llvm::IRBuilder<> builder(params.basic_blocks[0], params.basic_blocks[0]->begin());
        BlockScopeNode* _current_node;
        func_name += params.function_name;
        func_name += "@P";
        for (llvm::Type* arg_type : params.arg_type)
        {
            std::string arg_name = get_type_name(arg_type);
            func_name += std::to_string(arg_name.size());
            func_name += arg_name;
            params.function_name += std::to_string(arg_name.size());
            params.function_name += arg_name;
        }
        enter_scope(func_name);
        for (int var = 0; var < params.arg_name.size(); var++)
        {
            AllocaAlign = DL.getPrefTypeAlign(params.arg_type[var]);
            unsigned AddrSpace = DL.getAllocaAddrSpace();
            llvm::Value* rval = builder.Insert(new llvm::AllocaInst(params.arg_type[var], AddrSpace, nullptr, AllocaAlign), "");
            variable_define(params.arg_name[var], 0);
            _current_node = find_value(params.arg_name[var]);
            _current_node->set_current_value(rval, params.arg_type[var]);
            
        }
        current_node->is_func_root = true;
    }

    void BlockScopeManager::set_function(llvm::Module *m, FuncParam params, llvm::Type* return_type, const llvm::DataLayout& DL)
    {
        llvm::FunctionType* _Ft;
        BlockScopeNode* _current_node;
        std::string func_name = get_function_name();
        llvm::Value* rvalue, *lvalue;
        params.inserter++;
        llvm::IRBuilder<> builder(params.basic_blocks[0], params.inserter);
        llvm::MaybeAlign Align;
        if (params.is_static || tmp_node->get_parent()->is_root()) _Ft = llvm::FunctionType::get(return_type, params.arg_type, params.is_var_args);
        else
        {
            params.arg_type.insert(params.arg_type.begin(), llvm::PointerType::get(m->getContext(), 0));
            _Ft = llvm::FunctionType::get(return_type, params.arg_type, params.is_var_args);
        }
        llvm::Function* fn = llvm::Function::Create(_Ft, llvm::GlobalValue::LinkageTypes::ExternalLinkage, func_name, m);
        for (llvm::BasicBlock* bb : params.basic_blocks)
        {
            bb->insertInto(fn);
        }
        if (params.return_block)
        {
            params.return_block->insertInto(fn);
        }
        for (int var = 0; var < params.arg_name.size(); var++)
        {
            _current_node = current_node->get_child_block(params.arg_name[var]);
            Align = DL.getABITypeAlign(params.arg_type[var]);
            rvalue = fn->getArg(var);
            lvalue = _current_node->get_address();
            builder.Insert(new llvm::StoreInst(rvalue, lvalue, false, *Align));
        }
        current_node->set_function(std::pair(fn, params.is_static));
        llvm::verifyFunction(*current_node->get_function().first, &llvm::errs());
    }

    std::string BlockScopeManager::get_function_name()
    {
        BlockScopeNode* tmp_scope = current_node->parent;
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
        name += current_node->name;
        return std::move(name);
    }

    bool BlockScopeManager::type_define(llvm::StringRef _Name, llvm::Type *_Type)
    {
        return current_node->insert_type(_Name, _Type);   
    }

    BlockScopeNode *BlockScopeManager::get_function_root_node()
    {
        BlockScopeNode* node = current_node;
        while (!node->is_function_root())
        {
            node = node->get_parent();
        }
        return node;
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
        BlockScopeNode* value = nullptr, *tmp = current_node;
        while (tmp)
        {
            value = tmp->get_child_block(Name);
            if (value) return value;
            tmp = tmp->get_parent();
        }
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

    void BlockScopeManager::find_and_enter(llvm::StringRef Name)
    {
        tmp_node = tmp_node->get_child_block(Name);
        element_pointer_seq.push_back(tmp_node->StructPointerNo);
    }

    void BlockScopeManager::clear_tmp_scope()
    {
        tmp_node = current_node;
    }

    BlockScopeNode::BlockScopeNode(llvm::StringRef Name, BlockScopeNode *_parent, llvm::Value* _Value, size_t _pointer_level) : name(Name), name_no(0), parent(_parent), current_value(_Value), value_type(nullptr), pointer_level(_pointer_level), function_ptr(std::pair<llvm::Function*, bool>(nullptr, false)), is_struct(false)
    {
        is_struct = is_func_root = is_signed = has_return = false;
    }
    const std::pair<llvm::Function*, bool> BlockScopeNode::get_function(llvm::StringRef Name)
    {
        llvm::StringMap<std::unique_ptr<BlockScopeNode>>::const_iterator it = children.find(Name);
        if (it != children.end()) return it->second->function_ptr;
        return std::pair<llvm::Function*, bool>(nullptr, false);
    }
    BlockScopeNode *BlockScopeNode::get_child_block(llvm::StringRef Name)
    {
        llvm::StringMap<std::unique_ptr<BlockScopeNode>>::const_iterator it = children.find(Name);
        if (it != children.end()) return it->second.get();
        return nullptr;
    }
    void BlockScopeNode::insert_function(llvm::StringRef Name, std::pair<llvm::Function*, bool> Func)
    {
        if (!this->get_child_block(Name))
        {
            std::unique_ptr<BlockScopeNode> bs(new BlockScopeNode(Name, this, nullptr, 0));
            bs->set_function(std::move(Func));
            children.insert(std::pair(Name, std::move(bs)));
            has_return = false;
        }
        else 
        {
            throw std::runtime_error("current symbol name \"" + std::string(Name) + "\" has already exist!");
        }
        // member_functions.insert(std::pair(Name, std::move(Func)));
    }

    bool BlockScopeNode::insert_member_value(llvm::StringRef Name, llvm::Value *_Value, size_t _pointer_level)
    {
        if (!this->get_child_block(Name))
        {
            children.insert(std::pair(Name, std::unique_ptr<BlockScopeNode>(new BlockScopeNode(Name, this, _Value, _pointer_level))));
            return true;
        }
        else return false;
    }
    bool BlockScopeNode::insert_type(llvm::StringRef Name, llvm::Type *_Type)
    {

        if (!this->get_child_block(Name))
        {
            std::unique_ptr<BlockScopeNode> bs(new BlockScopeNode(Name, this, nullptr, 0));
            bs->set_type(_Type);
            children.insert(std::pair(Name, std::move(bs)));
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
    FunctionParameters::FunctionParameters()
    {
        clear();
    }
    ConditionParameters::ConditionParameters()
    {
        clear();
    }
};