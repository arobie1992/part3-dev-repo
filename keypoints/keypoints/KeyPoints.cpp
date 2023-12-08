// The starting skeleton for this code was from this post: https://www.cs.cornell.edu/~asampson/blog/llvm.html
#include "llvm/Pass.h"
#include "llvm/Passes/PassBuilder.h"
#include "llvm/Passes/PassPlugin.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/IR/DebugInfo.h"
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>

using namespace llvm;

namespace {

class BranchEntry {
    public: 
    const int id;
    const StringRef file_name;
    const int condition_line;
    const int block_start_line;

    BranchEntry(int id, StringRef file_name, int condition_line, int block_start_line): 
        id(id), 
        file_name(file_name), 
        condition_line(condition_line), 
        block_start_line(block_start_line) 
    {}
    friend std::ostream& operator <<(std::ostream &out, BranchEntry &BE);
};

std::ostream& operator << (std::ostream &out, BranchEntry &BE) {
    out << "br_" << BE.id << ": " << BE.file_name.str() << ", " << BE.condition_line << ", " << BE.block_start_line;
    return out;
}

void writeBranchDictionary(std::vector<BranchEntry> &branchEntries) {
    std::ofstream branch_dict("branch_dictionary.txt", std::ios_base::app);
    for (auto BE : branchEntries) {
        branch_dict << BE << std::endl;
    }
    branch_dict.close();
}

class SemInput {
    public:
    Value *instruction;
    StringRef variableName;
    SemInput(Value *instruction, StringRef variableName): instruction(instruction) {
        if(variableName == "") {
            TinyPtrVector<DbgDeclareInst *> DIs = FindDbgDeclareUses(instruction);
            for (DbgDeclareInst *ddi: DIs) {
                this->variableName = ddi->getVariable()->getName();
                // we only care about the first one
                break;
            }
        }
        else {
            this->variableName = variableName;
        }
    }
};

struct KeyPointsPass : public PassInfoMixin<KeyPointsPass> {
    private: 
    int counter;
    std::set<BasicBlock*> seen;
    std::vector<BranchEntry> branchEntries;
    int getStartLine(BasicBlock &BB) {
        for (auto &I : BB) {
            if (I.getDebugLoc()) {
                return I.getDebugLoc().getLine();
            }
        }
        // this thankfully hasn't come up so far, but if it does, 
        // it shouldn't cause program issues, just some funky output
        return -1;
    };
    void addFilePrint(Module &M, Instruction &I, BranchEntry &BE) {
        // info on linking to externally defined library from: https://www.cs.cornell.edu/~asampson/blog/llvm.html
        LLVMContext &context = M.getContext();
        // hopefully this name is unique enough to not cause collisions
        auto logFunc = M.getOrInsertFunction("csc512project_log_branch", Type::getVoidTy(context), Type::getInt32Ty(context));
        IRBuilder<> builder(&I);
        Value *arg(builder.getInt32(BE.id));
        builder.CreateCall(logFunc, arg, "brtag" + std::to_string(BE.id));
    };
    void addBranchTag(Module &M, int condition_line, BasicBlock &BB) {
        if(!seen.insert(&BB).second) {
            // we've already seen this one and transformed it
            return;
        }
        BranchEntry BE(counter++, M.getName(), condition_line, getStartLine(BB));
        branchEntries.push_back(BE);
        for (auto &I : BB) {
            addFilePrint(M, I, BE);
            // break after the first instruction because we only want to insert the tag at the start of the block
            break;
        }
    };
    void handleSwitch(Module &M, SwitchInst &SI) {
        if (!SI.getDebugLoc()) {
            // invalid debug location so don't attempt since getting the condition line will fail
            // this results in the plugin essentially being a no-op if clang is run without -g
            return;
        }
        auto condition_line = SI.getDebugLoc().getLine();
        auto &D = *SI.getDefaultDest();
        for (auto i = 0; i < SI.getNumSuccessors(); i++) {
            auto &S = *SI.getSuccessor(i);
            if (&S == &D) {
                // skip the default block for the end
                continue;
            }
            addBranchTag(M, condition_line, S);
        }
        addBranchTag(M, condition_line, D);
    };
    void handleBranch(Module &M, BranchInst &BI) {
        if (!BI.getDebugLoc()) {
            // invalid debug location so don't attempt since getting the condition line will fail
            // this results in the plugin essentially being a no-op if clang is run without -g
            return;
        }

        if (BI.isUnconditional()) {
            return;
        }

        auto condition = BI.getSuccessor(0);
        addBranchTag(M, BI.getDebugLoc().getLine(), *condition);
        auto alternative = BI.getSuccessor(1);
        addBranchTag(M, BI.getDebugLoc().getLine(), *alternative);
    };
    void handleCall(Module &M, CallInst &CI) {
        if(!CI.isIndirectCall()) {
            // if it's a direct call, it's not through a function pointer so we don't care
            return;
        }
        auto op = CI.getCalledOperand();
        LLVMContext &context = M.getContext();
        auto voidptr = Type::getVoidTy(context)->getPointerTo();
        // hopefully this name is unique enough to not cause collisions
        auto logFunc = M.getOrInsertFunction("csc512project_log_fp", Type::getVoidTy(context), voidptr);
        IRBuilder<> builder(&CI);
        Value *arg(op);
        builder.CreateCall(logFunc, arg, "fptag");
    }
    void recordCounter(int counter) {
        std::ofstream f("counter.log");
        f << counter;
        f.close();
    };
    int initCounter() {
        std::filesystem::path counter_log{ "counter.log" };
        if (std::filesystem::exists(counter_log)) {
            std::ifstream in("counter.log");
            std::string content((std::istreambuf_iterator<char>(in)),(std::istreambuf_iterator<char>()));
            auto ctr = std::stoi(content);
            return ctr;
        } else {
            return 0;
        }
    }
    void part1(Module &M, ModuleAnalysisManager &AM) {
        counter = initCounter();
        for (auto &F : M) {
            for (auto &B : F) {
                for (auto &I : B) {
                    if (isa<SwitchInst>(I)) {
                        auto SI = dyn_cast<SwitchInst>(&I);
                        handleSwitch(M, *SI);
                    }
                    if (isa<BranchInst>(I)) {
                        auto BI = dyn_cast<BranchInst>(&I);
                        handleBranch(M, *BI);
                    }
                    if (isa<CallInst>(I)) {
                        auto CI = dyn_cast<CallInst>(&I);
                        handleCall(M, *CI);
                    }
                }
            }
        }
        writeBranchDictionary(branchEntries);
        recordCounter(counter);
    }
    void part2(Module &M) {
        //list of values that will determine 
        std::vector<SemInput> vector;
        errs() << "Analyzing file: " << M.getName() << "\n";
        for (Function &F : M){
            // errs() << F << "\n";
            for (BasicBlock &BB : F) {
                for (Instruction &I : BB) {
                    //Look for certain branches
                    
                    //In the event instruction makes a call
                    if(isa<CallInst>(&I)){
                        // errs() << "Call: " << I << "\n";
                        auto &callType = cast<CallInst>(I);
                        if(callType.isIndirectCall()) continue;
                        if(!callType.getDebugLoc()) continue;
                        //get the name of the function call
                        auto function = callType.getCalledOperand()->getName();
                        // errs() << "function name:" << function << "\n";
                        if(function == "getc") {
                            auto semInput = SemInput(callType.getOperand(0), "");
                            vector.push_back(semInput);
                        }
                        if(function == "fopen") {
                            auto semInput = SemInput(callType.getOperand(1), "");
                            vector.push_back(semInput);
                        }
                        if(function == "__isoc99_scanf"){ 
                            auto semInput = SemInput(callType.getOperand(1), "");
                            vector.push_back(semInput);
                        }
                        if(function == "fclose") {
                            auto semInput = SemInput(callType.getOperand(0), "");
                            vector.push_back(semInput);
                        }
                        if(function == "fread") {
                            auto semInput = SemInput(callType.getOperand(1), "");
                            vector.push_back(semInput);
                        }
                        if(function == "fwrite") {
                            auto semInput = SemInput(callType.getOperand(1), "");
                            vector.push_back(semInput);
                        }
                    }
                }
            }
        }
        //Second loop for seminal analysis
        while(!vector.empty()) {
            // get and remove the last element
            SemInput si = vector.back();
            vector.pop_back();
            // get the LLVM instruction
            Value *val = si.instruction;
        
            //Iterate through every value for instructions
            for(User *user : val->users()){
                if(isa<LoadInst>(user) || isa<ICmpInst>(user)) {
                    // we want to follow references to the value through whatever path they may take to an eventual branch or switch
                    auto addedSi = SemInput(user, si.variableName);
                    vector.push_back(addedSi);
                }
                //Check branch instructions for seminal length instructions
                if(isa<BranchInst>(user)){
                    auto branchType = dyn_cast<BranchInst>(user);
                    errs() << "Line " << branchType->getDebugLoc().getLine() << ": " << si.variableName << "\n";
                }
                //repeat for switch instructions
                if(isa<SwitchInst>(user)){
                    auto switchType = dyn_cast<SwitchInst>(user);
                    errs() << "Line " << switchType->getDebugLoc().getLine() << ": "<< si.variableName << "\n";
                }
                //repeat for call functions
                if(isa<CallInst>(user)){
                    auto callType = dyn_cast<CallInst>(user);
                    if(callType == val){
                        if(auto debugLoc = callType->getDebugLoc()){
                            errs() << "Line " << debugLoc.getLine() << ": size of " << si.variableName << "\n";
                        }
                    }
                }
            }
        }
    }
    public:
    PreservedAnalyses run(Module &M, ModuleAnalysisManager &AM) {
        if(M.getName().find("branchlog.c") != -1) {
            // skip branchlog since it's not part of the user program we want to instrument
            return PreservedAnalyses::all();
        }
        // running part 2 first may seem counterintuitive, but avoids analyzing the instrumented code
        part2(M);
        part1(M, AM);
        return PreservedAnalyses::none();
    };
};

}

extern "C" LLVM_ATTRIBUTE_WEAK ::llvm::PassPluginLibraryInfo
llvmGetPassPluginInfo() {
    return {
        .APIVersion = LLVM_PLUGIN_API_VERSION,
        .PluginName = "KeyPoints pass",
        .PluginVersion = "v0.1",
        .RegisterPassBuilderCallbacks = [](PassBuilder &PB) {
            PB.registerPipelineStartEPCallback(
                [](ModulePassManager &MPM, OptimizationLevel Level) {
                    MPM.addPass(KeyPointsPass());
                });
        }
    };
}
