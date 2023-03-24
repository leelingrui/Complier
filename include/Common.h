#ifndef COMMON_H
#define COMMON_H
#include <vector>
#include <string>
#include <variant>
#include <Token.h>
#include <exception>
#include <cstring>
#include <map>
#include <stack>
#include <functional>
#include <SymbolTable.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Constants.h>

#define DFA_INVALID_STATUS -1

namespace lpp
{
    std::string get_type_name(llvm::Type* type);
    using status = long long;
    class DFA
    {
    public:
        enum class DFA_PROCESS_TYPE
        {
            S, R
        };
        using DFA_STATUS_CHANGE_NODE = std::variant<status, std::function<void()>>;
        DFA(size_t status_size = 0, status start = 0);
        bool load(std::istream& input_stream);
        bool save(std::ostream& output_stream);
        virtual status get_current_status();
    protected:
        std::stack<status> status_stack;
    };

    class digit2llvmvlue final
    {
    public:
        digit2llvmvlue(llvm::LLVMContext& _context) : context(_context) {}
        llvm::Value* operator()(const Digit& digit);
    private:
        llvm::LLVMContext& context;
    };
};

#endif