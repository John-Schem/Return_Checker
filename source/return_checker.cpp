#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <map>

using namespace clang;
using namespace llvm;

class FunctionReturnCheckVisitor : public RecursiveASTVisitor<FunctionReturnCheckVisitor>
{
public:
    explicit FunctionReturnCheckVisitor() {}

    bool VisitDeclRefExpr(DeclRefExpr * declRefExpr)
    {
        /*
            At DeclRefExpr, get line information, and name of function if non-class function call
            Above in MemberExpr, if method call, get function call
            Above that in CallExpr or CXXMemberCallExpr, get function return value

            Then traverse upwards looking for next ...Stmt class
            If next is CompoundStmt, then return value is not consumed,
            so print warning information
        */

        declRefExpr->dump();

        return true;
    }
};

class ReturnCheckerConsumer : public clang::ASTConsumer 
{
public:
    explicit ReturnCheckerConsumer(SourceManager & sourceManager) : sourceManager(sourceManager) {}

    /*
        RecursiveASTVisitor will recurse down and visit everything
        We could either pass it the root(always TranslationUnitDecl at top) and traverse everything
        or we can pass it individual items of interest we want it to traverse
        When it finds something we care about, it will then "visit" it

        First map all functions to their return types
        Then check all user defined functions to see if they consume
        return values from called functions
    */
    void HandleTranslationUnit(clang::ASTContext & astContext) override
    {
        auto declarations = astContext.getTranslationUnitDecl()->decls();

        for(auto & declaration : declarations)
        {
            const auto & fileID = sourceManager.getFileID(declaration->getLocation());

            /* Only check user defined functions */
            if (fileID == sourceManager.getMainFileID())
            {
                functionReturnCheckVisitor.TraverseDecl(declaration);
            }
        }
    }

private:
    SourceManager & sourceManager;
    FunctionReturnCheckVisitor functionReturnCheckVisitor;
};

class ReturnCheckerAction : public clang::PluginASTAction 
{
public:
    std::unique_ptr<clang::ASTConsumer> 
    CreateASTConsumer(clang::CompilerInstance & compilerInstance, llvm::StringRef inFile)
    override
    {
        return std::unique_ptr<clang::ASTConsumer>(std::make_unique<ReturnCheckerConsumer>(compilerInstance.getSourceManager()));
    }

    bool ParseArgs(const CompilerInstance & compilerInstance, const std::vector<std::string> & args) override
    {
        return true;
    }
};


static FrontendPluginRegistry::Add<ReturnCheckerAction> 
X(/* Name = */ "Return_Checker",
  /* Description = */ "Checks to see if return values from functions are being consumed.");
