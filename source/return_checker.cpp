#include "clang/AST/ASTConsumer.h"
#include "clang/AST/RecursiveASTVisitor.h"
#include "clang/Basic/FileManager.h"
#include "clang/Frontend/CompilerInstance.h"
#include "clang/Frontend/FrontendPluginRegistry.h"
#include "llvm/Support/raw_ostream.h"

#include <map>
#include <sstream>
#include <vector>

using namespace clang;
using namespace llvm;

class FunctionReturnCheckVisitor : public RecursiveASTVisitor<FunctionReturnCheckVisitor>
{
public:
    explicit FunctionReturnCheckVisitor(ASTContext & astContext, SourceManager & sourceManager) : astContext(astContext), sourceManager(sourceManager) {}

    bool VisitFunctionDecl(FunctionDecl * functionDecl)
    {
        if(!functionDecl->hasBody())
        {
            return true;
        }

        Stmt * body = functionDecl->getBody();
        for(auto stmtIterator = body->child_begin(); stmtIterator != body->child_end(); stmtIterator++)
        {
            RecurseDown(*stmtIterator, true, "CompoundStmt", "ERROR", "ERROR", "ERROR");
        }

        return true;
    }

    /*
        Recursively descend, just pass in last Stmt seen and we can know immediately if it's consumed or not
        If we run into DeclRefExpr and last seen was CompoundStmt, then it's not consumed

        CallExpr or CXXMemberCallExpr = get return value
        |
        ImplicitCastExpr or MemberExpr = if MemberExpr get function name
        |
        DeclRefExpr = get function name if contains "Function 0x"
    */
    void RecurseDown(Stmt * currentPoint, bool lastSeenWasCompoundStmt, std::string parentClass,
                     std::string locationInformation, std::string nameOfFunction, std::string returnTypeOfFunction)
    {
        std::string stmtClass = currentPoint->getStmtClassName();

        if(stmtClass == "CompoundStmt")
        {
            lastSeenWasCompoundStmt = true;
        }
        else if(stmtClass.find("Stmt") != std::string::npos)
        {
            lastSeenWasCompoundStmt = false;
        }
        else if(stmtClass.find("CallExpr") != std::string::npos)
        {
            if(parentClass.find("CallExpr") != std::string::npos)
            {
                lastSeenWasCompoundStmt = false;
            }
        
            returnTypeOfFunction = SplitString(GetFirstLineOfStmtDump(*currentPoint), "'")[1];
        }
        else if(stmtClass == "DeclRefExpr")
        {
            if(GetFirstLineOfStmtDump(*currentPoint).find(" Function 0x") != std::string::npos)
            {
                nameOfFunction = SplitString(GetFirstLineOfStmtDump(*currentPoint), "'")[3];
            }

            if(lastSeenWasCompoundStmt)
            {
                SourceLocation sourceLocation = currentPoint->getBeginLoc();
                std::string locationString = GetDeclLocation(sourceLocation);

                outs() << locationString << " warning: " << "function '" << nameOfFunction
                    << "' return type '" << returnTypeOfFunction << "' is not used \n";
            }
        }
        else if(stmtClass == "MemberExpr")
        {
            std::vector<std::string> splitStrings = SplitString(GetFirstLineOfStmtDump(*currentPoint), ".");
            nameOfFunction = SplitString(splitStrings[splitStrings.size() - 1], " 0x")[0];
        }
        

        for(auto stmtIterator = currentPoint->child_begin(); stmtIterator != currentPoint->child_end(); stmtIterator++)
        {
            RecurseDown(*stmtIterator, lastSeenWasCompoundStmt, stmtClass, locationInformation, nameOfFunction, returnTypeOfFunction);
        }
    }

private:
    ASTContext & astContext;
    SourceManager & sourceManager;

    std::string GetDeclLocation(SourceLocation & sourceLocation)
    {
        std::stringstream stringStream;

        stringStream << sourceManager.getFilename(sourceLocation).str() << ":"
                     << sourceManager.getSpellingLineNumber(sourceLocation) << ":"
                     << sourceManager.getSpellingColumnNumber(sourceLocation) << ":";

        return stringStream.str();
    }

    std::vector<std::string> SplitString(std::string stringToSplit, std::string delineator)
    {
        int pos;
        std::vector<std::string> strings;

        while((pos = stringToSplit.find(delineator)) != std::string::npos)
        {
            strings.push_back(stringToSplit.substr(0, pos));
            stringToSplit.erase(0, pos + delineator.size());
        }
        
        strings.push_back(stringToSplit);
        return strings;
    }

    std::string GetFirstLineOfStmtDump(Stmt & stmt)
    {
        std::string temp;
        raw_string_ostream dump_output(temp); 
        stmt.dump(dump_output, astContext);
        return SplitString(temp, "\n")[0];
    }
};

class ReturnCheckerConsumer : public clang::ASTConsumer 
{
public:
    explicit ReturnCheckerConsumer(SourceManager & sourceManager, ASTContext & astContext)
    : sourceManager(sourceManager), astContext(astContext), functionReturnCheckVisitor(astContext, sourceManager) {}

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
    ASTContext & astContext;
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
        return std::unique_ptr<clang::ASTConsumer>(std::make_unique<ReturnCheckerConsumer>(compilerInstance.getSourceManager(), compilerInstance.getASTContext()));
    }

    bool ParseArgs(const CompilerInstance & compilerInstance, const std::vector<std::string> & args) override
    {
        return true;
    }
};


static FrontendPluginRegistry::Add<ReturnCheckerAction> 
X(/* Name = */ "Return_Checker",
  /* Description = */ "Checks to see if return values from functions are being consumed.");
