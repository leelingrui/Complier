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
#include <llvm/IR/PassManager.h>
#include <llvm/IR/PassInstrumentation.h>
#include <llvm/Passes/PassBuilder.h>
#include <llvm/CodeGen/MachineFunctionPass.h>
#include <llvm/CodeGen/LiveInterval.h>
#include <llvm/CodeGen/SelectionDAGISel.h>
#include <llvm/CodeGen/Register.h>
#include <iostream>
#include <Parser.h>

#include <iostream>
#include <Common.h>
#include <Lexer.h>
#include <fstream>
#include <iterator>
#include <Expression.h>
#include <CodeGen.h>
#include <VM.h>
#include <magic_enum.hpp>

using namespace llvm;

// int main()
// {
// 	static LLVMContext MyGlobalContext;
//     LLVMContext &context = MyGlobalContext;
// 	SMDiagnostic smd;
// 	// //创建一个module
// 	// //
// 	std::unique_ptr<Module> module(new Module("test", context));//parseIRFile("./test.ll", smd, context);
// 	// IRBuilder<> b(context);
// 	// 声明 int main() 函数----------------------------------
// 	FunctionType *main_type = FunctionType::get(Type::getInt32PtrTy(context), false);//TypeBuilder<int(), false>::get(context);
// 	Function *test_fun = Function::Create(main_type, GlobalValue::LinkageTypes::ExternalLinkage, "test", *module);
// 	Function *main_fun = Function::Create(main_type, GlobalValue::LinkageTypes::ExternalLinkage, "main", *module);
//     llvm::SmallVector<llvm::Type*> args;
//     args.push_back(Type::getInt128Ty(context));
//     args.push_back(Type::getInt1Ty(context));
//     llvm::StructType* _Tp = llvm::StructType::create(args, "struct.test");
//     llvm::PointerType* _Ptp = llvm::PointerType::get(_Tp, 0);
// 	//创建main函数的entry代码块
// 	BasicBlock *entry_mian = BasicBlock::Create(context, "entry", test_fun);
// 	IRBuilder<> builder_main(entry_mian);

//     llvm::Value* struc = builder_main.CreateAlloca(_Ptp);
//     llvm::Type* tp =  struc->getType();
// 	//创建一个i32常量
// 	Value *a_value = builder_main.CreateAlloca(Type::getInt32Ty(context));
	
// 	Value *b_value = builder_main.CreateAlloca(Type::getInt32Ty(context));
// 	Value *ptr = builder_main.CreateAlloca(PointerType::getInt32PtrTy(context));
// 	// builder_main.CreateStore(builder_main.getInt32(5), a_value);
// 	builder_main.CreateStore(a_value, ptr);
// 	// b_value->get
// 	builder_main.CreateRet(ptr);
// 	Type::TypeID id = ptr->getType()->getTypeID();
// 	// bool able = iret.isSuccess();
// 	bool val = verifyFunction(*main_fun, &errs());
//     entry_mian->insertInto(main_fun);
// 	module->print(errs(), nullptr);
// 	// 创建返回值
// 	// ----------------------------------------------------
// 	// builder_main.CreateLoad()
	
// 	//使用JIT引擎---------------------------------------
// 	//https://blog.csdn.net/xfxyy_sxfancy/article/details/50485090
// 	InitializeNativeTarget();
//     InitializeNativeTargetAsmPrinter();
//     InitializeNativeTargetAsmParser();
// 	Function* main = module->getFunction("main");
// 	//创建ExecutionEngine
// 	ExecutionEngine *ee = EngineBuilder(std::move(module)).setEngineKind(EngineKind::JIT).create();
// 	//bool b = main_fun->hasAvailableExternallyLinkage();
// 	//生成机器指令
	
//     void *mainAddr = ee->getPointerToFunction(main);
// 	//运行机器指令
//     typedef int (*FuncType)();
//     FuncType mainFunc = (FuncType)mainAddr;//使用类型转换将mainAddr转换成一个函数mianFunc，然后调用
//     ee->finalizeObject();
// 	printf("%p", mainFunc());
//     //std::cout << mianFunc() << std::endl;
//     //---------------------------------------------------
	
// 	//delete module;
//     return 0;
	

// }





class test
{
public:
    test(std::string&& _str)
    {
        str = std::move(_str);
        std::cout << _str; 
    }
    test(std::string& _str)
    {
        str = _str;
    }
    std::string str;
};

int main()
{
    // llvm::SelectionDAGISel dag;
    std::fstream fs("D:\\Cfile\\Complier\\testparser.lpp", std::ios::in);
    std::error_code err;
    raw_fd_stream output("D:\\Cfile\\Complier\\testparser.ll", err);
    lpp::Parser parser(&fs);
	std::unique_ptr<llvm::Module> mod = parser.parse();
    lpp::CodeGenerator codegen;
    // PassBuilder PB;
    // LoopAnalysisManager LAM;
    // FunctionAnalysisManager FAM;
    // CGSCCAnalysisManager CGAM;
    // ModuleAnalysisManager MAM;
    // PB.registerModuleAnalyses(MAM);
    // PB.registerCGSCCAnalyses(CGAM);
    // PB.registerFunctionAnalyses(FAM);
    // PB.registerLoopAnalyses(LAM);
    // PB.crossRegisterProxies(LAM, FAM, CGAM, MAM);
    // ModulePassManager MPM = PB.buildPerModuleDefaultPipeline(OptimizationLevel::O0);
    // MPM.run(*mod, MAM);
    mod->print(output, nullptr);
    // codegen.GenerateAsm(std::move(mod));
    // MCRegister reg2(1);
    // MCRegister reg2(2);
    // RAGreedy RAG;
    // TargetRegisterClass regs;
    // RAG.selectOrSplit()
    // MachinePM.addPass(RAG);
    InitializeNativeTarget();
    InitializeNativeTargetAsmPrinter();
    InitializeNativeTargetAsmParser();
	Function* main = mod->getFunction("_FN4main@P");
    ExecutionEngine *ee = EngineBuilder(std::move(mod)).setEngineKind(EngineKind::JIT).create();
    void *mainAddr = ee->getPointerToFunction(main);
    typedef int (*FuncType)();
    FuncType mainFunc = (FuncType)mainAddr;
    ee->finalizeObject();
    std::cout << mainFunc() << std::endl;
    return EXIT_SUCCESS;
}