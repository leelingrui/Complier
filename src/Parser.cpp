#include <Parser.h>

namespace lpp
{
    Parser::Parser() : struct_finder(new StructTypeFinder), builder(context)
    {
    //     static llvm::LLVMContext MyGlobalContext;
    //     llvm::LLVMContext &context = MyGlobalContext;
	
	// //创建一个module
	//     llvm::Module *module = new llvm::Module("test", context);
    //     llvm::SmallVector<llvm::Type *, 2> functionArgs;
    // //各种类型都是通过get方法来构造对应的实例
    //     functionArgs.push_back(llvm::Type::getInt32Ty(context)); //32为整型参数Tina及到vector数组中
    //     functionArgs.push_back(llvm::Type::getInt32Ty(context));
    //     llvm::Type *returnType = llvm::Type::getInt32Ty(context);//返回值类型
    //     llvm::FunctionType *max_type = llvm::FunctionType::get(returnType, functionArgs, /*isVarArg*/ false);
    //     llvm::FunctionCallee max_fun = module->getOrInsertFunction("max", max_type);//将函数插入module
    }

    Parser::Parser(std::string_view source_file) : struct_finder(new StructTypeFinder), builder(context)
    {
        
    }

    void Parser::reduce_operatorbrace()
    {
        std::vector<llvm::Value*> args;
        llvm::SmallString<256> func_name("_FN");
        while (exprs)
        {
            args.push_back(std::get<llvm::Value*>(symbol_stack.top()));
            symbol_stack.pop();
            status_stack.pop();
            exprs--;
        }
        parser_input& callee = symbol_stack.top();
        symbol_stack.pop();
        status_stack.pop();
        switch (static_cast<ParserInputType>(callee.index()))
        {
        case ParserInputType::value:
        {
            llvm::Value* value = std::get<llvm::Value*>(callee);
            func_name.append(value->getName());
            func_name.append("10operator()@P");
            for (int var = 0; var < args.size(); var++)
            {
                func_name.append(std::to_string(args[var]->getName().size()));
                func_name.append(args[var]->getName());
            }
            llvm::Function* func_callee = builder.GetInsertBlock()->getParent()->getParent()->getFunction(func_name);
            symbol_stack.emplace(parser_input(static_cast<llvm::Value*>(builder.CreateCall(func_callee, args))));
            break;
        }
        case ParserInputType::token:
        {
            Token& token = std::get<Token>(callee);
            switch (static_cast<TokenType>(token.token->index()))
            {
            case TokenType::SYMBOL:
            {
                Identifier& id = std::get<Identifier>(*token.token);
                func_name.append(std::to_string(id.name.size()));
                func_name.append(id.name);
                for (int var = 0; var < args.size(); var++)
                {
                    func_name.append(std::to_string(args[var]->getName().size()));
                    func_name.append(args[var]->getName());
                }
                llvm::Function* func_callee = builder.GetInsertBlock()->getParent()->getParent()->getFunction(func_name);
                symbol_stack.emplace(parser_input(static_cast<llvm::Value*>(builder.CreateCall(func_callee, args))));
                break;
            }
            case TokenType::KEYWORD:
            {
                if (std::get<KeyWord>(*token.token) == KeyWord::SIZEOF && exprs == 1)
                {
                    llvm::DataLayout layout = builder.GetInsertBlock()->getModule()->getDataLayout();
                    llvm::TypeSize size = layout.getTypeAllocSize(args[0]->getType());
                    symbol_stack.push(builder.getInt64(size.getFixedSize()));
                }
                else throw std::runtime_error("multiple argument has been passed");
                break;
            }
            default:
                except_token = std::move(token);
                throw std::runtime_error(std::string("symbol uncallable"));
                break;
            }
            break;
        }
        case ParserInputType::function:
        {
            llvm::Function* func = std::get<llvm::Function*>(callee);
            symbol_stack.emplace(parser_input(static_cast<llvm::Value*>(builder.CreateCall(func, args))));
            break;
        }
        }
        std::map<parser_input, status>::iterator&& iter = goto_table[get_current_status()].find(symbol_stack.top());
        if (iter == goto_table[get_current_status()].end()) status_stack.emplace(iter->second);
        else
        {
            throw std::runtime_error("undefind next status");
        }
    }

    void Parser::reduce_rbraces()
    {
        bool flag = false;
        while (!symbol_stack.empty())
        {
            parser_input &token = symbol_stack.top();
            process_stack.push_back(std::move(token));
            symbol_stack.pop();
            if (static_cast<ParserInputType>(token.index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(token).token->index()) == TokenType::PUNCHATION && std::get<Punchation>(*std::get<Token>(token).token) == Punchation::RBRACES)
            {
                flag = true;
                break;
            }
        }
        if (flag)
        {
            if (static_cast<ParserInputType>(symbol_stack.top().index()) == ParserInputType::value || static_cast<ParserInputType>(symbol_stack.top().index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(symbol_stack.top()).token->index()) == TokenType::SYMBOL)
            {
                reduce_operatorbrace();
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
        Punchation punc = 0;
        bool flag = false;
        while (!symbol_stack.empty())
        {
            parser_input &token = symbol_stack.top();
            process_stack.push_back(std::move(token));
            symbol_stack.pop();
            if (static_cast<ParserInputType>(token.index()) == ParserInputType::token && static_cast<TokenType>(std::get<Token>(token).token->index()) == TokenType::PUNCHATION && std::get<Punchation>(*std::get<Token>(token).token) == Punchation::RBRACES)
            {
                flag = true;
                break;
            }
        }
    }

    void Parser::init_node(status start, const parser_input& status_change, DFA_STATUS_CHANGE_NODE end)
    {
        std::map<parser_input, DFA_STATUS_CHANGE_NODE>::iterator&& iter = status_change_map[start].find(status_change);
        if (iter != status_change_map[start].end())
        {
            iter->second = std::move(end);
        }
        else 
        {
            throw std::runtime_error("undefind next status");
        }
        
    }
    void Parser::next_status(parser_input next)
    {
        std::map<parser_input, DFA_STATUS_CHANGE_NODE>::iterator&& iter = status_change_map[status_stack.top()].find(next);
        if (iter != status_change_map[status_stack.top()].end())
        {
            DFA_STATUS_CHANGE_NODE& status_change_node = iter->second;
            switch (status_change_node.index())
            {
            //true means reduce
            case 0:
                std::get<std::function<void()>>(status_change_node)();
                break;
            case 1:
                status_stack.push(std::get<status>(status_change_node));
                symbol_stack.push(std::move(next));
                break;
            }
        }
        else 
        {
            throw std::runtime_error("undefind next status");
        }
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
        current_namespace.insert(std::move(space));
    }
    void NameSpaceManager::insert_tmp(std::string space)
    {
        tmp_namespace.insert(std::move(space));
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
}
