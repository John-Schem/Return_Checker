#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/ADT/StringMap.h"
#include "llvm/Support/raw_ostream.h"

using namespace clang;
using namespace llvm;

class ReturnCheckerVisitor : public RecursiveASTVisitor<ReturnCheckerVisitor>
{
public:
    explicit ReturnCheckerVisitor(ASTContext *Context) : astContext(Context) {}

    bool VisitCXXRecordDecl(CXXRecordDecl *cxxRecordDecl)
    {
        FullSourceLoc FullLocation = astContext->getFullLoc(cxxRecordDecl->getBeginLoc());

        // Basic sanity checking
        if (!FullLocation.isValid())
        {
            return true;
        }

        // There are 2 types of source locations: in a file or a macro expansion. The
        // latter contains the spelling location and the expansion location (both are
        // file locations), but only the latter is needed here (i.e. where the macro
        // is expanded). File locations are just that - file locations.
        if (FullLocation.isMacroID())
        {
            FullLocation = FullLocation.getExpansionLoc();
        }

        SourceManager &SrcMgr = astContext->getSourceManager();
    }

private:
    ASTContext *astContext;
};

class ReturnCheckerConsumer : public clang::ASTConsumer 
{
public:
    explicit ReturnCheckerConsumer(ASTContext *Ctx) : returnCheckerVisitor(Ctx) {}

    void HandleTranslationUnit(clang::ASTContext &Ctx) override
    {
        returnCheckerVisitor.TraverseDecl(Ctx.getTranslationUnitDecl());
    }

private:
  ReturnCheckerVisitor returnCheckerVisitor;
};

class ReturnCheckerAction : public clang::PluginASTAction 
{
public:
    std::unique_ptr<clang::ASTConsumer> 
    CreateASTConsumer(clang::CompilerInstance &Compiler,
                      llvm::StringRef InFile)
    override
    {
        return std::unique_ptr<clang::ASTConsumer>(
        std::make_unique<ReturnCheckerConsumer>(&Compiler.getASTContext()));
    }

    bool ParseArgs(const CompilerInstance &CI, const std::vector<std::string> &args) override
    {
        return true;
    }
};


static FrontendPluginRegistry::Add<ReturnCheckerAction> 
X(/*Name=*/"Return_Checker",
  /*Description=*/"Checks to see if return values from functions are being consumed.");