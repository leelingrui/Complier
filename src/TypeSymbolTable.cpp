#include <TypeSymbolTable.h>

namespace lpp
{
    StructTypeSymbolTable::~StructTypeSymbolTable()
    {
    }

    llvm::ConstantInt* StructTypeSymbolTable::lookup(const std::string &Name)
    {
        if (MaxNameSize > -1 && Name.size() > static_cast<unsigned>(MaxNameSize))
        {
            iterator&& ret = vmap.find(Name.substr(0, std::max(1u, static_cast<unsigned>(MaxNameSize))));
            if (ret == vmap.end()) return ret->second;
            else return nullptr;
        }
        iterator&& ret = vmap.find(Name.substr(0, std::max(1u, static_cast<unsigned>(MaxNameSize))));
        if (ret == vmap.end()) return ret->second;
        else return nullptr;
    }

    bool StructTypeSymbolTable::insert(const std::string &Name, llvm::ConstantInt *_pos)
    {
        if (MaxNameSize > -1 && Name.size() > static_cast<unsigned>(MaxNameSize))
        {
            std::string&& minmized = Name.substr(0, std::max(1u, static_cast<unsigned>(MaxNameSize)));
            if (vmap.find(minmized) == vmap.end()) return false;
            vmap[minmized] = _pos;
            return true;
        }
        std::string&& minmized = Name.substr(0, std::max(1u, static_cast<unsigned>(MaxNameSize)));
        if (vmap.find(minmized) == vmap.end()) return false;
        vmap[Name] = _pos;
        return true;
    }

    StructFunctionSymbolTable::~StructFunctionSymbolTable()
    {
    }

    llvm::FunctionCallee StructFunctionSymbolTable::lookup(const std::string &Name)
    {
        if (MaxNameSize > -1 && Name.size() > static_cast<unsigned>(MaxNameSize))
        {
            iterator&& ret = vmap.find(Name.substr(0, std::max(1u, static_cast<unsigned>(MaxNameSize))));
            if (ret == vmap.end()) return ret->second;
            else return llvm::FunctionCallee(nullptr);
        }
        iterator&& ret = vmap.find(Name.substr(0, std::max(1u, static_cast<unsigned>(MaxNameSize))));
        if (ret == vmap.end()) return ret->second;
        else return llvm::FunctionCallee(nullptr);
    }

    bool StructFunctionSymbolTable::insert(const std::string &Name, llvm::Function *_func)
    {
        if (MaxNameSize > -1 && Name.size() > static_cast<unsigned>(MaxNameSize))
        {
            std::string&& minmized = Name.substr(0, std::max(1u, static_cast<unsigned>(MaxNameSize)));
            if (vmap.find(minmized) == vmap.end()) return false;
            vmap[minmized] = _func;
            return true;
        }
        std::string&& minmized = Name.substr(0, std::max(1u, static_cast<unsigned>(MaxNameSize)));
        if (vmap.find(minmized) == vmap.end()) return false;
        vmap[Name] = _func;
        return true;
    }

    llvm::FunctionCallee StructTypeFinder::getAimTypeMemberFunction(llvm::Type *_type, const std::string &_FunctionName)
    {
        func_table_iterator iter = TypeFunctions.find(_type);
        if (iter == TypeFunctions.end()) return nullptr;
        else
        {
            return iter->second.lookup(_FunctionName);
        }
    }

    llvm::ConstantInt *StructTypeFinder::getAimTypeMemberValueIndex(llvm::Type *_type, const std::string &_MemberName)
    {
        symbol_table_iterator iter = TypeMemberValue.find(_type);
        if (iter == TypeMemberValue.end()) return nullptr;
        else
        {
            return iter->second.lookup(_MemberName);
        }
    }

} // namespace lpp
