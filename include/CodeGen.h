#ifndef CODEGEN_H
#define CODEGEN_H
#include <llvm/IR/Module.h>
#include <llvm/IR/InstVisitor.h>
#include <fstream>

namespace lpp
{
    class CodeGenerator
    {
    public:
        std::unique_ptr<std::fstream> GenerateAsm(std::unique_ptr<llvm::Module> mod);
    private:
    };
} // namespace lpp

#endif