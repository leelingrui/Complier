#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/ExecutionEngine/MCJIT.h"
#include "llvm/Support/TargetSelect.h"
#include <llvm/Transforms/Utils/Cloning.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Transforms/IPO/AlwaysInliner.h>
#include <iostream>
 
#include <iostream>
#include <Common.h>
#include <Lexer.h>
#include <fstream>
#include <iterator>
#include <Expression.h>


using namespace llvm;

int main()
{
	static LLVMContext MyGlobalContext;
    LLVMContext &context = MyGlobalContext;
	
	//创建一个module
	Module *module = new Module("test", context);
	IRBuilder<> b(context);
    
	//声明一个函数-----------------------------------
	SmallVector<Type *, 2> functionArgs;
    //各种类型都是通过get方法来构造对应的实例
    functionArgs.push_back(Type::getInt32Ty(context)); //32为整型参数Tina及到vector数组中
    functionArgs.push_back(Type::getInt32Ty(context));
	Type *returnType = Type::getInt32Ty(context);//返回值类型
	FunctionType *max_type = FunctionType::get(returnType, functionArgs, /*isVarArg*/ false);
    SmallVector<Type*, 2> structArgs;
    structArgs.push_back(Type::getInt1Ty(context)); //32为整型参数Tina及到vector数组中
    structArgs.push_back(Type::getInt64Ty(context));
	//构建函数签名，包括返回类型和参数，这种方式有些特殊有些特殊，到/usr/local/include/llvm/IR目录下看TypeBuilder.h源码就知道了
	//下面这种构建函数类型的方式和上面的选一个就可以了，很显然下面这种可以用C++的类型，方便很多，语法也很简单，长得也好看
	//FunctionType *max_type = TypeBuilder<int(int, int), false>::get(context);
    
	//声明 int max(int,a int b); 函数
	//cast将指针或者引用从基类转向为派生类，可以从http://llvm.org/docs/ProgrammersManual.html#the-c-standard-template-library这个链接学习
	FunctionCallee max_funee = module->getOrInsertFunction("max", max_type);//将函数插入module
	//-----------------------------------------------
	Function* max_fun = module->getFunction("max");
	//函数体-----------------------------------------------
	//存储参数（获取参数的引用）
	max_fun->getArg(0)->setName("a");
	max_fun->getArg(1)->setName("b");
	
	//创建max函数的entry代码块
	BasicBlock *entry_max = BasicBlock::Create(context, "entry", max_fun);
	IRBuilder<> builder_max(entry_max);//创建builder实例，每个basicblock对应一个irbulider

    builder_max.CreateRet(builder_max.getInt32(5));
	//比较arg_a和arg_b的大小
	Value *cmp_value = builder_max.CreateICmpSGT(max_fun->getArg(0), max_fun->getArg(1));
	//创建一个if.then代码块
	BasicBlock *if_then = BasicBlock::Create(context, "if_then", max_fun);
	IRBuilder<> builder_then(if_then);
	
	//创建一个if.else代码块
	BasicBlock *if_else = BasicBlock::Create(context, "if_else", max_fun);
	IRBuilder<> builder_else(if_else);
	
	//根据cmp的值跳转，也就是if条件
	builder_max.CreateCondBr(cmp_value, if_then, if_else);
	StructType* _Tp = StructType::create(context, structArgs, "class.test");
    llvm::Value* test_value = builder_max.CreateAlloca(_Tp, nullptr, "test");
    llvm::Value* dataField1 = builder_max.CreateGEP(_Tp, test_value, {builder_max.getInt32(0), builder_max.getInt32(0)});
    llvm::Value* dataField2 = builder_max.CreateStructGEP(_Tp, test_value, 1);
	llvm::DataLayout layout = builder_max.GetInsertBlock()->getModule()->getDataLayout();
	auto size_Tp = layout.getTypeAllocSize(_Tp);
	size_t s = size_Tp.getFixedSize();
    builder_max.CreateStore(llvm::ConstantInt::get(Type::getInt32Ty(context), 50), dataField1);
    builder_max.CreateStore(llvm::ConstantInt::get(Type::getInt64Ty(context), 100), dataField2);
	//创建返回值
	builder_then.CreateRet(max_fun->getArg(0));
	builder_else.CreateRet(max_fun->getArg(1));
	//-----------------------------------------------------
    Value * v = max_fun->getValueSymbolTable()->lookup("test");
	
	//声明 int main() 函数----------------------------------
	FunctionType *main_type = FunctionType::get(Type::getInt32Ty(context), false);//TypeBuilder<int(), false>::get(context);
	module->getOrInsertFunction("main", main_type);
	Function *main_fun = module->getFunction("main");
	//创建main函数的entry代码块
	BasicBlock *entry_mian = BasicBlock::Create(context, "entry", main_fun);
	IRBuilder<> builder_main(entry_mian);
	//创建一个i32常量
	Value *a_value = ConstantInt::get(Type::getInt32Ty(context), -10);
	Value *b_value = ConstantInt::get(Type::getInt32Ty(context),20);
	//构造实参列表
	std::vector<Value*> putsargs; 
	putsargs.push_back(a_value);
	putsargs.push_back(b_value);
	ArrayRef<Value*>  argsRef(putsargs);
	
	// 调用函数max
	CallInst *ret = builder_main.CreateCall(max_funee, argsRef);
	llvm::InlineFunctionInfo ifi;
	Value *val = ret;
	module->print(errs(), nullptr);
	//InlineResult iret = llvm::InlineFunction(*ret, ifi);
	
	//bool able = iret.isSuccess();
	module->print(errs(), nullptr);
	//创建返回值
	//builder_main.CreateRet(ret);
	//builder_main.CreateRet(ret);
	//----------------------------------------------------
	auto stus = module->getIdentifiedStructTypes();

	
	//使用JIT引擎---------------------------------------
	//https://blog.csdn.net/xfxyy_sxfancy/article/details/50485090
	InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
	
	//创建ExecutionEngine
	ExecutionEngine *ee = EngineBuilder(std::unique_ptr<Module>(module)).setEngineKind(EngineKind::JIT).create();
	//bool b = main_fun->hasAvailableExternallyLinkage();
	//生成机器指令
    void *mainAddr = ee->getPointerToFunction(main_fun);
	
	//运行机器指令
    typedef int (*FuncType)();
    FuncType mianFunc = (FuncType)mainAddr;//使用类型转换将mainAddr转换成一个函数mianFunc，然后调用
    ee->finalizeObject();
    std::cout << mianFunc() << std::endl;
    //---------------------------------------------------
	
	delete module ;
    return 0;
}





// class test
// {
// public:
//     test(std::string&& _str)
//     {
//         str = std::move(_str);
//         std::cout << _str; 
//     }
//     test(std::string& _str)
//     {
//         str = _str;
//     }
//     std::string str;
// };

// int main()
// {
//     std::stack<lpp::Token> st;
//     lpp::Lexer lexer;
// 	lpp::Expression exp;
// 	static llvm::LLVMContext context;
// 	llvm::IRBuilder ir(context);
//     std::fstream lex("D:\\Cfile\\Complier\\Lethon.lex", std::ios::in);
//     std::fstream fs("D:\\Cfile\\Complier\\test.lpp", std::ios::in);
//     bool o = lex.is_open();
//     bool ret = lexer.load(lex);
//     std::vector<lpp::Token> tokens;
//     lexer.set_source(&fs);
//     tokens = lexer.tokenize();
//     for (int var = 0; var < tokens.size(); var++)
//     {
// 		if (tokens[var].token.has_value())
// 			exp.insert(tokens[var]);
//         std::cout << tokens[var];
//     }
// 	std::cout << "\n";
// 	exp.get_expression_result(ir);
//     return EXIT_SUCCESS;
// }