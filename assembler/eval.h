#pragma once
#include <string>
#include <stack>
using namespace std;

int precedence(char op);
int applyOp(int a, int b, char op);
int evaluate(string tokens);
