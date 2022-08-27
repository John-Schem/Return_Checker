#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <map>

using namespace clang;
using namespace llvm;


/*
    Steps:
    - Create list of all functions and what they return
    - Create list of all classes, their methods and what they return
    - Checks all function calls(not methods) inside user defined functions/methods for consuming return values
        - Ignore functions that return void
        - Two ways return can be consumed: 1) assignment, 2) being passed to another function
    - Checks all method calls inside user defined functions/methods for consuming return values
        - Needs to have awareness of class type which this method is being called on since different
          classes can have methods with the same name
        - Same two ways return values can be consumed: 1) assignment, 2) being passed to another function
*/
class ReturnCheckerVisitor : public RecursiveASTVisitor<ReturnCheckerVisitor>
{
public:
    explicit ReturnCheckerVisitor(ASTContext * astContext) : astContext(astContext) {}

    void PrintAllInfoForFunctions()
    {
        outs() << "Printing all info for functions... \n";

        for(std::map<std::string, std::string>::iterator it = functionNameToReturnType.begin(); it != functionNameToReturnType.end(); it++)
        {
            outs() << it->second << " " << it->first << "\n";
        }

        outs() << "Done printing all info for functions... \n";
    }

    bool VisitFunctionDecl(FunctionDecl * functionDecl)
    {
        {
            std::string functionName = functionDecl->getNameInfo().getName().getAsString();
            std::string functionReturnType = functionDecl->getReturnType().getAsString();

            outs() << "Adding = " <<  functionDecl->getReturnType().getAsString() << " " << functionDecl->getNameInfo().getName().getAsString() << "\n";

            functionNameToReturnType[std::move(functionName)] = std::move(functionReturnType);
        }



        return true;
    }

private:
    ASTContext * astContext;

    std::map<std::string, std::string> functionNameToReturnType;
};

class ReturnCheckerConsumer : public clang::ASTConsumer 
{
public:
    explicit ReturnCheckerConsumer(ASTContext * astContext) : returnCheckerVisitor(astContext) {}

    void HandleTranslationUnit(clang::ASTContext & astContext) override
    {
        returnCheckerVisitor.TraverseDecl(astContext.getTranslationUnitDecl());

        returnCheckerVisitor.PrintAllInfoForFunctions();
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
X(/* Name = */ "Return_Checker",
  /* Description = */ "Checks to see if return values from functions are being consumed.");
