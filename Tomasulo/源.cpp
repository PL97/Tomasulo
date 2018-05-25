#include<iostream>
#include<map>
#include<fstream>
#include<sstream>
#include<string>
#include<vector>
#include <windows.h> 
#include <thread>
#include<mutex>
#include<queue>
std::mutex some_mutex;

using namespace std;

#define MAXINSTRUCTION 10
#define FILENAME "data.txt"
#define INSTRUCTION pair<OP, pair<REG, pair<REG, REG>>>
#define PIPOUT 0
#define EXE 1
#define WRITEBACK 2

#define ADDSNUM 3
#define MULSNUM 2
#define LOADSNUM 2
#define SAVESNUM 2

#define ADDCIRCLE 2
#define MULCIRCLE 10
#define DIVCIRCLE 40
#define LOADCIRCLE 1
#define SAVECIECLE 1
#define CIRCLETIME 100


//���ܲ���
const int fu_size = 5;
enum FU{ FU_NIL = fu_size, Load1 = 0, Load2, Store1, Store2, Add1, Add2, Add3, Mult1, Mult2};

//��������
enum OP{ OP_NIL, L_D, S_D, SUB_D, ADD_D, DIV_D, MULT_D };

//�Ĵ�������
const int R_size = 31;
enum REG{
	R_NIL = R_size, R0 = 0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14,
	R15, R16, R17, R18, R19, R20, R21, R22, R23, R24, R25, R26,
	R27, R28, R29, R30
};
const char * const reg[] = {
	"R0", "R1", "R2", "R3", "R4", "R5", "R6", "R7", "R8", "R9", "R10", "R11", "R12", "R13", "R14", "R15",
	"R16", "R17", "R18", "R19", "R20", "R21", "R22", "R23", "R24", "R25", "R26", "R27", "R328", "R29", "R30" };

int *Regs = new int[R_size];
int M_size = 1024 * 1024;
int *Mem = new int[M_size];

//ָ��״̬��
struct instructStatus{
	OP op;
	REG D;
	REG S1;
	REG S2;
	int imm;
	FU saveNum;//����վ���
	bool *status;
	instructStatus(){
		D = REG::R_NIL;
		S1 = REG::R_NIL;
		S2 = REG::R_NIL;
		status = new bool[3];// �ĸ�״̬�ֱ��ʾ������ ���������� ִ�У� д���
		memset(this, 0, sizeof(instructStatus));
	}
	instructStatus(OP _op, REG _D, REG _S1, REG _S2, int _imm){//ָ���ʽ��op r0, r1, r2
		status = new bool[3];// �ĸ�״̬�ֱ��ʾ������ ���������� ִ�У� д���
		memset(status, 0, sizeof(bool)* 4);
		op = _op;
		D = _D;
		S1 = _S1;
		S2 = _S2;
		imm = _imm;
	}
	instructStatus(OP _op, REG _D, REG _S1, int _imm){//ָ���ʽ��op r0, r1
		status = new bool[3];// �ĸ�״̬�ֱ��ʾ������ ���������� ִ�У� д���
		memset(status, 0, sizeof(bool)* 4);
		op = _op;
		D = _D;
		S1 = _S1;
		imm = _imm;
	}
};

//����Ĵ���״̬��
struct resultRstatus{
	FU* resultStatus;
	resultRstatus(){
		resultStatus = new FU[R_size + 1];
		for (int i = 0; i < R_size + 1; i++){
			resultStatus[i] = FU::FU_NIL;
		}
	}
};

//����վ
struct saveStation{
	map<FU, bool> busy;
	map<FU, OP> op;
	map<FU, int> Vj;
	map<FU, int> Vk;
	map<FU, FU> Qj;
	map<FU, FU> Qk;
	map<FU, int> A;
	map<FU, int> buffer;//��ʱ�洢����,��WB�׶ζ�ȡ����
	//��ʼ������״̬��
	saveStation(){
		for (FU tempFu = FU::Load1; tempFu<fu_size; tempFu = (FU)(tempFu + 1)){
			busy.insert(make_pair(tempFu, false));
			op.insert(make_pair(tempFu, OP::OP_NIL));
			Vj.insert(make_pair(tempFu, -1));
			Vk.insert(make_pair(tempFu, -1));
			Qj.insert(make_pair(tempFu, FU::FU_NIL));
			Qk.insert(make_pair(tempFu, FU::FU_NIL));
			A.insert(make_pair(tempFu, -1));
			buffer.insert(make_pair(tempFu, -1));
		}
	}

};

//Tomasulo
struct Tomasulo{
	instructStatus **I_status;
	resultRstatus *R_status;
	saveStation *S_status;
	queue<FU> load;
	queue<FU> save;
	int instr_size;
	int current;
	Tomasulo(){
		current = 0;
		instr_size = 0;
		I_status = new instructStatus*[MAXINSTRUCTION];
		R_status = new resultRstatus();
		S_status = new saveStation();
	}
};

vector<string> instructions;

int loadData(Tomasulo* T);

INSTRUCTION parse(char* s, int &imm);

int getCirlce(instructStatus *Ipointer);

FU isSSAvailable(Tomasulo *T);

bool pipOut(Tomasulo *T);

bool execute(Tomasulo *T);

bool writeBack(Tomasulo *T);

void display(Tomasulo *T, int i);

pair<bool, bool> isSAVEorLOAD(FU fun);

int completedI = 0;

int main(){
	fstream f1("pipline0.txt");
	f1.close();
	memset(Regs, 0, sizeof(int)*R_size);
	Tomasulo* T = new Tomasulo();
	T->instr_size = loadData(T);
	T->instr_size = loadData(T);
	while (completedI < T->instr_size){
		pipOut(T);
		execute(T);
		if (writeBack(T)) completedI++;
		display(T, 0);
	}
	system("pause");
}

map<instructStatus*, int> simExe;

int loadData(Tomasulo *T){
	int data_size = 0;
	fstream fout(FILENAME, std::ios::in);
	char* buffer = new char[20];
	stringstream ss;
	string temp, d_r;
	int count = 0, imm = -1;
	if (fout){
		while (!fout.eof()){
			fout.getline(buffer, 20);
			instructions.push_back((string)buffer);
			INSTRUCTION instr = parse(buffer, imm);
			T->I_status[count++] = new instructStatus(instr.first, instr.second.first,
				instr.second.second.first, instr.second.second.second, imm);
			data_size++;
		}
	}
	delete[] buffer;
	return data_size;
}

bool pipOut(Tomasulo *T){
	if (T->current >= T->instr_size)
		return false;
	bool result = false;
	instructStatus *Ipointer = T->I_status[T->current];
	FU fun = isSSAvailable(T);
	if (fun != FU::FU_NIL){	//�жϱ���վ�Ƿ��п���λ��
		Ipointer->saveNum = fun;
		FU r1f = T->R_status->resultStatus[Ipointer->S1];
		FU r2f = T->R_status->resultStatus[Ipointer->S2];
		bool isloadorsave, isload;
		pair<bool, bool> temptest = isSAVEorLOAD(fun);
		isloadorsave = temptest.first;
		isload = temptest.second;
		if (r1f != FU::FU_NIL){//��һλ������û��׼����
			T->S_status->Qj[fun] = r1f;
		}
		else{
			T->S_status->Qj[fun] = FU::FU_NIL;
			//put actual value here
			T->S_status->Vj[fun] = Regs[Ipointer->S1];
		}

		if (!isloadorsave){//���ڸ�������ָ��
			if (r2f != FU::FU_NIL){
				T->S_status->Qk[fun] = r2f;
			}
			else{
				T->S_status->Qk[fun] = FU::FU_NIL;
				//put actual value here
				T->S_status->Vj[fun] = Regs[Ipointer->S2];
			}
		}
		else{
			T->S_status->A[fun] = Ipointer->imm;
			if (isload){//����loadָ�ԤԼ�Ĵ���ֵ���ں������
				T->R_status->resultStatus[Ipointer->D] = fun;
				T->load.push(fun);
			}
			else{//����saveָ��ж������Ƿ����
				FU df = T->R_status->resultStatus[Ipointer->D];
				T->save.push(fun);
				if (df != FU::FU_NIL){//����׼������
					T->S_status->Qj[fun] = df;
				}
				else{
					T->S_status->Vj[fun] = Regs[Ipointer->D];
					T->S_status->Qj[fun] = FU::FU_NIL;
				}
			}
		}
		T->S_status->busy[fun] = true;
		Ipointer->status[PIPOUT] = true;
		T->current++;//�ƶ���ǰ����ָ��
		result = true;
	}
	return result;
}

bool execute(Tomasulo *T){
	FU fun;
	bool exe = false;
	for (int i = T->current - 1; i >= 0; i--){
		instructStatus *instr = T->I_status[i];
		if (instr->status[PIPOUT] && !instr->status[EXE]){
			//�˴�������չ��Ӿ������
			if (simExe.count(instr) == 0){
				simExe.insert(make_pair(instr, getCirlce(instr)));
			}
			simExe[instr]--;
			if (simExe[instr] == 0){//ִ�����ڽ���
				bool isloadorsave, isload;
				fun = instr->saveNum;
				pair<bool, bool> temptest = isSAVEorLOAD(fun);
				isloadorsave = temptest.first;
				isload = temptest.second;
				if (!isloadorsave){
					if (T->R_status->resultStatus[instr->S1] == FU::FU_NIL && T->R_status->resultStatus[instr->S2] == FU::FU_NIL){
						//������������������buffer��Ϊ���������һ�����������
						T->S_status->buffer[fun] = Regs[T->S_status->Vj[fun]] + Regs[T->S_status->Vk[fun]];
					}
				}
				else{//load and save
					if (T->S_status->A[fun] == FU::FU_NIL && isload && T->load.front() == instr->saveNum){
						T->S_status->A[fun] = T->S_status->A[fun] + Regs[instr->S1];//���㲢д���ַ
						//���ڴ���ȡ��
						T->S_status->buffer[fun] = Mem[T->S_status->A[fun]];
					}
					else if(T->S_status->Qj[fun] == FU::FU_NIL && !isload && T->save.front() == instr->saveNum){
						T->S_status->A[fun] = T->S_status->A[fun] + Regs[instr->S1];//���㲢д���ַ
					}
				}
				instr->status[EXE] = true;
				simExe.erase(instr);
				exe = true;
			}
		}
	}
	return exe;
}

bool writeBack(Tomasulo *T){
	bool result = false;
	for (int i = 0; i<T->current; i++){
		instructStatus *Ipointer = T->I_status[i];
		if (Ipointer->status[EXE] && !Ipointer->status[WRITEBACK]){//���������Ѿ�ִ����ϵ���û��д�ص�ָ��
			result = true;
			FU fun_finish = T->I_status[i]->saveNum;
			bool isloadorsave, isload;
			pair<bool, bool> temptest = isSAVEorLOAD(fun_finish);
			isloadorsave = temptest.first;
			isload = temptest.second;
			if (!isloadorsave || isload){//���ڸ�������ָ���loadָ��
				for (int j = T->current-1; j >= 0; j--){
					FU fun = T->I_status[j]->saveNum;
					if (T->S_status->Qj[fun] == fun_finish){//��Q���û��Ҫ�ȴ����豸���ҽ�Vд������
						T->S_status->Qj[fun] = FU::FU_NIL;
						T->S_status->Vj[fun] = T->S_status->buffer[fun_finish];
					}
					if (T->S_status->Qk[fun] == fun_finish){
						T->S_status->Qk[fun] = FU::FU_NIL;
						T->S_status->Vk[fun] = T->S_status->buffer[fun_finish];
					}
				}
				if (isload){
					T->load.pop();//���ü�¼�Ӷ�����ȡ��
				}
				//����������Ҫ��ֵ�ļĴ���д�ؼĴ���
				for (int k = 0; k < 30; k++){
					if (T->R_status->resultStatus[k] == fun_finish){//����üĴ�������ǵȴ���������ֵ
						Regs[k] = T->S_status->buffer[fun_finish];//�޸�ֵ
						T->R_status->resultStatus[k] = FU::FU_NIL;//ȡ�����
					}
				}
			}
			else{//����saveָ��
				Mem[T->S_status->A[fun_finish]] = T->S_status->buffer[fun_finish];//������д���ڴ�
				T->save.pop();
			}
			Ipointer->status[WRITEBACK] = true;
			T->S_status->busy[fun_finish] = false;
		}
	}
	return result;
}

INSTRUCTION parse(char* s, int &imm){
	string temp = s;
	while (temp.find(",") != -1){
		temp.replace(temp.find(","), 1, "");
	}
	stringstream ss;
	ss << temp;
	string s_op, s_d, s_s1, s_s2, s_imm;
	ss >> s_op;
	ss >> s_d;
	ss >> s_s1;
	ss >> s_s2;
	OP op;
	REG d, s1, s2;
	bool isMem = false;
	if (s_op.find("L.D") != -1){
		op = OP::L_D;
		isMem = true;
	}
	else if (s_op.find("S.D") != -1){
		op = OP::S_D;
		isMem = true;
	}
	else if (s_op.find("MULT.D") != -1){
		op = OP::MULT_D;
	}
	else if (s_op.find("SUB.D") != -1){
		op = OP::SUB_D;
	}
	else if (s_op.find("ADD.D") != -1){
		op = OP::ADD_D;
	}
	else if (s_op.find("DIV.D") != -1){
		op = OP::DIV_D;
	}
	else{
		op = OP::OP_NIL;
	}
	//��ȡĿ�ļĴ���
	s_d = s_d.substr(1);
	d = (REG)atoi(s_d.c_str());
	//��ȡԴ�Ĵ���
	if (isMem){
		s_imm = s_s1.substr(0, s_s1.find("("));
		imm = atoi(s_imm.c_str());
		s_s1 = s_s1.substr(s_s1.find("R") + 1, s_s1.length() - s_s1.find("R") - 2);
		s1 = (REG)atoi(s_s1.c_str());
		s2 = REG::R_NIL;
	}
	else{
		s_s1 = s_s1.substr(1);
		s1 = (REG)atoi(s_s1.c_str());
		s_s2 = s_s2.substr(1);
		s2 = (REG)atoi(s_s2.c_str());
	}
	return make_pair(op, make_pair(d, make_pair(s1, s2)));
}

int getCirlce(instructStatus *Ipointer){
	OP operation = Ipointer->op;
	int circle = -1;
	if (OP::ADD_D == operation || OP::SUB_D == operation)
		circle = ADDCIRCLE;
	else if (OP::DIV_D == operation)
		circle = DIVCIRCLE;
	else if (OP::MULT_D == operation)
		circle = MULCIRCLE;
	else if (OP::L_D == operation)
		circle = LOADCIRCLE;
	else if (OP::S_D == operation)
		circle = SAVECIECLE;
	return circle;
}

void display(Tomasulo *T, int i){
	string filename = "pipline" + to_string(i) + ".txt";
	ofstream fout(filename, ios::app);
	fout << "ָ��״̬��" << endl;
	fout << "ָ�� \t ���� \t ִ��\t д���\t" << endl;
	for (int i = 0; i < T->instr_size; i++){
		fout << instructions[i] << "\t" << T->I_status[i]->status[PIPOUT] << "\t"
			<< T->I_status[i]->status[EXE] << "\t" << T->I_status[i]->status[WRITEBACK] << endl;
	}
	fout << "-------------------------------------------------------------------------" << endl;
	//cout << "���ܲ�����"
	fout.close();
}

FU isSSAvailable(Tomasulo *T){
	instructStatus *Ipointer = T->I_status[T->current];
	FU result = FU::FU_NIL;
	if (Ipointer->op == OP::ADD_D || Ipointer->op == OP::SUB_D){
		for (FU temp = FU::Add1; temp < FU::Add1 + ADDSNUM; temp = (FU)(temp+1)){
			if (!T->S_status->busy[temp]){
				result = temp;
				break;
			}
		}
	}
	else if (Ipointer->op == OP::DIV_D || Ipointer->op == OP::MULT_D){
		for (FU temp = FU::Mult1; temp < FU::Mult1 + MULSNUM; temp = (FU)(temp + 1)){
			if (!T->S_status->busy[temp]){
				result = temp;
				break;
			}
		}
	}
	else if (Ipointer->op == OP::L_D){
		for (FU temp = FU::Load1; temp < FU::Load1 + LOADSNUM; temp = (FU)(temp + 1)){
			if (!T->S_status->busy[temp]){
				result = temp;
				break;
			}
		}
	}
	else if (Ipointer->op == OP::S_D){
		for (FU temp = FU::Store1; temp < FU::Store1 + SAVESNUM; temp = (FU)(temp + 1)){
			if (!T->S_status->busy[temp]){
				result = temp;
				break;
			}
		}
	}
	return result;
}

pair<bool, bool> isSAVEorLOAD(FU fun){
	pair<bool, bool> result;
	if (fun == FU::Load1 || fun == FU::Load2){
		result = make_pair(true, true);
	}
	else if (fun == FU::Store1 || fun == FU::Store2){
		result = make_pair(true, false);
	}
	return result;
}