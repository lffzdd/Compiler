#include "../1-lex/lexer.c"
#include <stdlib.h>
#include <string.h>

Token currentToken;
int pos = 0;
const char* src; // 全局保存源码

// 辅助函数
void nextToken() { currentToken = getNextToken(src, &pos); }

void expect(TokenType type, const char* value) {
	if (currentToken.type != type || (value && strcmp(currentToken.value, value) != 0)) {
		printf("Syntax Error: Expected %s '%s', got %s '%s'\n", tokenTypeToString(type), value ? value : "",
			tokenTypeToString(currentToken.type), currentToken.value);
		exit(1);
	}
	nextToken();
}

// 递归下降解析器
void parseProgram();
void parseBlock();  // 新增：解析大括号内的语句块

void parseStatement();
void parseVariableDeclaration();
void parseAssignment();
void parseIfStatement();
void parseWhileStatement();

void parseExpression();
void parseTerm();
void parseFactor();

// program ::= statement | statement program
void parseProgram() {
	while (currentToken.type != TOKEN_EOF) {
		parseStatement();
	}
}

// 解析大括号内的语句块，直到遇到右大括号
void parseBlock() {
	while (currentToken.type != TOKEN_EOF &&
		!(currentToken.type == TOKEN_RIGHT_BRACE && strcmp(currentToken.value, "}") == 0)) {
		parseStatement();
	}
}

// statement ::= variable_declaration | assignment | if_statement |
// while_statement
void parseStatement() {
	if (currentToken.type == TOKEN_KEYWORD &&
		(strcmp(currentToken.value, "int") == 0 || strcmp(currentToken.value, "float") == 0 ||
			strcmp(currentToken.value, "string") == 0 || strcmp(currentToken.value, "bool") == 0 ||
			strcmp(currentToken.value, "char") == 0)) {
		parseVariableDeclaration();
	}
	else if (currentToken.type == TOKEN_IDENTIFIER && strcmp(currentToken.value, "=") == 0) {
		parseAssignment();
	}
	else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "if") == 0) {
		parseIfStatement();
	}
	else if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "while") == 0) {
		parseWhileStatement();
	}
	else {
		printf("Syntax Error: Unexpected token %s '%s'\n", tokenTypeToString(currentToken.type), currentToken.value);
		exit(1);
	}
}

// variable_declaration ::= type identifier ('=' expression)? ';'
void parseVariableDeclaration() {
	nextToken(); // 跳过类型
	expect(TOKEN_IDENTIFIER, NULL);
	if (currentToken.type == TOKEN_OPERATOR && strcmp(currentToken.value, "=") == 0) {
		nextToken();
		parseExpression();
	}
	expect(TOKEN_SEMICOLON, ";");
}

// assignment ::= identifier '=' expression ';'
void parseAssignment() {
	expect(TOKEN_IDENTIFIER, NULL);
	expect(TOKEN_OPERATOR, "=");
	parseExpression();
	expect(TOKEN_SEMICOLON, ";");
}


void parseIfStatement() {
	expect(TOKEN_KEYWORD, "if");
	expect(TOKEN_LEFT_PAREN, "(");
	parseExpression();
	expect(TOKEN_RIGHT_PAREN, ")");
	expect(TOKEN_LEFT_BRACE, "{");
	parseBlock();
	expect(TOKEN_RIGHT_BRACE, "}");
	if (currentToken.type == TOKEN_KEYWORD && strcmp(currentToken.value, "else") == 0) {
		nextToken();
		expect(TOKEN_LEFT_BRACE, "{");
		parseBlock();
		expect(TOKEN_RIGHT_BRACE, "}");
	}
}

void parseWhileStatement() {
	expect(TOKEN_KEYWORD, "while");
	expect(TOKEN_LEFT_PAREN, "(");
	parseExpression();
	expect(TOKEN_RIGHT_PAREN, ")");
	expect(TOKEN_LEFT_BRACE, "{");
	parseBlock();
	expect(TOKEN_RIGHT_BRACE, "}");
}

// 解析表达式
void parseExpression() {
	parseTerm();
	while (currentToken.type == TOKEN_OPERATOR &&
		(strcmp(currentToken.value, "+") == 0 ||
			strcmp(currentToken.value, "-") == 0 ||
			strcmp(currentToken.value, ">") == 0 ||
			strcmp(currentToken.value, "<") == 0)) {
		nextToken();
		parseTerm();
	}
}

void parseTerm() {
	parseFactor();
	while (currentToken.type == TOKEN_OPERATOR &&
		(strcmp(currentToken.value, "*") == 0 ||
			strcmp(currentToken.value, "/") == 0 ||
			strcmp(currentToken.value, "==") == 0 ||
			strcmp(currentToken.value, "!=") == 0 ||
			strcmp(currentToken.value, "<=") == 0 ||
			strcmp(currentToken.value, ">=") == 0)) {
		nextToken();
		parseFactor();
	}
}

void parseFactor() {
	if (currentToken.type == TOKEN_INTEGER || currentToken.type == TOKEN_FLOAT ||
		currentToken.type == TOKEN_STRING || currentToken.type == TOKEN_CHAR ||
		currentToken.type == TOKEN_BOOL) {
		nextToken();
	}
	else if (currentToken.type == TOKEN_IDENTIFIER) {
		nextToken();
	}
	else if (currentToken.type == TOKEN_LEFT_PAREN && strcmp(currentToken.value, "(") == 0) {
		nextToken();
		parseExpression();
		expect(TOKEN_RIGHT_PAREN, ")");
	}
	else {
		printf("Syntax Error: Unexpected token in factor %s '%s'\n", tokenTypeToString(currentToken.type), currentToken.value);
		exit(1);
	}
}

int parser(const char* source) {
	src = source;
	pos = 0;
	nextToken(); // 初始化currentToken

	parseProgram();

	if (currentToken.type != TOKEN_EOF) {
		printf("Syntax Error: Unexpected token after program end %s '%s'\n", tokenTypeToString(currentToken.type), currentToken.value);
		return 1;
	}

	printf("Parsing completed successfully.\n");
	return 0;
}

int main(const int argc, const char* argv[]) {
	// 接收命令行参数文件路径
	const char* file_path = argv[1];
	// 读取文件内容
	FILE* file = fopen(file_path, "r");
	if (!file) {
		perror("Failed to open file");
		return 1;
	}
	fseek(file, 0, SEEK_END);
	long file_size = ftell(file);
	fseek(file, 0, SEEK_SET);
	char* source = (char*)malloc(file_size + 1);
	fread(source, 1, file_size, file);
	source[file_size] = '\0';
	fclose(file);

	// 词法分析
	printf("----- Lexical Analysis -----\n");
	lexer(source);

	// 语法分析
	printf("\n----- Syntax Analysis -----\n");
	int result = parser(source);
	free(source);
	return result;

}