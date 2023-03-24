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
#include <llvm/Support/SourceMgr.h>
#include <llvm/IRReader/IRReader.h>
#include <iostream>
#include <Parser.h>
 
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
	SMDiagnostic smd;
	// //创建一个module
	// //
	std::unique_ptr<Module> module(new Module("test", context));//parseIRFile("./test.ll", smd, context);
	// IRBuilder<> b(context);
	// 声明 int main() 函数----------------------------------
	FunctionType *main_type = FunctionType::get(Type::getInt32PtrTy(context), false);//TypeBuilder<int(), false>::get(context);
	module->getOrInsertFunction("main", main_type);
	Function *main_fun = module->getFunction("main");
	//创建main函数的entry代码块
	BasicBlock *entry_mian = BasicBlock::Create(context, "entry", main_fun);
	IRBuilder<> builder_main(entry_mian);
	//创建一个i32常量
	Value *a_value = builder_main.CreateAlloca(Type::getInt32Ty(context));
	
	Value *b_value = builder_main.CreateAlloca(Type::getInt32Ty(context));
	Value *ptr = builder_main.CreateAlloca(PointerType::getInt32PtrTy(context));
	// builder_main.CreateStore(builder_main.getInt32(5), a_value);
	builder_main.CreateStore(a_value, ptr);
	// b_value->get
	builder_main.CreateRet(ptr);
	Type::TypeID id = ptr->getType()->getTypeID();
	// bool able = iret.isSuccess();
	bool val = verifyFunction(*main_fun, &errs());
	module->print(errs(), nullptr);
	// 创建返回值
	// ----------------------------------------------------
	// builder_main.CreateLoad()
	
	//使用JIT引擎---------------------------------------
	//https://blog.csdn.net/xfxyy_sxfancy/article/details/50485090
	InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
	Function* main = module->getFunction("main");
	//创建ExecutionEngine
	ExecutionEngine *ee = EngineBuilder(std::move(module)).setEngineKind(EngineKind::JIT).create();
	//bool b = main_fun->hasAvailableExternallyLinkage();
	//生成机器指令
	
    void *mainAddr = ee->getPointerToFunction(main);
	//运行机器指令
    typedef int (*FuncType)();
    FuncType mainFunc = (FuncType)mainAddr;//使用类型转换将mainAddr转换成一个函数mianFunc，然后调用
    ee->finalizeObject();
	printf("%p", mainFunc());
    //std::cout << mianFunc() << std::endl;
    //---------------------------------------------------
	
	//delete module;
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
//     std::fstream fs("D:\\Cfile\\Complier\\testparser.lpp", std::ios::in);
//     lpp::Parser parser(&fs);
// 	parser.parse();

//     return EXIT_SUCCESS;
// }