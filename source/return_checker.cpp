#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace llvm;
using namespace std;

class ReturnCheckerVisitor : public RecursiveASTVisitor<ReturnCheckerVisitor>
{
public:
    explicit ReturnCheckerVisitor(ASTContext * astContext) : astContext(astContext) {}

    bool VisitFunctionDecl(FunctionDecl * functionDecl)
    {
        outs() << functionDecl->getReturnType().getAsString() << "|" << functionDecl->getNameInfo().getName().getAsString() << "\n";

        /*
            Todo:
            - Create dictionary of all functions that are defined and their return values
            - Checks all function calls inside user defined functions for consuming return values
        */

        return true;
    }

private:
    ASTContext *astContext;
};

class ReturnCheckerConsumer : public clang::ASTConsumer 
{
public:
    explicit ReturnCheckerConsumer(ASTContext * astContext) : returnCheckerVisitor(astContext) {}

    void HandleTranslationUnit(clang::ASTContext & astContext) override
    {
        returnCheckerVisitor.TraverseDecl(astContext.getTranslationUnitDecl());
    }

private:
  ReturnCheckerVisitor returnCheckerVisitor;
};

class ReturnCheckerAction : public clang::PluginASTAction 
{
public:
    std::unique_ptr<clang::ASTConsumer> 
    CreateASTConsumer(clang::CompilerInstance & compilerInstance, llvm::StringRef inFile)
    override
    {
        return std::unique_ptr<clang::ASTConsumer>(std::make_unique<ReturnCheckerConsumer>(&compilerInstance.getASTContext()));
    }

    bool ParseArgs(const CompilerInstance & compilerInstance, const std::vector<std::string> & args) override
    {
        return true;
    }
};


static FrontendPluginRegistry::Add<ReturnCheckerAction> 
X(/*Name=*/"Return_Checker",
  /*Description=*/"Checks to see if return values from functions are being consumed.");
