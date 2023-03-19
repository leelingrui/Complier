#ifndef TYPE_H
#define TYPE_H
#include <llvm/IR/Type.h>
#include <llvm/ADT/StringMap.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Value.h>
#include <llvm/IR/Constants.h>
#include <variant>
#include <map>
#include <string>
#include <optional>
namespace lpp
{
    //using StructInnerType = std::variant<std::monostate, , llvm::Function*>
    class StructTypeSymbolTable
    {
    public:
        using TypeMap = std::map<std::string, llvm::ConstantInt*>;
        using iterator = TypeMap::iterator;
        using const_iterator = TypeMap::const_iterator;
        StructTypeSymbolTable(int MaxNameSize = -1) : vmap(), MaxNameSize(MaxNameSize) {}
        ~StructTypeSymbolTable();
        llvm::ConstantInt* lookup(const std::string& Name);

        inline bool empty() const { return vmap.empty(); }
        inline unsigned size() const { return unsigned(vmap.size()); }
        inline iterator begin() { return vmap.begin(); }
        inline const_iterator begin() const { return vmap.begin(); }
        inline iterator end() { return vmap.end(); }
        inline const_iterator end() const { return vmap.end(); }
        bool insert(const std::string& Name, llvm::ConstantInt* _pos);
    private:
        //TypeName *createValueName(StringRef Name, Value *V);
        //void removeValueName(ValueName *V);
        TypeMap vmap;
        int MaxNameSize;
        mutable uint32_t LastUnique = 0;
    };

    class StructFunctionSymbolTable
    {
    public:
        using TypeMap = std::map<std::string, llvm::FunctionCallee>;
        using iterator = TypeMap::iterator;
        using const_iterator = TypeMap::const_iterator;
        StructFunctionSymbolTable(int MaxNameSize = -1) : vmap(), MaxNameSize(MaxNameSize) {}
        ~StructFunctionSymbolTable();
        llvm::FunctionCallee lookup(const std::string& Name);
        bool insert(const std::string& Name, llvm::Function* _func);
        inline bool empty() const { return vmap.empty(); }
        inline unsigned size() const { return unsigned(vmap.size()); }
        inline iterator begin() { return vmap.begin(); }
        inline const_iterator begin() const { return vmap.begin(); }
        inline iterator end() { return vmap.end(); }
        inline const_iterator end() const { return vmap.end(); }
    private:
        //TypeName *createValueName(StringRef Name, Value *V);
        //void removeValueName(ValueName *V);
        TypeMap vmap;
        int MaxNameSize;
        mutable uint32_t LastUnique = 0;
    };

    class StructTypeFinder
    {
        using func_table_iterator = std::map<llvm::Type*, StructFunctionSymbolTable>::iterator;
        using func_table_const_iterator = std::map<llvm::Type*, StructFunctionSymbolTable>::const_iterator;
        using symbol_table_iterator = std::map<llvm::Type*, StructTypeSymbolTable>::iterator;
        using symbol_table_const_iterator = std::map<llvm::Type*, StructTypeSymbolTable>::const_iterator;

    public:
        llvm::FunctionCallee getAimTypeMemberFunction(llvm::Type* _type, const std::string& _FunctionName);
        llvm::ConstantInt* getAimTypeMemberValueIndex(llvm::Type* _type, const std::string& _MemberName);

    private:
        std::map<llvm::Type*, StructFunctionSymbolTable> TypeFunctions;
        std::map<llvm::Type*, StructTypeSymbolTable> TypeMemberValue;
    };

} // namespace lpp




#endif