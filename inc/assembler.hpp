#pragma once

#include <iostream>
#include <set>
#include <string>
#include <regex>
#include <unordered_map>
#include <list>

using namespace std;

regex inputFileName ("[0-9a-zA-Z_]+.s");
regex outputFileName ("[0-9a-zA-Z_]+.(txt|o)");
regex singleReg ("((r[0-8])|sp|pc)");
regex firstRegOfTwo ("((r[0-8])|sp|pc),?");
regex secondRegOfTwo (",?((r[0-8])|sp|pc)");
regex regIndirectAddress ("\\*\\[[ ]*((r[0-8])|sp|pc)[ ]*\\]");
regex regIndirectData ("\\[[ ]*((r[0-8])|sp|pc)[ ]*\\]");
regex regIndAddLitAddress ("([ ]*\\*\\[[ ]*((r[0-8])|sp|pc)[ ]+\\+[ ]+-?(0x[0-9a-fA-F]+|[0-9]+)[ ]*\\][ ]*)");
regex regIndAddLitData ("([ ]*\\[[ ]*((r[0-8])|sp|pc)[ ]+\\+[ ]+-?(0x[0-9a-fA-F]+|[0-9]+)[ ]*\\][ ]*)");
regex regIndAddSymAddress ("([ ]*\\*\\[[ ]*((r[0-8])|sp|pc)[ ]+\\+[ ]+([0-9a-zA-Z_]+)[ ]*\\][ ]*)");
regex regIndAddSymData ("([ ]*\\[[ ]*((r[0-8])|sp|pc)[ ]+\\+[ ]+([0-9a-zA-Z_]+)[ ]*\\][ ]*)");
regex literal ("([ ]*(-?(0x[0-9a-fA-F]+|[0-9]+)),?[ ]*)");
regex symbol ("(^([_a-zA-Z])([0-9a-zA-Z_]*)),?[ ]*");
regex label ("(^([a-zA-Z])([0-9a-zA-Z_]+)):[ ]*");
regex symbolsList ("[ ]*([0-9a-zA-Z_]+)(,[ ]+[0-9a-zA-Z_]+)*");
regex valueOperand ("([ ]*\\[[ ]*((r[0-8])|sp|pc)[ ]*\\])|([ ]*((r[0-8])|sp|pc)[ ]*)|([ ]*[$]?(-?(0x[0-9a-fA-F]+)|([0-9]+))[ ]*)|([ ]*[$%]?[0-9a-zA-Z_]+[ ]*)|([ ]*\\[[ ]*((r[0-8])|sp|pc)[ ]+\\+[ ]+-?(0x[0-9a-fA-F]+|[0-9]+)[ ]*\\][ ]*)|([ ]*\\[[ ]*((r[0-8])|sp|pc)[ ]+\\+[ ]+([0-9a-zA-Z_]+)[ ]*\\][ ]*)");
regex addressOperand ("([ ]*\\*\\[[ ]*((r[0-8])|sp|pc)[ ]*\\][ ]*)|([ ]*\\*((r[0-8])|sp|pc)[ ]*)|([ ]*[*]?-?(0x[0-9a-fA-F]+|[0-9]+)[ ]*)|([ ]*[*%]?([0-9a-zA-Z_]+)[ ]*)|([ ]*\\*\\[[ ]*((r[0-8])|sp|pc)[ ]+\\+[ ]+-?(0x[0-9a-fA-F]+|[0-9]+)[ ]*\\][ ]*)|([ ]*\\*\\[[ ]*((r[0-8])|sp|pc)[ ]+\\+[ ]+([0-9a-zA-Z_]+)[ ]*\\][ ]*)");
regex jmpAddressRegIndRegDir ("([ ]*\\*\\[[ ]*((r[0-8])|sp|pc)[ ]*\\][ ]*)|([ ]*\\*((r[0-8])|sp|pc)[ ]*)");
regex valueAddressRegIndRegDir ("([ ]*\\[[ ]*((r[0-8])|sp|pc)[ ]*\\])|([ ]*((r[0-8])|sp|pc)[ ]*)");

struct tableOfSymbolsRow {
    string name;
    unsigned short section;
    unsigned short value; //symbol value or section size
    char visibility;
    unsigned short number;

    bool operator < (const tableOfSymbolsRow& str) const
    {
        return (number < str.number);
    }

};

std::ostream& operator <<(std::ostream& os, const tableOfSymbolsRow& sym)
{
        printf("%-18s", sym.name.data());
    //os  << sym.name << "   ";
        printf("%x   ", sym.section);
        printf("%-4x   ", sym.value);
        //<< sym.section << "   " 
        //<< sym.value << "   " 
    os  << sym.visibility << "   ";
        printf("%x   ", sym.number);
        //<< sym.number << endl;
        
    return os;
}


struct tableOfRelocationsRow{
    unsigned short offset;
    string type;
    unsigned short symbolNumber;
    unsigned short section;

    bool operator < (const tableOfRelocationsRow& str) const
    {
        return (section < str.section);
    }
};

vector<vector<string>> sourceCode;
list<tableOfSymbolsRow> tableOfSymbols;
list<tableOfRelocationsRow> tableOfRelocations;
unordered_map<string, unsigned char> instructionCode;
string push_IC = "b0 60 12";
string pop_IC = "a0 60 42";
//unsigned char push_IC[] = {0xb, 0x0, 0x6, 0x0, 0x1, 0x2};
//unsigned char pop_IC[] = {0xa, 0x0, 0x6, 0x0, 0x4, 0x2};

set<string> directives {
    ".global",
    ".extern",
    ".section",
    ".word",
    ".skip",
    ".equ",
    ".end"
};

set<string> mnemonics {
    "halt",
    "int",
    "iret",
    "call",
    "ret",
    "jmp",
    "jeq",
    "jne",
    "jgt",
    "push",
    "pop",
    "xchg",
    "add",
    "sub",
    "mul",
    "div",
    "cmp",
    "not",
    "and",
    "or",
    "xor",
    "test",
    "shl",
    "shr",
    "ldr",
    "str"
};

set<string> onlyMnemonics {
    "halt",
    "iret",
    "ret"
};

set<string> singleRegMnemonics {
    "int",
    "push",
    "pop",
    "not"
};

set<string> doubleRegMnemonics {
    "xchg",
    "add",
    "sub",
    "mul",
    "div",
    "cmp",
    "and",
    "or",
    "xor",
    "test",
    "shl",
    "shr"
};

set<string> singleOperandMnemonics {
    "call",
    "jmp",
    "jeq",
    "jne",
    "jgt"
};

set<string> regOperandMnemonics {
    "ldr",
    "str"
};