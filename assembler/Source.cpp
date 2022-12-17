#include <iostream>
#include <Windows.h>
#include <fstream>
#include <string>
#include <algorithm>
#include <iostream>
#include <deque>
#include <map>
#include <utility>
#include <queue>
#include <regex>
#include <eval.h>
#include <sstream>
using namespace std;

#define xe 48;
#define indirect 32;
#define immediate 16;
#define indexed 8;
#define baseRel 4;
#define pcRel 2;
#define extended 1;

map<string,pair<int,int>> ops;//opcode table to get format and value
map<int, deque<string>>instructions;//code with location counter
map<string, int>symtab;//symbol table
deque<string>externdef;
map <string, pair<int, int>>litr;//literal table
map<string, short>regs;//register numbers
int startp;//start location of the program
int endp;//end location o...
string base;//string to be able to store name or value
int x;//register x
int nextpc;
string progName;//program name
//vector<deque<string>>instructionlist;



//extern "C" int evaluate(string tokens);
//extern "C" int init(char *ops, int* format, int* opcode, char * code);
//extern "C" int passOne(int *loctr);

deque<string> splitstr(string str);
int passOne(deque<string>buff, int loctr);
int getSize(string data);
int getVal(string data);
int symReplace(deque<string>line);
bool isDirective(string str);
int getObjectCode(deque<string>line);
int f4resolver(deque<string>line);
int f3resolver(deque<string>line, int op);
int f2resolver(deque<string>line, int op);
int f1resolver(deque<string>line, int op);
void initRegs();

int main()
{ 
	initRegs();

	fstream opcodesf,sicodef;
	ofstream output("hte.txt");
	opcodesf.open ("instructions.txt");
	//regs.open("registers.txt");
	sicodef.open("sample-Input.txt");
	string tempName;
	int tempFor, tempCode;
	//read instructions file
	for(int i=0;i<59;i++)
	{
		//take opName
		opcodesf >> tempName;
		//format
		opcodesf >> tempFor;
		//hex opcode
		opcodesf >> hex >> tempCode;

		ops[tempName] = make_pair(tempFor, tempCode);

	}
	//pass one
	string buffer;
	deque<string>buff;
	
	int count = 0;
	int pc=0;

	getline(sicodef, buffer);
	buff = splitstr(buffer);
	progName = buff[0];
	pc += stoi(buff[2], 0, 16); //program start not counted but we add the start value of the program
	startp = pc;
	while (getline(sicodef, buffer)) {
		//cout << buffer << "--------------" << endl;
		buff=splitstr(buffer);
		//instructionlist.push_back(buff);
		pc += passOne(buff, pc); //start pass one counting for every instruction

		count++;
	}

	sicodef.clear();
	sicodef.seekg(0, ios::beg);

	ostringstream objectCode;
	
	int beginning, len,pccounter=0;
	bool inRec = 0;
	output << "H" << progName << startp << to_string(endp - startp) << endl;
	//int endFlag = 0;
	for (auto i : instructions) {
		nextpc = 0;
		//if (endFlag == instructions.size())break;
		if(next(instructions.begin(), pccounter + 1)!=instructions.end()){
			pair<const int, deque<string>>& nextel = *next(instructions.begin(), pccounter+1);
			nextpc = nextel.first;
		}
		
		//cout << "current = " << i.first << "  next =" << pc << endl;
		if (isDirective(i.second[1])) {
			continue;
		}
		else if (i.second[1] == "RESW" || i.second[1] == "resw" || i.second[1] == "RESB" || i.second[1] == "resb") {
			output << "H"<<" "<<hex<<beginning<<" "<<len <<" "<<objectCode.str() << endl;
			objectCode.str("");
			inRec = 0;

		}
		else if (inRec == 0) {
			inRec = 1;
			beginning = i.first;
			len = 0;

		}
		if (inRec == 1) {
			len = i.first - beginning;
			cout << "inst __" << i.second[0] << "__" << i.second[1] << "__" << i.second[2] << "__" << endl;
			objectCode <<hex<<getObjectCode(i.second)<<",";
			//cout<<hex << objectCode.str() << endl;
		}
		pccounter++;
		
	}
	return 0;
}



int getSize(string data){
	if (data[0] == 'c' || data[0] == 'C') {
		return data.size() - 3;
	}
	else if (data[0] == 'x' || data[0] == 'X') {
		int len = data.size() - 3;
		if (len % 2 == 0)return len / 2;
		else return (len / 2) + 1;
	}
	else {
		return 3;
	}
}
int getVal(string data) {
	if (data[0] == 'c' || data[0] == 'C') {
		int res=0;
		for (int i = 2; i < data.size() - 1; i++) {
			res = (res * 100) + (int)data[i];
		}
	}
	else if (data[0] == 'x' || data[0] == 'X') {
		return stoi(data.substr(2,data.size()-3),0,16);
	}
	else {
		return -1;
	}
}
int symReplace(string oper){
	for (auto i : symtab) {

		//size_t finder = line[2].find(i.first);
		//if (finder != string::npos) {
		oper = regex_replace(oper, std::regex(i.first), to_string(i.second));
		//}
	}
	return evaluate(oper);
	
}
bool isDirective(string str) {
	string arr[] = {"end","END","LTORG","ltorg","BASE","base","EQU","equ"};
	for(auto i:arr){
		if (str == i)return true;
	}
	return false;
}
//int getLitralIndex(string litr) {

//}
int passOne(deque<string>line,int loctr) {
	//cout<< line[0]<<" "<<line[1]<<" "<<line[2]<<" "<< hex << loctr << endl;
	instructions[loctr] = line;

	if (line[0] == base) {
		base = to_string(loctr);
	}
	//tags----------------------------------------
	if (line[0] != "") {
		symtab[line[0]] = loctr;
	}
	//instruction---------------------------------
	if (line[1][0] == '+') {
		return 0x4;
	}
	if (line[2][0] == '=') {
		litr[line[2]]= make_pair(-1, -1);
	}
	if (ops.find(line[1])!=ops.end()) {
		return ops[line[1]].first;
	}
	if (line[1] == "RESB" || line[1] == "resb") {
		return stoi(line[2],0,10);
	}
	else if (line[1] == "RESW" || line[1] == "resw") {
		return 3*stoi(line[2], 0, 10);
	}
	else if (line[1] == "BYTE" || line[1] == "byte") {
		return getSize(line[2]);
	}
	else if (line[1] == "WORD" || line[1] == "word") {
		return 3;
	}
	else if (line[1] == "BASE" || line[1] == "base") {
		base = line[2];
		for (auto i : symtab) {
			if (i.first == base) {
				base = to_string(i.second);
				break;
			}
		}
		return 0;
	}
	if (line[1] == "EQU" || line[1] == "equ") {
		
		if (line[2] == "*") {
			symtab[line[0]] = loctr;
			return 0;
		}
		else {
			symtab[line[0]] = getVal(line[2]);
			return 0;
		}
	}
	if (line[1] == "LTORG" || line[1] == "ltorg" || line[1] == "END" || line[1] == "end") {
		int offset = 0,size=0;
		deque<string>literalBuffer;
		for (auto i : litr) {
			if (i.second.second == -1) {
				i.second.first = getVal(i.first.substr(1, i.first.size() - 1));
				i.second.second = loctr;
				size = getSize(i.first.substr(1, i.first.size() - 1));
				literalBuffer.clear();
				literalBuffer.push_back("*");
				literalBuffer.push_back(i.first);
				literalBuffer.push_back("");
				instructions[loctr] = literalBuffer;
				loctr += size;
				endp = loctr;
				offset += size;
			}	
		}
		return offset;
	}
	if (line[1] == "EXTREF" || line[1] == "extref") {
		string temp = "";
		for (auto i : line[2]) {
			if (i == ',') {
				symtab[temp] = 0;
				temp = "";
			}
			else {
				temp += i;
			}
		}
	}
	return 0;
}

int f4resolver(deque<string>line){
	int ob = 0;
	line[1] = line[1].substr(1);
	ob = ops[line[1]].second;
	ob = ob << 4;
	ob = ob | extended;
	ob = ob | xe;
	if (line[2][0] == '#') {
		ob = ob | immediate;
		ob = ob << 20;
		ob = ob | symtab[line[2].substr(1)];
		cout << hex << ob << endl;
		return ob;
	}
	else{
		ob = ob << 20;
		ob = ob | symtab[line[2]];
		cout << hex << ob << endl;
		return ob;
	}
}
int f3resolver(deque<string>line, int op){
	op = op << 4;
	if (line[1] == "RSUB" || line[1] == "rsub") {
		op = op | xe;
		op = op << 12;
		cout << hex << op << endl;
		return op;
	}
	if (line[2][0] == '#') {
		op = op | immediate;
		line[2] = line[2].substr(1);

	}
	else if (line[2][0] == '@') {
		op = op | indirect;
		line[2] = line[2].substr(1);
	}
	else {
		op = op | xe;
		if (line[2][(line[2].size() - 2)] == ',') {
			op = op | indexed;
			cout << line[2] << endl;
			line[2] = line[2].substr(0, (line[2].size() - 2));
		}
	}
	//cout << "target = " << symtab[line[2]] <<" pc = "<<nextpc << endl;
	int disp = symtab[line[2]] - nextpc;
	//cout << "disp = " << disp << endl;
	if (disp <= 2047 || disp >= -2048) {
		op = op | pcRel;
		op = op << 12;
		op = op | disp;
	}
	else {
		op = op | baseRel;
		disp= symtab[line[2]] - stoi(base);
		op = op << 12;
		op = op | disp;
	}
	cout << hex << op << endl;
	return op;
	
}
int f2resolver(deque<string>line, int op){
	string regis="";
	op = op << 4;
	for (auto i : line[2]) {
		if (i == ',') {
			
			op = op | regs[regis];
			op = op << 4;
			regis = "";
			continue;
		}
		regis += i;
	}
	cout << hex << op << endl;
	return op;
}
int f1resolver(deque<string>line, int op){
	cout << hex << op << endl;
	return op;
}

int getObjectCode(deque<string>line) {
	int ob = 0;
	if (line[1] == "LDX" || line[1] == "ldx") {
		if (line[2][0] == '#') {
			x = symtab[line[2].substr(1)];
		}
		else {
			x = symtab[line[2]];
		}
	}
	if (line[0] == "*") {
		ob = litr[line[1]].first;
		if (ob == -1) {
			ob = symReplace(line[1]);
		}
		return ob;
	}
	else {
		if (line[1] == "BYTE" || line[1] == "byte") {
			ob = getVal(line[2]);
			if (ob == -1) {
				ob = symReplace(line[1]);
			}
			return ob;
		}
		else if (line[1] == "WORD" || line[1] == "word") {
			return stoi(line[2], 0, 10);
		}
		else {
			if (line[1][0] == '+') {
				return f4resolver(line);
			}
			if (ops.find(line[1]) == ops.end()) {
				cout << "error at " << line[1] << endl;
			}
			else {
				pair<int,int> f = ops[line[1]];
				if (f.first == 3) {
					return f3resolver(line, f.second);
				}
				else if (f.first == 2) {
					return f2resolver(line, f.second);
				}
				else return f1resolver(line, f.second);

			}
		}
	}
}
deque<string> splitstr(string str) {
	deque<string>lista;
	lista.push_back("");
	lista.push_back("");
	lista.push_back("");
	vector<string>temp;
	for (auto i : str) {
		if (i == ' ')str.replace(0,1,"");
		else break;
	}
	temp.push_back("");
	int pos = 0;
	for (auto i : str) {
		if (i == ' ') {
			temp.push_back("");
			pos++;
		}
		else {
			temp[pos] += i;
		}
	}
	for (int i = 0; i < temp.size(); i++) {
		if (temp[i] == "") {
			temp.erase(temp.begin() + i);
		}
	}
	if (temp.size() == 1) {
		lista[1] = temp[0];
	}
	else if (temp.size() == 2) {
		lista[2] = temp[1];
		lista[1] = temp[0];
	}
	else if (temp.size() == 3) {
		lista[0] = temp[0];
		lista[1] = temp[1];
		lista[2] = temp[2];
	}

	return lista;
}

void initRegs() {
	regs["A"] = 0;
	regs["X"] = 1;
	regs["L"] = 2;
	regs["PC"] = 8;
	regs["SW"] = 0;
	regs["B"] = 3;
	regs["S"] = 4;
	regs["T"] = 5;
	regs["F"] = 6;
}