#ifndef CODEGEN_H
#define CODEGEN_H
#include <llvm/IR/Module.h>
#include <llvm/IR/InstVisitor.h>
#include <llvm/CodeGen/ISDOpcodes.h>
#include <llvm/CodeGen/RegAllocGreedy.h>
#include <llvm/CodeGen/Register.h>
#include <llvm/CodeGen/RegAllocPBQP.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/BinaryByteStream.h>
#include <streambuf>
#include <sstream>
#include <queue>
#include <VM.h>
#include <fstream>
#include <iostream>
#include <magic_enum.hpp>

namespace lpp
{
    class CodeGenerator
    {
    public: 
        CodeGenerator();
        void dump();
        void CreateAdd(llvm::Value* lvalue, llvm::Value* rvalue);
        void AllocLocalVariable(llvm::Value* value, llvm::Type* type);
        void LoadRegester(lethon::LethonRegName reg_name, llvm::Value* value, llvm::Value* ptr, lethon::LethonRegName offset_reg = lethon::LethonRegName::RSP);
        lethon::LethonRegName getRegester(llvm::Value* value);
        std::unique_ptr<std::fstream> GenerateAsm(std::unique_ptr<llvm::Module> _mod);
        void clear();
    private:
        std::unique_ptr<llvm::Module> mod;
        std::stringbuf asm_stream;
        size_t stack_offset;
        std::queue<lethon::LethonRegName> swap_queue;
        std::vector<llvm::Value*> reg_vec;
        std::map<llvm::Value*, lethon::LethonRegName> value_map;
        std::map<llvm::Value*, size_t> local_variable; 
    };
} // namespace lpp

#endif