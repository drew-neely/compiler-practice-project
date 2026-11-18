#include <string>
#include <fstream>
#include <vector>

#include "lexer.h"
#include "parser.h"

ASTNode* parseBlock(vector<LexToken>& tokens, int& cursor);
ASTNode* parseStatement(vector<LexToken>& tokens, int& cursor);
ASTNode* parseInputList(vector<LexToken>& tokens, int& cursor);
ASTNode* parseExpression(vector<LexToken>& tokens, int& cursor);
ASTNode* parseSum(vector<LexToken>& tokens, int& cursor);
ASTNode* parseTerm(vector<LexToken>& tokens, int& cursor);
ASTNode* parseFactor(vector<LexToken>& tokens, int& cursor);

ASTNode* parse(vector<LexToken>& tokens) {
    int cursor = 0;
    ASTNode* root = parseStatement(tokens, cursor);
    ASTNode* leaf = root;
    while(ASTNode* stmt = parseStatement(tokens, cursor)) {
        leaf->next = stmt;
        leaf = stmt;
    }
    return root;
}

ASTNode* parseBlock(vector<LexToken>& tokens, int& cursor) {
    if(tokens[cursor].type == LexTokenType::SmRBrace) {
        return nullptr;
    }
    ASTNode* root = parseStatement(tokens, cursor);
    ASTNode* leaf = root;
    while(tokens[cursor].type != LexTokenType::SmRBrace) {
        ASTNode* stmt = parseStatement(tokens, cursor);
        if(stmt == nullptr) {
            throw invalid_argument("Unexpected EOF: " + tokens[cursor].toString());
        }
        leaf->next = stmt;
        leaf = stmt;
    }
    return root;
}

ASTNode* parseStatement(vector<LexToken>& tokens, int& cursor) {
    if(tokens[cursor].type == LexTokenType::SmEOF) {
        return nullptr;
    } else if(tokens[cursor].type == LexTokenType::ValIdentifier) {
        ASTNode* stmt = ASTNode::allocNode();
        stmt->type = ASTNodeType::StmtAssign;
        stmt->line = tokens[cursor].line;
        stmt->lhs = ASTNode::allocNode();
        stmt->lhs->type = ASTNodeType::ValIdentifier;
        stmt->lhs->strVal = tokens[cursor].strVal;
        stmt->lhs->line = tokens[cursor].line;
        cursor++;
        if(tokens[cursor].type != LexTokenType::OpAssign) {
            throw invalid_argument("Expected assignment op after identifier: " + tokens[cursor].toString());
        }
        cursor++;
        stmt->rhs = parseExpression(tokens, cursor);
        if(tokens[cursor].type != LexTokenType::SmSemi) {
            throw invalid_argument("Expected semi at end of assignment: " + tokens[cursor].toString());
        }
        cursor++;
        return stmt;
    } else if(tokens[cursor].type == LexTokenType::KwInput) {
        ASTNode* stmt = ASTNode::allocNode();
        stmt->type = ASTNodeType::StmtInput;
        stmt->line = tokens[cursor].line;
        cursor++;
        stmt->lhs = parseInputList(tokens, cursor);
        return stmt;
    } else if(tokens[cursor].type == LexTokenType::KwIf) {
        ASTNode* stmt = ASTNode::allocNode();
        stmt->type = ASTNodeType::StmtIf;
        stmt->line = tokens[cursor].line;
        cursor++;
        if(tokens[cursor].type != LexTokenType::SmLParen) {
            throw invalid_argument("Expected '(' after if: " + tokens[cursor].toString());
        }
        cursor++;
        stmt->cond = parseExpression(tokens, cursor);
        if(tokens[cursor].type != LexTokenType::SmRParen) {
            throw invalid_argument("Expected ')' after if condition: " + tokens[cursor].toString());
        }
        cursor++;
        if(tokens[cursor].type != LexTokenType::SmLBrace) {
            throw invalid_argument("Expected '{' after if condition: " + tokens[cursor].toString());
        }
        cursor++;
        stmt->lhs = parseBlock(tokens, cursor);
        if(tokens[cursor].type != LexTokenType::SmRBrace) {
            throw invalid_argument("Expected '}' after if block: " + tokens[cursor].toString());
        }
        cursor++;
        if(tokens[cursor].type == LexTokenType::KwElse) {
            cursor++;
            if(tokens[cursor].type != LexTokenType::SmLBrace) {
                throw invalid_argument("Expected '{' after else: " + tokens[cursor].toString());
            }
            cursor++;
            stmt->rhs = parseBlock(tokens, cursor);
            if(tokens[cursor].type != LexTokenType::SmRBrace) {
                throw invalid_argument("Expected '}' after else block: " + tokens[cursor].toString());
            }
            cursor++;
        }
        return stmt;
    } else if(tokens[cursor].type == LexTokenType::KwWhile) {
        ASTNode* stmt = ASTNode::allocNode();
        stmt->type = ASTNodeType::StmtWhile;
        stmt->line = tokens[cursor].line;
        cursor++;
        if(tokens[cursor].type != LexTokenType::SmLParen) {
            throw invalid_argument("Expected '(' after while: " + tokens[cursor].toString());
        }
        cursor++;
        stmt->cond = parseExpression(tokens, cursor);
        if(tokens[cursor].type != LexTokenType::SmRParen) {
            throw invalid_argument("Expected ')' after while condition: " + tokens[cursor].toString());
        }
        cursor++;
        if(tokens[cursor].type != LexTokenType::SmLBrace) {
            throw invalid_argument("Expected '{' after while condition: " + tokens[cursor].toString());
        }
        cursor++;
        stmt->lhs = parseBlock(tokens, cursor);
        if(tokens[cursor].type != LexTokenType::SmRBrace) {
            throw invalid_argument("Expected '}' after while block: " + tokens[cursor].toString());
        }
        cursor++;
        return stmt;
    } else if(tokens[cursor].type == LexTokenType::kwReturn) {
        ASTNode* stmt = ASTNode::allocNode();
        stmt->type = ASTNodeType::StmtReturn;
        stmt->line = tokens[cursor].line;
        cursor++;
        stmt->lhs = parseExpression(tokens, cursor);
        if(tokens[cursor].type != LexTokenType::SmSemi) {
            throw invalid_argument("Expected semi at end of return: " + tokens[cursor].toString());
        }
        cursor++;
        return stmt;
    } else {
        throw invalid_argument("Unexpected start of statement: " + tokens[cursor].toString());
    }
}

ASTNode* parseInputList(vector<LexToken>& tokens, int& cursor) {
    if(tokens[cursor].type == LexTokenType::SmSemi) {
        cursor++;
        return nullptr;
    } else if(tokens[cursor].type == LexTokenType::ValIdentifier) {
        ASTNode* param = ASTNode::allocNode();
        param->type = ASTNodeType::ValIdentifier;
        param->line = tokens[cursor].line;
        param->strVal = tokens[cursor].strVal;
        cursor++;
        param->next = parseInputList(tokens, cursor);
        return param;
    } else {
        throw invalid_argument("Unexpected token in input list" + tokens[cursor].toString());
    }
}

ASTNode* parseExpression(vector<LexToken>& tokens, int& cursor) {
    ASTNode* sum = parseSum(tokens, cursor);
    ASTNodeType compType;
    switch(tokens[cursor].type) {
        case LexTokenType::OpEQ   : compType = ASTNodeType::OpEQ  ; break;
        case LexTokenType::OpGT   : compType = ASTNodeType::OpGT  ; break;
        case LexTokenType::OpGTEQ : compType = ASTNodeType::OpGTEQ; break;
        case LexTokenType::OpLT   : compType = ASTNodeType::OpLT  ; break;
        case LexTokenType::OpLTEQ : compType = ASTNodeType::OpLTEQ; break;
        default                   : compType = ASTNodeType::None  ;
    }
    if(compType != ASTNodeType::None) {
        ASTNode* expr = ASTNode::allocNode();
        expr->type = compType;
        expr->line = tokens[cursor].line;
        expr->lhs = sum;
        cursor++;
        expr->rhs = parseSum(tokens, cursor);
        sum = expr;
    }
    return sum;
}

ASTNode* parseSum(vector<LexToken>& tokens, int& cursor) {
    ASTNode* term = parseTerm(tokens, cursor);
    ASTNodeType sumType;
    do {
        switch(tokens[cursor].type) {
            case LexTokenType::OpAdd : sumType = ASTNodeType::OpAdd  ; break;
            case LexTokenType::OpSub : sumType = ASTNodeType::OpSub  ; break;
            default                  : sumType = ASTNodeType::None  ;
        }
        if(sumType != ASTNodeType::None) {
            ASTNode* sum = ASTNode::allocNode();
            sum->type = sumType;
            sum->line = tokens[cursor].line;
            sum->lhs = term;
            cursor++;
            sum->rhs = parseTerm(tokens, cursor);
            term = sum;
        }
    } while (sumType != ASTNodeType :: None);
    return term;
}

ASTNode* parseTerm(vector<LexToken>& tokens, int& cursor) {
    ASTNode* factor = parseFactor(tokens, cursor);
    ASTNodeType termType;
    do {
        switch(tokens[cursor].type) {
            case LexTokenType::OpMul : termType = ASTNodeType::OpMul  ; break;
            case LexTokenType::OpMod : termType = ASTNodeType::OpMod  ; break;
            case LexTokenType::OpDiv : termType = ASTNodeType::OpDiv  ; break;
            default                  : termType = ASTNodeType::None   ;
        }
        if(termType != ASTNodeType::None) {
            ASTNode* term = ASTNode::allocNode();
            term->type = termType;
            term->line = tokens[cursor].line;
            term->lhs = factor;
            cursor++;
            term->rhs = parseFactor(tokens, cursor);
            factor = term;
        }
    } while (termType != ASTNodeType :: None);
    return factor;
}

ASTNode* parseFactor(vector<LexToken>& tokens, int& cursor) {
    ASTNode* factor;
    switch(tokens[cursor].type) {
        case LexTokenType::ValIdentifier : 
            factor = ASTNode::allocNode();
            factor->type = ASTNodeType::ValIdentifier;
            factor->line = tokens[cursor].line;
            factor->strVal = tokens[cursor].strVal;
            cursor++;
            return factor;
        case LexTokenType::ValInteger : 
            factor = ASTNode::allocNode();
            factor->type = ASTNodeType::ValInt;
            factor->line = tokens[cursor].line;
            factor->intVal = tokens[cursor].intVal;
            cursor++;
            return factor;
        case LexTokenType::OpSub : 
            factor = ASTNode::allocNode();
            factor->type = ASTNodeType::ValInt;
            factor->line = tokens[cursor].line;
            cursor++;
            if(tokens[cursor].type != LexTokenType::ValInteger) {
                throw invalid_argument("Expected integer after '-' : " + tokens[cursor].toString());
            }
            factor->intVal = -1 * tokens[cursor].intVal;
            cursor++;
            return factor;
        case LexTokenType::SmLParen : 
            cursor++;
            factor = parseExpression(tokens, cursor);
            if(tokens[cursor].type != LexTokenType::SmRParen) {
                throw invalid_argument("Expected ')' after expression: " + tokens[cursor].toString());
            }
            cursor++;
            return factor;
        default : 
            throw invalid_argument("Expected factor - identifier, int, or '(' -: " + tokens[cursor].toString());
    }
}