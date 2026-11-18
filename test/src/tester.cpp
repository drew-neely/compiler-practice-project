#include <print>
#include <string>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <vector>

#include "lexer.h"
#include "parser.h"
#include "cfg.h"
#include "cfg_interpreter.h"

using namespace std;
namespace fs = std::filesystem;


struct ManifestEntry {
    std::string path;
    int expected;
    std::vector<int> inputs;
    int line;
};

bool flagRun = false;
bool flagRunList = false;
string runPath = "";
bool flagParams = false;
vector<int> runParams = {};
bool flagExpect = false;
int expectedReturn = 0;
bool flagTestLiveness = false; // if true fill unused values with garbage
bool dumpLexerOutput = false;
bool dumpASTOutput = false;
bool dumpCFGOutput = false;

inline std::vector<ManifestEntry> readManifest(const std::string& manifestPath) {
    std::ifstream file(manifestPath);
    if (!file) {
        throw std::runtime_error("Cannot open manifest: " + manifestPath);
    }
    fs::path manifestDir = fs::absolute(manifestPath).parent_path();

    std::vector<ManifestEntry> entries;
    std::string text;
    int lineNum = 0;
    while (std::getline(file, text)) {
        lineNum++;
        std::istringstream words(text);
        std::string pathStr;
        if (!(words >> pathStr) || pathStr[0] == '#') {
            continue;  // blank line or comment
        }
        auto fail = [&]() {
            return std::runtime_error(manifestPath + ":" + std::to_string(lineNum) +
                                      ": expected \"<path> <expected> [input ...]\"");
        };
        int expected;
        if (!(words >> expected)) {
            throw fail();
        }
        std::vector<int> inputs;
        int value;
        while (words >> value) {
            inputs.push_back(value);
        }
        if (!words.eof()) {
            throw fail();  // stopped on something that isn't an integer
        }
        fs::path p(pathStr);
        if (p.is_relative()) {
            p = manifestDir / p;
        }
        entries.push_back({p.lexically_normal(), expected, std::move(inputs), lineNum});
    }
    return entries;
}

int runTest(string path, vector<int> params) {
    ifstream file(path);
    if (!file.is_open()) {
        throw logic_error(string() + "Error: Could not open file: " + path);
    }

    ostringstream buffer;
    buffer << file.rdbuf();
    string programStr = buffer.str();

    vector<LexToken> tokens = lex(programStr);

    if(dumpLexerOutput) {
        for(size_t i = 0; i < tokens.size(); i++) {
            println("{}", tokens[i].toString());
        }
    }

    ASTNode* ast = parse(tokens);
    
    if(dumpASTOutput) {
        println("{}", ast->toString());
    }

    CFG* cfg = lower(ast);

    if(dumpCFGOutput) {
        println("{}", cfg->toString());
    }

    int res = interpretCFG(cfg, params, flagTestLiveness);

    return res;

}

int main(int argc, char *argv[]) {
    for(int i = 1; i < argc; i++) {
        string arg = argv[i];
        if(arg == "--run") {
            if(flagRun) {
                throw invalid_argument("Found mulitple --run in args");
            }
            flagRun = 1;
            if(i+1 < argc) {
                i++;
                runPath = argv[i];
            } else {
                throw invalid_argument("No string found after --run in args");
            }
        } else if(arg == "--run-list") {
            if(flagRunList) {
                throw invalid_argument("Found mulitple --run-list in args");
            }
            flagRunList = 1;
            if(i+1 < argc) {
                i++;
                runPath = argv[i];
            } else {
                throw invalid_argument("No string found after --run-list in args");
            }
        } else if(arg == "--expect") {
            if(flagExpect) {
                throw invalid_argument("Found mulitple --expect in args");
            }
            flagExpect = 1;
            if(i+1 < argc) {
                i++;
                try {
                    expectedReturn = stoi(argv[i]);
                } catch(const exception&) {
                    throw invalid_argument("Invalid number after --expect in args");
                }
            } else {
                throw invalid_argument("No number found after --expect in args");
            }
        } else if(arg == "--parameters") {
            if(flagParams) {
                throw invalid_argument("Found multiple --parameters in args");
            }
            flagParams = true;
            while(i+1 < argc && string(argv[i+1]).rfind("--", 0) != 0) {
                i++;
                string numStr = argv[i];
                size_t pos = 0;
                try {
                    runParams.push_back(stoi(numStr, &pos));
                } catch(const exception&) {
                    pos = 0;
                }
                if(pos != numStr.size()) {
                    throw invalid_argument("Invalid number after --parameters in args: " + numStr);
                }
            }
            if(runParams.empty()) {
                throw invalid_argument("No numbers found after --parameters in args");
            }
        } else if(arg == "--test-liveness") {
            flagTestLiveness = true;
        } else if(arg == "--dump-lex") {
            dumpLexerOutput = true;
        } else if(arg == "--dump-ast") {
            dumpASTOutput = true;
        } else if(arg == "--dump-cfg") {
            dumpCFGOutput = true;
        } else if (arg == "--dump-all") {
            dumpLexerOutput = true;
            dumpASTOutput = true;
            dumpCFGOutput = true;
        } else {
            throw invalid_argument(string() + "Unexpected arg found in args : " + arg);
        }
    }

    if(!flagRun&& !flagRunList) {
        println(stderr, "No test specified to run");
        return 1;
    } else if(flagRun && flagRunList) {
        throw invalid_argument("both --run and --run-list found in arg list");
    } else if(flagRunList && (flagExpect || dumpLexerOutput || dumpASTOutput || dumpCFGOutput)) {
        println(stderr, "Flags invalid with --run-list will be ignored");
    }
    
    ///////////////////////////////

    if(flagRun) {
        int res = runTest(runPath, runParams);
        if(flagExpect) {
            if(res == expectedReturn) {
                println("Pass!");
                return 0;
            } else {
                println("Fail!");
                println("Actual: {}, Expected: {}", res, expectedReturn);
                return 1;
            }
        } else {
            println("Result: {}", res);
            return 0;
        }
    } else if(flagRunList) {
        vector<ManifestEntry> manifest = readManifest(runPath);
        int passCount = 0;
        int failCount = 0;
        for(size_t i = 0; i < manifest.size(); i++) {
            int res = runTest(manifest[i].path, manifest[i].inputs);
            int expected = manifest[i].expected;
            int testNum = manifest[i].line;
            if(res == expected) {
                // println("Test [{:3}]: Pass!", testNum);
                passCount++;
            } else {
                println("Test [{:3}]: Fail!", testNum);
                println("\tActual: {}, Expected: {}", res, expected);
                failCount++;
            }
        }
        println("Ran {} tests: {} Pass, {} Fail", passCount + failCount, passCount, failCount);
        return failCount != 0;
    }
    
}