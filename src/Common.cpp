#include <Common.h>

namespace lpp
{
    DFA::DFA(size_t state_size, status start)
    {
    }
    // bool DFA::load(std::istream &input_stream)
    // {
    //     size_t state_num;
    //     status start, change, end;
    //     input_stream >> state_num >> start_state;
    //     status_change_map.resize(state_num, std::vector<status>(state_num, -1));
    //     if (input_stream.good())
    //     {
    //         while (input_stream >> start >> change >> end)
    //         {
    //             status_change_map[start][change] = end;
    //         }
    //         return true;
    //     }
    //     else return false;
    // }
    // bool DFA::save(std::ostream &output_stream)
    // {
    //     output_stream << status_change_map.size() << start_state;
    //     if (output_stream.good())
    //     {
    //         for (status var1 = 0; var1 < status_change_map.size(); var1++)
    //         {
    //             for (status var2 = 0; var2 < status_change_map[var1].size(); var2++)
    //             {
    //                 if (status_change_map[var1][var2] != -1)
    //                 {
    //                     output_stream << var1 << ' ' << var2 << status_change_map[var1][var2] << '\n';
    //                 }
    //             }
    //         }
    //         return true;
    //     }
    //     else return false;
    // }
    status DFA::get_current_status()
    {
        if (status_stack.size()) return status_stack.top();
        else return 0;
    }
    llvm::Value* digit2llvmvlue::operator()(const Digit& digit)
    {
        llvm::Value* result;
        switch (static_cast<ValueType>(digit.index()))
        {
        case ValueType::Invalid:
            throw std::runtime_error("invalid number type");
        case ValueType::BOOL:
            result = llvm::ConstantInt::get(llvm::Type::getInt1Ty(context), std::visit(DigitVisitor(), digit), true);
            break;
        case ValueType::i8: 
            result = llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), std::visit(DigitVisitor(), digit), true);
            break;
        case ValueType::i16:
            result = llvm::ConstantInt::get(llvm::Type::getInt16Ty(context), std::visit(DigitVisitor(), digit), true);
            break;
        case ValueType::i32:
            result = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), std::visit(DigitVisitor(), digit), true);
            break;
        case ValueType::i64:
            result = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), std::visit(DigitVisitor(), digit), true);
            break;
        case ValueType::f32:
            result = llvm::ConstantInt::get(llvm::Type::getFloatTy(context), std::visit(DigitVisitor(), digit), true);
            break;
        case ValueType::f64:
            result = llvm::ConstantInt::get(llvm::Type::getDoubleTy(context), std::visit(DigitVisitor(), digit), true);
            break;
        case ValueType::u8:
            result = llvm::ConstantInt::get(llvm::Type::getInt8Ty(context), std::visit(DigitVisitor(), digit), false);
            break;
        case ValueType::u16:
            result = llvm::ConstantInt::get(llvm::Type::getInt16Ty(context), std::visit(DigitVisitor(), digit), false);
            break;
        case ValueType::u32:
            result = llvm::ConstantInt::get(llvm::Type::getInt32Ty(context), std::visit(DigitVisitor(), digit), false);
            break;
        case ValueType::u64:
            result = llvm::ConstantInt::get(llvm::Type::getInt64Ty(context), std::visit(DigitVisitor(), digit), false);
            break;
        default:
            throw std::runtime_error("unknow type exception");
            break;
        }
        return result;
    }
    std::string get_type_name(llvm::Type *type)
    {
        std::string name;
        switch (static_cast<llvm::Type::TypeID>(type->getTypeID()))
        {
        case llvm::Type::TypeID::IntegerTyID:
        {
            name += 'i';
            name += std::to_string(static_cast<llvm::IntegerType*>(type)->getBitWidth());
            break;
        }
        case llvm::Type::TypeID::FloatTyID:
        {
            name += "float";
            break;
        }
        case llvm::Type::TypeID::DoubleTyID:
        {
            name += "double";
            break;
        }
        case llvm::Type::TypeID::StructTyID:
        {
            name += static_cast<llvm::IntegerType*>(type)->getStructName();
            break;
        }
        default:
            break;
        }
        return std::move(name);
    }
}
