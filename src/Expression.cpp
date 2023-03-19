// #include <Expression.h>

// namespace lpp
// {
//     Expression::Expression(StructTypeFinder* _finder) : finder(_finder)
//     {
//         solve_expression = new PrecedenceClimbing(_finder);
//     }

//     void Expression::insert(Token &_token)
//     {
//         solve_expression->insert(_token);
//     }

//     void Expression::insert(Token &&_token)
//     {
//         solve_expression->insert(std::move(_token));
//     }

//     ExpressionEvaluator::ExpressionEvaluator(StructTypeFinder* _finder) : finder(_finder)
//     {
//     }

//     void ExpressionEvaluator::insert(Token &_token)
//     {
//         tokens.emplace(_token);
//     }

//     void ExpressionEvaluator::insert(Token &&_token)
//     {
//         tokens.emplace(std::move(_token));
//     }

//     void Expression::get_expression_result(llvm::IRBuilder<>& builder)
//     {
//         solve_expression->get_expression(builder);
//     }

//     void RecursiveDescent::get_expression(llvm::IRBuilder<>& builder)
//     {
//         Token current_token = std::move(tokens.front());
//         tokens.pop();
//         get_expression(builder);
//     }

//     PrecedenceClimbing::PrecedenceClimbing(StructTypeFinder* _finder) : ExpressionEvaluator(_finder), OPTR(), OPTD()
//     {
//     }

//     void PrecedenceClimbing::get_expression(llvm::IRBuilder<> &builder)
//     {
//         while (!tokens.empty())
//         {
//             Token& current_token = tokens.front();
//             switch (static_cast<TokenType>(current_token.token->index()))
//             {
//             case TokenType::DIGIT:
//             {
//                 OPTD.emplace(current_token);
//                 last_type = TokenType::DIGIT;
//                 break;
//             }
//             case TokenType::SYMBOL:
//                 OPTD.emplace(current_token);
//                 break;
//             case TokenType::PUNCHATION:
                
//                 if (!OPTR.empty())
//                 {
//                     if (priority_map[*current_token.token] > priority_map[OPTR.top().token])
//                     {
//                         take_one(builder);
//                     }
//                 }
//                 OPTR.emplace(std::move(current_token));
//                 break;
//             default:
//                 break;
//             }
//             tokens.pop();
//         }
//         try
//         {
//             while(!OPTR.empty()) take_one(builder);
//         }
//         catch(const std::exception& e)
//         {
//             std::cerr << e.what() << '\n';
//         }
//         while (!OPTR.empty())
//         {
//         }
//     }

//     ExpressionEvaluator::~ExpressionEvaluator()
//     {
//     }

//     void PrecedenceClimbing::take_one(llvm::IRBuilder<> &builder)
//     {
//         OPTDValue RHS = std::move(OPTD.top());
//         OPTDValue LHS = std::move(OPTD.top());
//         llvm::Value* lhs_value;
//         llvm::FunctionCallee functionCallee;
//         switch (static_cast<OPTDType>(LHS.index()))
//         {
//         case OPTDType::Symbol:
//             lhs_value = token_to_value(builder, std::get<Token>(LHS));
//             break;
//         case OPTDType::Value:
//             lhs_value = std::get<llvm::Value*>(LHS);
//             break;
//         case OPTDType::Function:
//             functionCallee = std::get<llvm::FunctionCallee>(LHS);
//         default:
//             break;
//         }
//         Token& current_operator = OPTR.top();
//         switch (std::get<Punchation>(*current_operator.token))
//         {
//         case Punchation::ADD:
//             //builder.CreateAdd();
//             break;
//         case Punchation::SUB:
//             //builder.CreateSub();
//             break;
//         case Punchation::MUL:
//             //builder.CreateMul();
//             break;
//         case Punchation::DIV:
//             //builder.CreateSDiv
//         default:
//             break;
//         }
//         OPTD.emplace();
//         OPTR.pop();
//     }

//     llvm::Value *token_to_value(const llvm::IRBuilder<> &builder, Token &_token)
//     {
//         switch (static_cast<TokenType>(_token.token->index()))
//         {
//         case TokenType::DIGIT:
//         {
//             digit2llvmvlue transformer(builder.getContext());
//             return transformer(std::get<Digit>(*_token.token));
//         }
//         case TokenType::SYMBOL:
//         {
//             llvm::BasicBlock* bb = builder.GetInsertBlock();
//             llvm::ValueSymbolTable* st = bb->getValueSymbolTable();
//             llvm::Value* value = st->lookup(std::get<Identifier>(*_token.token).name);
//             if (value) return value;
//             bb->getModule()->getValueSymbolTable();
//             return value;
//         }
//         default:
//             throw std::runtime_error("current type can't convert to llvm value");
//             break;
//         }
//     }
// } // namespace lpp
