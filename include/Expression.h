#ifndef EXPRESSION_H
#define EXPRESSION_H

#include <Common.h>
#include <Token.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/ValueSymbolTable.h>
#include <llvm/IR/Module.h>
#include <stack>
#include <unordered_map>
#include <functional>
#include <queue>
#include <TypeSymbolTable.h>
#include <vector>

namespace lpp
{
    class ExpressionEvaluator
    {
    public:
        ExpressionEvaluator(StructTypeFinder* _finder);
        virtual void insert(Token& _token);
        virtual void insert(Token&& _token);
        virtual void get_expression(llvm::IRBuilder<>& builder) = 0;
        virtual ~ExpressionEvaluator();
    protected:
        StructTypeFinder* finder;
        TokenType last_type;
        std::queue<Token> tokens;
        
    };

    class RecursiveDescent : public ExpressionEvaluator
    {
    public:
        void get_expression(llvm::IRBuilder<>& builder) override;
    protected:
        
    };
    class PrecedenceClimbing : public ExpressionEvaluator
    {
    public:
        using OPTDValue = std::variant<Token, llvm::Value*, llvm::FunctionCallee>;
        PrecedenceClimbing(StructTypeFinder* _finder);
        void get_expression(llvm::IRBuilder<>& builder) override;
        void take_one(llvm::IRBuilder<> &builder);
    private:
        enum class OPTDType : unsigned char
        {
            Symbol, Value, Function
        };
        std::stack<OPTDValue> OPTD;
        std::stack<Token> OPTR;
    };

    class Expression
    {
    public:
        Expression(StructTypeFinder* _finder);
        void insert(Token &_token);
        void insert(Token &&_token);
        void get_expression_result(llvm::IRBuilder<>& builder); 
    protected:
        StructTypeFinder* finder;
        ExpressionEvaluator* solve_expression;
        
    };
    llvm::Value*token_to_value(const llvm::IRBuilder<>& builder, Token& _token);
}

#endif