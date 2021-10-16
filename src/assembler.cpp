#include "assembler.hpp"
#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <iomanip>

using namespace std;

string inputFile;
string outputFile = "assembler.txt";

list<tableOfSymbolsRow> symbols;
list<tableOfSymbolsRow> undefinedSymbols;
list<tableOfSymbolsRow> sections = 
{
    {"UND", 0x0, 0x0, 'l', 0x0},
    {"abs", 0x1, 0x0, 'l', 0x1}
};

unsigned short symbolCnt = 0x0;
unsigned short undefinedSymbolsCnt = 0x0;
unsigned short sectionCnt = 0x2;
unsigned short locationCnt = 0x0;
unsigned short absLocationCnt = 0x0;

void syntaxCheck(vector<string> lineSplit, string line, int lineCnt)
{
    if(mnemonics.count(lineSplit.front()) == 0 && directives.count(lineSplit.front()) == 0 && !regex_match(lineSplit.front(), label))
    {
        cout << "Sintaksna greska na liniji: " << lineCnt << endl;
        cout << "Hint: sve asemblerske direktive i instrukcije moraju imati razmak iza sebe." << endl;
        exit(-20);
    }

    if(lineSplit.front() != *directives.find(".end"))
    { 
        if(directives.count(lineSplit.front()) && 
        lineSplit.size() < 2)
        {
            cout << "Nepotpuna asemblerska direktiva!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-8);
        }
        return;
    }

    if(lineSplit.front() == *directives.find(".end"))
    {
        return;
    }

    if(lineSplit.front() == *directives.find(".global"))
    {
        // if(!regex_match(line.substr(directives.find(".global")->size()), symbolsList) || lineSplit.size() == 1)
        // {
        //     cout << "Direktiva .global zahteva simbol ili listu simbola!" << endl;
        //     cout << "Greska na liniji: " << lineCnt << endl;
        //     exit(-4);
        // }

        for(vector<string>::iterator str = lineSplit.begin() + 1; str != lineSplit.end(); str++)
        {
            if(!regex_match(*str, symbol) && (*str != ",")) 
            {
                cout << "Neispravan format za naziv simbola! Direktiva .global zahteva simbol ili listu simbola!" << endl;
                cout << "Greska na liniji: " << lineCnt << endl;
                exit(-4);
            }
        }

        return;

    }

    if(lineSplit.front() == *directives.find(".extern"))
    {
        for(vector<string>::iterator str = lineSplit.begin() + 1; str != lineSplit.end(); str++)
        {
            if(!regex_match(*str, symbol) && (*str != ",")) 
            {
                cout << "Neispravan format za naziv simbola! Direktiva .extern zahteva simbol ili listu simbola!" << endl;
                cout << "Greska na liniji: " << lineCnt << endl;
                exit(-5);
            }
        }
        return;

    }

    if(lineSplit.front() == *directives.find(".section"))
    {
        if((lineSplit.size() != 2) || (lineSplit.size() == 2 && !regex_match(lineSplit[1], symbol)))
        {
            cout << "Neispravan format za naziv sekcije!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-5);
        }
        return;
    }

    if(lineSplit.front() == *directives.find(".word"))
    {
        for(vector<string>::iterator str = lineSplit.begin() + 1; str != lineSplit.end(); str++)
        {
            if(!regex_match(*str, symbol) && !regex_match(*str, literal) && (*str != ",")) 
            {
                cout << "Neispravan format za naziv inizijalizatora!" << endl;
                cout << "Greska na liniji: " << lineCnt << endl;
                exit(-7);
            }
        }
        return;
    }

    if(lineSplit.front() == *directives.find(".skip"))
    {
        if(lineSplit.size() > 2 || !regex_match(lineSplit[1], literal))
        {
            cout << "Neispravan format za literal!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-9);
        }
        return;
    }

    if(lineSplit.front() == *directives.find(".equ"))
    {
        if(lineSplit.size() < 2 || lineSplit.size() > 4)
        {
            cout << "Neispravan broj argumenata za .equ direktivu!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-10);
        }

        if(!regex_match(lineSplit[1].substr(0, lineSplit[1].find_last_not_of(',') + 1), symbol))
        {
            cout << "Neispravan format za naziv simbola!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-10);
        }

        if(!regex_match(line.substr(line.find_first_of(',') + 1), literal))
        {
            cout << "Neispravan format za literal!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-10);
        }

        return;
    }

    if((lineSplit.front() == *mnemonics.find("halt") || 
        lineSplit.front() == *mnemonics.find("ret") || 
        lineSplit.front() == *mnemonics.find("iret")))
    {
        if(lineSplit.size() > 1) 
        {
            cout << "Sintaksna greska na liniji: " << lineCnt << endl;
            exit(-11);
        }
        return;
    }

    if(lineSplit.front() == *mnemonics.find("int") ||
        lineSplit.front() == *mnemonics.find("push") || 
        lineSplit.front() == *mnemonics.find("pop") || 
        lineSplit.front() == *mnemonics.find("not"))
    {
        if(lineSplit.size() != 2 || !regex_match(lineSplit[1], singleReg ))
        {
            cout << "Sintaksna greska na liniji: " << lineCnt << endl;
            exit(-12);
        }
        return;
    }

    if(lineSplit.front() == *mnemonics.find("xchg") ||
        lineSplit.front() == *mnemonics.find("add") || 
        lineSplit.front() == *mnemonics.find("sub") || 
        lineSplit.front() == *mnemonics.find("mul") ||
        lineSplit.front() == *mnemonics.find("div") ||
        lineSplit.front() == *mnemonics.find("cmp") ||
        lineSplit.front() == *mnemonics.find("and") ||
        lineSplit.front() == *mnemonics.find("or") ||
        lineSplit.front() == *mnemonics.find("xor") ||
        lineSplit.front() == *mnemonics.find("test") ||
        lineSplit.front() == *mnemonics.find("shl") ||
        lineSplit.front() == *mnemonics.find("shr"))
    {
        if(lineSplit.size() < 2 || lineSplit.size() > 4)
        {
            cout << "Neispravan broj argumenata, instrukcija ocekuje 2 registra!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-13);
        }

        if(!regex_match(lineSplit[1].substr(0, lineSplit[1].find_last_not_of(',') + 1), singleReg) || !regex_match(line.substr(line.find_first_of(',')), secondRegOfTwo))
        {
            cout << "Neispravan naziv registra!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-13);
        }
        return;

    }

    if(lineSplit.front() == *mnemonics.find("call") ||
        lineSplit.front() == *mnemonics.find("jmp") ||
        lineSplit.front() == *mnemonics.find("jeq") ||
        lineSplit.front() == *mnemonics.find("jne") ||
        lineSplit.front() == *mnemonics.find("jgt"))
    {
        if(!regex_match(line.substr(lineSplit.front().size()), addressOperand))
        {
            cout << line.substr(lineSplit.front().size()) << endl;
            cout << "Neispravan adresni operand!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-14);
        }
        return;
    }

    if(lineSplit.front() == *mnemonics.find("ldr") ||
        lineSplit.front() == *mnemonics.find("str"))
    {
        if(lineSplit.size() < 3 || !regex_match(line.substr(line.find_first_of(',') + 1), valueOperand))
        {
            cout << line.substr(line.find_first_of(',') + 1) << endl;
            cout << "Neispravan operand!" << endl;
            cout << "Greska na liniji: " << lineCnt << endl;
            exit(-15);
        }
        return;
    }

}

unsigned short storeAsHex(string literal)
{
    literal = literal.substr(literal.find_first_not_of(" \t\n\v\f\r"));

    unsigned short temp;
    stringstream ss;
    if(literal.front() == '0')
    { 
        ss << hex << literal;
    }
    else
    {
        ss << hex << (short)stoi(literal);
    }
    ss >> temp;

    return temp;
}

void firstPassAndSyntaxCheck() 
{
    ifstream inputFileStream(inputFile); 

    if(!inputFileStream.is_open())
    {
        cout << "Fajl sa izvornim kodom zadatog imena nije pronadjen!" << endl;
        exit(-21);
    }

    string line;
    int lineCnt = 0;

    while (getline(inputFileStream, line))
    {
        lineCnt++;

        if(line.find_first_not_of(" \t\n\v\f\r") == string::npos) continue;
        line = line.substr(line.find_first_not_of(" \t\n\v\f\r"));
        if(line.front() == '#') continue;
        if(line.find('#')) 
        {
            line = line.substr(0, line.find('#'));    
        }

        istringstream iss(line);
        string temp;

        vector<string> lineSplit;

        while(iss >> temp)
        {

            if(temp == *directives.find(".end")) 
            {
                lineSplit.push_back(temp);
                sourceCode.push_back(lineSplit);
                break;
            }

            if(lineSplit.size() > 0) 
            {
                if(regex_match(temp, label))
                {
                    cout << "Moze postojati samo jedna labela po liniji koda i ona mora stajati na pocetku te linije!" << endl;
                    cout << "Greska na liniji: " << lineCnt << endl;
                    exit(-2);
                }

                if(mnemonics.count(temp) || directives.count(temp))
                {
                    int i = regex_match(lineSplit.front(), label) ? 1 : 0;
                    for(i; i < lineSplit.size(); i++)
                    {
                        if(mnemonics.count(lineSplit[i]) || directives.count(lineSplit[i])) 
                        {
                            cout << "Moze postojati samo jedna direktiva ili naredba po liniji koda!" << endl;
                            cout << "Greska na liniji: " << lineCnt << endl;
                            exit(-3);
                        }
                    }
                }
            }

            lineSplit.push_back(temp);
        }

        if(lineSplit.size() > 0) 
        {
            //sintaksna provera koda_________________________________________________________
            if (regex_match(lineSplit.front(), label) && lineSplit.size() > 1)
            {
                vector<string> tempLineSplit(lineSplit.begin() + 1, lineSplit.end());
                syntaxCheck(tempLineSplit, line.substr(lineSplit.front().size()), lineCnt);
            }
            else
            {
                syntaxCheck(lineSplit, line, lineCnt);
            }
            
            //cout << line << endl;
            //sourceCode.push_back(lineSplit);

            //dodavanje simbola u tabelu simbola i azuriranje njihovih parametara_____________
            if(regex_match(lineSplit.front(), label)) 
            {
                string newSymbolName = lineSplit[0].substr(0, lineSplit[0].find_last_not_of(':') + 1);

                list<tableOfSymbolsRow>::iterator it = undefinedSymbols.begin();
                for(; it != undefinedSymbols.end(); ++it)
                {
                    if(it->name == newSymbolName)
                    {
                        cout << "Simbol sa istim imenom je definisan u drugom fajlu!" << endl;
                        cout << "Greska na liniji: " << lineCnt << endl;
                        exit(-18);
                    }
                }

                bool exists = false;
                for(list<tableOfSymbolsRow>::iterator it = symbols.begin(); it != symbols.end(); ++it)
                {
                    if(it->name == newSymbolName)
                    {
                        if(it->visibility == 'g')
                        {
                            it->section = sectionCnt - 0x1;
                            it->number = symbolCnt++;
                            it->value = locationCnt;
                            exists = true;
                        }
                        else
                        {
                            cout << "Simbol sa istim imenom definisan na 2 mesta u programu!" << endl;
                            cout << "Greska na liniji: " << lineCnt << endl;
                            exit(-16);
                        }
                    }
                }

                if(!exists) 
                    symbols.push_back({newSymbolName, (unsigned short)(sectionCnt - 0x1), locationCnt, 'l', symbolCnt++});

                //lineSplit = vector<string> (lineSplit.begin() + 1, lineSplit.end());
                lineSplit.erase(lineSplit.begin());
                
            }

            if(lineSplit.front() == *directives.find(".global"))
            {
                for(vector<string>::iterator str = lineSplit.begin() + 1; str != lineSplit.end(); str++)
                {
                    if(*str != ",")
                    {
                        string newSymbolName = str->substr(0, str->find_last_not_of(',') + 1);

                        list<tableOfSymbolsRow>::iterator it = symbols.begin();
                        for(; it != symbols.end(); ++it)
                        {
                            if(it->name == newSymbolName)
                            {
                                it->visibility = 'g';
                                break;
                            }
                        }

                        if(it == symbols.end())
                            symbols.push_back({newSymbolName, 0x0, 0x0, 'g', 0x0});
                    }
                }
            }

            if(lineSplit.front() == *directives.find(".extern"))
            {
                for(vector<string>::iterator str = lineSplit.begin() + 1; str != lineSplit.end(); str++)
                {
                    if(*str != ",")
                    {
                        string newSymbolName = str->substr(0, str->find_last_not_of(',') + 1);

                        list<tableOfSymbolsRow>::iterator it = undefinedSymbols.begin();
                        for(; it != undefinedSymbols.end(); ++it)
                        {
                            if(it->name == newSymbolName)
                                break;
                        }

                        if(it == undefinedSymbols.end())
                            undefinedSymbols.push_back({newSymbolName, 0x0, 0x0, 'g', undefinedSymbolsCnt++});
                    }
                }
                
            }
            
            if(lineSplit.front() == *directives.find(".section"))
            {
                list<tableOfSymbolsRow>::iterator it = sections.begin();
                for(; it != sections.end(); ++it)
                {
                    if(it->name == lineSplit[1])
                    {
                        cout << "Sekcija sa istim imenom definisana na 2 mesta u programu!" << endl;
                        cout << "Greska na liniji: " << lineCnt << endl;
                        exit(-17);
                    }

                    if(it->number == sectionCnt - 1)
                    {
                        it->value = locationCnt;
                    }
                }

                locationCnt = 0;
                sections.push_back({lineSplit[1], sectionCnt, 0x0, 'l', sectionCnt++});   
                            
            }

            if(lineSplit.front() == *directives.find(".equ"))
            {
                string newSymbolName = lineSplit[1].substr(0, lineSplit[1].find_last_not_of(',') + 1);


                list<tableOfSymbolsRow>::iterator it = undefinedSymbols.begin();
/*               for(; it != undefinedSymbols.end(); it++)
                {
                    if(it->name == newSymbolName)
                    {
                        cout << "Simbol sa istim imenom je definisan u drugom fajlu!" << endl;
                        cout << "Greska na liniji: " << lineCnt << endl;
                        exit(-18);
                    }
                }
*/
                bool exists = false;

                it = symbols.begin();
                for(; it != symbols.end(); it++)
                {
                    if(it->name == newSymbolName && it->visibility == 'l')
                    {
                        cout << "Simbol sa istim imenom definisan na 2 mesta u fajlu!" << endl;
                        cout << "Greska na liniji: " << lineCnt << endl;
                        exit(-16);
                    }

                    if(it->name == newSymbolName && it->visibility == 'g')
                    {
                        it->value = storeAsHex(lineSplit[2]);
                        it->number = symbolCnt++;
                        exists = true;
                    }
                }

                if(!exists)
                {
                    absLocationCnt += sizeof(short);

                    it = sections.begin();
                    for(; it != sections.end(); it++)
                    {
                        if(it->name == "abs")
                        {
                            it->value = absLocationCnt;
                            break;
                        }
                    }

                    unsigned short temp = storeAsHex(lineSplit.back());

                    symbols.push_back({newSymbolName, 0x1, temp, 'l', symbolCnt++});
                }
            }
        
            if(lineSplit.front() == *directives.find(".end"))
            {
                list<tableOfSymbolsRow>::iterator it = sections.begin();
                for(; it != sections.end(); it++)
                {
                    if(it->number == (sectionCnt - 0x1))
                    {
                        //it->value = locationCnt;  // velicina sekcije
                    }
                    it->value = 0x0; // brisanje velicine
                    tableOfSymbols.push_back(*it);
                }

                symbols.sort();

                it = symbols.begin();
                for(; it != symbols.end(); it++)
                {
                   it->number += sections.size();
                   tableOfSymbols.push_back(*it);
                }

                it = undefinedSymbols.begin();
                for(; it != undefinedSymbols.end(); it++)
                {
                   it->number += symbols.size() + sections.size();
                   tableOfSymbols.push_back(*it);
                }

                return;
            }
        
            if(lineSplit.front() == *mnemonics.find("halt") || 
            lineSplit.front() == *mnemonics.find("ret") || 
            lineSplit.front() == *mnemonics.find("iret"))
            {
                locationCnt += 0x1;
            }

            if(doubleRegMnemonics.count(lineSplit.front()) || 
            singleRegMnemonics.count(lineSplit.front()))
            {
                if(lineSplit.front() == *mnemonics.find("push") || 
                lineSplit.front() == *mnemonics.find("pop"))
                {
                    locationCnt += 0x3;
                }
                else
                {
                    locationCnt += 0x2;
                }
                
            }

            if(lineSplit.front() == *directives.find(".skip"))
            {
                locationCnt += storeAsHex(lineSplit[1]);
            }

            if(lineSplit.front() == *mnemonics.find("call") ||
            lineSplit.front() == *mnemonics.find("jmp") ||
            lineSplit.front() == *mnemonics.find("jeq") ||
            lineSplit.front() == *mnemonics.find("jne") ||
            lineSplit.front() == *mnemonics.find("jgt"))
            {
                if(regex_match(lineSplit[1], jmpAddressRegIndRegDir))
                {
                    locationCnt += 0x3;
                }
                else
                {
                    locationCnt += 0x5;
                }
            }

            if(lineSplit.front() == *mnemonics.find("ldr") ||
            lineSplit.front() == *mnemonics.find("str"))
            {
                if(regex_match(lineSplit[1], valueAddressRegIndRegDir))
                {
                    locationCnt += 0x3;
                }
                else
                {
                    locationCnt += 0x5;
                }
            }

            if(lineSplit.front() == *directives.find(".word"))
            {
                for(vector<string>::iterator str = lineSplit.begin() + 1; str != lineSplit.end(); str++)
                {
                    if(*str != ",")
                    {
                        locationCnt += 0x2;
                    }
                }
            }
            
        }

    }
}

void instructionCodeInit()
{
    instructionCode.insert({string("halt"), (unsigned char)0x00});
    instructionCode.insert({string("int"), (unsigned char)0x10});
    instructionCode.insert({string("iret"), (unsigned char)0x20});
    instructionCode.insert({string("call"), (unsigned char)0x30});
    instructionCode.insert({string("ret"), (unsigned char)0x40});
    instructionCode.insert({string("jmp"), (unsigned char)0x50});
    instructionCode.insert({string("jeq"), (unsigned char)0x51});
    instructionCode.insert({string("jne"), (unsigned char)0x52});
    instructionCode.insert({string("jgt"), (unsigned char)0x53});
    instructionCode.insert({string("xchg"), (unsigned char)0x60});
    instructionCode.insert({string("add"), (unsigned char)0x70});
    instructionCode.insert({string("sub"), (unsigned char)0x71});
    instructionCode.insert({string("mul"), (unsigned char)0x72});
    instructionCode.insert({string("div"), (unsigned char)0x73});
    instructionCode.insert({string("cmp"), (unsigned char)0x74});
    instructionCode.insert({string("not"), (unsigned char)0x80});
    instructionCode.insert({string("and"), (unsigned char)0x81});
    instructionCode.insert({string("or"), (unsigned char)0x82});
    instructionCode.insert({string("xor"), (unsigned char)0x83});
    instructionCode.insert({string("test"), (unsigned char)0x84});
    instructionCode.insert({string("shl"), (unsigned char)0x90});
    instructionCode.insert({string("shr"), (unsigned char)0x91});
    instructionCode.insert({string("ldr"), (unsigned char)0xA0});
    instructionCode.insert({string("str"), (unsigned char)0xB0});

}

void secondPassAndOutputGeneration()
{
    ifstream inputFileStream(inputFile);
    ofstream outputFileStream(outputFile);
    string line;
    int lineCnt = 0;

    locationCnt = 0x0;
    sectionCnt = 0x2;

    list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
    for(; it != tableOfSymbols.end(); it++)
    {
        outputFileStream << setw(16) << it->name;
        outputFileStream << "   ";
        outputFileStream << setfill('0') << setw(2) << hex << it->section;
        outputFileStream << "   ";
        outputFileStream << setfill('0') << setw(2) << hex << it->value;
        outputFileStream << "   ";
        outputFileStream << setfill(' ') << setw(3) << it->visibility;
        outputFileStream << "   ";
        outputFileStream << setfill('0') << setw(2) << hex << it->number;
        outputFileStream << setfill(' ');
        outputFileStream << endl;
    }
    outputFileStream << setfill('_') << setw(50) << "_" << endl;
    outputFileStream << setfill(' ') << endl;

    outputFileStream << ".section abs" << endl;
    it = tableOfSymbols.begin();
    for(; it != tableOfSymbols.end(); it++)
    {
        if(it->section == 0x1 && it->name != "abs")
        {
            outputFileStream << it->name << "   ";
            outputFileStream << setfill('0') << setw(4) << hex << it->value << endl;
            outputFileStream << setfill(' ');
        }
    }

    while (getline(inputFileStream, line))
    {
        lineCnt++;

        if(line.find_first_not_of(" \t\n\v\f\r") == string::npos) continue;
        line = line.substr(line.find_first_not_of(" \t\n\v\f\r"));
        if(line.front() == '#') continue;
        if(line.find('#')) 
        {
            line = line.substr(0, line.find('#'));    
        }

        istringstream iss(line);
        string temp;

        vector<string> lineSplit;

        while(iss >> temp)
        {
            lineSplit.push_back(temp);

            if(temp == *directives.find(".end")) 
            {
                break;
            }
        }

        if(lineSplit.front() == *directives.find(".section"))
        {
            locationCnt = 0x0;
            sectionCnt++;
            outputFileStream << endl;
            outputFileStream << line << endl;
            continue;
        }

        if(lineSplit.front() == *mnemonics.find("halt") || 
        lineSplit.front() == *mnemonics.find("ret") || 
        lineSplit.front() == *mnemonics.find("iret"))
        {
            outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
            locationCnt += 0x1;
            outputFileStream << ":    ";
            outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
            outputFileStream << setfill(' ');
            outputFileStream << "   " << line << endl;
        }

        if(doubleRegMnemonics.count(lineSplit.front())) //logic, aritmetic...
        {
            outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
            locationCnt += 0x2;
            outputFileStream << ":    ";
            outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
            outputFileStream << setfill(' ');
            outputFileStream << " ";

            unsigned short temp;
            if(lineSplit[1].substr(0, lineSplit[1].find_last_not_of(',') + 1) == "sp")
                temp = 0x6;
            else 
            if (lineSplit[1].substr(0, lineSplit[1].find_last_not_of(',') + 1) == "pc")
                temp = 0x7;
            else
                temp = storeAsHex(lineSplit[1].substr(lineSplit[1].find_last_not_of(','), 1));

            if(line.substr(line.find_first_of(',') + 1) == "sp")
                temp = (temp << 4) | (unsigned short)0x6;
            else 
            if (line.substr(line.find_first_of(',') + 1) == "pc")
                temp = (temp << 4) | (unsigned short)0x7;
            else
                temp = (temp << 4) | (storeAsHex(string(1, lineSplit.back().back())));               

            outputFileStream << setfill('0') << setw(2) << hex << temp;
            outputFileStream << setfill(' ');
            outputFileStream << "    " << line << endl;

        }

        if(singleRegMnemonics.count(lineSplit.front())) //push, pop, int, not
        {
            if(lineSplit.front() == *mnemonics.find("push") || 
            lineSplit.front() == *mnemonics.find("pop"))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << setfill(' ');
                locationCnt += 0x3;

                char temp;
                if(lineSplit[1] == "sp")
                    temp = '6';
                else 
                if (lineSplit[1] == "pc")
                    temp = '7';
                else
                    temp = lineSplit[1].back();

                string tempp;
                if(lineSplit.front() == *mnemonics.find("push"))
                {   
                    tempp = push_IC;
                    tempp.at(4) = temp;
                    outputFileStream << ":    " << tempp;
                }
                else
                {
                    tempp = pop_IC;
                    tempp.at(4) = temp;
                    outputFileStream << ":    " << tempp;
                }

                outputFileStream << "    " << line << endl;

            }
            else //__________________________not, int 
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                locationCnt += 0x2;

                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " ";

                unsigned short temp;
                if(lineSplit[1].substr(0, lineSplit[1].find_last_not_of(',') + 1) == "sp")
                    temp = (0x6 << 4) | (unsigned short)0xf;
                else 
                if (lineSplit[1].substr(0, lineSplit[1].find_last_not_of(',') + 1) == "pc")
                    temp = (0x7 << 4) | (unsigned short)0xf;
                else
                    temp = ((storeAsHex(lineSplit[1].substr(lineSplit[1].length() - 1, 1))) << 4) | (unsigned short)0xf;

                outputFileStream << setfill('0') << setw(2) << hex << temp;
                outputFileStream << setfill(' ');
                outputFileStream << "    " << line << endl;
            }
        }

        if(lineSplit.front() == *directives.find(".skip"))
        {
            outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
            outputFileStream << setfill(' ');
            outputFileStream << ":    ";

            unsigned short bytes = storeAsHex(lineSplit[1]);
            locationCnt += bytes;
            
            for(int i = 0; i < bytes; i++)
            {
                outputFileStream << "00 ";
            }

            outputFileStream << "    " << line << endl;
        }

        if(singleOperandMnemonics.count(lineSplit.front())) // skokovi
        {

            if(regex_match(lineSplit[1], literal))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << " ff 00 ";
                outputFileStream << setfill('0') << setw(4) << hex << storeAsHex(lineSplit[1]);
                outputFileStream << setfill(' ');
                outputFileStream << "    " << line << endl;
            }

            if(lineSplit[1].front() == '*' && regex_match(lineSplit[1].substr(1), literal))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << " ff 04 ";

                outputFileStream << setfill('0') << setw(4) << hex << storeAsHex(lineSplit[1].substr(1));
                outputFileStream << setfill(' ');
                outputFileStream << "    " << line << endl;
            }

            if(lineSplit[1].front() == '*' && regex_match(lineSplit[1].substr(1), singleReg))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " f";

                if(lineSplit[1].substr(1) == "sp")
                    outputFileStream << '6';
                else
                if(lineSplit[1].substr(1) == "pc")
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].back();

                outputFileStream << " 01 ";
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(lineSplit[1], regIndirectAddress))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " f";

                if(lineSplit[1].substr(lineSplit[1].find_first_of('[') + 1, lineSplit[1].find_last_not_of(']') - lineSplit[1].find_first_of('[')) == "sp")
                    outputFileStream << '6';
                else
                if(lineSplit[1].substr(lineSplit[1].find_first_of('[') + 1, lineSplit[1].find_last_not_of(']') - lineSplit[1].find_first_of('[')) == "pc")
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_last_not_of(']'));

                outputFileStream << " 02 ";
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(line.substr(lineSplit.front().size()), regIndAddLitAddress))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " f";

                if(line.substr(line.find_first_of('[') + 1, line.find_last_not_of('+') - line.find_first_of('[') - 1) == "sp")
                    outputFileStream << '6';
                else
                if(line.substr(line.find_first_of('[') + 1, line.find_last_not_of('+') - line.find_first_of('[') - 1) == "pc")
                    outputFileStream << '7';
                else
                    outputFileStream << line.at(line.find_first_of('r') + 1);

                outputFileStream << " 05 ";
                outputFileStream << setfill('0') << setw(4) << hex << storeAsHex(line.substr(line.find_first_of('+') + 1, line.find_last_not_of(']') - line.find_first_of('+')));
                outputFileStream << setfill(' ');
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(lineSplit[1], symbol))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " ff 00 ";

                list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
                for(; it != tableOfSymbols.end(); it++)
                {
                    if(it->name == lineSplit[1])
                    {
                        if(it->visibility == 'g') 
                        {
                            outputFileStream << "00 00";
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_abs16", it->number, (unsigned short)(sectionCnt - 0x1)});
                            break;
                        }
                        else
                        {
                            outputFileStream << setfill('0') << setw(4) << hex << it->value;
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_abs16", it->section, (unsigned short)(sectionCnt - 0x1)});
                            outputFileStream << setfill(' ');
                            break;
                        }
                    }
                }
                outputFileStream << "    " << line << endl;
            }

            if(lineSplit[1].front() == '%' && regex_match(lineSplit[1].substr(1), symbol))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " f7 05 ";

                list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
                for(; it != tableOfSymbols.end(); it++)
                {
                    if(it->name == lineSplit[1].substr(1))
                    {
                        if(it->visibility == 'g') 
                        {
                            outputFileStream << "fffe";
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_pc16", it->number, (unsigned short)(sectionCnt - 0x1)});
                            break;
                        }
                        else
                        {
                            if(it->section == (sectionCnt - 1))
                            {
                                outputFileStream << setfill('0') << setw(4) << hex << (unsigned short)(it->value - locationCnt - 0x5);
                                outputFileStream << setfill(' ');
                                break;
                            }
                            else
                            {
                                outputFileStream << setfill('0') << setw(4) << hex << (unsigned short)(it->value + 0x3);
                                tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_pc16", it->section, (unsigned short)(sectionCnt - 0x1)});
                                outputFileStream << setfill(' ');
                                break;
                            }
                        }
                    }
                }
                outputFileStream << "    " << line << endl;
            }

            if(lineSplit[1].front() == '*' && regex_match(lineSplit[1].substr(1), symbol) && !regex_match(lineSplit[1].substr(1), singleReg))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ') << setw(2);
                outputFileStream << " ff 04 ";

                list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
                for(; it != tableOfSymbols.end(); it++)
                {
                    if(it->name == lineSplit[1].substr(1))
                    {
                        if(it->visibility == 'g') 
                        {
                            outputFileStream << "00 00";
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_mem16", it->number, (unsigned short)(sectionCnt - 0x1)});
                            break;
                        }
                        else
                        {
                            outputFileStream << setfill('0') << setw(4) << hex << it->value;
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_mem16", it->section, (unsigned short)(sectionCnt - 0x1)});
                            outputFileStream << setfill(' ');
                            break;
                        }
                    }
                }
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(line.substr(lineSplit.front().size()), regIndAddSymAddress))
            {
                string tempName = line.substr(line.find_first_of('+') + 1, line.find_last_not_of(']') - line.find_first_of('+'));

                if(regex_match(tempName, symbol)) 
                {
                    outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                    outputFileStream << ":    ";
                    outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                    outputFileStream << setfill(' ');
                    outputFileStream << " f";

                    if(line.substr(line.find_first_of('[') + 1, line.find_last_not_of('+') - line.find_first_of('[') - 1) == "sp")
                        outputFileStream << '6';
                    else
                    if(line.substr(line.find_first_of('[') + 1, line.find_last_not_of('+') - line.find_first_of('[') - 1) == "pc")
                        outputFileStream << '7';
                    else
                        outputFileStream << line.at(line.find_first_of('r') + 1);

                    outputFileStream << " 05 ";

                    list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
                    for(; it != tableOfSymbols.end(); it++)
                    {
                        if(it->name == tempName)
                        {
                            if(it->visibility == 'g') 
                            {
                                outputFileStream << "00 00";
                                tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_regIndAddSym16", it->number, (unsigned short)(sectionCnt - 0x1)});
                                break;
                            }
                            else
                            {
                                outputFileStream << setfill('0') << setw(4) << hex << it->value;
                                tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_regIndAddSym16", it->section, (unsigned short)(sectionCnt - 0x1)});
                                outputFileStream << setfill(' ');
                                break;
                            }
                        }
                    }

                    outputFileStream << "    " << line << endl;
                }

            }

            if(regex_match(lineSplit[1], jmpAddressRegIndRegDir))
            {
                locationCnt += 0x3;
            }
            else
            {
                locationCnt += 0x5;
            }

        }

        if(regOperandMnemonics.count(lineSplit.front()))
        {

            if(lineSplit.back().front() == '$' && regex_match(lineSplit.back().substr(1), literal))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << " ";
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);
                
                outputFileStream << "f 00 ";
                outputFileStream << setfill('0') << setw(4) << hex << storeAsHex(lineSplit.back().substr(1));
                outputFileStream << setfill(' ');
                outputFileStream << "    " << line << endl;
            }

            if(lineSplit.back().front() == '$' && regex_match(lineSplit.back().substr(1), symbol))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " ";
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);
                
                outputFileStream << "f 00 ";

                list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
                for(; it != tableOfSymbols.end(); it++)
                {
                    if(it->name == lineSplit.back().substr(1))
                    {
                        outputFileStream << setfill('0') << setw(4) << hex << it->value;
                        outputFileStream << setfill(' ');
                        break;
                    }
                }
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(lineSplit.back(), literal))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << " ";
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);
                
                outputFileStream << "f 04 ";
                outputFileStream << setfill('0') << setw(4) << hex << storeAsHex(lineSplit.back());
                outputFileStream << setfill(' ');
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(lineSplit.back(), symbol) && !regex_match(lineSplit.back(), secondRegOfTwo))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " ";
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);
                
                outputFileStream << "f 04 ";

                list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
                for(; it != tableOfSymbols.end(); it++)
                {
                    if(it->name == lineSplit.back())
                    {
                        if(it->visibility == 'g')
                        {
                            outputFileStream << "00 00 ";
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_abs16", it->number, (unsigned short)(sectionCnt - 0x1)});

                        }
                        else
                        {
                            outputFileStream << setfill('0') << setw(4) << hex << it->value;
                            outputFileStream << setfill(' ');
                            break;
                        }
                    }
                }
                outputFileStream << "    " << line << endl;
            }

            if(lineSplit.back().front() == '%' && regex_match(lineSplit.back().substr(1), symbol))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " ";
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);
                
                outputFileStream << "7 03 ";

                list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
                for(; it != tableOfSymbols.end(); it++)
                {
                    if(it->name == lineSplit.back().substr(1))
                    {
                        if(it->visibility == 'g') 
                        {
                            outputFileStream << "ff fe";
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_pc16", it->number, (unsigned short)(sectionCnt - 0x1)});
                            break;
                        }
                        else
                        {
                            if(it->section == (sectionCnt - 1))
                            {
                                outputFileStream << setfill('0') << setw(4) << hex << (unsigned short)(it->value - locationCnt - 0x5);
                                outputFileStream << setfill(' ');
                                break;
                            }
                            else
                            {
                                outputFileStream << setfill('0') << setw(4) << hex << (unsigned short)(it->value + 0x3);
                                tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_pc16", it->section, (unsigned short)(sectionCnt - 0x1)});
                                outputFileStream << setfill(' ');
                                break;
                            }
                        }
                    }
                }
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(lineSplit.back(), singleReg))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);

                if(lineSplit.back() == "sp")
                    outputFileStream << '6';
                else
                if(lineSplit.back() == "pc")
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit.back().back();

                outputFileStream << " 01 ";
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(lineSplit.back(), regIndirectData))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " ";
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);

                if(lineSplit.back().substr(lineSplit[1].find_first_of('[') + 1, lineSplit[1].find_last_not_of(']') - lineSplit[1].find_first_of('[')) == "sp")
                    outputFileStream << '6';
                else
                if(lineSplit.back().substr(lineSplit[1].find_first_of('[') + 1, lineSplit[1].find_last_not_of(']') - lineSplit[1].find_first_of('[')) == "pc")
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit.back().at(lineSplit.back().find_last_not_of(']'));

                outputFileStream << " 02 ";
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(line.substr(lineSplit.front().size()), regIndAddLitData))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " ";
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);

                if(line.substr(line.find_first_of('[') + 1, line.find_last_not_of('+') - line.find_first_of('[') - 1) == "sp")
                    outputFileStream << '6';
                else
                if(line.substr(line.find_first_of('[') + 1, line.find_last_not_of('+') - line.find_first_of('[') - 1) == "pc")
                    outputFileStream << '7';
                else
                    outputFileStream << line.at(line.find_first_of('r') + 1);

                outputFileStream << " 05 ";
                outputFileStream << setfill('0') << setw(2) << hex << storeAsHex(line.substr(line.find_first_of('+') + 1, line.find_last_not_of(']') - line.find_first_of('+')));
                outputFileStream << setfill(' ');
                outputFileStream << "    " << line << endl;
            }

            if(regex_match(line.substr(lineSplit.front().size()), regIndAddSymData))
            {
                outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
                outputFileStream << ":    ";
                outputFileStream << setfill('0') << setw(2) << hex << (unsigned short)instructionCode.find(lineSplit.front())->second;
                outputFileStream << setfill(' ');
                outputFileStream << " ";
                if(lineSplit[1].substr(0,2) == "sp")
                    outputFileStream << '6';
                else
                if((lineSplit[1].substr(0,2) == "pc"))
                    outputFileStream << '7';
                else
                    outputFileStream << lineSplit[1].at(lineSplit[1].find_first_of('r') + 1);
                
                if(line.substr(line.find_first_of('[') + 1, line.find_last_not_of('+') - line.find_first_of('[') - 1) == "sp")
                    outputFileStream << '6';
                else
                if(line.substr(line.find_first_of('[') + 1, line.find_last_not_of('+') - line.find_first_of('[') - 1) == "pc")
                    outputFileStream << '7';
                else
                    outputFileStream << line.at(line.find_first_of('r') + 1);

                outputFileStream << " 05 ";

                string tempName = line.substr(line.find_first_of('+') + 1, line.find_last_not_of(']') - line.find_first_of('+'));

                list<tableOfSymbolsRow>::iterator it = tableOfSymbols.begin();
                for(; it != tableOfSymbols.end(); it++)
                {
                    if(it->name == tempName)
                    {
                        if(it->visibility == 'g') 
                        {
                            outputFileStream << "00 00";
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_regIndAddSym16", it->number, (unsigned short)(sectionCnt - 0x1)});
                            break;
                        }
                        else
                        {
                            outputFileStream << setfill('0') << setw(4) << hex << it->value;
                            tableOfRelocations.push_back({(unsigned short)(locationCnt + 0x3), "R_hypo_regIndAddSym16", it->section, (unsigned short)(sectionCnt - 0x1)});
                            outputFileStream << setfill(' ');
                            break;
                        }
                    }
                }
            }

            if(regex_match(lineSplit[1], valueAddressRegIndRegDir))
            {
                locationCnt += 0x3;
            }
            else
            {
                locationCnt += 0x5;
            }

        }

        if(lineSplit.front() == *directives.find(".word"))
        {
            outputFileStream << setfill('0') << setw(2) << hex << locationCnt;
            outputFileStream << setfill(' ');
            outputFileStream << ":    ";

            vector<string>::iterator ite = lineSplit.begin();
            for(; ite != lineSplit.end(); ite++)
            {
                if(regex_match(*ite, symbol))
                {
                    
                    list<tableOfSymbolsRow>::iterator SymIt = tableOfSymbols.begin();
                    for(; SymIt != tableOfSymbols.end(); SymIt++)
                    {
                        string tName;
                        if((*ite).back() == ',')
                            tName = (*ite).substr(0, (*ite).find_last_not_of(',') + 1);
                        else
                            tName = (*ite);

                        if(SymIt->name == tName)
                        {

                            if(SymIt->visibility == 'g') 
                            {
                                outputFileStream << " 0000 ";
                                tableOfRelocations.push_back({(unsigned short)(locationCnt), "R_hypo_abs16", SymIt->number, (unsigned short)(sectionCnt - 0x1)});
                                
                            }
                            else
                            {
                                outputFileStream << setfill('0') << setw(4) << hex << SymIt->value;
                                outputFileStream << setfill(' ');
                                tableOfRelocations.push_back({(unsigned short)(locationCnt), "R_hypo_abs16", SymIt->section, (unsigned short)(sectionCnt - 0x1)});
                                                                
                            }
                            locationCnt += 0x2;
                        }
                    }

                }

                if(regex_match(*ite, literal))
                {
                    locationCnt += 0x2;
                    if((*ite).back() == ',')
                        outputFileStream << setfill('0') << setw(4) << hex << storeAsHex((*ite).substr(0, (*ite).find_last_not_of(',') + 1));
                    else
                        outputFileStream << setfill('0') << setw(4) << hex << storeAsHex((*ite));
                    outputFileStream << " ";
                }
                
            }
            outputFileStream << "    " << line << endl;
            
        }


    }

    outputFileStream << setfill('_') << setw(50) << "_" << endl;
    outputFileStream << setfill(' ') << endl;

    tableOfRelocations.sort();
    list<tableOfRelocationsRow>::iterator ittt = tableOfRelocations.begin();
    unsigned short currSection = ittt->section;
    list<tableOfSymbolsRow>::iterator secIt = sections.begin();
    for(; secIt != sections.end(); secIt++)
    {
        if(secIt->section == currSection)
        {
            outputFileStream << secIt->name << " section:" << endl;
        }
    }
    for(; ittt != tableOfRelocations.end(); ittt++)
    {
        if(ittt->section != currSection)
        {
            currSection = ittt->section;

            list<tableOfSymbolsRow>::iterator secItt = sections.begin();
            for(; secItt != sections.end(); secItt++)
            {
                if(secItt->section == currSection)
                {
                    outputFileStream << endl;
                    outputFileStream << secItt->name << " section:" << endl;
                }
            }
        }
        outputFileStream << setfill('0') << setw(2) << hex << ittt->offset;
        outputFileStream << "   ";
        outputFileStream << setfill(' ');
        outputFileStream << setw(20) << ittt->type;
        outputFileStream << "   ";
        outputFileStream << setfill('0') << setw(2) << hex << ittt->symbolNumber;
        outputFileStream << setfill(' ');
        outputFileStream << endl;
    }

    
}

int main(int argc, char *argv[])
{
    if(argc != 2 && argc != 4)
    {
        cout << "Neodgovarajuci broj argumenata komandne linije!" << endl;
        cout << "Ocekuje se komanda oblika ./assembler -o izlaz.(o|txt) ulaz.s ili ./assembler ulaz.s  " << endl;
        exit(-19);
    }

    if(argc == 4)
    {
        if(string(argv[1]) != "-o" || !regex_match(argv[2], outputFileName) || !regex_match(argv[3], inputFileName))
        {
            cout << "Greska u argumentima komandne linije!" << endl;
            cout << "Ocekuje se komanda oblika ./assembler -o izlaz.(o|txt) ulaz.s  " << endl;
            exit(-19);
        }

        string tempName(argv[2]);
        outputFile = tempName.substr(0, tempName.find_last_not_of('.') + 1);
    }

    if(argc == 2 && !regex_match(argv[1], inputFileName))
    {
        cout << "Greska u argumentima komandne linije!" << endl;
        cout << "Ocekuje se komanda oblika ./assembler ulaz.s  " << endl;
        exit(-19);
    }

    inputFile = argv[argc-1];

    firstPassAndSyntaxCheck();

    instructionCodeInit();

    secondPassAndOutputGeneration();

    return 0;
}
