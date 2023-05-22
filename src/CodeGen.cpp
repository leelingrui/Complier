#include <CodeGen.h>

namespace lpp
{
    CodeGenerator::CodeGenerator()
    {
        
    }

    void CodeGenerator::dump()
    {
        std::cout << asm_stream.view();
    }

    void CodeGenerator::CreateAdd(llvm::Value * lvalue, llvm::Value * rvalue)
    {

    }

    void CodeGenerator::AllocLocalVariable(llvm::Value * value, llvm::Type* type)
    {
        local_variable.insert(std::pair(value, stack_offset));
        const llvm::DataLayout& DL = mod->getDataLayout();
        stack_offset += DL.getTypeAllocSize(type);
    }
    void CodeGenerator::LoadRegester(lethon::LethonRegName reg_name, llvm::Value * value, llvm::Value* ptr, lethon::LethonRegName offset_reg)
    {
        std::string_view reg_name_str = magic_enum::enum_name(reg_name);
        asm_stream.sputn("MOV %", 5);
        asm_stream.sputn(reg_name_str.data(), reg_name_str.size());
        asm_stream.sputn(", ", 2);
        std::map<llvm::Value*, size_t>::iterator iter = local_variable.find(ptr);
        std::string&& offset_str = std::to_string(iter->second);
        asm_stream.sputn(offset_str.c_str(), offset_str.size());
        asm_stream.sputn("(%", 2);
        std::string_view offset_reg_str = magic_enum::enum_name(offset_reg);
        asm_stream.sputn(offset_reg_str.data(), offset_reg_str.size());
        asm_stream.sputn(")\n", 2);
    }
    lethon::LethonRegName CodeGenerator::getRegester(llvm::Value * value)
    {
        std::map<llvm::Value*, lethon::LethonRegName>::iterator iter = value_map.find(value);
        lethon::LethonRegName reg_name;
        if (iter == value_map.end())
        {
            iter = value_map.insert(std::pair(value, lethon::LethonRegName::NUMS)).first;
        }
        if (iter->second == lethon::LethonRegName::NUMS)
        {
            reg_name = swap_queue.front();
            swap_queue.pop();
            // LoadRegester(reg_name, value);
            iter->second = reg_name;
        }
        return iter->second;

    }
    std::unique_ptr<std::fstream> CodeGenerator::GenerateAsm(std::unique_ptr<llvm::Module> _mod)
    {
        // LoadRegester(lethon::LethonRegName::RAX, nullptr);
        llvm::Module::FunctionListType& flist = mod->getFunctionList();
        for (llvm::Function& func : flist)
        {
            std::cout << std::string(func.getName()) << std::endl;
            llvm::Function::BasicBlockListType& bbs =  func.getBasicBlockList();
            for (llvm::BasicBlock& bb : bbs)
            {
                llvm::BasicBlock::InstListType& insts = bb.getInstList();
                std::cout << std::string() << std::endl;
                for (llvm::Instruction& inst : insts)
                {
                    llvm::Use* use = inst.getOperandList();
                    use->get();

                }
            }
        }
    }
    void CodeGenerator::clear()
    {
        asm_stream.str("");
        stack_offset = 0;
        reg_vec.resize(4, nullptr);
        local_variable.clear();
        value_map.clear();
    }
} // namespace lpp
